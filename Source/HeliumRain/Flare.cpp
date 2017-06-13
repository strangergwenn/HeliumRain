
#include "Flare.h"
#include "UI/Style/FlareStyleSet.h"


IMPLEMENT_PRIMARY_GAME_MODULE(FFlareModule, HeliumRain, "HeliumRain");

DEFINE_LOG_CATEGORY(LogFlare)

FFlareStyleSet* FFlareModule::StyleInstance = NULL;


/*----------------------------------------------------
	Module loading / unloading code
----------------------------------------------------*/

void FFlareModule::StartupModule()
{
	FDefaultGameModuleImpl::StartupModule();
	FSlateStyleRegistry::UnRegisterSlateStyle("FlareStyle");

	StyleInstance = new FFlareStyleSet();
	StyleInstance->Initialize();
}

void FFlareModule::ShutdownModule()
{
	FDefaultGameModuleImpl::ShutdownModule();
	StyleInstance->Shutdown();
	delete StyleInstance;
}
