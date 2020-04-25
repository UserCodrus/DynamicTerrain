#include "TerrainRender.h"
#include "TerrainComponent.h"
#include "Terrain.h"

#include "Engine.h"
#include "Materials/Material.h"

/// Rendering Buffers ///

FTerrainVertexBuffer::FTerrainVertexBuffer(int32 DataSize, uint32 BufferStride, uint8 BufferFormat) : StaticFlag(BUF_Dynamic), Size(DataSize), Stride(BufferStride), Format(BufferFormat), Vertices(0)
{
	// Initalizers only
}

void FTerrainVertexBuffer::InitRHI()
{
	if (Size > 0 && Vertices > 0)
	{
		// Create the RHI buffer
		FRHIResourceCreateInfo info;
		VertexBufferRHI = RHICreateVertexBuffer(Vertices * Size, StaticFlag | BUF_ShaderResource, info);

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

void FTerrainVertexBuffer::Reset(uint32 VertexCount)
{
	Vertices = VertexCount;

	//ReleaseResource();
	//InitResource();
}

FTerrainPositionBuffer::FTerrainPositionBuffer() : FTerrainVertexBuffer(sizeof(FVector), sizeof(float), PF_R32_FLOAT)
{
	// Initializers only
}

void FTerrainPositionBuffer::Set()
{
	if (Size > 0 && Vertices > 0)
	{
		// Make sure the RHI buffer is initialized
		check(VertexBufferRHI.IsValid());

		// Copy data to the RHI buffer
		void* buffer = RHILockVertexBuffer(VertexBufferRHI, 0, Data.Num() * Size, RLM_WriteOnly);
		FMemory::Memcpy(buffer, Data.GetData(), Data.Num() * Size);
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

void FTerrainUVBuffer::Set()
{
	if (Size > 0 && Vertices > 0)
	{
		check(VertexBufferRHI.IsValid());

		void* buffer = RHILockVertexBuffer(VertexBufferRHI, 0, Data.Num() * Size, RLM_WriteOnly);
		FMemory::Memcpy(buffer, Data.GetData(), Data.Num() * Size);
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

void FTerrainTangentBuffer::Set()
{
	if (Size > 0 && Vertices > 0)
	{
		check(VertexBufferRHI.IsValid());

		void* buffer = RHILockVertexBuffer(VertexBufferRHI, 0, Data.Num() * Size, RLM_WriteOnly);
		FMemory::Memcpy(buffer, Data.GetData(), Data.Num() * Size);
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

FTerrainColorBuffer::FTerrainColorBuffer() : FTerrainVertexBuffer(sizeof(FColor), sizeof(FColor), PF_R8G8B8A8)
{
	// Initializers only
}

void FTerrainColorBuffer::Set()
{
	if (Size > 0 && Vertices > 0)
	{
		check(VertexBufferRHI.IsValid());

		void* buffer = RHILockVertexBuffer(VertexBufferRHI, 0, Data.Num() * Size, RLM_WriteOnly);
		FMemory::Memcpy(buffer, Data.GetData(), Data.Num() * Size);
		RHIUnlockVertexBuffer(VertexBufferRHI);
	}
}

void FTerrainColorBuffer::Bind(FLocalVertexFactory::FDataType& DataType)
{
	// Add color data
	DataType.ColorComponent = FVertexStreamComponent(this, 0, Size, VET_Color, EVertexStreamUsage::ManualFetch);
	DataType.ColorComponentsSRV = ShaderResourceView;
}

/// Terrain Scene Proxy ///

FTerrainComponentSceneProxy::FTerrainComponentSceneProxy(UTerrainComponent* Component) : FPrimitiveSceneProxy(Component), VertexFactory(GetScene().GetFeatureLevel(), "FTerrainComponentSceneProxy"), MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
{
	// Fill the buffers
	IndexBuffer.Indices = Component->IndexBuffer;

	// Fill the vertex buffers
	//TArray<FDynamicMeshVertex> vertices;
	//vertices.AddUninitialized(Component->VertexBuffer.Num());
	for (int32 i = 0; i < Component->VertexBuffer.Num(); ++i)
	{
		//FDynamicMeshVertex& dv = vertices[i];
		FTerrainVertex& tv = Component->VertexBuffer[i];

		/*dv.Position = tv.Position;
		dv.Color = FColor(255, 255, 255);
		dv.TextureCoordinate[0] = tv.UV;
		dv.TextureCoordinate[1] = tv.UV;
		dv.TextureCoordinate[2] = tv.UV;
		dv.TextureCoordinate[3] = tv.UV;
		dv.TangentX = tv.Tangent;
		dv.TangentZ = tv.Normal;*/

		PositionBuffer.Data.Add(tv.Position);
		UVBuffer.Data.Add(tv.UV);
		FTerrainTangents tans;
		tans.Normal = tv.Normal;
		tans.Tangent = tv.Tangent;
		TangentBuffer.Data.Add(tans);
		ColorBuffer.Data.Add(FColor(255, 255, 255));
	}

	SizeBuffers();

	// Initialize render resources
	BeginInitResource(&PositionBuffer);
	BeginInitResource(&UVBuffer);
	BeginInitResource(&TangentBuffer);
	BeginInitResource(&ColorBuffer);

	BeginInitResource(&IndexBuffer);

	FillBuffers();
	BindData();

	// Get the material
	Material = Component->GetMaterial(0);
	if (Material == nullptr)
	{
		Material = UMaterial::GetDefaultMaterial(MD_Surface);
	}
}

FTerrainComponentSceneProxy::~FTerrainComponentSceneProxy()
{
	//VertexBuffers.PositionVertexBuffer.ReleaseResource();
	//VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
	//VertexBuffers.ColorVertexBuffer.ReleaseResource();

	PositionBuffer.ReleaseResource();
	UVBuffer.ReleaseResource();
	TangentBuffer.ReleaseResource();
	ColorBuffer.ReleaseResource();

	IndexBuffer.ReleaseResource();

	VertexFactory.ReleaseResource();
}

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
			element.MaxVertexIndex = PositionBuffer.Data.Num();
			//element.MaxVertexIndex = VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;

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
	Result.bVelocityRelevance = IsMovable() && Result.bOpaqueRelevance && Result.bRenderInMainPass;
	return Result;
}

void FTerrainComponentSceneProxy::SizeBuffers()
{
	ENQUEUE_RENDER_COMMAND(FComponentSizeBuffers)([this](FRHICommandListImmediate& RHICmdList) {
		// Set buffer sizes
		PositionBuffer.Reset(PositionBuffer.Data.Num());
		UVBuffer.Reset(UVBuffer.Data.Num());
		TangentBuffer.Reset(TangentBuffer.Data.Num());
		ColorBuffer.Reset(ColorBuffer.Data.Num());
		});
}

void FTerrainComponentSceneProxy::FillBuffers()
{
	ENQUEUE_RENDER_COMMAND(FComponentFillBuffers)([this](FRHICommandListImmediate& RHICmdList) {
		// Load data for all buffers
		PositionBuffer.Set();
		UVBuffer.Set();
		TangentBuffer.Set();
		ColorBuffer.Set();
		});
}

void FTerrainComponentSceneProxy::BindData()
{
	ENQUEUE_RENDER_COMMAND(FComponentBindData)([this](FRHICommandListImmediate& RHICmdList) {
		// Load vertex factory data
		FLocalVertexFactory::FDataType datatype;
		PositionBuffer.Bind(datatype);
		UVBuffer.Bind(datatype);
		TangentBuffer.Bind(datatype);
		ColorBuffer.Bind(datatype);

		// Initalize the vertex factory
		VertexFactory.SetData(datatype);
		VertexFactory.InitResource();
		});
}