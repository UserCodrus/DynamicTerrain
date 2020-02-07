#pragma once

#include "CoreMinimal.h"
#include "Terrain.h"

class DYNAMICTERRAIN_API FTerrainBrush
{
public:
	// Retrive the name of the brush for the editor UI
	virtual FText GetName() const = 0;

	// Get the falloff of the brush
	virtual float GetMagnitude(float Distance, float Radius, float Falloff) const = 0;
};

// A brush with a sharp, linear shape
class DYNAMICTERRAIN_API FBrushLinear : public FTerrainBrush
{
public:
	virtual FText GetName() const override;
	virtual float GetMagnitude(float Distance, float Radius, float Falloff) const override;
};

// A brush with a steep but smooth shape
class DYNAMICTERRAIN_API FBrushSmooth : public FTerrainBrush
{
public:
	virtual FText GetName() const override;
	virtual float GetMagnitude(float Distance, float Radius, float Falloff) const override;
};

// A brush with a smooth, gradually rounded shape
class DYNAMICTERRAIN_API FBrushRound : public FTerrainBrush
{
public:
	virtual FText GetName() const override;
	virtual float GetMagnitude(float Distance, float Radius, float Falloff) const override;
};

// A brush with a spherical shape
class DYNAMICTERRAIN_API FBrushSphere : public FTerrainBrush
{
public:
	virtual FText GetName() const override;
	virtual float GetMagnitude(float Distance, float Radius, float Falloff) const override;
};

class DYNAMICTERRAIN_API FTerrainTool
{
public:
	// Use the tool to manipulate a heightmap
	virtual void Use(UHeightMap* Map, FVector2D Center, float Delta) = 0;

	// Retrive the name of the tool for the editor UI
	virtual FText GetName() const = 0;

	virtual void Tick(float DeltaTime) = 0;

	virtual void Activate();
	virtual void Deactivate();

	// Select a specific terrain object
	virtual void Select(ATerrain* Target);
	
	// Select a brush
	virtual void SetBrush(FTerrainBrush* NewBrush);

	// Apply the tool to the terrain
	virtual void Apply(FVector2D Center, float Delta) const;

	// Get the location of the mouse cursor on the terrain
	bool MouseToTerrainPosition(const FSceneView* View, FHitResult& Result);
	bool MouseToTerrainPosition(const APlayerController* Controller, FHitResult& Result);

	// Convert a world vector to heightmap coordinates
	FVector2D WorldVectorToMapVector(FVector WorldPosition);

	float Size = 10.0f;				// The radius of the tool circle
	float Strength = 0.01f;			// The strength of the tool
	float Falloff = 5.0f;			// The distance from the center that the strength begins to fall

	static float TraceDistance;		// The distance to check for the mouse cursor touching the terrain

protected:
	//virtual float GetMagnitude(float Distance) const;

	ATerrain* Terrain = nullptr;	// The currently selected terrain object
	FTerrainBrush* Brush = nullptr;	// The currently selected brush

	bool Active = false;			// Set to true when the tool should operate every tick
};

// A tool for raising the terrain
class DYNAMICTERRAIN_API FRaiseTool : public FTerrainTool
{
public:
	virtual void Use(UHeightMap* Map, FVector2D Center, float Delta) override;

	virtual void Tick(float DeltaTime) override;

	virtual FText GetName() const override;
};

// A tool for lowering the terrain
class DYNAMICTERRAIN_API FLowerTool : public FTerrainTool
{
public:
	virtual void Use(UHeightMap* Map, FVector2D Center, float Delta) override;

	virtual void Tick(float DeltaTime) override;

	virtual FText GetName() const override;
};