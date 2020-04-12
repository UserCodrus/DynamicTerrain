#include "TerrainComponent.h"

#include "Terrain.h"

#include "HAL/RunnableThread.h"
#include "KismetProceduralMeshLibrary.h"

#include "Engine.h"
#include "PrimitiveSceneProxy.h"
#include "DynamicMeshBuilder.h"
#include "Materials/Material.h"
#include "Engine/CollisionProfile.h"

#define HMAPMAX 65535

ComponentBuilder::ComponentBuilder(const ATerrain* ParentTerrain)
{
	Terrain = ParentTerrain;

	ComponentX = 0;
	ComponentY = 0;

	Idle = true;

	// Create the thread
	Thread = FRunnableThread::Create(this, TEXT("ComponentBuilder worker"));
}

ComponentBuilder::~ComponentBuilder()
{
	if (Thread != nullptr)
	{
		delete Thread;
		Thread = nullptr;
	}
}

/// FRunnable Interface ///

bool ComponentBuilder::Init()
{
	return true;
}

uint32 ComponentBuilder::Run()
{
	while (Counter.GetValue() == 0)
	{
		if (!Idle)
		{
			// Build a component section if the thread is not idling
			Data.Allocate(Terrain->GetComponentSize());
			Terrain->GenerateMeshSection(ComponentX, ComponentY, Data, true);

			Idle = true;
		}
		else
		{
			// Sleep when idle
			FPlatformProcess::Sleep(0.01);
		}
	}

	return 0;
}

void ComponentBuilder::Stop()
{
	Counter.Increment();
}

/// Builder Interface ///

bool ComponentBuilder::IsIdle()
{
	return Idle;
}

void ComponentBuilder::Build(int32 component_x, int32 component_y)
{
	ComponentX = component_x;
	ComponentY = component_y;

	Idle = false;
}

ComponentData* ComponentBuilder::GetData()
{
	if (Idle)
	{
		return &Data;
	}
	else
	{
		return nullptr;
	}
}

int32 ComponentBuilder::GetSection()
{
	return ComponentY * Terrain->GetXWidth() + ComponentX;
}

/// Component Scene Proxy ///

class FTerrainComponentSceneProxy : public FPrimitiveSceneProxy
{
public:
	FTerrainComponentSceneProxy(UTerrainComponent* Component) : FPrimitiveSceneProxy(Component), VertexFactory(GetScene().GetFeatureLevel(), "FTerrainComponentSceneProxy"), MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
	{
		// Fill the index buffer
		IndexBuffer.Indices = Component->IndexBuffer;

		// Fill the vertex buffers
		TArray<FDynamicMeshVertex> vertices;
		vertices.AddUninitialized(Component->VertexBuffer.Num());
		for (int32 i = 0; i < vertices.Num(); ++i)
		{
			FDynamicMeshVertex& dv = vertices[i];
			FTerrainVertex& tv = Component->VertexBuffer[i];

			dv.Position = tv.Position;
			dv.Color = FColor(255, 255, 255);
			dv.TextureCoordinate[0] = tv.UV;
			dv.TextureCoordinate[1] = tv.UV;
			dv.TextureCoordinate[2] = tv.UV;
			dv.TextureCoordinate[3] = tv.UV;
			dv.TangentX = tv.Tangent;
			dv.TangentZ = tv.Normal;
		}

		VertexBuffers.InitFromDynamicVertex(&VertexFactory, vertices);

		// Initialize render resources
		BeginInitResource(&VertexBuffers.PositionVertexBuffer);
		BeginInitResource(&VertexBuffers.StaticMeshVertexBuffer);
		BeginInitResource(&VertexBuffers.ColorVertexBuffer);
		BeginInitResource(&IndexBuffer);
		BeginInitResource(&VertexFactory);

		// Get the material
		Material = Component->GetMaterial(0);
		if (Material == nullptr)
		{
			Material = UMaterial::GetDefaultMaterial(MD_Surface);
		}
	}

	virtual ~FTerrainComponentSceneProxy()
	{
		VertexBuffers.PositionVertexBuffer.ReleaseResource();
		VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
		VertexBuffers.ColorVertexBuffer.ReleaseResource();

		IndexBuffer.ReleaseResource();

		VertexFactory.ReleaseResource();
	}

	SIZE_T GetTypeHash() const override
	{
		static size_t unique_pointer;
		return reinterpret_cast<size_t>(&unique_pointer);
	}

	virtual uint32 GetMemoryFootprint() const override
	{
		return (sizeof(*this) + GetAllocatedSize());
	}

	uint32 GetAllocatedSize() const
	{
		return(FPrimitiveSceneProxy::GetAllocatedSize());
	}

	virtual bool CanBeOccluded() const override
	{
		return !MaterialRelevance.bDisableDepthTest;
	}

	virtual void GetDynamicMeshElements(const TArray< const FSceneView* >& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, class FMeshElementCollector& Collector) const override
	{
		// Get the wireframe material
		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;
		FColoredMaterialRenderProxy* WireframeMaterialInstance = new FColoredMaterialRenderProxy(GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : nullptr, FLinearColor(0, 0.5f, 1.f));
		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

		// Get the material proxy from either the current material or the wireframe material
		FMaterialRenderProxy* material_proxy = nullptr;
		if (bWireframe)
		{
			material_proxy = WireframeMaterialInstance;
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
				mesh.bWireframe = bWireframe;
				mesh.VertexFactory = &VertexFactory;
				mesh.MaterialRenderProxy = material_proxy;
				mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
				mesh.Type = PT_TriangleList;
				mesh.DepthPriorityGroup = SDPG_World;
				mesh.bCanApplyViewModeOverrides = false;

				// Set up the first element of the mesh 
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
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
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

protected:
	UMaterialInterface* Material;
	FStaticMeshVertexBuffers VertexBuffers;
	FDynamicMeshIndexBuffer32 IndexBuffer;
	FLocalVertexFactory VertexFactory;

	FMaterialRelevance MaterialRelevance;
};

/// Mesh Component Interface ///

UTerrainComponent::UTerrainComponent(const FObjectInitializer& ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
}

FPrimitiveSceneProxy* UTerrainComponent::CreateSceneProxy()
{
	FPrimitiveSceneProxy* proxy = nullptr;

	if (VertexBuffer.Num() > 0 && IndexBuffer.Num() > 0)
	{
		proxy = new FTerrainComponentSceneProxy(this);
	}

	return proxy;
}

int32 UTerrainComponent::GetNumMaterials() const
{
	return 1;
}

FBoxSphereBounds UTerrainComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBox bound(ForceInit);

	for (int32 i = 0; i < VertexBuffer.Num(); ++i)
	{
		bound += LocalToWorld.TransformPosition(VertexBuffer[i].Position);
	}

	FBoxSphereBounds boxsphere;
	boxsphere.BoxExtent = bound.GetExtent();
	boxsphere.Origin = bound.GetCenter();
	boxsphere.SphereRadius = boxsphere.BoxExtent.Size();

	return boxsphere;
}

/// Terrain Interface ///

void UTerrainComponent::GenerateVertices(int32 X, int32 Y, ATerrain* Terrain)
{
	Terrain->GenerateMeshSection(X, Y, VertexBuffer, IndexBuffer);
	SetMaterial(0, Terrain->GetMaterials());
	MarkRenderStateDirty();
	UpdateBounds();
}