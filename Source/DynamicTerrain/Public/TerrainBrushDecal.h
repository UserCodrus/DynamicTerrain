#pragma once

#include "TerrainTools.h"

#include "CoreMinimal.h"
#include "Components/DecalComponent.h"

#include "TerrainBrushDecal.generated.h"

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DYNAMICTERRAIN_API UBrushDecal : public UDecalComponent
{
	GENERATED_BODY()

public:
	UBrushDecal();

	virtual void BeginPlay() override;

	// Change the size of the cursor
	void Resize(FTerrainTool* Tool, ATerrain* Terrain);
	// Change the color of the cursor
	void ChangeColor(FColor Color);
	// Create a material instance for the decal
	void CreateMaterialInstance();

protected:
	// The default material used for the tool brushes
	UPROPERTY(Transient)
		UMaterial* BrushMaterial;
	// The material instance for the brush material
	UPROPERTY(Transient)
		UMaterialInstanceDynamic* BrushInstance;
};