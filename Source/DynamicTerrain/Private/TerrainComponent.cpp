#include "TerrainComponent.h"

#include "Terrain.h"
#include "TerrainRender.h"

#include "Engine.h"
#include "PrimitiveSceneProxy.h"
#include "DynamicMeshBuilder.h"
#include "Materials/Material.h"
#include "Engine/CollisionProfile.h"

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

int32 UTerrainComponent::GetNumMaterials() const
{
	return 1;
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

	SetSize(Terrain->GetComponentSize());

	MapProxy = Terrain->GetProxy()->SectionProxies[Y * Terrain->GetXWidth() + X];
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

	// Update collision vertex cache
	for (uint32 y = 0; y < Size; ++y)
	{
		for (uint32 x = 0; x < Size; ++x)
		{
			Vertices[y * Size + x].Z = MapProxy->Data[(y + 1) * Size + x + 1];
		}
	}

	FTerrainComponentSceneProxy* proxy = (FTerrainComponentSceneProxy*)SceneProxy;
	ENQUEUE_RENDER_COMMAND(FComponentUpdate)([proxy, NewSection](FRHICommandListImmediate& RHICmdList) {
		proxy->Update(NewSection);
		});

	// Update bounds and notify the proxy that bounds have changed
	UpdateBounds();
	MarkRenderTransformDirty();
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