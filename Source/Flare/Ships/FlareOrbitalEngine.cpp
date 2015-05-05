
#include "../Flare.h"
#include "FlareShip.h"
#include "FlareAirframe.h"
#include "FlareOrbitalEngine.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareOrbitalEngine::UFlareOrbitalEngine(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	HasFlickeringLights = true;
	HasLocalHeatEffect = true;
}
