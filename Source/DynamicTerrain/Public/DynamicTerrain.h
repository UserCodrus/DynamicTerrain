// Copyright © 2019 Created by Brian Faubion

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FDynamicTerrainModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
