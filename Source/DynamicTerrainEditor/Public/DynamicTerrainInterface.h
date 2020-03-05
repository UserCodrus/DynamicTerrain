#pragma once

#include "DynamicTerrainMode.h"
#include "DynamicTerrainModeToolkit.h"

#include "Framework/Commands/Commands.h"
#include "IDetailCustomization.h"

class FDynamicTerrainEditorCommands : public TCommands<FDynamicTerrainEditorCommands>
{
public:
	FDynamicTerrainEditorCommands();

	// Register the names of the commands
	virtual void RegisterCommands() override;
	// Map commands to toolkit functions
	void MapCommands(FDynamicTerrainModeToolkit* Toolkit) const;

	// UI commands
	TSharedPtr<FUICommandInfo> ManageMode;
	TSharedPtr<FUICommandInfo> GenerateMode;
	TSharedPtr<FUICommandInfo> SculptMode;

	TSharedPtr<FUICommandInfo> SculptTool;
	TSharedPtr<FUICommandInfo> SmoothTool;
	TSharedPtr<FUICommandInfo> FlattenTool;

private:
	// Map a command to an interface mode
	void MapCommandToMode(FDynamicTerrainModeToolkit* Toolkit, TSharedPtr<FUICommandInfo> Command, TerrainModeID ModeID) const;
	// Map a command to an interface tool
	void MapCommandToTool(FDynamicTerrainModeToolkit* Toolkit, TSharedPtr<FUICommandInfo> Command, TerrainToolID ToolID) const;
};

class FDynamicTerrainDetails : public IDetailCustomization
{
public:
	// Create an instance of this detail class
	static TSharedRef<IDetailCustomization> CreateInstance();
	// Called by the engine when details are ready to be customized
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

protected:
	// Get the editor mode using these details
	FDynamicTerrainMode* GetMode();
	// Update brush settings
	void UpdateBrush();
	// Update terrain settings
	void UpdateTerrain();
};