#include "DynamicTerrainEditor/Public/DynamicTerrainEditor.h"
#include "DynamicTerrainEditor/Public/DynamicTerrainMode.h"

IMPLEMENT_MODULE(FDynamicTerrainEditorModule, DynamicTerrainEditor);

#define LOCTEXT_NAMESPACE "FDynamicTerrainEditorModule"

void FDynamicTerrainEditorModule::StartupModule()
{
	FEditorModeRegistry::Get().RegisterMode<FDynamicTerrainMode>(FDynamicTerrainMode::DynamicTerrainModeID, LOCTEXT("DynamicTerrainModeName", "DynamicTerrain"), FSlateIcon(), true);
}

void FDynamicTerrainEditorModule::ShutdownModule()
{
	FEditorModeRegistry::Get().UnregisterMode(FDynamicTerrainMode::DynamicTerrainModeID);
}

#undef LOCTEXT_NAMESPACE