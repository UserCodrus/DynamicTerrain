#include "TerrainFoliage.h"
#include "Terrain.h"
#include "TerrainAlgorithms.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include <random>
#include <limits>

void UTerrainFoliageGroup::AddFoliageCluster(ATerrain* Terrain, FVector Location, uint32 Seed) const
{
	// Get the bounds of the map
	float xbounds = (float)(Terrain->GetMap()->GetWidthX() - 3) / 2.0f * Terrain->GetActorScale3D().X;
	float ybounds = (float)(Terrain->GetMap()->GetWidthY() - 3) / 2.0f * Terrain->GetActorScale3D().Y;
	FVector center = Terrain->GetActorLocation();

	FVector2D min(center.X - xbounds, center.Y - ybounds);
	FVector2D max(center.X + xbounds, center.Y + ybounds);

	// Get a cluster of points
	std::default_random_engine rando(Seed);
	std::uniform_int_distribution<uint32> random_seed(0, std::numeric_limits<uint32>::max());
	std::uniform_int_distribution<uint32> cluster(ClusterMin, ClusterMax);
	RandomCirclePointNoise noise(Radius, cluster(rando), random_seed(rando));

	// Add meshes at each point generated
	const TArray<FVector2D>& points = noise.getPoints();
	UInstancedStaticMeshComponent* component = nullptr;
	for (int32 i = 0; i < points.Num(); ++i)
	{
		// Set the location of the mesh
		FVector location = Location;
		location.X += points[i].X;
		location.Y += points[i].Y;

		// Make sure the point is within the bounds
		if (location.X < max.X && location.X > min.X && location.Y < max.Y && location.Y > min.Y)
		{
			// Set the height to match the terrain
			location.Z = Terrain->GetHeight(location);

			// Pick a new mesh
			if (!MatchClusters || component == nullptr)
			{
				component = GetRandomComponent(Terrain, random_seed(rando));
			}

			// Set the rotation to match the terrain normal
			FRotator rotation;
			if (AlignToNormal)
			{
				rotation = UKismetMathLibrary::MakeRotFromZ(Terrain->GetNormal(location));
			}

			// Add a mesh
			FTransform transform;
			transform.SetLocation(location);
			transform.SetRotation(rotation.Quaternion());
			component->AddInstance(transform);
		}
	}
}

void UTerrainFoliageGroup::AddFoliageUnit(ATerrain* Terrain, FVector Location, uint32 Seed) const
{
	UInstancedStaticMeshComponent* components = GetRandomComponent(Terrain, Seed);

	if (components != nullptr)
	{
		// Set the rotation
		FVector location = Location;
		location.Z = Terrain->GetHeight(location);
		
		// Set the rotation
		FRotator rotation;
		if (AlignToNormal)
		{
			rotation = UKismetMathLibrary::MakeRotFromZ(Terrain->GetNormal(location));
		}

		// Add a mesh
		FTransform transform;
		transform.SetLocation(location);
		transform.SetRotation(rotation.Quaternion());
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