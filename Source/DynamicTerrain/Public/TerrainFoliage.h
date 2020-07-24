#pragma once

#include "CoreMinimal.h"

#include "TerrainFoliage.generated.h"

class ATerrain;
class UInstancedStaticMeshComponent;

USTRUCT(BlueprintType)
struct DYNAMICTERRAIN_API FFoliageMesh
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
		UStaticMesh* Mesh = nullptr;
	UPROPERTY(EditAnywhere)
		uint32 Weight = 1;
};

UCLASS(BlueprintType)
class DYNAMICTERRAIN_API UTerrainFoliageGroup : public UObject
{
	GENERATED_BODY()

public:
	// The static meshes used by this foliage group
	UPROPERTY(EditAnywhere)
		TArray<FFoliageMesh> Meshes;

	// The minimum number of meshes in each cluster
	UPROPERTY(EditAnywhere)
		uint32 ClusterMin = 1;
	// The maximum number of meshes in each cluster
	UPROPERTY(EditAnywhere)
		uint32 ClusterMax = 1;
	// If true each mesh in a given cluster will all be the same, otherwise they will be random
	UPROPERTY(EditAnywhere)
		bool MatchClusters = true;

	// If true the foliage will line up with the slope of the terrain
	UPROPERTY(EditAnywhere)
		bool AlignToNormal = true;
	// Set to true to rotate the mesh randomly
	UPROPERTY(EditAnywhere)
		bool RandomRotation = true;

	// Create a cluster of foliage at the designated location in world space
	void AddFoliageCluster(ATerrain* Terrain, FVector Location, FRotator Rotation, uint32 Seed) const;
	// Create a single piece of foliage at the designated location in world space
	void AddFoliageUnit(ATerrain* Terrain, FVector Location, FRotator Rotation, uint32 Seed) const;

protected:
	// Get instanced static mesh components from a terrain matching this foliage group
	UInstancedStaticMeshComponent* GetRandomComponent(ATerrain* Terrain, uint32 Seed) const;
};