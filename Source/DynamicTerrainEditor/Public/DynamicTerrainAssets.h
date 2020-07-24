#pragma once

#include "Factories/Factory.h"
#include "AssetTypeActions_Base.h"

#include "DynamicTerrainAssets.generated.h"

static EAssetTypeCategories::Type TerrainAssetCategory;

// The asset factory for creating terrain foliage sets in the editor
UCLASS()
class UTerrainFoliageFactory : public UFactory
{
	GENERATED_BODY()

public:
	UTerrainFoliageFactory(const FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

// Asset actions to make the foliage factory work in the editor
class FAssetTypeActions_TerrainFoliageFactory : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;
};