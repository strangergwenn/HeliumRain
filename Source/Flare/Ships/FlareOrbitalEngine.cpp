
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
    MaxThrust = 000000; // TODO remove
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareOrbitalEngine::UpdateAlpha(float DeltaTime)
{
	Super::UpdateAlpha(DeltaTime);
	AFlareShip* OwnerShip = Cast<AFlareShip>(Ship);
	if (OwnerShip && OwnerShip->IsFakeThrust())
	{
		ExhaustAlpha = 1.0f;
	}
}


void UFlareOrbitalEngine::UpdateEffects()
{
	Super::UpdateEffects();

	// Smooth the command value
	float AverageCoeff = 0.005;
	float TemperatureAlpha = ExhaustAlpha = AverageCoeff * ExhaustAlpha + (1 - AverageCoeff) * AverageAlpha;
	AverageAlpha = TemperatureAlpha;

	SetTemperature(1500 * TemperatureAlpha);
}
