
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
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareOrbitalEngine::UpdateEffects()
{
	Super::UpdateEffects();

	// Smooth the command value
	float AverageCoeff = 0.005;
	float TemperatureAlpha = AverageCoeff * ExhaustAccumulator + (1 - AverageCoeff) * AverageAlpha;
	AverageAlpha = TemperatureAlpha;

	SetTemperature(1500 * TemperatureAlpha);
}

bool UFlareOrbitalEngine::IsOrbitalEngine() const
{
    return true;
}
