#include "Component.h"

#include "HAL/RunnableThread.h"
#include "KismetProceduralMeshLibrary.h"

#define HMAPMAX 65535

void ComponentData::allocate(uint16 size, uint16 section_id)
{
	//section = section_id;

	// Empty all of the arrays
	vertices.Empty();
	triangles.Empty();
	UV0.Empty();
	normals.Empty();
	tangents.Empty();

	// Preallocate the arrays
	vertices.Reserve(size * size);
	triangles.Reserve((size - 1) * (size - 1) * 6);
	UV0.Reserve(size * size);
	normals.Reserve(size * size);
	tangents.Reserve(size * size);
}

ComponentBuilder::ComponentBuilder(const UHeightMap* Map, const TArray<FVector>* Normals, const TArray<FProcMeshTangent>* Tangents, float map_height, int32 component_size)
{
	heightmap = Map;
	normalmap = Normals;
	tangentmap = Tangents;
	total_height = map_height;
	component_width = component_size;

	component_x = 0;
	component_y = 0;

	idle = true;
	counter;

	// Create the thread
	thread = FRunnableThread::Create(this, TEXT("ComponentBuilder worker"));
}

ComponentBuilder::~ComponentBuilder()
{
	if (thread != nullptr)
	{
		delete thread;
		thread = nullptr;
	}
}

/// FRunnable Interface ///

bool ComponentBuilder::Init()
{
	return true;
}

uint32 ComponentBuilder::Run()
{
	while (counter.GetValue() == 0)
	{
		if (!idle)
		{
			// Build a component section if the thread is not idling
			data.allocate((unsigned)component_width, 0);

			// The number of polygons in each component
			uint16 polygons = component_width - 1;
			int32 heightmap_x = heightmap->GetWidthX();
			int32 heightmap_y = heightmap->GetWidthY();

			// The x and y coordinates of the current component in the world
			float world_x = (float)(heightmap_x - 1) / 2.0f - polygons * component_x;
			float world_y = (float)(heightmap_y - 1) / 2.0f - polygons * component_y;

			// The x and y coordinates of the current component in the heightmap
			int32 map_x = component_x * polygons;
			int32 map_y = component_y * polygons;

			// Create vertex data
			for (int32 y = 0; y < component_width; ++y)
			{
				for (int32 x = 0; x < component_width; ++x)
				{
					// Vertices
					data.vertices.Add(FVector(x - world_x, world_y - y, heightmap->GetHeight(map_x + x, map_y + y) * total_height));

					// Normals
					data.normals.Add((*normalmap)[(map_y + y) * heightmap_x + (map_x + x)]);

					// Tangents
					data.tangents.Add((*tangentmap)[(map_y + y) * heightmap_x + (map_x + x)]);

					// UVs
					data.UV0.Add(FVector2D(x + map_x, y + map_y));
				}
			}

			// Create triangles
			for (int32 y = 0; y < polygons; ++y)
			{
				for (int32 x = 0; x < polygons; ++x)
				{
					data.triangles.Add(x + (y * component_width));
					data.triangles.Add(1 + x + y * component_width);
					data.triangles.Add(1 + x + (y + 1) * component_width);

					data.triangles.Add(x + (y * component_width));
					data.triangles.Add(1 + x + (y + 1) * component_width);
					data.triangles.Add(x + (y + 1) * component_width);
				}
			}

			idle = true;
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
	counter.Increment();
}

/// Builder Interface ///

bool ComponentBuilder::IsIdle()
{
	return idle;
}

ComponentData* ComponentBuilder::GetData()
{
	if (idle)
	{
		return &data;
	}
	else
	{
		return nullptr;
	}
}

void ComponentBuilder::Build(int32 _component_x, int32 _component_y, uint16 section_number)
{
	component_x = _component_x;
	component_y = _component_y;

	//data.allocate((unsigned)component_width, section_number);
	data.section = section_number;

	idle = false;
}