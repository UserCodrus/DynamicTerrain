#include "DynamicTerrainEditor.h"
#include "DynamicTerrainMode.h"
#include "DynamicTerrainStyle.h"
#include "DynamicTerrainInterface.h"

IMPLEMENT_MODULE(FDynamicTerrainEditorModule, DynamicTerrainEditor);

#define LOCTEXT_NAMESPACE "FDynamicTerrainEditorModule"

void FDynamicTerrainEditorModule::StartupModule()
{
	FDynamicTerrainStyle::Initialize();

	FPropertyEditorModule& property_editor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	property_editor.RegisterCustomClassLayout("DynamicTerrainSettings", FOnGetDetailCustomizationInstance::CreateStatic(&FDynamicTerrainDetails::CreateInstance));
	property_editor.NotifyCustomizationModuleChanged();

	FEditorModeRegistry::Get().RegisterMode<FDynamicTerrainMode>(FDynamicTerrainMode::DynamicTerrainModeID, LOCTEXT("DynamicTerrainModeName", "Terrain Editor"), FSlateIcon(FDynamicTerrainStyle::Get()->GetStyleSetName(), "Plugins.Tab"), true);
}

void FDynamicTerrainEditorModule::ShutdownModule()
{
	FEditorModeRegistry::Get().UnregisterMode(FDynamicTerrainMode::DynamicTerrainModeID);

	FPropertyEditorModule& property_editor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	property_editor.UnregisterCustomClassLayout("DynamicTerrainSettings");

	FDynamicTerrainStyle::Shutdown();
}

#undef LOCTEXT_NAMESPACE