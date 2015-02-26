
#ifndef __FLARE_H__
#define __FLARE_H__

#include "Engine.h"
#include "UI/Style/FlareStyleSet.h"


/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

#define NULL_QUAT           (FQuat::MakeFromEuler(FVector::ZeroVector))

#define FLOG(Format)   UE_LOG(LogTemp, Display, TEXT(Format))
#define FLOGV(Format, ...)   UE_LOG(LogTemp, Display, TEXT(Format), __VA_ARGS__)



DECLARE_LOG_CATEGORY_EXTERN(LogFlare, Log, All);


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
