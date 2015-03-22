#pragma once

#include "ModuleManager.h"

class InternalComponentVisualizerImpl : public IModuleInterface
{
public:
	void StartupModule();
	void ShutdownModule();
};
