#pragma once

#include "TerrainTools.h"

#include "CoreMinimal.h"
#include "Components/DecalComponent.h"

#include "TerrainBrushDecal.generated.h"

UCLASS()
class DYNAMICTERRAIN_API UBrushDecal : public UDecalComponent
{
	GENERATED_BODY()

public:
	UBrushDecal();

	virtual void OnComponentCreated() override;

	// Change the size of the cursor
	void Resize(FTerrainTool* Tool, ATerrain* Terrain);
	// Change the color of the cursor
	void ChangeColor(FColor Color);

protected:
	// The material used for the tool brushes
	UPROPERTY(Transient)
		UMaterial* BrushMaterial = nullptr;
	// The material instance for the brush material
	UPROPERTY(VisibleAnywhere)
		UMaterialInstanceDynamic* BrushInstance = nullptr;
};