
#include "FlareStationDock.h"
#include "../Flare.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareStationDock::UFlareStationDock(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	SetCollisionProfileName("NoCollision");
}
