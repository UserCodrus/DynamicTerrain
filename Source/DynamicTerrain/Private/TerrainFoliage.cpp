#include "TerrainFoliage.h"
#include "Terrain.h"

#include "Components/InstancedStaticMeshComponent.h"

#include <random>

void UTerrainFoliageGroup::AddFoliageCluster(ATerrain* Terrain, FVector Location, FRotator Rotation, uint32 Seed) const
{

}

void UTerrainFoliageGroup::AddFoliageUnit(ATerrain* Terrain, FVector Location, FRotator Rotation, uint32 Seed) const
{
	UInstancedStaticMeshComponent* components = GetRandomComponent(Terrain, Seed);

	if (components != nullptr)
	{
		FTransform transform;
		transform.SetLocation(Location);
		transform.SetRotation(Rotation.Quaternion());
		components->AddInstance(transform);
	}
}

UInstancedStaticMeshComponent* UTerrainFoliageGroup::GetRandomComponent(ATerrain* Terrain, uint32 Seed) const
{
	// Calculate the total weight of all the meshes
	int32 total_weight = 0;
	for (int32 i = 0; i < Meshes.Num(); ++i)
	{
		total_weight += Meshes[i].Weight;
	}

	if (total_weight > 0)
	{
		// Get a random index value
		std::default_random_engine rando(Seed);
		std::uniform_int_distribution<int32> dist(1, total_weight);
		int32 n = dist(rando);

		// Find which component corresponds to the random index
		UStaticMesh* mesh = nullptr;
		for (int32 i = 0; i < Meshes.Num(); ++i)
		{
			n -= Meshes[i].Weight;
			if (n <= 0)
			{
				mesh = Meshes[i].Mesh;
				break;
			}
		}

		if (mesh != nullptr)
		{
			// Find an instanced mesh for the chosen mesh
			return Terrain->FindInstancedMesh(mesh);
		}
	}

	return nullptr;
}