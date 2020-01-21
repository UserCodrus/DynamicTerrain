#pragma once

#include "Modules/ModuleInterface.h"

class FDynamicTerrainEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};