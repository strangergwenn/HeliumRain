
#include "../Flare.h"
#include "FlareSpacecraft.h"
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
