#include "DynamicTerrainEditor.h"
#include "DynamicTerrainMode.h"
#include "DynamicTerrainStyle.h"

IMPLEMENT_MODULE(FDynamicTerrainEditorModule, DynamicTerrainEditor);

#define LOCTEXT_NAMESPACE "FDynamicTerrainEditorModule"

void FDynamicTerrainEditorModule::StartupModule()
{
	FDynamicTerrainStyle::Initialize();

	FEditorModeRegistry::Get().RegisterMode<FDynamicTerrainMode>(FDynamicTerrainMode::DynamicTerrainModeID, LOCTEXT("DynamicTerrainModeName", "Terrain Editor"), FSlateIcon(FDynamicTerrainStyle::Get()->GetStyleSetName(), "Plugins.Tab"), true);
}

void FDynamicTerrainEditorModule::ShutdownModule()
{
	FEditorModeRegistry::Get().UnregisterMode(FDynamicTerrainMode::DynamicTerrainModeID);

	FDynamicTerrainStyle::Shutdown();
}

#undef LOCTEXT_NAMESPACE