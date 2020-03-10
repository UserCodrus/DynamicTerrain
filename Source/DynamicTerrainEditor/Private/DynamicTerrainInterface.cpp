#include "DynamicTerrainInterface.h"

#include "DynamicTerrainStyle.h"
#include "Tools.h"

#include "Editor.h"
#include "EditorModeManager.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/SButton.h"

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
		TerrainModeID current_mode = mode->GetMode();
		
		if (current_mode == TerrainModeID::MANAGE)
		{
			// Add a button to the manager interface
			IDetailCategoryBuilder& category_manage = DetailBuilder.EditCategory("Terrain Settings", FText::GetEmpty(), ECategoryPriority::Important);
			category_manage.AddCustomRow(FText::GetEmpty())
				[
					SNew(SButton).Text(LOCTEXT("ChangeTerrainButton", "Change")).HAlign(HAlign_Center)
				];

			// Hide categories
			DetailBuilder.EditCategory("Brush Settings", FText::GetEmpty(), ECategoryPriority::Important).SetCategoryVisibility(false);
		}
		else if (current_mode == TerrainModeID::SCULPT)
		{
			// Create the tool selection widget
			IDetailCategoryBuilder& category_tools = DetailBuilder.EditCategory("Tools", FText::GetEmpty(), ECategoryPriority::Important);

			FToolBarBuilder ToolButtons(command_list, FMultiBoxCustomization::None);
			ToolButtons.AddToolBarButton(FDynamicTerrainEditorCommands::Get().SculptTool, NAME_None,
				LOCTEXT("SculptToolName", "Sculpt"),
				LOCTEXT("SculptToolDesc", "Shape terrain by raising or lowering it"),
				FSlateIcon(FDynamicTerrainStyle::GetName(), "Plugins.Tool.Sculpt"));
			ToolButtons.AddToolBarButton(FDynamicTerrainEditorCommands::Get().SmoothTool, NAME_None,
				LOCTEXT("SmoothToolName", "Smooth"),
				LOCTEXT("SmoothToolDesc", "Smooth out bumpy terrain"),
				FSlateIcon(FDynamicTerrainStyle::GetName(), "Plugins.Tool.Smooth"));
			ToolButtons.AddToolBarButton(FDynamicTerrainEditorCommands::Get().FlattenTool, NAME_None,
				LOCTEXT("FlattenToolName", "Flatten"),
				LOCTEXT("FlattenToolDesc", "Level the terrain around the cursor"),
				FSlateIcon(FDynamicTerrainStyle::GetName(), "Plugins.Tool.Flatten"));

			category_tools.AddCustomRow(FText::GetEmpty())
				[
					SNew(SBox).Padding(5).HAlign(HAlign_Center)
					[
						ToolButtons.MakeWidget()
					]
				];

			// Create the brush widget
			IDetailCategoryBuilder& category_brushes = DetailBuilder.EditCategory("Brushes", FText::GetEmpty(), ECategoryPriority::Important);

			FToolBarBuilder BrushButtons(command_list, FMultiBoxCustomization::None);
			BrushButtons.AddToolBarButton(FDynamicTerrainEditorCommands::Get().LinearBrush, NAME_None,
				LOCTEXT("LinearBrushName", "Linear"),
				LOCTEXT("LinearBrushDesc", "Linear falloff"),
				FSlateIcon(FDynamicTerrainStyle::GetName(), "Plugins.Brush.Linear"));
			BrushButtons.AddToolBarButton(FDynamicTerrainEditorCommands::Get().SmoothBrush, NAME_None,
				LOCTEXT("SmoothBrushName", "Smooth"),
				LOCTEXT("SmoothBrushDesc", "Smooth falloff"),
				FSlateIcon(FDynamicTerrainStyle::GetName(), "Plugins.Brush.Smooth"));
			BrushButtons.AddToolBarButton(FDynamicTerrainEditorCommands::Get().RoundBrush, NAME_None,
				LOCTEXT("RoundBrushName", "Round"),
				LOCTEXT("RoundBrushDesc", "Round falloff"),
				FSlateIcon(FDynamicTerrainStyle::GetName(), "Plugins.Brush.Round"));
			BrushButtons.AddToolBarButton(FDynamicTerrainEditorCommands::Get().SphereBrush, NAME_None,
				LOCTEXT("SphereBrushName", "Sphere"),
				LOCTEXT("SphereBrushDesc", "Sphere falloff"),
				FSlateIcon(FDynamicTerrainStyle::GetName(), "Plugins.Brush.Sphere"));

			category_brushes.AddCustomRow(FText::GetEmpty())
				[
					SNew(SBox).Padding(5).HAlign(HAlign_Center)
					[
						BrushButtons.MakeWidget()
					]
				];

			// Hide categories
			//DetailBuilder.EditCategory("Brush Settings", FText::GetEmpty(), ECategoryPriority::Important).SetCategoryVisibility(true);
			DetailBuilder.EditCategory("Terrain Settings", FText::GetEmpty(), ECategoryPriority::Important).SetCategoryVisibility(false);
		}

		///TODO

		DetailBuilder.GetProperty("Strength")->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FDynamicTerrainDetails::UpdateBrush));
		DetailBuilder.GetProperty("Size")->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FDynamicTerrainDetails::UpdateBrush));
		DetailBuilder.GetProperty("Falloff")->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FDynamicTerrainDetails::UpdateBrush));
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
		FTerrainTool* tool = mode->GetTools()->GetTool();
		tool->Strength = mode->Settings->Strength;
		tool->Size = mode->Settings->Size;
		tool->Falloff = mode->Settings->Falloff;
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