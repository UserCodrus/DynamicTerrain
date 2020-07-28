#include "DynamicTerrainInterface.h"

#include "DynamicTerrainStyle.h"
#include "TerrainTools.h"

#include "Editor.h"
#include "EditorModeManager.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/SButton.h"
#include "PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "TerrainInterface"

/// Generator Widget ///

void SGeneratorBox::Construct(const FArguments& InArgs)
{
	// Create the combobox widget
	FDynamicTerrainMode* mode = (FDynamicTerrainMode*)GLevelEditorModeTools().GetActiveMode(FDynamicTerrainMode::DynamicTerrainModeID);
	ChildSlot
		[
			SNew(SComboBox<TSharedPtr<FTerrainGenerator>>)
			.OptionsSource(&mode->Generators)
			.OnGenerateWidget(this, &SGeneratorBox::OptionWidget)
			.OnSelectionChanged(this, &SGeneratorBox::OptionSelect)
			.InitiallySelectedItem(mode->GetGenerator())
				[
					SNew(STextBlock).Text(FText::FromName(mode->GetGenerator()->Name))
				]
		];
}

TSharedRef<SWidget> SGeneratorBox::OptionWidget(TSharedPtr<FTerrainGenerator> Option)
{
	return SNew(STextBlock).Text(FText::FromName((*Option).Name));
}

void SGeneratorBox::OptionSelect(TSharedPtr<FTerrainGenerator> SelectOption, ESelectInfo::Type)
{
	((FDynamicTerrainMode*)GLevelEditorModeTools().GetActiveMode(FDynamicTerrainMode::DynamicTerrainModeID))->SelectGenerator(SelectOption);
}

/// Editor Commands ///

FDynamicTerrainEditorCommands::FDynamicTerrainEditorCommands() : TCommands<FDynamicTerrainEditorCommands>("DynamicTerrainCommands", NSLOCTEXT("TerrainContexts", "DynamicTerrainEditorMode", "Dynamic Terrain Edit Mode"), NAME_None, FDynamicTerrainStyle::GetName())
{
	// Nothing
}

void FDynamicTerrainEditorCommands::RegisterCommands()
{
	// Initialize each command
	UI_COMMAND(CreateMode, "Create Mode", "Create terrain objects", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(ManageMode, "Manage Mode", "Select and resize terrain objects", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(GenerateMode, "Generate Mode", "Generate new terrain", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(SculptMode, "Sculpt Mode", "Sculpt the terrain", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(FoliageMode, "Foliage Mode", "Change terrain foliage", EUserInterfaceActionType::RadioButton, FInputChord());

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
	MapCommandToMode(Toolkit, CreateMode, TerrainModeID::CREATE);
	MapCommandToMode(Toolkit, ManageMode, TerrainModeID::MANAGE);
	MapCommandToMode(Toolkit, GenerateMode, TerrainModeID::GENERATE);
	MapCommandToMode(Toolkit, SculptMode, TerrainModeID::SCULPT);
	MapCommandToMode(Toolkit, FoliageMode, TerrainModeID::FOLIAGE);

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

		DetailBuilder.EditCategory("Generator", FText::GetEmpty(), ECategoryPriority::Important).SetCategoryVisibility(false);
		
		if (current_mode == TerrainModeID::MANAGE)
		{
			if (mode->GetSelected() != nullptr)
			{
				// Add a button to edit terrain
				IDetailCategoryBuilder& category_manage = DetailBuilder.EditCategory("Terrain Settings", FText::GetEmpty(), ECategoryPriority::Default);
				category_manage.AddCustomRow(FText::GetEmpty())
					[
						SNew(SButton).Text(LOCTEXT("ChangeTerrainButton", "Change")).OnClicked_Static(&FDynamicTerrainDetails::ResizeButton)
					];
			}
			else
			{
				// Add a text box with instructions for selecting terrain if none are selected
				IDetailCategoryBuilder& category_text = DetailBuilder.EditCategory("Select", FText::GetEmpty(), ECategoryPriority::Default);
				category_text.AddCustomRow(FText::GetEmpty())
					[
						SNew(STextBlock).Text(LOCTEXT("NoSelectionManage", "Click on a terrain to select it, or click 'Create' to create a new terrain if none exist"))
					];

				DetailBuilder.EditCategory("Terrain Settings", FText::GetEmpty(), ECategoryPriority::Important).SetCategoryVisibility(false);
			}

			// Hide categories
			DetailBuilder.EditCategory("Foliage Settings", FText::GetEmpty(), ECategoryPriority::Important).SetCategoryVisibility(false);
			DetailBuilder.EditCategory("Brush Settings", FText::GetEmpty(), ECategoryPriority::Important).SetCategoryVisibility(false);
		}
		else if (current_mode == TerrainModeID::CREATE)
		{
			// Add a button to create a new terrain
			IDetailCategoryBuilder& category_create = DetailBuilder.EditCategory("Terrain Settings", FText::GetEmpty(), ECategoryPriority::Default);
			category_create.AddCustomRow(FText::GetEmpty())
				[
					SNew(SButton).Text(LOCTEXT("NewTerrainButton", "Create")).OnClicked_Static(&FDynamicTerrainDetails::CreateButton)
				];

			// Hide categories
			DetailBuilder.EditCategory("Foliage Settings", FText::GetEmpty(), ECategoryPriority::Important).SetCategoryVisibility(false);
			DetailBuilder.EditCategory("Brush Settings", FText::GetEmpty(), ECategoryPriority::Important).SetCategoryVisibility(false);
		}
		else if (current_mode == TerrainModeID::GENERATE)
		{
			IDetailCategoryBuilder& category_generate = DetailBuilder.EditCategory("Terrain Generator", FText::GetEmpty(), ECategoryPriority::Default);

			// Generator selection combo box
			category_generate.AddCustomRow(FText::GetEmpty())
				[
					SNew(SGeneratorBox)
				];

			// Display parameters for the current generator
			FTerrainGenerator* generator = mode->GetGenerator().Get();
			for (int32 i = 0; i < generator->Parameters.Num(); ++i)
			{
				// Create the name of the property
				FString param_name;
				if (generator->IsFloat[i])
				{
					param_name = "FloatProperties[";
				}
				else
				{
					param_name = "IntProperties[";
				}
				param_name.AppendInt(i);
				param_name.Append("]");

				// Add the property with the matching name
				category_generate.AddCustomRow(FText::GetEmpty())
					[
						SNew(SProperty, DetailBuilder.GetProperty(*param_name)).DisplayName(FText::FromString(generator->Parameters[i]))
					];
			}

			category_generate.AddCustomRow(FText::GetEmpty())
				[
					SNew(SButton).Text(LOCTEXT("GenerateButton", "Generate")).OnClicked_Static(&FDynamicTerrainDetails::GenerateButton)
				];

			// Hide categories
			DetailBuilder.EditCategory("Foliage Settings", FText::GetEmpty(), ECategoryPriority::Important).SetCategoryVisibility(false);
			DetailBuilder.EditCategory("Brush Settings", FText::GetEmpty(), ECategoryPriority::Important).SetCategoryVisibility(false);
			DetailBuilder.EditCategory("Terrain Settings", FText::GetEmpty(), ECategoryPriority::Important).SetCategoryVisibility(false);
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
			DetailBuilder.EditCategory("Foliage Settings", FText::GetEmpty(), ECategoryPriority::Important).SetCategoryVisibility(false);
			DetailBuilder.EditCategory("Terrain Settings", FText::GetEmpty(), ECategoryPriority::Important).SetCategoryVisibility(false);
		}
		else if (current_mode == TerrainModeID::FOLIAGE)
		{
			// Add the foliage widget
			IDetailCategoryBuilder& category_foliage = DetailBuilder.EditCategory("Foliage Settings", FText::GetEmpty(), ECategoryPriority::Default);
			category_foliage.AddCustomRow(FText::GetEmpty())
				[
					SNew(SButton).Text(LOCTEXT("ChangeFoliageButton", "Change")).OnClicked_Static(&FDynamicTerrainDetails::FoliageButton)
				];

			// Hide categories
			DetailBuilder.EditCategory("Brush Settings", FText::GetEmpty(), ECategoryPriority::Important).SetCategoryVisibility(false);
			DetailBuilder.EditCategory("Terrain Settings", FText::GetEmpty(), ECategoryPriority::Important).SetCategoryVisibility(false);
		}

		DetailBuilder.GetProperty("Strength")->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FDynamicTerrainDetails::UpdateBrush));
		DetailBuilder.GetProperty("Size")->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FDynamicTerrainDetails::UpdateBrush));
		DetailBuilder.GetProperty("Falloff")->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FDynamicTerrainDetails::UpdateBrush));
	}
}

FReply FDynamicTerrainDetails::ResizeButton()
{
	GetMode()->ResizeTerrain();
	return FReply::Handled();
}

FReply FDynamicTerrainDetails::CreateButton()
{
	GetMode()->CreateTerrain();
	return FReply::Handled();
}

FReply FDynamicTerrainDetails::GenerateButton()
{
	GetMode()->ProcessGenerateCommand();
	return FReply::Handled();
}

FReply FDynamicTerrainDetails::FoliageButton()
{
	GetMode()->ChangeFoliage();
	return FReply::Handled();
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

TSharedRef<IPropertyHandle> FDynamicTerrainDetails::GetGeneratorParameter(IDetailLayoutBuilder& DetailBuilder, int32 Ref)
{
	return DetailBuilder.GetProperty("asdf", nullptr, "asf");
}

#undef LOCTEXT_NAMESPACE