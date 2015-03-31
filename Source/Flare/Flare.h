
#ifndef __FLARE_H__
#define __FLARE_H__

#include "Engine.h"
#include "UI/Style/FlareStyleSet.h"


/*----------------------------------------------------
	UI enums
----------------------------------------------------*/

/** Possible display targets for the subsystem display widgets */
UENUM()
namespace EFlareInfoDisplay
{
	enum Type
	{
		ID_Subsystem,
		ID_Spacer,
		ID_Speed,
		ID_Sector
	};
}


/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

#define NULL_QUAT           (FQuat::MakeFromEuler(FVector::ZeroVector))

#define FLOG(Format)        UE_LOG(LogTemp, Display, TEXT(Format))
#define FLOGV(Format, ...)  UE_LOG(LogTemp, Display, TEXT(Format), __VA_ARGS__)

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
