
#include "../Flare.h"
#include "FlareShip.h"
#include "FlareEngine.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareEngine::UFlareEngine(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	ExhaustAlpha = 0.0;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareEngine::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Compute the command
	UpdateAlpha(DeltaTime);

	// Smooth the command value
	float AverageCoeff = 0.15;
	ExhaustAlpha = FMath::Clamp(ExhaustAlpha, 0.0f, 1.0f);
	ExhaustAlpha = AverageCoeff * ExhaustAlpha + (1 - AverageCoeff) * ExhaustAccumulator;
	ExhaustAccumulator = ExhaustAlpha;

	// Apply effects
	UpdateEffects();
}

void UFlareEngine::UpdateAlpha(float DeltaTime)
{
	ExhaustAlpha = 0.0f;
}

void UFlareEngine::UpdateEffects()
{
	// Smooth the alpha value
	float AverageCoeff = 0.3;
	ExhaustAlpha = FMath::Clamp(ExhaustAlpha, 0.0f, 1.0f);
	ExhaustAlpha = AverageCoeff * ExhaustAlpha + (1 - AverageCoeff) * ExhaustAccumulator;
	ExhaustAccumulator = ExhaustAlpha;

	// Apply the glow value
	if (EffectMaterial)
	{
		EffectMaterial->SetScalarParameterValue(TEXT("Opacity"), ExhaustAlpha);
	}
}
