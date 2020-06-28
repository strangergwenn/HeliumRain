#pragma once

#include <Modules/ModuleManager.h>

class InternalComponentVisualizerImpl : public IModuleInterface
{
public:
	void StartupModule();
	void ShutdownModule();
};
