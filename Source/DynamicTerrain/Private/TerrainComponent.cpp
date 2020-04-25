#include "TerrainComponent.h"

#include "Terrain.h"
#include "TerrainRender.h"

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

/// Mesh Component Interface ///

UTerrainComponent::UTerrainComponent(const FObjectInitializer& ObjectInitializer)
{
	// Disable ticking for the component to save some CPU cycles
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