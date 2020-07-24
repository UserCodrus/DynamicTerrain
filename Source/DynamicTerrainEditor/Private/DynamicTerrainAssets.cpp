#include "DynamicTerrainAssets.h"

#include "TerrainFoliage.h"

#define LOCTEXT_NAMESPACE "DynamicTerrainAssets"

/// UTerrainFoliageFactory ///

UTerrainFoliageFactory::UTerrainFoliageFactory(const FObjectInitializer& ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UTerrainFoliageGroup::StaticClass();
}

UObject* UTerrainFoliageFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UTerrainFoliageGroup>(InParent, InClass, InName, Flags);
}

/// FAssetTypeActions_TerrainFoliageFactory ///

FText FAssetTypeActions_TerrainFoliageFactory::GetName() const
{
	return LOCTEXT("FoliageAssetName", "Dynamic Terrain Foliage");
}

FColor FAssetTypeActions_TerrainFoliageFactory::GetTypeColor() const
{
	return FColor::Green;
}

UClass* FAssetTypeActions_TerrainFoliageFactory::GetSupportedClass() const
{
	return UTerrainFoliageGroup::StaticClass();
}

uint32 FAssetTypeActions_TerrainFoliageFactory::GetCategories()
{
	return TerrainAssetCategory;
}

#undef LOCTEXT_NAMESPACE