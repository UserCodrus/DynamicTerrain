#pragma once

#include "CoreMinimal.h"
#include "Terrain.h"

class DYNAMICTERRAIN_API FTerrainBrush
{
public:
	// Retrive the name of the brush for the editor UI
	virtual FText GetName() const = 0;

	// Get the falloff of the brush
	virtual float GetStrength(float Distance, float Radius, float Falloff) const = 0;
};

// A brush with a sharp, linear shape
class DYNAMICTERRAIN_API FBrushLinear : public FTerrainBrush
{
public:
	virtual FText GetName() const override;
	virtual float GetStrength(float Distance, float Radius, float Falloff) const override;
};

// A brush with a steep but smooth shape
class DYNAMICTERRAIN_API FBrushSmooth : public FTerrainBrush
{
public:
	virtual FText GetName() const override;
	virtual float GetStrength(float Distance, float Radius, float Falloff) const override;
};

// A brush with a smooth, gradually rounded shape
class DYNAMICTERRAIN_API FBrushRound : public FTerrainBrush
{
public:
	virtual FText GetName() const override;
	virtual float GetStrength(float Distance, float Radius, float Falloff) const override;
};

// A brush with a spherical shape
class DYNAMICTERRAIN_API FBrushSphere : public FTerrainBrush
{
public:
	virtual FText GetName() const override;
	virtual float GetStrength(float Distance, float Radius, float Falloff) const override;
};

class DYNAMICTERRAIN_API FBrushStroke
{
public:
	FBrushStroke() : Bounds() {}
	FBrushStroke(FIntRect StrokeBounds) : Bounds(StrokeBounds)
	{
		Mask.SetNumZeroed(Bounds.Area());
	}

	FIntRect GetBounds()
	{
		return Bounds;
	}

	float& GetData(int X, int Y)
	{
		return Mask[(Y - Bounds.Min.Y) * Bounds.Width() + (X - Bounds.Min.X)];
	}

protected:
	FIntRect Bounds;		// The boundaries of the mask within its parent heightmap
	TArray<float> Mask;		// The alpha mask of the brush
};

class DYNAMICTERRAIN_API FTerrainTool
{
public:
	// Retrive the name of the tool for the editor UI
	virtual FText GetName() const = 0;

	// Calculate a brush mask using the currently selected brush
	virtual FBrushStroke GetStroke(FVector2D Center) const = 0;

	virtual void Tick(float DeltaTime);

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

	bool Invert = false;			// Set to true to invert the effect of the tool

	static float TraceDistance;		// The distance to check for the mouse cursor touching the terrain

protected:
	ATerrain* Terrain = nullptr;	// The currently selected terrain object
	FTerrainBrush* Brush = nullptr;	// The currently selected brush

	bool Active = false;			// Set to true when the tool should operate every tick
};

// A tool for sculpting the terrain
class DYNAMICTERRAIN_API FSculptTool : public FTerrainTool
{
public:
	virtual FBrushStroke GetStroke(FVector2D Center) const override;

	virtual FText GetName() const override;
};

// A tool for smoothing terrain
class DYNAMICTERRAIN_API FSmoothTool : public FTerrainTool
{
public:
	virtual FBrushStroke GetStroke(FVector2D Center) const override;

	virtual FText GetName() const override;
};

// A tool for flattening terrain
class DYNAMICTERRAIN_API FFlattenTool : public FTerrainTool
{
public:
	virtual FBrushStroke GetStroke(FVector2D Center) const override;

	virtual FText GetName() const override;
};