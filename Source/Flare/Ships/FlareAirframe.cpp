;
#include "../Flare.h"
#include "FlareAirframe.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareAirframe::UFlareAirframe(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFlareAirframe::OnRegister()
{
	Super::OnRegister();
	SetCollisionProfileName("BlockAllDynamic");
}
