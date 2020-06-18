#pragma once

#include "Terrain.h"
#include "TerrainTools.h"
#include "TerrainBrushDecal.h"

#include "EdMode.h"
#include "EditorModeRegistry.h"
#include "DynamicTerrainMode.generated.h"

enum class TerrainModeID
{
	CREATE,
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

struct FTerrainGenerator
{
	FTerrainGenerator(FName GennyName)
	{
		Name = GennyName;
	}

	FName Name;
	TArray<FString> Parameters;
	TArray<bool> IsFloat;
};

UCLASS()
class ABrushProxy : public AActor
{
	GENERATED_BODY()

public:
	ABrushProxy();

	virtual bool IsSelectable() const override;

	// Initialize the decal materials
	void Initialize();
	// Show or hide the brush decal
	void ShowBrush(bool Visible);
	// Set the location and size of the brush decal
	void SetBrush(FVector Location, FTerrainTool* Tool, ATerrain* Terrain);

protected:
	UPROPERTY()
		UBrushDecal* Decal = nullptr;
};

constexpr unsigned NUM_PROPERTIES = 10;

UCLASS()
class UDynamicTerrainSettings : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Terrain Settings")
		int32 ComponentSize = 6;
	UPROPERTY(EditAnywhere, Category = "Terrain Settings")
		int32 WidthX = 3;
	UPROPERTY(EditAnywhere, Category = "Terrain Settings")
		int32 WidthY = 3;
	UPROPERTY(EditAnywhere, Category = "Terrain Settings")
		float UVTiling = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Brush Settings")
		float Size = 10.0f;
	UPROPERTY(EditAnywhere, Category = "Brush Settings")
		float Falloff = 5.0f;
	UPROPERTY(EditAnywhere, Category = "Brush Settings")
		float Strength = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Generator")
		FString FunctionName;
	UPROPERTY(EditAnywhere, Category = "Generator")
		int32 IntProperties[NUM_PROPERTIES];
	UPROPERTY(EditAnywhere, Category = "Generator")
		float FloatProperties[NUM_PROPERTIES];
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

	// Draw lines around terrain
	virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;

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
	FToolSet* GetTools();
	// Get the selected terrain
	ATerrain* GetSelected();

	// Get the ID of the current mode
	TerrainModeID GetMode();
	// Get the name of the current mode
	const FName GetModeName();
	// Change the current mode
	void SetMode(TerrainModeID ModeID);

	// Update the tool settings when the toolkit changes a tool
	void ToolUpdate();
	// Update the terrain settings when the mode changes
	void ModeUpdate();
	// Rebuild the terrain to match the chosen settings
	void ResizeTerrain();
	// Create a new terrain
	void CreateTerrain();
	// Select a terrain object
	void SelectTerrain(ATerrain* Terrain);

	// Process a generator command
	void ProcessGenerateCommand();
	// Select a different generator
	void SelectGenerator(TSharedPtr<FTerrainGenerator> Generator);
	// Get the currently selected generator
	TSharedPtr<FTerrainGenerator> GetGenerator();

	// The identifier string for this editor mode
	const static FEditorModeID DynamicTerrainModeID;
	// The settings data used to display settings in the editor via detail customization
	UDynamicTerrainSettings* Settings;
	// Terrain generator data for creating menus and processing commands
	TArray<TSharedPtr<FTerrainGenerator>> Generators;

protected:
	// Set to true when clicking the left mouse button
	bool MouseClick = false;
	// Inverts the tool when shift is held
	bool InvertTool = false;

	// Terrain tools for sculpting terrain
	FToolSet Tools;
	// The map generator object used to generate heightmaps
	UMapGenerator* MapGen;

	// Tool modes
	TArray<FDynamicTerrainToolMode*> Modes;
	// The currently selected mode
	FDynamicTerrainToolMode* CurrentMode = nullptr;

	// The brush decal used to display the range of sculpting brushes
	ABrushProxy* Brush = nullptr;

	// The terrain object being edited
	ATerrain* SelectedTerrain = nullptr;
	// The name of the last terrain selected
	FString TerrainName;

	// The current generator to use when the generate button is clicked in the editor
	TSharedPtr<FTerrainGenerator> CurrentGenerator;
};