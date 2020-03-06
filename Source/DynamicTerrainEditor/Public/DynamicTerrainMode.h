#pragma once

#include "Terrain.h"
#include "Tools.h"

#include "EdMode.h"
#include "EditorModeRegistry.h"
#include "DynamicTerrainMode.generated.h"

enum class TerrainModeID
{
	MANAGE,
	GENERATE,
	SCULPT,
	NUM
};

struct FDynamicTerrainToolMode
{
	FDynamicTerrainToolMode(FName Name, TerrainModeID ID) : ModeName(Name), ModeID(ID) {};

	const FName ModeName;
	const TerrainModeID ModeID;
};

UCLASS()
class UDynamicTerrainSettings : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Brush Settings")
		float Size = 10.0f;
	UPROPERTY(EditAnywhere, Category = "Brush Settings")
		float Falloff = 5.0f;
	UPROPERTY(EditAnywhere, Category = "Brush Settings")
		float Strength = 1.0f;
};

class FDynamicTerrainMode : public FEdMode
{
public:
	FDynamicTerrainMode();
	~FDynamicTerrainMode();

	/// Engine Functions ///

	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;

	// Disable delta tracking for a smoother experience when using tools
	virtual bool DisallowMouseDeltaTracking() const override;
	// Handle clicks in the viewport
	virtual bool HandleClick(FEditorViewportClient* ViewportClient, HHitProxy* HitProxy, const FViewportClick& Click) override;
	// Handle key input
	virtual bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;

	// Always returns true because this mode uses toolkits
	bool UsesToolkits() const override;

	/// Command Functions ///

	// Get the command list from the mode toolkit
	TSharedRef<FUICommandList> GetCommandList() const;

	// Get the tools for this mode
	FToolSet* GetToolSet();
	// Get the brushes for this mode
	FBrushSet* GetBrushSet();

	// Get the ID of the current mode
	TerrainModeID GetMode();
	// Get the name of the current mode
	const FName GetModeName();
	// Change the current mode
	void SetMode(TerrainModeID ModeID);

	// The identifier string for this editor mode
	const static FEditorModeID DynamicTerrainModeID;
	// The settings data used to display setting in the editor
	UDynamicTerrainSettings* Settings;

protected:
	// Activates the tool when clicking
	bool UseTool = false;
	// Inverts the tool when shift is held
	bool InvertTool = false;

	// Tool brushes
	FBrushSet Brushes;
	// Terrain tools
	FToolSet Tools;

	// Tool modes
	TArray<FDynamicTerrainToolMode*> Modes;
	// The currently selected mode
	FDynamicTerrainToolMode* CurrentMode = nullptr;

	// The terrain object being edited
	ATerrain* Terrain = nullptr;
};