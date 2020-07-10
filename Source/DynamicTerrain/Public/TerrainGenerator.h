#pragma once

#include "CoreMinimal.h"

#include "TerrainHeightMap.h"
#include "TerrainGenerator.generated.h"

UCLASS()
class DYNAMICTERRAIN_API UMapGenerator : public UObject
{
	GENERATED_BODY()

public:
	/// Map Generator Functions ///

	UPROPERTY()
		UHeightMap* Map = nullptr;

	// Flatten the heightmap
	UFUNCTION(BlueprintCallable)
		void Flat(float Height);

	// Generate a map using plasma noise
	UFUNCTION(BlueprintCallable)
		void Plasma(int32 Scale, float MaxHeight);

	// Generate a map using multiple layers of perlin noise
	UFUNCTION(BlueprintCallable)
		void Perlin(int32 Frequency, int32 Octaves, float Persistence, float MaxHeight);
};