#pragma once

#include "CoreMinimal.h"
#include "HeightMap.generated.h"

UCLASS()
class UHeightMap : public UObject
{
	GENERATED_BODY()

public:
	/// Blueprint Functions ///

	// Resize the heightmap
	// X, Y = The width of the heightmap
	// Z = The maximum height of the heightmap
	UFUNCTION(BlueprintCallable)
		void Resize(int32 X, int32 Y, int32 Z);

	// Get the value of the heightmap at the specified coordinates
	UFUNCTION(BlueprintPure)
		float BPGetHeight(int32 X, int32 Y) const;

	// Calculate the normals and tangents of the map mesh
	UFUNCTION(BlueprintPure)
		void CalculateNormalsAndTangents(TArray<FVector>& Normals, TArray<FProcMeshTangent>& Tangents) const;

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

	// The maximum height of the heightmap
	UPROPERTY(VisibleAnywhere)
		float MaxHeight = 1.0f;
};

UCLASS(Abstract)
class UMapGenerator : public UObject
{
	GENERATED_BODY()

public:
	/// Map Generator Functions ///

	// Flatten the heightmap
	UFUNCTION(BlueprintCallable)
		static void Flat(UHeightMap* Map);

	// Generate a map using plasma noise
	UFUNCTION(BlueprintCallable)
		static void Plasma(UHeightMap* Map, int32 Scale);

	// Generate a map using multiple layers of perlin noise
	UFUNCTION(BlueprintCallable)
		static void Perlin(UHeightMap* Map, int32 Frequency, int32 Octaves, float Persistence);
};