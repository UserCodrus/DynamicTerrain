#pragma once

#include "CoreMinimal.h"

#include "TerrainFoliage.generated.h"

class ATerrain;
class UHierarchicalInstancedStaticMeshComponent;

UCLASS(BlueprintType)
class DYNAMICTERRAIN_API UTerrainFoliage : public UObject
{
	GENERATED_BODY()

public:
	// The mesh that will be generated
	UPROPERTY(EditAnywhere, Category = "Mesh")
		UStaticMesh* Mesh = nullptr;

	// If set to true this foliage will ignore the shade distance and use safe distance
	UPROPERTY(EditAnywhere, Category = "Spacing")
		bool GrowInShade = false;
	// The minimum safe space between this mesh and other meshes
	UPROPERTY(EditAnywhere, Category = "Spacing")
		float SafeDistance = 100.0f;
	// The safe distance between this mesh and meshes that grow outside shade
	UPROPERTY(EditAnywhere, Category = "Spacing")
		float ShadeDistance = 200.0f;

	// The minimum elevation this mesh will generate at
	UPROPERTY(EditAnywhere, Category = "Placement")
		float MinElevation = -200000.0f;
	// The maximum elevation this mesh will generate at
	UPROPERTY(EditAnywhere, Category = "Placement")
		float MaxElevation = 200000.0f;

	// If true the foliage will line up with the slope of the terrain
	UPROPERTY(EditAnywhere, Category = "Placement")
		bool AlignToNormal = true;
	// Set to true to rotate the mesh randomly
	UPROPERTY(EditAnywhere, Category = "Placement")
		bool RandomRotation = true;
};

USTRUCT(BlueprintType)
struct DYNAMICTERRAIN_API FWeightedFoliage
{
	GENERATED_BODY()

	// The foliage asset that will be generated
	UPROPERTY(EditAnywhere)
		UTerrainFoliage* Asset = nullptr;
	// The odds that this foliage will appear in a cluster
	UPROPERTY(EditAnywhere)
		uint32 Weight = 1;
};

UCLASS(BlueprintType)
class DYNAMICTERRAIN_API UTerrainFoliageSpawner : public UObject
{
	GENERATED_BODY()

public:
	// The static meshes used by this foliage group
	UPROPERTY(EditAnywhere, Category = "Foliage")
		TArray<FWeightedFoliage> Foliage;

	// The minimum number of meshes in each cluster
	UPROPERTY(EditAnywhere, Category = "Clustering")
		uint32 ClusterMin = 1;
	// The maximum number of meshes in each cluster
	UPROPERTY(EditAnywhere, Category = "Clustering")
		uint32 ClusterMax = 1;
	// The radius of clusters in world space units
	UPROPERTY(EditAnywhere, Category = "Clustering")
		float Radius = 500.0f;
	// If true each mesh in a given cluster will all be the same, otherwise they will be random
	UPROPERTY(EditAnywhere, Category = "Clustering")
		bool MatchClusters = true;

	// Create a cluster of foliage at the designated location in world space
	void AddFoliageCluster(ATerrain* Terrain, FVector Location, uint32 Seed) const;
	// Create a single piece of foliage at the designated location in world space
	void AddFoliageUnit(ATerrain* Terrain, FVector Location, uint32 Seed) const;

protected:
	// Pick a random foliage asset from the spawner
	UTerrainFoliage* GetRandomFoliage(uint32 Seed) const;
};