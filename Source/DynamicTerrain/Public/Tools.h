#pragma once

#include "CoreMinimal.h"
#include "Terrain.h"

class DYNAMICTERRAIN_API FTerrainTool
{
public:
	virtual void Use(UHeightMap* Map, FVector2D Center, float Delta) = 0;

	virtual void Activate() = 0;
	virtual void Deactivate() = 0;

	virtual void Tick(float DeltaTime) = 0;

	virtual FText GetName() = 0;

	float Size = 10.0f;
	float Strength = 0.01f;
	float Falloff = 0.9f;
	float Hardness = 0.1f;

protected:
	virtual float GetMagnitude(float Distance);

	ATerrain* Target = nullptr;
};

class DYNAMICTERRAIN_API FRaiseTool : public FTerrainTool
{
public:
	virtual void Use(UHeightMap* Map, FVector2D Center, float Delta) override;

	virtual void Activate() override;
	virtual void Deactivate() override;

	virtual void Tick(float DeltaTime) override;

	virtual FText GetName() override;
};

