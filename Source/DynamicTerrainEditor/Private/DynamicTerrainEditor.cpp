#include "DynamicTerrainEditor.h"
#include "DynamicTerrainMode.h"
#include "DynamicTerrainStyle.h"
#include "DynamicTerrainInterface.h"
#include "DynamicTerrainAssets.h"

IMPLEMENT_MODULE(FDynamicTerrainEditorModule, DynamicTerrainEditor);

#define LOCTEXT_NAMESPACE "DynamicTerrainEditorModule"

void FDynamicTerrainEditorModule::StartupModule()
{
	FDynamicTerrainStyle::Initialize();

	// Register the custom class layout for the terrain settings panel
	FPropertyEditorModule& property_editor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	property_editor.RegisterCustomClassLayout("DynamicTerrainSettings", FOnGetDetailCustomizationInstance::CreateStatic(&FDynamicTerrainDetails::CreateInstance));
	property_editor.NotifyCustomizationModuleChanged();

	// Register the foliage asset factory
	IAssetTools& asset_tools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	TerrainAssetCategory = asset_tools.RegisterAdvancedAssetCategory(FName("TerrainAssetCategory"), LOCTEXT("AssetCategoryName", "Terrain"));
	TSharedRef<IAssetTypeActions> action = MakeShareable(new FAssetTypeActions_TerrainFoliageFactory());
	asset_tools.RegisterAssetTypeActions(action);

	// Register the terrain editor mode
	FEditorModeRegistry::Get().RegisterMode<FDynamicTerrainMode>(FDynamicTerrainMode::DynamicTerrainModeID, LOCTEXT("DynamicTerrainModeName", "Terrain Editor"), FSlateIcon(FDynamicTerrainStyle::Get()->GetStyleSetName(), "Plugins.Tab"), true);
}

void FDynamicTerrainEditorModule::ShutdownModule()
{
	FEditorModeRegistry::Get().UnregisterMode(FDynamicTerrainMode::DynamicTerrainModeID);

	// Remove the custom class layout
	FPropertyEditorModule& property_editor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	property_editor.UnregisterCustomClassLayout("DynamicTerrainSettings");

	// Remove new asset types
	/*IAssetTools& asset_tools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	TWeakPtr<IAssetTypeActions> action = asset_tools.GetAssetTypeActionsForClass(UTerrainFoliageGroup::StaticClass());
	if (action.IsValid())
	{
		asset_tools.UnregisterAssetTypeActions(action.Pin().ToSharedRef());
	}*/

	FDynamicTerrainStyle::Shutdown();
}

#undef LOCTEXT_NAMESPACE