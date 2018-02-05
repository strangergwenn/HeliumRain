#pragma once

#include "EngineMinimal.h"


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

#define FCHECK(Expression)  do \
	{ \
		if (!ensure(Expression)) \
		{ \
			FFlareModule::ReportError(__FUNCTION__); \
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

	/** Send a crash report, display the stack and exit */
	static void ReportError(FString FunctionName);

protected:

	static class FFlareStyleSet* StyleInstance;

};

