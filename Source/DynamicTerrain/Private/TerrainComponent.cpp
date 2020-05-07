#include "TerrainComponent.h"

#include "Terrain.h"
#include "TerrainRender.h"
#include "TerrainStat.h"

#include "Engine.h"
#include "PrimitiveSceneProxy.h"
#include "DynamicMeshBuilder.h"
#include "Materials/Material.h"
#include "Engine/CollisionProfile.h"

DECLARE_CYCLE_STAT(TEXT("Dynamic Terrain - Rebuild Collision"), STAT_DynamicTerrain_RebuildCollision, STATGROUP_DynamicTerrain)

/// Mesh Component Interface ///

UTerrainComponent::UTerrainComponent(const FObjectInitializer& ObjectInitializer)
{
	Size = 0;
	Tiling = 1.0f;

	// Disable ticking for the component to save some CPU cycles
	PrimaryComponentTick.bCanEverTick = false;

	SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
}

FPrimitiveSceneProxy* UTerrainComponent::CreateSceneProxy()
{
	FPrimitiveSceneProxy* proxy = nullptr;
	VerifyMapProxy();

	if (Vertices.Num() > 0 && IndexBuffer.Num() > 0 && MapProxy.IsValid())
	{
		proxy = new FTerrainComponentSceneProxy(this);
	}

	return proxy;
}

UBodySetup* UTerrainComponent::GetBodySetup()
{
	if (BodySetup == nullptr)
	{
		BodySetup = CreateBodySetup();
	}
	return BodySetup;
}

int32 UTerrainComponent::GetNumMaterials() const
{
	return 1;
}

bool UTerrainComponent::GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData)
{
	// Copy vertex and triangle data
	CollisionData->Vertices = Vertices;
	int32 num_triangles = IndexBuffer.Num() / 3;
	for (int32 i = 0; i < num_triangles; ++i)
	{
		FTriIndices tris;
		tris.v0 = IndexBuffer[i * 3];
		tris.v1 = IndexBuffer[i * 3 + 1];
		tris.v2 = IndexBuffer[i * 3 + 2];
		CollisionData->Indices.Add(tris);
	}

	CollisionData->bFlipNormals = true;
	CollisionData->bDeformableMesh = true;
	CollisionData->bFastCook = true;

	return true;
}

FBoxSphereBounds UTerrainComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBox bound(ForceInit);

	for (int32 i = 0; i < Vertices.Num(); ++i)
	{
		bound += LocalToWorld.TransformPosition(Vertices[i]);
	}

	FBoxSphereBounds boxsphere;
	boxsphere.BoxExtent = bound.GetExtent();
	boxsphere.Origin = bound.GetCenter();
	boxsphere.SphereRadius = boxsphere.BoxExtent.Size();

	return boxsphere;
}

/// Terrain Interface ///

void UTerrainComponent::Initialize(ATerrain* Terrain, int32 X, int32 Y)
{
	XOffset = X;
	YOffset = Y;
	AsyncCooking = Terrain->GetAsyncCookingEnabled();

	SetSize(Terrain->GetComponentSize());

	MapProxy = Terrain->GetProxy()->SectionProxies[Y * Terrain->GetXWidth() + X];
	UpdateCollision();
}

void UTerrainComponent::CreateMeshData()
{
	// Create vertex data
	Vertices.Empty();
	Vertices.SetNumUninitialized(Size * Size);
	for (uint32 y = 0; y < Size; ++y)
	{
		for (uint32 x = 0; x < Size; ++x)
		{
			Vertices[y * Size + x] = FVector(x, y, 0.0f);
		}
	}

	// Create triangles
	uint32 polygons = Size - 1;
	IndexBuffer.Empty();
	IndexBuffer.SetNumUninitialized(polygons * polygons * 6);
	for (uint32 y = 0; y < polygons; ++y)
	{
		for (uint32 x = 0; x < polygons; ++x)
		{
			uint32 i = (y * polygons + x) * 6;

			IndexBuffer[i] = x + (y * Size);
			IndexBuffer[i + 1] = 1 + x + (y + 1) * Size;
			IndexBuffer[i + 2] = 1 + x + y * Size;

			IndexBuffer[i + 3] = x + (y * Size);
			IndexBuffer[i + 4] = x + (y + 1) * Size;
			IndexBuffer[i + 5] = 1 + x + (y + 1) * Size;
		}
	}
}

void UTerrainComponent::SetSize(uint32 NewSize)
{
	if (NewSize > 1 && NewSize != Size)
	{
		Size = NewSize;
		CreateMeshData();
		MarkRenderStateDirty();
	}
}

void UTerrainComponent::GenerateVertices(ATerrain* Terrain)
{
	//Terrain->GenerateMeshSection(XOffset, YOffset, VertexBuffer, IndexBuffer);
	SetMaterial(0, Terrain->GetMaterials());
	MarkRenderStateDirty();
	UpdateBounds();
}

void UTerrainComponent::Update(TSharedPtr<FMapSection, ESPMode::ThreadSafe> NewSection)
{
	MapProxy = NewSection;

	// Update collision data and bounds
	for (uint32 y = 0; y < Size; ++y)
	{
		for (uint32 x = 0; x < Size; ++x)
		{
			Vertices[y * Size + x].Z = MapProxy->Data[(y + 1) * NewSection->X + x + 1];
		}
	}
	BodyInstance.UpdateTriMeshVertices(Vertices);
	UpdateBounds();

	// Update the scene proxy
	FTerrainComponentSceneProxy* proxy = (FTerrainComponentSceneProxy*)SceneProxy;
	ENQUEUE_RENDER_COMMAND(FComponentUpdate)([proxy, NewSection](FRHICommandListImmediate& RHICmdList) {
		proxy->Update(NewSection);
		});
	MarkRenderTransformDirty();
}

void UTerrainComponent::UpdateCollision()
{
	SCOPE_CYCLE_COUNTER(STAT_DynamicTerrain_RebuildCollision);

	UWorld* world = GetWorld();
	bool async = world->IsGameWorld() && AsyncCooking;

	if (async)
	{
		// Abort previous cooks
		for (UBodySetup* body : BodySetupQueue)
		{
			body->AbortPhysicsMeshAsyncCreation();
		}

		// Start cooking a new body
		BodySetupQueue.Add(CreateBodySetup());
		BodySetupQueue.Last()->CreatePhysicsMeshesAsync(FOnAsyncPhysicsCookFinished::CreateUObject(this, &UTerrainComponent::FinishCollision, BodySetupQueue.Last()));
	}
	else
	{
		// Create a new body setup and clean out the async queue
		BodySetupQueue.Empty();
		GetBodySetup();

		// Change GUID for new collision data
		BodySetup->BodySetupGuid = FGuid::NewGuid();

		// Cook collision data
		BodySetup->bHasCookedCollisionData = true;
		BodySetup->InvalidatePhysicsData();
		BodySetup->CreatePhysicsMeshes();
		RecreatePhysicsState();
	}
}

void UTerrainComponent::FinishCollision(bool Success, UBodySetup* NewBodySetup)
{
	// Create a new queue for async cooking
	TArray<UBodySetup*> new_queue;
	new_queue.Reserve(BodySetupQueue.Num());

	// Find the body setup
	int32 location;
	if (BodySetupQueue.Find(NewBodySetup, location))
	{
		if (Success)
		{
			// Use the new body setup
			BodySetup = NewBodySetup;
			RecreatePhysicsState();

			// Remove any earlier requests
			for (int32 i = location + 1; i < BodySetupQueue.Num(); ++i)
			{
				new_queue.Add(BodySetupQueue[i]);
			}
			BodySetupQueue = new_queue;
		}
		else
		{
			// Remove failed bake
			BodySetupQueue.RemoveAt(location);
		}
	}
}

UBodySetup* UTerrainComponent::CreateBodySetup()
{
	UBodySetup* newbody = NewObject<UBodySetup>(this, NAME_None, IsTemplate() ? RF_Public : RF_NoFlags);
	newbody->BodySetupGuid = FGuid::NewGuid();

	newbody->bGenerateMirroredCollision = false;
	newbody->bDoubleSidedGeometry = true;
	newbody->CollisionTraceFlag = CTF_UseComplexAsSimple;

	return newbody;
}

TSharedPtr<FMapSection, ESPMode::ThreadSafe> UTerrainComponent::GetMapProxy()
{
	VerifyMapProxy();
	return MapProxy;
}

void UTerrainComponent::VerifyMapProxy()
{
	if (!MapProxy.IsValid())
	{
		if (Size > 1)
		{
			MapProxy = MakeShareable(new FMapSection(Size + 2, Size + 2));
		}
	}
	else
	{
		if (MapProxy->X != Size + 2 || MapProxy->Y != Size + 2)
		{
			MapProxy = MakeShareable(new FMapSection(Size + 2, Size + 2));
		}
	}
}