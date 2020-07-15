#pragma once

#include "CoreMinimal.h"

#include "TerrainHeightMap.h"
#include "TerrainGenerator.generated.h"

class ATerrain;

UCLASS()
class DYNAMICTERRAIN_API UMapGenerator : public UObject
{
	GENERATED_BODY()

public:
	/// Map Generator Functions ///

	UPROPERTY()
		ATerrain* Terrain = nullptr;
	UPROPERTY(EditAnywhere)
		uint32 Seed = 0;

	// Generate a new seed for RNG
	void NewSeed();
	// Set the RNG seed
	void SetSeed(int32 NewSeed);

	// Flatten the heightmap
	UFUNCTION(BlueprintCallable)
		void Flat(float Height);

	// Generate a map using plasma noise
	UFUNCTION(BlueprintCallable)
		void Plasma(int32 Scale, int32 Foliage, float MaxHeight);

	// Generate a map using multiple layers of perlin noise
	UFUNCTION(BlueprintCallable)
		void Perlin(int32 Frequency, int32 Octaves, float Persistence, float MaxHeight);

protected:
	/// Map Generator Components ///

	// Generate a flat map at a fixed height
	void MapFlat(float Height);
	// Generate a heightmap using plasma noise
	void MapPlasma(int32 Scale, float MaxHeight);
	// Generate a heightmap using perlin noise
	void MapPerlin(int32 Frequency, int32 Octaves, float Persistence, float MaxHeight);

	// Generate random foliage on the terrain
	void FoliageRandom(uint32 NumPoints);
	// Generate foliage evenly distributed around the map
	void FoliageUniform(uint32 XPoints, uint32 YPoints);
};