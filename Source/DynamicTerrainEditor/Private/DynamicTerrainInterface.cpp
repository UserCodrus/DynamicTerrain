#include "DynamicTerrainInterface.h"

#include "DynamicTerrainStyle.h"
#include "Tools.h"

#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "TerrainInterface"

/// Editor Commands ///

FDynamicTerrainEditorCommands::FDynamicTerrainEditorCommands() : TCommands<FDynamicTerrainEditorCommands>("DynamicTerrainCommands", NSLOCTEXT("TerrainContexts", "DynamicTerrainEditorMode", "Dynamic Terrain Edit Mode"), NAME_None, FDynamicTerrainStyle::GetName())
{
	// Nothing
}

void FDynamicTerrainEditorCommands::RegisterCommands()
{
	// Initialize each command
	UI_COMMAND(ManageMode, "Manage Mode", "Create and resize terrain objects", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(GenerateMode, "Generate Mode", "Generate new terrain", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(SculptMode, "Sculpt Mode", "Sculpt the terrain", EUserInterfaceActionType::RadioButton, FInputChord());

	UI_COMMAND(SculptTool, "Sculpt Tool", "Raise or lower the terrain", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(SmoothTool, "Smooth Tool", "Smooth bumpy terrain", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(FlattenTool, "Flatten Tool", "Flatten the terrain", EUserInterfaceActionType::RadioButton, FInputChord());

	UI_COMMAND(LinearBrush, "Linear Brush", "Linear falloff", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(SmoothBrush, "Smooth Brush", "Smooth falloff", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(RoundBrush, "Round Brush", "Exponential falloff", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(SphereBrush, "Sphere Brush", "Spherical falloff", EUserInterfaceActionType::RadioButton, FInputChord());
}

void FDynamicTerrainEditorCommands::MapCommands(FDynamicTerrainModeToolkit* Toolkit) const
{
	// Map commands to toolkit callbacks
	MapCommandToMode(Toolkit, ManageMode, TerrainModeID::MANAGE);
	MapCommandToMode(Toolkit, GenerateMode, TerrainModeID::GENERATE);
	MapCommandToMode(Toolkit, SculptMode, TerrainModeID::SCULPT);

	MapCommandToTool(Toolkit, SculptTool, TerrainToolID::SCULPT);
	MapCommandToTool(Toolkit, SmoothTool, TerrainToolID::SMOOTH);
	MapCommandToTool(Toolkit, FlattenTool, TerrainToolID::FLATTEN);

	MapCommandToBrush(Toolkit, LinearBrush, TerrainBrushID::LINEAR);
	MapCommandToBrush(Toolkit, SmoothBrush, TerrainBrushID::SMOOTH);
	MapCommandToBrush(Toolkit, RoundBrush, TerrainBrushID::ROUND);
	MapCommandToBrush(Toolkit, SphereBrush, TerrainBrushID::SPHERE);
}

void FDynamicTerrainEditorCommands::MapCommandToMode(FDynamicTerrainModeToolkit* Toolkit, TSharedPtr<FUICommandInfo> Command, TerrainModeID ModeID) const
{
	TSharedRef<FUICommandList> commands = Toolkit->GetToolkitCommands();

	// Map the registered command to toolkit callbacks
	commands->MapAction(Command,
		FUIAction(
			// Select the mode when the command is executed
			FExecuteAction::CreateRaw(Toolkit, &FDynamicTerrainModeToolkit::ChangeMode, ModeID),
			// Check to see if the mode is available
			FCanExecuteAction::CreateRaw(Toolkit, &FDynamicTerrainModeToolkit::IsModeEnabled, ModeID),
			// Check to see if the mode is already active
			FIsActionChecked::CreateRaw(Toolkit, &FDynamicTerrainModeToolkit::IsModeActive, ModeID))
	);
}

void FDynamicTerrainEditorCommands::MapCommandToTool(FDynamicTerrainModeToolkit* Toolkit, TSharedPtr<FUICommandInfo> Command, TerrainToolID ToolID) const
{
	TSharedRef<FUICommandList> commands = Toolkit->GetToolkitCommands();

	commands->MapAction(Command,
		FUIAction(
			FExecuteAction::CreateRaw(Toolkit, &FDynamicTerrainModeToolkit::ChangeTool, ToolID),
			FCanExecuteAction::CreateRaw(Toolkit, &FDynamicTerrainModeToolkit::IsToolEnabled, ToolID),
			FIsActionChecked::CreateRaw(Toolkit, &FDynamicTerrainModeToolkit::IsToolActive, ToolID))
			);
}

void FDynamicTerrainEditorCommands::MapCommandToBrush(FDynamicTerrainModeToolkit* Toolkit, TSharedPtr<FUICommandInfo> Command, TerrainBrushID BrushID) const
{
	TSharedRef<FUICommandList> commands = Toolkit->GetToolkitCommands();

	commands->MapAction(Command,
		FUIAction(
			FExecuteAction::CreateRaw(Toolkit, &FDynamicTerrainModeToolkit::ChangeBrush, BrushID),
			FCanExecuteAction::CreateRaw(Toolkit, &FDynamicTerrainModeToolkit::IsBrushEnabled, BrushID),
			FIsActionChecked::CreateRaw(Toolkit, &FDynamicTerrainModeToolkit::IsBrushActive, BrushID))
	);
}

/// Details Panel ///

TSharedRef<IDetailCustomization> FDynamicTerrainDetails::CreateInstance()
{
	return MakeShareable(new FDynamicTerrainDetails);
}

void FDynamicTerrainDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	FDynamicTerrainMode* mode = GetMode();

	if (mode != nullptr)
	{
		TSharedPtr<FUICommandList> command_list = mode->GetCommandList();

		///TODO
	}
}

FDynamicTerrainMode* FDynamicTerrainDetails::GetMode()
{
	return (FDynamicTerrainMode*)GLevelEditorModeTools().GetActiveMode(FDynamicTerrainMode::DynamicTerrainModeID);
}

void FDynamicTerrainDetails::UpdateBrush()
{
	FDynamicTerrainMode* mode = GetMode();
	if (mode != nullptr)
	{
		///TODO
	}
}

void FDynamicTerrainDetails::UpdateTerrain()
{
	FDynamicTerrainMode* mode = GetMode();
	if (mode != nullptr)
	{
		///TODO
	}
}

#undef LOCTEXT_NAMESPACE