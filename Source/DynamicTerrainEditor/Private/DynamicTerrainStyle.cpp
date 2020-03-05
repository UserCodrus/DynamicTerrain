#include "DynamicTerrainStyle.h"

#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr<FSlateStyleSet> FDynamicTerrainStyle::StyleSet = nullptr;

void FDynamicTerrainStyle::Initialize()
{
	// Initialize only once
	if (StyleSet.IsValid())
	{
		return;
	}

	// Create the style set
	StyleSet = MakeShareable(new FSlateStyleSet("DynamicTerrainStyle"));
	StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	// Set the style icons
	const FString content = IPluginManager::Get().FindPlugin(TEXT("DynamicTerrain"))->GetContentDir() / "Icons";
	const FVector2D size(40.0f, 40.0f);

	StyleSet->Set("Plugins.Tab", new FSlateImageBrush(content / "icon_Test.png", size));

	StyleSet->Set("Plugins.Mode.Manage", new FSlateImageBrush(content / "icon_Test.png", size));
	StyleSet->Set("Plugins.Mode.Generate", new FSlateImageBrush(content / "icon_Test.png", size));
	StyleSet->Set("Plugins.Mode.Sculpt", new FSlateImageBrush(content / "icon_Test.png", size));

	StyleSet->Set("Plugins.Tool.Sculpt", new FSlateImageBrush(content / "icon_Test.png", size));
	StyleSet->Set("Plugins.Tool.Smooth", new FSlateImageBrush(content / "icon_Test.png", size));
	StyleSet->Set("Plugins.Tool.Flatten", new FSlateImageBrush(content / "icon_Test.png", size));

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
}

void FDynamicTerrainStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}

TSharedPtr<ISlateStyle> FDynamicTerrainStyle::Get()
{
	return StyleSet;
}

const FName FDynamicTerrainStyle::GetName()
{
	return StyleSet->GetStyleSetName();
}