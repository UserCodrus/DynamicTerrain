#pragma once

#include "CoreMinimal.h"

#include "TerrainHeightMap.generated.h"

struct FMapSection
{
	TArray<float> Data;
	int32 X = 0;
	int32 Y = 0;

	FMapSection() {};
	FMapSection(int32 XWidth, int32 YWidth)
	{
		X = XWidth;
		Y = YWidth;
		Data.SetNumZeroed(X * Y);
	}
};

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

	// Get a copy of a portion of the map
	inline void GetMapSection(FMapSection* Section, FIntPoint Min);

	// Get the height at a given vertex
	inline float GetHeight(uint32 X, uint32 Y) const;
	// Get the height of the map at a given point
	inline float GetLinearHeight(float X, float Y) const;
	// Get the normal of a given vertex
	inline FVector GetNormal(uint32 X, uint32 Y) const;
	// Get the normal of the map at a given point
	inline FVector GetLinearNormal(float X, float Y) const;
	// Get the X tangent at a given vertex
	inline FVector GetTangent(uint32 X, uint32 Y) const;
	// Get the X tangent of the map at a given point
	inline FVector GetLinearTangent(float X, float Y) const;
	// Set the height of the heightmap at the given vertex
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