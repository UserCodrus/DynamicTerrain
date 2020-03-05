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
	// Initialize each command and map it to its identifier
	UI_COMMAND(ManageMode, "Manage Mode", "Create and resize terrain objects", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(GenerateMode, "Generate Mode", "Generate new terrain", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(SculptMode, "Sculpt Mode", "Sculpt the terrain", EUserInterfaceActionType::RadioButton, FInputChord());

	UI_COMMAND(SculptTool, "Sculpt Tool", "Raise or lower the terrain", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(SmoothTool, "Smooth Tool", "Smooth bumpy terrain", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(FlattenTool, "Flatten Tool", "Flatten the terrain", EUserInterfaceActionType::RadioButton, FInputChord());
}

void FDynamicTerrainEditorCommands::MapCommands(FDynamicTerrainModeToolkit* Toolkit) const
{
	MapCommandToMode(Toolkit, ManageMode, TerrainModeID::MANAGE);
	MapCommandToMode(Toolkit, GenerateMode, TerrainModeID::GENERATE);
	MapCommandToMode(Toolkit, SculptMode, TerrainModeID::SCULPT);

	MapCommandToTool(Toolkit, SculptTool, TerrainToolID::SCULPT);
	MapCommandToTool(Toolkit, SmoothTool, TerrainToolID::SMOOTH);
	MapCommandToTool(Toolkit, FlattenTool, TerrainToolID::FLATTEN);
}

void FDynamicTerrainEditorCommands::MapCommandToMode(FDynamicTerrainModeToolkit* Toolkit, TSharedPtr<FUICommandInfo> Command, TerrainModeID ModeID) const
{
	TSharedRef<FUICommandList> commands = Toolkit->GetToolkitCommands();

	// Map the registered command to toolkit callbacks
	commands->MapAction(Command,
		FUIAction(
			// Select the mode when the command is executed
			FExecuteAction::CreateRaw(Toolkit, &FDynamicTerrainModeToolkit::ChangeMode, ModeID),
			// Check to see if the modeis available
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