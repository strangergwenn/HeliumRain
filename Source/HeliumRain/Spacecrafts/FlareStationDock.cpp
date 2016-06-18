
#include "../Flare.h"
#include "FlareStationDock.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareStationDock::UFlareStationDock(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	SetCollisionProfileName("NoCollision");
}
