#pragma once

#include "CoreMinimal.h"

#include "TerrainHeightMap.generated.h"

UCLASS()
class DYNAMICTERRAIN_API UHeightMap : public UObject
{
	GENERATED_BODY()

public:
	/// Blueprint Functions ///

	// Resize the heightmap
	// X, Y = The width of the heightmap
	UFUNCTION(BlueprintCallable)
		void Resize(int32 X, int32 Y);

	// Get the value of the heightmap at the specified coordinates
	UFUNCTION(BlueprintPure)
		float BPGetHeight(int32 X, int32 Y) const;

	/// Native Functions ///

	inline float GetHeight(uint32 X, uint32 Y) const;
	inline void SetHeight(uint32 X, uint32 Y, float Height);

	inline int32 GetWidthX() const;
	inline int32 GetWidthY() const;

protected:
	// The height data for the map
	UPROPERTY(VisibleAnywhere)
		TArray<float> MapData;

	// The dimensions of the heightmap
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		int32 WidthX = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		int32 WidthY = 0;
};

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