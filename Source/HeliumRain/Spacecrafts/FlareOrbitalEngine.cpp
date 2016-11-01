
#include "../Flare.h"
#include "FlareSpacecraft.h"
#include "FlareOrbitalEngine.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareOrbitalEngine::UFlareOrbitalEngine(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	LocalHeatEffect = true;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

bool UFlareOrbitalEngine::IsDestroyedEffectRelevant()
{
	// Always smoke
	return true;
}
