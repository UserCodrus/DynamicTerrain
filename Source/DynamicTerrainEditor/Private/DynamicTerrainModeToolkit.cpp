#include "DynamicTerrainModeToolkit.h"

#include "DynamicTerrainMode.h"
#include "DynamicTerrainStyle.h"
#include "DynamicTerrainInterface.h"

#include "EditorModeManager.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "TerrainTools"

/// Engine Functions ///

void FDynamicTerrainModeToolkit::Init(const TSharedPtr< class IToolkitHost >& InitToolkitHost)
{
	// Register toolkit commands
	FDynamicTerrainEditorCommands::Register();
	FDynamicTerrainEditorCommands::Get().MapCommands(this);

	// Create the mode buttons
	FToolBarBuilder ModeButtons(GetToolkitCommands(), FMultiBoxCustomization::None);
	ModeButtons.AddToolBarButton(FDynamicTerrainEditorCommands::Get().ManageMode, NAME_None,
		LOCTEXT("ManageModeName", "Manage"),
		LOCTEXT("ManageModeDesc", "Create or resize a terrain object"),
		FSlateIcon(FDynamicTerrainStyle::GetName(), "Plugins.Mode.Manage"));
	ModeButtons.AddToolBarButton(FDynamicTerrainEditorCommands::Get().GenerateMode, NAME_None,
		LOCTEXT("GenerateModeName", "Generate"),
		LOCTEXT("GenerateModeDesc", "Procedurally generate a new terrain"),
		FSlateIcon(FDynamicTerrainStyle::GetName(), "Plugins.Mode.Generate"));
	ModeButtons.AddToolBarButton(FDynamicTerrainEditorCommands::Get().SculptMode, NAME_None,
		LOCTEXT("SculptModeName", "Sculpt"),
		LOCTEXT("SculptModeDesc", "Sculpt and reshape the surface of a terrain"),
		FSlateIcon(FDynamicTerrainStyle::GetName(), "Plugins.Mode.Sculpt"));

	// Create the details panel
	FPropertyEditorModule& property_editor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs details_args(false, false, false, FDetailsViewArgs::HideNameArea);
	DetailsPanel = property_editor.CreateDetailView(details_args);
	//DetailsPanel->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateRaw(this, &FDynamicTerrainModeToolkit::PropertyVisible));

	// Put the editor mode settings in the detail panel
	FDynamicTerrainMode* mode = (FDynamicTerrainMode*)GetEditorMode();
	if (mode)
	{
		DetailsPanel->SetObject(mode->Settings, true);
	}

	// Add the mode buttons and details panel
	SAssignNew(ToolkitWidget, SScrollBox)
		+ SScrollBox::Slot().Padding(5).HAlign(HAlign_Center)
		[
			SNew(SBox)
			[
				ModeButtons.MakeWidget()
			]
		]
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox) +
				SVerticalBox::Slot().Padding(0)
				[
					DetailsPanel.ToSharedRef()
				]
		];

	FModeToolkit::Init(InitToolkitHost);
}

FName FDynamicTerrainModeToolkit::GetToolkitFName() const
{
	return FName("DynamicTerrainToolkit");
}

FText FDynamicTerrainModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("TerrainToolkitName", "Dynamic Terrain Toolkit");
}

FEdMode* FDynamicTerrainModeToolkit::GetEditorMode() const
{
	return GLevelEditorModeTools().GetActiveMode(FDynamicTerrainMode::DynamicTerrainModeID);
}

TSharedPtr<class SWidget> FDynamicTerrainModeToolkit::GetInlineContent() const
{
	return ToolkitWidget;
}

/// Command List Delegates ///

void FDynamicTerrainModeToolkit::ChangeMode(TerrainModeID ModeID)
{
	FDynamicTerrainMode* mode = (FDynamicTerrainMode*)GetEditorMode();
	if (mode != nullptr)
	{
		mode->SetMode(ModeID);
	}
}

bool FDynamicTerrainModeToolkit::IsModeEnabled(TerrainModeID ModeID)
{
	// Modes are always available
	FDynamicTerrainMode* mode = (FDynamicTerrainMode*)GetEditorMode();
	if (mode != nullptr)
	{
		return true;
	}
	return false;
}

bool FDynamicTerrainModeToolkit::IsModeActive(TerrainModeID ModeID)
{
	FDynamicTerrainMode* mode = (FDynamicTerrainMode*)GetEditorMode();
	if (mode != nullptr)
	{
		return mode->GetMode() == ModeID;
	}
	return false;
}

void FDynamicTerrainModeToolkit::ChangeTool(TerrainToolID ToolID)
{
	FDynamicTerrainMode* mode = (FDynamicTerrainMode*)GetEditorMode();
	if (mode != nullptr)
	{
		// Change the tool
		mode->GetTools()->SetTool(ToolID);

		// Refresh the details pane
		mode->ToolUpdate();
		DetailsPanel->ForceRefresh();;
	}
}

bool FDynamicTerrainModeToolkit::IsToolEnabled(TerrainToolID ToolID)
{
	// Tools are always available if their buttons are clickable
	FDynamicTerrainMode* mode = (FDynamicTerrainMode*)GetEditorMode();
	if (mode != nullptr)
	{
		return true;
	}
	return false;
}

bool FDynamicTerrainModeToolkit::IsToolActive(TerrainToolID ToolID)
{
	FDynamicTerrainMode* mode = (FDynamicTerrainMode*)GetEditorMode();
	if (mode != nullptr)
	{
		return mode->GetTools()->ToolID() == ToolID;
	}
	return false;
}

void FDynamicTerrainModeToolkit::ChangeBrush(TerrainBrushID BrushID)
{
	FDynamicTerrainMode* mode = (FDynamicTerrainMode*)GetEditorMode();
	if (mode != nullptr)
	{
		mode->GetTools()->SetBrush(BrushID);
	}
}

bool FDynamicTerrainModeToolkit::IsBrushEnabled(TerrainBrushID BrushID)
{
	// Brushes are always available if their buttons are clickable
	FDynamicTerrainMode* mode = (FDynamicTerrainMode*)GetEditorMode();
	if (mode != nullptr)
	{
		return true;
	}
	return false;
}

bool FDynamicTerrainModeToolkit::IsBrushActive(TerrainBrushID BrushID)
{
	FDynamicTerrainMode* mode = (FDynamicTerrainMode*)GetEditorMode();
	if (mode != nullptr)
	{
		return mode->GetTools()->BrushID() == BrushID;
	}
	return false;
}

bool FDynamicTerrainModeToolkit::PropertyVisible(const FPropertyAndParent& Property) const
{
	return true;
}

#undef LOCTEXT_NAMESPACE