#pragma once

#include "Modules/ModuleManager.h"

class FDynamicTerrainEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};