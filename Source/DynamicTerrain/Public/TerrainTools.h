#pragma once

#include "CoreMinimal.h"
#include "Terrain.h"

enum class TerrainBrushID
{
	LINEAR,
	SMOOTH,
	ROUND,
	SPHERE,
	NUM
};

enum class TerrainToolID
{
	SCULPT,
	SMOOTH,
	FLATTEN,
	NUM
};

/// Terrain Brushes ///

class DYNAMICTERRAIN_API FTerrainBrush
{
public:
	virtual ~FTerrainBrush() {};

	// Get the name of the brush for the editor UI
	virtual FText GetName() const = 0;

	// Get the ID value of the brush to identify it easily
	virtual TerrainBrushID GetID() const = 0;

	// Get the falloff of the brush
	virtual float GetStrength(float Distance, float Radius, float Falloff) const = 0;
};

// A brush with a sharp, linear shape
class DYNAMICTERRAIN_API FBrushLinear : public FTerrainBrush
{
public:
	virtual FText GetName() const override;
	virtual TerrainBrushID GetID() const;
	virtual float GetStrength(float Distance, float Radius, float Falloff) const override;
};

// A brush with a steep but smooth shape
class DYNAMICTERRAIN_API FBrushSmooth : public FTerrainBrush
{
public:
	virtual FText GetName() const override;
	virtual TerrainBrushID GetID() const;
	virtual float GetStrength(float Distance, float Radius, float Falloff) const override;
};

// A brush with a smooth, gradually rounded shape
class DYNAMICTERRAIN_API FBrushRound : public FTerrainBrush
{
public:
	virtual FText GetName() const override;
	virtual TerrainBrushID GetID() const;
	virtual float GetStrength(float Distance, float Radius, float Falloff) const override;
};

// A brush with a spherical shape
class DYNAMICTERRAIN_API FBrushSphere : public FTerrainBrush
{
public:
	virtual FText GetName() const override;
	virtual TerrainBrushID GetID() const;
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

/// Terrain Tools ///

class DYNAMICTERRAIN_API FTerrainTool
{
public:
	virtual ~FTerrainTool() {};

	// Retrive the name of the tool for the editor UI
	virtual FText GetName() const = 0;
	// Retrieve the tool's internal name
	virtual FName GetToolID() const = 0;
	// Retrieve the tool's ID value
	virtual TerrainToolID GetID() const = 0;
	
	// Select a brush
	virtual void SetBrush(FTerrainBrush* NewBrush);

	// Apply the tool to a terrain
	virtual void Apply(ATerrain* Terrain, FVector Center, float Delta) const;
	// Apply the tool directly to a heightmap
	virtual void Apply(UHeightMap* Map, FVector2D Center, float Delta) const;

	// Get the location of the mouse cursor on the terrain
	bool MouseToTerrainPosition(ATerrain* Terrain, const FSceneView* View, FHitResult& Result) const;
	bool MouseToTerrainPosition(ATerrain* Terrain, const APlayerController* Controller, FHitResult& Result) const;

	// Convert a world vector to heightmap coordinates
	FVector2D WorldVectorToMapVector(ATerrain* Terrain, FVector WorldPosition) const;

	float Size = 10.0f;				// The radius of the tool circle
	float Strength = 1.0f;			// The strength of the tool
	float Falloff = 5.0f;			// The distance from the center that the strength begins to fall

	bool Invert = false;			// Set to true to invert the effect of the tool

	static float TraceDistance;		// The distance to check for the mouse cursor touching the terrain

protected:
	// Calculate a brush mask using the currently selected brush
	virtual FBrushStroke GetStroke(UHeightMap* Map, FVector2D Center) const = 0;

	FTerrainBrush* Brush = nullptr;	// The currently selected brush
};

// A tool for sculpting the terrain
class DYNAMICTERRAIN_API FSculptTool : public FTerrainTool
{
public:
	virtual FBrushStroke GetStroke(UHeightMap* Map, FVector2D Center) const override;

	virtual FText GetName() const override;
	virtual FName GetToolID() const;
	virtual TerrainToolID GetID() const;

	const static FName ToolID;
};

// A tool for smoothing terrain
class DYNAMICTERRAIN_API FSmoothTool : public FTerrainTool
{
public:
	virtual FBrushStroke GetStroke(UHeightMap* Map, FVector2D Center) const override;

	virtual FText GetName() const override;
	virtual FName GetToolID() const;
	virtual TerrainToolID GetID() const;

	const static FName ToolID;
};

// A tool for flattening terrain
class DYNAMICTERRAIN_API FFlattenTool : public FTerrainTool
{
public:
	virtual FBrushStroke GetStroke(UHeightMap* Map, FVector2D Center) const override;

	virtual FText GetName() const override;
	virtual FName GetToolID() const;
	virtual TerrainToolID GetID() const;

	const static FName ToolID;
};

/// Tool and Brush Sets ///

class DYNAMICTERRAIN_API FToolSet
{
public:
	FToolSet();
	~FToolSet();

	// Set the active tool
	void SetTool(TerrainToolID Tool);
	// Get the active tool
	FTerrainTool* GetTool();
	// Get the active tool's ID
	TerrainToolID ToolID();

	// Set the active brush
	void SetBrush(TerrainBrushID Brush);
	// Get the active brush
	FTerrainBrush* GetBrush();
	// Get the active brush's ID
	TerrainBrushID BrushID();

private:
	TArray<FTerrainTool*> Tools;
	FTerrainTool* ActiveTool;

	TArray<FTerrainBrush*> Brushes;
	FTerrainBrush* ActiveBrush;
};