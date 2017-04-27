
#include "FlareOrbitalEngine.h"
#include "../Flare.h"
#include "FlareSpacecraft.h"


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
