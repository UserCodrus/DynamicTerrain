#include "TerrainComponent.h"

#include "HAL/RunnableThread.h"
#include "KismetProceduralMeshLibrary.h"

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