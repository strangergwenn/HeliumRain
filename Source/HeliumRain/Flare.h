
#ifndef __FLARE_H__
#define __FLARE_H__

#include "EngineMinimal.h"
#include "UI/Style/FlareStyleSet.h"


/*----------------------------------------------------
	Logging
----------------------------------------------------*/

DECLARE_LOG_CATEGORY_EXTERN(LogFlare, Log, All);

#define FLOG(Format)        UE_LOG(LogFlare, Display, TEXT(Format))
#define FLOGV(Format, ...)  UE_LOG(LogFlare, Display, TEXT(Format), __VA_ARGS__)

DECLARE_STATS_GROUP(TEXT("HeliumRain"), STATGROUP_Flare, STATCAT_Advanced);


/*----------------------------------------------------
	Error reporting
----------------------------------------------------*/

/** @brief Report an error through an OS box with the full call stack and exit */
static void FlareReportError(FString FunctionName)
{
	// Get stack trace
	TCHAR DescriptionString[4096];
	ANSICHAR StackTrace[4096];
	StackTrace[0] = 0;
	FPlatformStackWalk::StackWalkAndDumpEx(StackTrace, ARRAY_COUNT(StackTrace), 2, FGenericPlatformStackWalk::EStackWalkFlags::AccurateStackWalk);
	FCString::Strncat(DescriptionString, ANSI_TO_TCHAR(StackTrace), ARRAY_COUNT(DescriptionString) - 1);

	// Report to user
	FPlatformMisc::MessageBoxExt(EAppMsgType::Ok,
		*FString::Printf(TEXT("Helium Rain crashed in %s. Please report it on dev.helium-rain.com.\n\n%s"), *FunctionName, DescriptionString),
		TEXT("Sorry :("));

	// Crash
	check(false);
}

#define FCHECK(Expression)  do \
	{ \
		if (!ensure(Expression)) \
		{ \
			FlareReportError(__FUNCTION__); \
		} \
	} \
	while (0)


/*----------------------------------------------------
	Game module definition
----------------------------------------------------*/

class FFlareModule : public FDefaultGameModuleImpl
{

public:

	void StartupModule() override;

	void ShutdownModule() override;


protected:

	FFlareStyleSet StyleInstance;

};


#endif
