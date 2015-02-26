
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

void UFlareOrbitalEngine::UpdateAlpha(float DeltaTime)
{
	AFlareShip* OwnerShip = Cast<AFlareShip>(Ship);
	if (OwnerShip)
	{
		ExhaustAlpha = (OwnerShip->IsFakeThrust() ? 1.0f : OwnerShip->GetAttitudeCommandOrbitalThrust());
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
