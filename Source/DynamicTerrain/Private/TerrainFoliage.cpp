#include "TerrainFoliage.h"
#include "Terrain.h"
#include "TerrainAlgorithms.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include <random>
#include <limits>

void UTerrainFoliageSpawner::AddFoliageCluster(ATerrain* Terrain, FVector Location, uint32 Seed) const
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
	PointNoise noise(Radius, cluster(rando), random_seed(rando));

	// Add meshes at each point generated
	const TArray<FVector2D>& points = noise.GetPoints();
	UTerrainFoliage* foliage = nullptr;
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
			if (!MatchClusters || foliage == nullptr )
			{
				foliage = GetRandomFoliage(random_seed(rando));
			}

			// Set the rotation to match the terrain normal
			FRotator rotation;
			if (foliage->AlignToNormal)
			{
				rotation = UKismetMathLibrary::MakeRotFromZ(Terrain->GetNormal(location));
			}

			// Add a mesh
			FTransform transform;
			transform.SetLocation(location);
			transform.SetRotation(rotation.Quaternion());
			Terrain->FindInstancedMesh(foliage->Mesh)->AddInstance(transform);
		}
	}
}

void UTerrainFoliageSpawner::AddFoliageUnit(ATerrain* Terrain, FVector Location, uint32 Seed) const
{
	UTerrainFoliage* foliage = GetRandomFoliage(Seed);

	if (foliage != nullptr)
	{
		// Set the rotation
		FVector location = Location;
		location.Z = Terrain->GetHeight(location);
		
		// Set the rotation
		FRotator rotation;
		if (foliage->AlignToNormal)
		{
			rotation = UKismetMathLibrary::MakeRotFromZ(Terrain->GetNormal(location));
		}

		// Add a mesh
		FTransform transform;
		transform.SetLocation(location);
		transform.SetRotation(rotation.Quaternion());
		Terrain->FindInstancedMesh(foliage->Mesh)->AddInstance(transform);
	}
}

UTerrainFoliage* UTerrainFoliageSpawner::GetRandomFoliage(uint32 Seed) const
{
	// Calculate the total weight of all the meshes
	int32 total_weight = 0;
	for (int32 i = 0; i < Foliage.Num(); ++i)
	{
		total_weight += Foliage[i].Weight;
	}

	if (total_weight > 0)
	{
		// Get a random index value
		std::default_random_engine rando(Seed);
		std::uniform_int_distribution<int32> dist(1, total_weight);
		int32 n = dist(rando);

		// Find which component corresponds to the random index
		UStaticMesh* mesh = nullptr;
		for (int32 i = 0; i < Foliage.Num(); ++i)
		{
			n -= Foliage[i].Weight;
			if (n <= 0)
			{
				return Foliage[i].Asset;
			}
		}
	}

	return nullptr;
}