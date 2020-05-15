#pragma once

#include "TerrainTools.h"
#include "TerrainBrushDecal.h"

#include "Components/DecalComponent.h"

#include "TerrainToolComponent.generated.h"

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DYNAMICTERRAIN_API  UTerrainToolComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UTerrainToolComponent();

	/// USceneComponent Interface ///

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/// Terrain Tool Component Interface ///

	// Change the current tool
	UFUNCTION(BlueprintCallable)
		void SelectTool(TerrainToolID ToolID);
	// Change the current brush
	UFUNCTION(BlueprintCallable)
		void SelectBrush(TerrainBrushID BrushID);

	// Switch to the next tool in the toolkit
	UFUNCTION(BlueprintCallable)
		void NextTool();
	// Switch to the previous brush in the toolkit
	UFUNCTION(BlueprintCallable)
		void PreviousTool();
	// Switch to the next brush in the toolkit
	UFUNCTION(BlueprintCallable)
		void NextBrush();
	// Switch to the previous brush in the toolkit
	UFUNCTION(BlueprintCallable)
		void PreviousBrush();

	// Get the name of the currently selected tool
	UFUNCTION(BlueprintPure)
		FText GetToolName();
	// Get the name of the currently selected brush
	UFUNCTION(BlueprintPure)
		FText GetBrushName();
	// Get the brush decal
	UFUNCTION(BlueprintPure)
		UBrushDecal* GetBrushDecal();

	// Set to true when the current tool is in use
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Status")
		bool ToolActive = false;
	// Set to true to invert the terrain tool
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Status")
		bool ToolInvert = false;
	// Set to true to enable the tool and show the brush
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Status")
		bool ToolEnabled = true;

	// The maximum distance that the brush will be usable from
	UPROPERTY(EditAnywhere, Category = "Settings")
		float MaxBrushDistance = 50000.0f;
	// If set to true the brush cursor will disappear when it isn't over a valid terrain
	// If false the brush color will change when its position is invalid
	UPROPERTY(EditAnywhere, Category = "Settings")
		bool HideInvalidBrush = false;
	// The color of the brush when it is usable
	UPROPERTY(EditAnywhere, Category = "Settings")
		FColor BrushColor;
	// The color of the brush when it isn't touching a valid terrain
	UPROPERTY(EditAnywhere, Category = "Settings")
		FColor BrushInvalidColor;

protected:
	// The decal component that shows the terrain tool's brush
	UPROPERTY(EditAnywhere, Category = "Brush")
		UBrushDecal* BrushDecal;

	// The tools used to manipulate terrain
	FToolSet TerrainTools;
};