#pragma once

#include "TerrainHeightMap.h"

#include "CoreMinimal.h"

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
		void Plasma(
			UPARAM(meta = (Default = 4)) int32 Scale,
			UPARAM(meta = (Default = 5)) int32 Foliage,
			UPARAM(meta = (Default = 256)) float MaxHeight);

	// Generate a map using multiple layers of perlin noise
	UFUNCTION(BlueprintCallable)
		void Perlin(
			UPARAM(meta = (Default = 2)) int32 Frequency,
			UPARAM(meta = (Default = 3)) int32 Octaves,
			UPARAM(meta = (Default = 0.5f)) float Persistence,
			UPARAM(meta = (Default = 256)) float MaxHeight);

	UFUNCTION(BlueprintCallable)
		void TestGenerator(
			UPARAM(meta = (Default=4)) int32 MountainFrequency,
			UPARAM(meta = (Default=256)) float MaxHeight);

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