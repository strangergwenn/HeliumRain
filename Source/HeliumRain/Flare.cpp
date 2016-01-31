
#include "Flare.h"


IMPLEMENT_PRIMARY_GAME_MODULE(FFlareModule, Flare, "HeliumRain");

DEFINE_LOG_CATEGORY(LogFlare)


/*----------------------------------------------------
	Module loading / unloading code
----------------------------------------------------*/

void FFlareModule::StartupModule()
{
	FDefaultGameModuleImpl::StartupModule();
	FSlateStyleRegistry::UnRegisterSlateStyle("FlareStyle");
	StyleInstance.Initialize();
}

void FFlareModule::ShutdownModule()
{
	FDefaultGameModuleImpl::ShutdownModule();
	StyleInstance.Shutdown();
}
