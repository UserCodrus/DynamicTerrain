#pragma once

#include "CoreMinimal.h"
#include "Terrain.h"

class DYNAMICTERRAIN_API FTerrainTool
{
public:
	// Use the tool to manipulate a heightmap
	virtual void Use(UHeightMap* Map, FVector2D Center, float Delta) = 0;

	// Retrive the name of the tool for the editor UI
	virtual FText GetName() = 0;

	virtual void Tick(float DeltaTime) = 0;

	virtual void Activate();
	virtual void Deactivate();

	// Select a specific terrain object
	virtual void Select(ATerrain* Target);

	// Get the location of the mouse cursor on the terrain
	bool MouseToTerrainPosition(const FSceneView* View, FHitResult& Result);

	float Size = 10.0f;				// The radius of the tool circle
	float Strength = 0.01f;			// The strength of the tool
	float Falloff = 0.0f;			// The distance from the center that the strength begins to fall
	float Hardness = 0.0f;			// The amount that the edges of the cirle contribute

	static float TraceDistance;		// The distance to check for the mouse cursor touching the terrain

protected:
	virtual float GetMagnitude(float Distance);

	ATerrain* Terrain = nullptr;	// The currently selected terrain object

	bool Active = false;			// Set to true when the tool should operate every tick
};

class DYNAMICTERRAIN_API FRaiseTool : public FTerrainTool
{
public:
	virtual void Use(UHeightMap* Map, FVector2D Center, float Delta) override;

	virtual void Tick(float DeltaTime) override;

	virtual FText GetName() override;
};

class DYNAMICTERRAIN_API FLowerTool : public FTerrainTool
{
public:
	virtual void Use(UHeightMap* Map, FVector2D Center, float Delta) override;

	virtual void Tick(float DeltaTime) override;

	virtual FText GetName() override;
};