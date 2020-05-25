#include "TerrainRender.h"
#include "TerrainComponent.h"
#include "Terrain.h"

#include "Engine.h"
#include "Materials/Material.h"

/// Rendering Buffers ///

FTerrainVertexBuffer::FTerrainVertexBuffer(int32 DataSize, uint32 BufferStride, uint8 BufferFormat) : ComponentSize(0), StaticFlag(BUF_Dynamic), Size(DataSize), Stride(BufferStride), Format(BufferFormat)
{
	// Initalizers only
}

void FTerrainVertexBuffer::InitRHI()
{
	if (Size > 0 && ComponentSize > 0)
	{
		// Create the RHI buffer
		FRHIResourceCreateInfo info;
		VertexBufferRHI = RHICreateVertexBuffer(ComponentSize * ComponentSize * Size, StaticFlag | BUF_ShaderResource, info);
		
		// Create the shader resource view if needed
		if (RHISupportsManualVertexFetch(GMaxRHIShaderPlatform))
		{
			ShaderResourceView = RHICreateShaderResourceView(VertexBufferRHI, Stride, Format);
		}
	}
}

void FTerrainVertexBuffer::ReleaseRHI()
{
	ShaderResourceView.SafeRelease();
	FVertexBuffer::ReleaseRHI();
}

FTerrainPositionBuffer::FTerrainPositionBuffer() : FTerrainVertexBuffer(sizeof(FVector), sizeof(float), PF_R32_FLOAT)
{
	// Initializers only
}

void FTerrainPositionBuffer::FillBuffer()
{
	if (Size > 0 && ComponentSize > 0)
	{
		// Make sure the RHI buffer is initialized
		check(VertexBufferRHI.IsValid());

		// Copy data to the RHI buffer
		void* buffer = RHILockVertexBuffer(VertexBufferRHI, 0, Data.Num() * Size, RLM_WriteOnly);
		FMemory::Memcpy(buffer, Data.GetData(), Data.Num() * Size);
		RHIUnlockVertexBuffer(VertexBufferRHI);
	}
}

void FTerrainPositionBuffer::UpdateBuffer(TSharedPtr<FMapSection, ESPMode::ThreadSafe> Map)
{
	if (!Map.IsValid())
		return;

	if (Map->X == ComponentSize + 2 && Map->Y == ComponentSize + 2)
	{
		FVector* buffer = (FVector*)RHILockVertexBuffer(VertexBufferRHI, 0, ComponentSize * ComponentSize * Size, RLM_WriteOnly);

		// Change the z value of vertices to match the map subsection
		for (uint32 y = 0; y < ComponentSize; ++y)
		{
			for (uint32 x = 0; x < ComponentSize; ++x)
			{
				*buffer = FVector(x, y, Map->Data[(y + 1) * Map->X + x + 1]);
				++buffer;
			}
		}

		RHIUnlockVertexBuffer(VertexBufferRHI);
	}
}

void FTerrainPositionBuffer::Bind(FLocalVertexFactory::FDataType& DataType)
{
	// Add position data
	DataType.PositionComponent = FVertexStreamComponent(this, 0, Size, VET_Float3);
	DataType.PositionComponentSRV = ShaderResourceView;
}

FTerrainUVBuffer::FTerrainUVBuffer() : FTerrainVertexBuffer(sizeof(FVector2D), sizeof(FVector2D), PF_G32R32F)
{
	// Initializers only
}

void FTerrainUVBuffer::FillBuffer(int32 XOffset, int32 YOffset, float Tiling)
{
	if (Size > 0 && ComponentSize > 0)
	{
		check(VertexBufferRHI.IsValid());

		// Fill the buffer with UVs
		FVector2D* buffer = (FVector2D*)RHILockVertexBuffer(VertexBufferRHI, 0, ComponentSize * ComponentSize * Size, RLM_WriteOnly);
		for (uint32 y = 0; y < ComponentSize; ++y)
		{
			for (uint32 x = 0; x < ComponentSize; ++x)
			{
				*buffer = FVector2D((XOffset + x) * Tiling, (YOffset + y) * Tiling);
				++buffer;
			}
		}
		RHIUnlockVertexBuffer(VertexBufferRHI);
	}
}

void FTerrainUVBuffer::Bind(FLocalVertexFactory::FDataType& DataType)
{
	// Empty the UV array
	DataType.TextureCoordinates.Empty();
	DataType.NumTexCoords = 1;

	// Add UV data
	DataType.TextureCoordinates.Add(FVertexStreamComponent(this, 0, Size, VET_Float4, EVertexStreamUsage::ManualFetch));
	DataType.TextureCoordinatesSRV = ShaderResourceView;
}

FTerrainTangentBuffer::FTerrainTangentBuffer() : FTerrainVertexBuffer(sizeof(FPackedNormal) * 2, sizeof(FPackedNormal), PF_R8G8B8A8_SNORM)
{
	// Initializers only
}

void FTerrainTangentBuffer::UpdateBuffer(TSharedPtr<FMapSection, ESPMode::ThreadSafe> Map)
{
	if (!Map.IsValid())
		return;

	if (Map->X == ComponentSize + 2 && Map->Y == ComponentSize + 2)
	{
		FTerrainTangents* buffer = (FTerrainTangents*)RHILockVertexBuffer(VertexBufferRHI, 0, ComponentSize * ComponentSize * Size, RLM_WriteOnly);

		// Fill the buffer with normals and tangents
		for (uint32 y = 0; y < ComponentSize; ++y)
		{
			for (uint32 x = 0; x < ComponentSize; ++x)
			{
				int32 map_offset_x = x + 1;
				int32 map_offset_y = y + 1;
				float s01 = Map->Data[map_offset_x - 1 + map_offset_y * Map->X];
				float s21 = Map->Data[map_offset_x + 1 + map_offset_y * Map->X];
				float s10 = Map->Data[map_offset_x + (map_offset_y - 1) * Map->X];
				float s12 = Map->Data[map_offset_x + (map_offset_y + 1) * Map->X];

				// Get tangents in the x and y directions
				FVector vx(2.0f, 0, s21 - s01);
				FVector vy(0, 2.0f, s10 - s12);

				// Calculate the cross product of the two tangents
				vx.Normalize();
				vy.Normalize();

				FTerrainTangents tangents;
				tangents.Normal = FVector::CrossProduct(vx, vy);
				tangents.Tangent = FVector(vx.X, vx.Y, vx.Z);

				*buffer = tangents;
				++buffer;
			}
		}

		RHIUnlockVertexBuffer(VertexBufferRHI);
	}
}

void FTerrainTangentBuffer::Bind(FLocalVertexFactory::FDataType& DataType)
{
	// Add tangents
	DataType.TangentBasisComponents[0] = FVertexStreamComponent(this, offsetof(FTerrainTangents, Tangent), sizeof(FTerrainTangents), VET_PackedNormal, EVertexStreamUsage::ManualFetch);
	// Add normals
	DataType.TangentBasisComponents[1] = FVertexStreamComponent(this, offsetof(FTerrainTangents, Normal), sizeof(FTerrainTangents), VET_PackedNormal, EVertexStreamUsage::ManualFetch);
	DataType.TangentsSRV = ShaderResourceView;
}

/// Terrain Scene Proxy ///

FTerrainComponentSceneProxy::FTerrainComponentSceneProxy(UTerrainComponent* Component) : FPrimitiveSceneProxy(Component), VertexFactory(GetScene().GetFeatureLevel(), "FTerrainComponentSceneProxy"), MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
{
	// Get the map proxy data for this component
	MapProxy = Component->GetMapProxy();
	Size = Component->GetSize();

	// Fill the buffers
	IndexBuffer.Indices = Component->IndexBuffer;
	//PositionBuffer.Data = Component->Vertices;

	// Set the size of the buffers
	//PositionBuffer.ComponentSize = Component->Size;
	//UVBuffer.ComponentSize = Component->Size;
	//TangentBuffer.ComponentSize = Component->Size;

	// Initialize render resources
	//BeginInitResource(&PositionBuffer);
	//BeginInitResource(&UVBuffer);
	//BeginInitResource(&TangentBuffer);

	//BeginInitResource(&IndexBuffer);

	Initialize(Component->XOffset * (Component->Size - 1), Component->YOffset * (Component->Size - 1), Component->Tiling);

	// Get the material
	Material = Component->GetMaterial(0);
	if (Material == nullptr)
	{
		Material = UMaterial::GetDefaultMaterial(MD_Surface);
	}
}

FTerrainComponentSceneProxy::~FTerrainComponentSceneProxy()
{
	VertexBuffers.PositionVertexBuffer.ReleaseResource();
	VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
	IndexBuffer.ReleaseResource();
	VertexFactory.ReleaseResource();
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
			element.IndexBuffer = &IndexBuffer;
			element.FirstIndex = 0;
			element.NumPrimitives = IndexBuffer.Indices.Num() / 3;
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
	ENQUEUE_RENDER_COMMAND(FComponentFillBuffers)([this, X, Y, Tiling](FRHICommandListImmediate& RHICmdList) {
		// Initialize buffers
		VertexBuffers.PositionVertexBuffer.Init(Size * Size);
		VertexBuffers.StaticMeshVertexBuffer.Init(Size * Size, 1);

		// Load data for all buffers
		UpdateMapData();
		UpdateUVData(X, Y, Tiling);

		// Initialize the buffers
		IndexBuffer.InitResource();
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
		});
}

/// Proxy Update Functions ///

void FTerrainComponentSceneProxy::UpdateUVData(int32 XOffset, int32 YOffset, float Tiling)
{
	// Fill UV data
	for (uint32 y = 0; y < Size; ++y)
	{
		for (uint32 x = 0; x < Size; ++x)
		{
			VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(y * Size + x, 0, FVector2D((XOffset + x)* Tiling, (YOffset + y)* Tiling));
		}
	}
}

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
	for (uint32 y = 0; y < Size; ++y)
	{
		for (uint32 x = 0; x < Size; ++x)
		{
			uint32 i = y * Size + x;

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
			FVector vy(0, 2.0f, s10 - s12);

			// Calculate the cross product of the two tangents
			vx.Normalize();
			vy.Normalize();

			FVector normal = FVector::CrossProduct(vx, vy);
			FVector tangent_x = FVector(vx.X, vx.Y, vx.Z);
			FVector tangent_y = FVector(vy.X, vy.Y, vy.Z);

			VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(i, tangent_x, tangent_y, normal);
		}
	}
}