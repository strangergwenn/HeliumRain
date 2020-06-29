
#include "HeliumRain/Flare.h"
#include "HeliumRain/UI/Style/FlareStyleSet.h"
#include "HeliumRain/Game/FlareGame.h"
#include "HeliumRain/Game/Save/FlareSaveGameSystem.h"

#include <Http.h>
#include <HttpManager.h>
#include <HAL/PlatformStackWalk.h>
#include <HAL/PlatformProcess.h>
#include <HAL/PlatformTime.h>
#include <Engine.h>


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


/*----------------------------------------------------
	Error reporting
----------------------------------------------------*/

FThreadSafeBool HasCrashed = false;

void FFlareModule::ReportError(FString Title)
{
	// Ensure no duplicates
	if (HasCrashed)
	{
		return;
	}
	HasCrashed = true;

	// Get stack trace
	ANSICHAR Callstack[4096];
	Callstack[0] = 0;
	TCHAR CallstackString[4096];
	CallstackString[0] = 0;
	FPlatformStackWalk::StackWalkAndDumpEx(Callstack, ARRAY_COUNT(Callstack), 2, FGenericPlatformStackWalk::EStackWalkFlags::AccurateStackWalk);
	FCString::Strncat(CallstackString, ANSI_TO_TCHAR(Callstack), ARRAY_COUNT(CallstackString) - 1);

#if UE_BUILD_SHIPPING

	// Parameters
	FString ReportURL = TEXT("http://deimos.games/report.php");
	FString GameString = TEXT("Helium Rain");
	FString GameParameter = TEXT("game");
	FString TitleParameter = TEXT("title");
	FString StackParameter = TEXT("callstack");

	// Format data
	FString RequestContent = GameParameter + TEXT("=") + GameString
	          + TEXT("&") + TitleParameter + TEXT("=") + Title
	          + TEXT("&") + StackParameter + TEXT("=") + FString(CallstackString);

	// Report to server
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(ReportURL);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-DeimosGames-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(RequestContent);
	Request->ProcessRequest();

	// Wait for end
	double CurrentTime = FPlatformTime::Seconds();
	while (EHttpRequestStatus::Processing == Request->GetStatus())
	{
		const double AppTime = FPlatformTime::Seconds();
		FHttpModule::Get().GetHttpManager().Tick(AppTime - CurrentTime);
		CurrentTime = AppTime;
		FPlatformProcess::Sleep(0.1f);
	}

#endif

	// Report to user
	FPlatformMisc::MessageBoxExt(EAppMsgType::Ok,
		*FString::Printf(TEXT("Helium Rain crashed (%s). Please report it on dev.helium-rain.com.\n\n%s"), *Title, CallstackString),
		TEXT("Sorry :("));

	// Crash
	FPlatformMisc::RequestExit(0);
}
