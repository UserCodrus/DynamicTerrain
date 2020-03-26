#pragma once

#include "DynamicTerrainMode.h"
#include "DynamicTerrainModeToolkit.h"

#include "Framework/Commands/Commands.h"
#include "IDetailCustomization.h"

class SGeneratorBox : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGeneratorBox)
	{}
	SLATE_END_ARGS()

	// Construct the widget
	void Construct(const FArguments& InArgs);
	// Called when widgets are generated for the combobox list
	TSharedRef<SWidget> OptionWidget(TSharedPtr<FTerrainGenerator> Option);
	// Called when a combobox button is selected
	void OptionSelect(TSharedPtr<FTerrainGenerator> SelectOption, ESelectInfo::Type);
};

class FDynamicTerrainEditorCommands : public TCommands<FDynamicTerrainEditorCommands>
{
public:
	FDynamicTerrainEditorCommands();

	// Register the names of the commands
	virtual void RegisterCommands() override;
	// Map commands to toolkit functions
	void MapCommands(FDynamicTerrainModeToolkit* Toolkit) const;

	// UI commands
	TSharedPtr<FUICommandInfo> CreateMode;
	TSharedPtr<FUICommandInfo> ManageMode;
	TSharedPtr<FUICommandInfo> GenerateMode;
	TSharedPtr<FUICommandInfo> SculptMode;

	TSharedPtr<FUICommandInfo> SculptTool;
	TSharedPtr<FUICommandInfo> SmoothTool;
	TSharedPtr<FUICommandInfo> FlattenTool;

	TSharedPtr<FUICommandInfo> LinearBrush;
	TSharedPtr<FUICommandInfo> SmoothBrush;
	TSharedPtr<FUICommandInfo> RoundBrush;
	TSharedPtr<FUICommandInfo> SphereBrush;

private:
	// Map a command to an interface mode
	void MapCommandToMode(FDynamicTerrainModeToolkit* Toolkit, TSharedPtr<FUICommandInfo> Command, TerrainModeID ModeID) const;
	// Map a command to an interface tool
	void MapCommandToTool(FDynamicTerrainModeToolkit* Toolkit, TSharedPtr<FUICommandInfo> Command, TerrainToolID ToolID) const;
	// Map a command to an interface tool
	void MapCommandToBrush(FDynamicTerrainModeToolkit* Toolkit, TSharedPtr<FUICommandInfo> Command, TerrainBrushID BrushID) const;
};

class FDynamicTerrainDetails : public IDetailCustomization
{
public:
	// Create an instance of this detail class
	static TSharedRef<IDetailCustomization> CreateInstance();
	// Called by the engine when details are ready to be customized
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	// Called when the resize button is clicked
	static FReply ResizeButton();
	// Called when the create button is clicked
	static FReply CreateButton();
	// Called when the generate button is clicked
	static FReply GenerateButton();

protected:
	// Get the editor mode using these details
	static FDynamicTerrainMode* GetMode();
	// Update brush settings
	void UpdateBrush();
	// Get the property handle of a terrain generator parameter
	TSharedRef<IPropertyHandle> GetGeneratorParameter(IDetailLayoutBuilder& DetailBuilder, int32 Ref);
};