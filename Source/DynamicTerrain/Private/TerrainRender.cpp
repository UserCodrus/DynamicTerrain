#include "TerrainRender.h"
#include "TerrainComponent.h"
#include "Terrain.h"

#include "Engine.h"
#include "SceneView.h"
#include "Materials/Material.h"

FTerrainComponentSceneProxy::FTerrainComponentSceneProxy(UTerrainComponent* Component) : FPrimitiveSceneProxy(Component), VertexFactory(GetScene().GetFeatureLevel(), "FTerrainComponentSceneProxy"), MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
{
	// Get map data from the parent component
	MapProxy = Component->GetMapProxy();
	Size = Component->Size;
	MaxLOD = Component->LODs;
	
	// Create LOD indices
	ScaleLODs(Component->LODScale);
	IndexBuffers.SetNum(MaxLOD);
	for (uint32 i = 0; i < MaxLOD; ++i)
	{
		UpdateIndexData(IndexBuffers[i].Indices, i);
	}

	// Get the material from the parent or use the engine default
	Material = Component->GetMaterial(0);
	if (Material == nullptr)
	{
		Material = UMaterial::GetDefaultMaterial(MD_Surface);
	}

	// Initialize the component on the rendering thread
	uint32 width = GetTerrainComponentWidth(Size) - 1;
	int32 xoffset = Component->XOffset;
	int32 yoffset = Component->YOffset;
	float tiling = Component->Tiling;
	ENQUEUE_RENDER_COMMAND(FComponentFillBuffers)([this, xoffset, yoffset, tiling](FRHICommandListImmediate& RHICmdList) {
		Initialize(xoffset, yoffset, tiling);
		});
}

FTerrainComponentSceneProxy::~FTerrainComponentSceneProxy()
{
	VertexBuffers.PositionVertexBuffer.ReleaseResource();
	VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
	VertexFactory.ReleaseResource();
	for (int32 i = 0; i < IndexBuffers.Num(); ++i)
	{
		IndexBuffers[i].ReleaseResource();
	}
}

/// Scene Proxy Interface ///

void FTerrainComponentSceneProxy::GetDynamicMeshElements(const TArray< const FSceneView* >& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, class FMeshElementCollector& Collector) const
{
	// Check to see if wireframe rendering is enabled
	const bool wireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

	// Get the material proxy from either the current material or the wireframe material
	FMaterialRenderProxy* material_proxy = nullptr;
	if (wireframe)
	{
		// Get the wireframe material
		FColoredMaterialRenderProxy* wireframe_material = new FColoredMaterialRenderProxy(GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : nullptr, FLinearColor(0.0f, 0.5f, 1.0f));
		Collector.RegisterOneFrameMaterialProxy(wireframe_material);

		material_proxy = wireframe_material;
	}
	else
	{
		material_proxy = Material->GetRenderProxy();
	}

	// Pass the mesh to every view it is visible to
	for (int32 view_index = 0; view_index < Views.Num(); ++view_index)
	{
		if (VisibilityMap & (1 << view_index))
		{
			const FSceneView* view = Views[view_index];

			// Get the LOD index of the mesh
			const FSceneView& lod_view = GetLODView(*view);
			const FBoxSphereBounds bounds = GetBounds();

			FCachedSystemScalabilityCVars cvars = GetCachedScalabilityCVars();
			float screen_scale = cvars.StaticMeshLODDistanceScale != 0.0f ? 1.0f / cvars.StaticMeshLODDistanceScale : 1.0f;

			uint32 LOD = 0;
			float radius = ComputeBoundsScreenRadiusSquared(bounds.Origin, bounds.SphereRadius, *view) * screen_scale * screen_scale * lod_view.LODDistanceFactor * lod_view.LODDistanceFactor;
			for (uint32 i = 0; i < MaxLOD; ++i)
			{
				if (FMath::Square(LODScales[i] * 0.5) > radius)
				{
					LOD = i;
				}
				else
				{
					break;
				}
			}

			// Set up the mesh
			FMeshBatch& mesh = Collector.AllocateMesh();
			mesh.bWireframe = wireframe;
			mesh.VertexFactory = &VertexFactory;
			mesh.MaterialRenderProxy = material_proxy;
			mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
			mesh.Type = PT_TriangleList;
			mesh.DepthPriorityGroup = SDPG_World;
			mesh.bCanApplyViewModeOverrides = false;

			// Set up the first element of the mesh (we only need one)
			FMeshBatchElement& element = mesh.Elements[0];
			element.IndexBuffer = &IndexBuffers[LOD];
			element.FirstIndex = 0;
			element.NumPrimitives = IndexBuffers[LOD].Indices.Num() / 3;
			element.MinVertexIndex = 0;
			element.MaxVertexIndex = VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;

			// Load uniform buffers
			bool bHasPrecomputedVolumetricLightmap;
			FMatrix PreviousLocalToWorld;
			int32 SingleCaptureIndex;
			bool bOutputVelocity;
			GetScene().GetPrimitiveUniformShaderParameters_RenderThread(GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld, SingleCaptureIndex, bOutputVelocity);

			FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
			DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap, DrawsVelocity(), bOutputVelocity);
			element.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;

			// Add the mesh
			Collector.AddMesh(view_index, mesh);
		}
	}

	// Draw bounds and collision in debug builds
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	for (int32 view_index = 0; view_index < Views.Num(); ++view_index)
	{
		if (VisibilityMap & (1 << view_index))
		{
			// Render the object bounds
			RenderBounds(Collector.GetPDI(view_index), ViewFamily.EngineShowFlags, GetBounds(), IsSelected());
		}
	}
#endif
}

FPrimitiveViewRelevance FTerrainComponentSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance = IsShown(View);
	Result.bShadowRelevance = IsShadowCast(View);

	Result.bDynamicRelevance = true;

	Result.bRenderInMainPass = ShouldRenderInMainPass();
	Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
	Result.bRenderCustomDepth = ShouldRenderCustomDepth();

	Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
	MaterialRelevance.SetPrimitiveViewRelevance(Result);
	Result.bVelocityRelevance = IsMovable() && Result.bOpaque && Result.bRenderInMainPass;
	return Result;
}

void FTerrainComponentSceneProxy::Initialize(int32 X, int32 Y, float Tiling)
{
	// Initialize buffers
	uint32 width = GetTerrainComponentWidth(Size);
	VertexBuffers.PositionVertexBuffer.Init(width * width);
	VertexBuffers.StaticMeshVertexBuffer.Init(width * width, 1);

	// Load data for all buffers
	UpdateMapData();
	UpdateUVData(X, Y, Tiling);

	// Initialize the buffers
	for (int32 i = 0; i < IndexBuffers.Num(); ++i)
	{
		IndexBuffers[i].InitResource();
	}
	VertexBuffers.PositionVertexBuffer.InitResource();
	VertexBuffers.StaticMeshVertexBuffer.InitResource();

	// Bind vertex factory data
	FLocalVertexFactory::FDataType datatype;
	VertexBuffers.PositionVertexBuffer.BindPositionVertexBuffer(&VertexFactory, datatype);
	VertexBuffers.StaticMeshVertexBuffer.BindTangentVertexBuffer(&VertexFactory, datatype);
	VertexBuffers.StaticMeshVertexBuffer.BindPackedTexCoordVertexBuffer(&VertexFactory, datatype);
	VertexBuffers.StaticMeshVertexBuffer.BindLightMapVertexBuffer(&VertexFactory, datatype, 0);
	FColorVertexBuffer::BindDefaultColorVertexBuffer(&VertexFactory, datatype, FColorVertexBuffer::NullBindStride::ZeroForDefaultBufferBind);

	// Initalize the vertex factory
	VertexFactory.SetData(datatype);
	VertexFactory.InitResource();
}

/// Proxy Update Functions ///

void FTerrainComponentSceneProxy::UpdateMap(TSharedPtr<FMapSection, ESPMode::ThreadSafe> SectionProxy)
{
	// Copy map data to buffers
	MapProxy = SectionProxy;
	UpdateMapData();

	// Copy buffers to RHI
	{
		auto& vertex_buffer = VertexBuffers.PositionVertexBuffer;
		void* vertex_data = RHILockVertexBuffer(vertex_buffer.VertexBufferRHI, 0, vertex_buffer.GetNumVertices() * vertex_buffer.GetStride(), RLM_WriteOnly);
		FMemory::Memcpy(vertex_data, vertex_buffer.GetVertexData(), vertex_buffer.GetNumVertices() * vertex_buffer.GetStride());
		RHIUnlockVertexBuffer(vertex_buffer.VertexBufferRHI);
	}

	{
		auto& vertex_buffer = VertexBuffers.StaticMeshVertexBuffer;
		void* vertex_data = RHILockVertexBuffer(vertex_buffer.TangentsVertexBuffer.VertexBufferRHI, 0, vertex_buffer.GetTangentSize(), RLM_WriteOnly);
		FMemory::Memcpy(vertex_data, vertex_buffer.GetTangentData(), vertex_buffer.GetTangentSize());
		RHIUnlockVertexBuffer(vertex_buffer.TangentsVertexBuffer.VertexBufferRHI);
	}
}

void FTerrainComponentSceneProxy::UpdateUVs(int32 XOffset, int32 YOffset, float Tiling)
{
	// Set UV data
	UpdateUVData(XOffset, YOffset, Tiling);

	// Copy buffers to RHI
	{
		auto& vertex_buffer = VertexBuffers.StaticMeshVertexBuffer;
		void* vertex_data = RHILockVertexBuffer(vertex_buffer.TexCoordVertexBuffer.VertexBufferRHI, 0, vertex_buffer.GetTexCoordSize(), RLM_WriteOnly);
		FMemory::Memcpy(vertex_data, vertex_buffer.GetTexCoordData(), vertex_buffer.GetTexCoordSize());
		RHIUnlockVertexBuffer(vertex_buffer.TexCoordVertexBuffer.VertexBufferRHI);
	}
}

void FTerrainComponentSceneProxy::UpdateMapData()
{
	uint32 width = GetTerrainComponentWidth(Size);
	for (uint32 y = 0; y < width; ++y)
	{
		for (uint32 x = 0; x < width; ++x)
		{
			uint32 i = y * width + x;

			// Position data
			VertexBuffers.PositionVertexBuffer.VertexPosition(i) = FVector(x, y, MapProxy->Data[(y + 1) * MapProxy->X + x + 1]);

			// Tangent data
			int32 map_offset_x = x + 1;
			int32 map_offset_y = y + 1;
			float s01 = MapProxy->Data[map_offset_x - 1 + map_offset_y * MapProxy->X];
			float s21 = MapProxy->Data[map_offset_x + 1 + map_offset_y * MapProxy->X];
			float s10 = MapProxy->Data[map_offset_x + (map_offset_y - 1) * MapProxy->X];
			float s12 = MapProxy->Data[map_offset_x + (map_offset_y + 1) * MapProxy->X];

			// Get tangents in the x and y directions
			FVector vx(2.0f, 0, s21 - s01);
			FVector vy(0, 2.0f, s12 - s10);

			// Calculate the cross product of the two tangents
			vx.Normalize();
			vy.Normalize();
			VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(i, vx, vy, FVector::CrossProduct(vx, vy));
		}
	}
}

void FTerrainComponentSceneProxy::UpdateUVData(int32 XOffset, int32 YOffset, float Tiling)
{
	// Fill UV data
	uint32 width = GetTerrainComponentWidth(Size);
	XOffset *= width - 1;
	YOffset *= width - 1;
	for (uint32 y = 0; y < width; ++y)
	{
		for (uint32 x = 0; x < width; ++x)
		{
			VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(y * width + x, 0, FVector2D((XOffset + x) * Tiling, (YOffset + y) * Tiling));
		}
	}
}

void FTerrainComponentSceneProxy::UpdateIndexData(TArray<uint32>& Indices, uint32 Stride)
{
	Stride = FMath::Exp2(Stride);
	uint32 width = GetTerrainComponentWidth(Size);
	uint32 polygons = (width - 1) / Stride;

	Indices.Empty();
	Indices.SetNumUninitialized(polygons * polygons * 6);
	for (uint32 y = 0; y < polygons; y++)
	{
		for (uint32 x = 0; x < polygons; x++)
		{
			uint32 i = (y * polygons + x) * 6;

			Indices[i] = x * Stride + y * Stride * width;
			Indices[i + 1] = (1 + x) * Stride + (y + 1) * Stride * width;
			Indices[i + 2] = (1 + x) * Stride + y * Stride * width;

			Indices[i + 3] = x * Stride + y * Stride * width;
			Indices[i + 4] = x * Stride + (y + 1) * Stride * width;
			Indices[i + 5] = (1 + x) * Stride + (y + 1) * Stride * width;
		}
	}
}

void FTerrainComponentSceneProxy::ScaleLODs(float Scale)
{
	LODScales.Empty();
	LODScales.SetNumUninitialized(MaxLOD);

	for (uint32 i = 0; i < MaxLOD; ++i)
	{
		LODScales[i] = FMath::Pow(Scale, i);
	}
}