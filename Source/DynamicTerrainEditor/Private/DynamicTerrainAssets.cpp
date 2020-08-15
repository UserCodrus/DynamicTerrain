#include "DynamicTerrainAssets.h"

#include "TerrainFoliage.h"

#define LOCTEXT_NAMESPACE "DynamicTerrainAssets"

/// UTerrainFoliageSpawnerFactory ///

UTerrainFoliageSpawnerFactory::UTerrainFoliageSpawnerFactory(const FObjectInitializer& ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UTerrainFoliageSpawner::StaticClass();
}

UObject* UTerrainFoliageSpawnerFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UTerrainFoliageSpawner>(InParent, InClass, InName, Flags);
}

/// UTerrainFoliageFactory ///

UTerrainFoliageFactory::UTerrainFoliageFactory(const FObjectInitializer& ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UTerrainFoliage::StaticClass();
}

UObject* UTerrainFoliageFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UTerrainFoliage>(InParent, InClass, InName, Flags);
}

/// FAssetTypeActions_TerrainFoliageSpawnerFactory ///

FText FAssetTypeActions_TerrainFoliageSpawnerFactory::GetName() const
{
	return LOCTEXT("DynamicTerrainFoliageSpawnerAssetName", "Dynamic Terrain Foliage Spawner");
}

FColor FAssetTypeActions_TerrainFoliageSpawnerFactory::GetTypeColor() const
{
	return FColor::Green;
}

UClass* FAssetTypeActions_TerrainFoliageSpawnerFactory::GetSupportedClass() const
{
	return UTerrainFoliageSpawner::StaticClass();
}

uint32 FAssetTypeActions_TerrainFoliageSpawnerFactory::GetCategories()
{
	return TerrainAssetCategory;
}

/// FAssetTypeActions_TerrainFoliageFactory ///

FText FAssetTypeActions_TerrainFoliageFactory::GetName() const
{
	return LOCTEXT("DynamicTerrainFoliageAssetName", "Dynamic Terrain Foliage");
}

FColor FAssetTypeActions_TerrainFoliageFactory::GetTypeColor() const
{
	return FColor::Green;
}

UClass* FAssetTypeActions_TerrainFoliageFactory::GetSupportedClass() const
{
	return UTerrainFoliage::StaticClass();
}

uint32 FAssetTypeActions_TerrainFoliageFactory::GetCategories()
{
	return TerrainAssetCategory;
}

#undef LOCTEXT_NAMESPACE