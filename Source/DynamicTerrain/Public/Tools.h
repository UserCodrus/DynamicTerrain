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
	// Retrive the name of the tool for the editor UI
	virtual FText GetName() const = 0;

	// Retrieve the tool's internal name
	virtual FName GetToolID() const = 0;

	// Retrieve the tool's ID value
	virtual TerrainToolID GetID() const = 0;

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
	float Strength = 1.0f;			// The strength of the tool
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
	virtual FName GetToolID() const;
	virtual TerrainToolID GetID() const;

	const static FName ToolID;
};

// A tool for smoothing terrain
class DYNAMICTERRAIN_API FSmoothTool : public FTerrainTool
{
public:
	virtual FBrushStroke GetStroke(FVector2D Center) const override;

	virtual FText GetName() const override;
	virtual FName GetToolID() const;
	virtual TerrainToolID GetID() const;

	const static FName ToolID;
};

// A tool for flattening terrain
class DYNAMICTERRAIN_API FFlattenTool : public FTerrainTool
{
public:
	virtual FBrushStroke GetStroke(FVector2D Center) const override;

	virtual FText GetName() const override;
	virtual FName GetToolID() const;
	virtual TerrainToolID GetID() const;

	const static FName ToolID;
};

/// Tool and Brush Sets ///

class DYNAMICTERRAIN_API FToolSet
{
public:
	FToolSet()
	{
		// Create tools
		Tools.SetNum((int)TerrainToolID::NUM);
		Tools[(int)TerrainToolID::SCULPT] = new FSculptTool;
		Tools[(int)TerrainToolID::SMOOTH] = new FSmoothTool;
		Tools[(int)TerrainToolID::FLATTEN] = new FFlattenTool;

		ActiveTool = Tools[0];
	}
	~FToolSet()
	{
		for (int32 i = 0; i < Tools.Num(); ++i)
		{
			delete Tools[i];
		}
		Tools.Empty();
	}

	// Set the active tool
	void Set(TerrainToolID Tool)
	{
		if (Tool != TerrainToolID::NUM)
		{
			ActiveTool = Tools[(int)Tool];
		}
	}
	// Get the active tool
	FTerrainTool* Get()
	{
		return ActiveTool;
	}
	// Get the active tool's ID
	TerrainToolID ID()
	{
		return ActiveTool->GetID();
	}

private:
	TArray<FTerrainTool*> Tools;
	FTerrainTool* ActiveTool;
};

class DYNAMICTERRAIN_API FBrushSet
{
public:
	FBrushSet()
	{
		// Create brushes
		Brushes.SetNum((int)TerrainBrushID::NUM);
		Brushes[(int)TerrainBrushID::LINEAR] = new FBrushLinear;
		Brushes[(int)TerrainBrushID::SMOOTH] = new FBrushSmooth;
		Brushes[(int)TerrainBrushID::ROUND] = new FBrushRound;
		Brushes[(int)TerrainBrushID::SPHERE] = new FBrushSphere;

		ActiveBrush = Brushes[0];
	}
	~FBrushSet()
	{
		for (int32 i = 0; i < Brushes.Num(); ++i)
		{
			delete Brushes[i];
		}
		Brushes.Empty();
	}

	// Set the active brush
	void Set(TerrainBrushID Brush)
	{
		if (Brush != TerrainBrushID::NUM)
		{
			ActiveBrush = Brushes[(int)Brush];
		}
	}
	// Get the active brush
	FTerrainBrush* Get()
	{
		return ActiveBrush;
	}
	// Get the active brush's ID
	TerrainBrushID ID()
	{
		return ActiveBrush->GetID();
	}

private:
	TArray<FTerrainBrush*> Brushes;
	FTerrainBrush* ActiveBrush;
};