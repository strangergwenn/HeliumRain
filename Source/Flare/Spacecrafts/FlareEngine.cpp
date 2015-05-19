
#include "../Flare.h"
#include "FlareSpacecraft.h"
#include "FlareEngine.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareEngine::UFlareEngine(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	ExhaustAlpha = 0.0;
	MaxThrust = 0.0;
	ExhaustAccumulator = 0.0;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareEngine::Initialize(const FFlareSpacecraftComponentSave* Data, UFlareCompany* Company, AFlareSpacecraftPawn* OwnerShip, bool IsInMenu)
{
	Super::Initialize(Data, Company, OwnerShip, IsInMenu);

	if (ComponentDescription)
	{
        MaxThrust = 1000 * ComponentDescription->EngineCharacteristics.EnginePower;
	}
}

void UFlareEngine::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Smooth the alpha value. Half-life time : 1/8 second
	float AverageCoeff = 8 * DeltaTime;
	ExhaustAccumulator = FMath::Clamp(AverageCoeff * GetEffectiveAlpha() + (1 - AverageCoeff) * ExhaustAccumulator, 0.0f, 1.0f);

	// Apply effects
	UpdateEffects();
}

void UFlareEngine::SetAlpha(float Alpha)
{
	ExhaustAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
}

float UFlareEngine::GetEffectiveAlpha() const
{
	if (Spacecraft)
	{
		return ExhaustAlpha * GetDamageRatio() * (IsPowered() ? 1 : 0)  * (Spacecraft->GetDamageSystem()->HasPowerOutage() ? 0 : 1);
	}
	else
	{
		return 1.f;
	}
}

void UFlareEngine::UpdateEffects()
{
	// Apply the glow value
	if (EffectMaterial && SpacecraftPawn)
	{
		if (!SpacecraftPawn->IsPresentationMode() && IsVisibleByPlayer())
		{
			EffectMaterial->SetScalarParameterValue(TEXT("Opacity"), ExhaustAccumulator);
		}
		else if (SpacecraftPawn->IsPresentationMode() || IsVisibleByPlayer())
		{
			EffectMaterial->SetScalarParameterValue(TEXT("Opacity"), 0);
		}
	}
}

FVector UFlareEngine::GetThrustAxis() const
{
	// Return the front vector in world space
	return GetComponentToWorld().GetRotation().RotateVector(FVector(1.0f, 0.0f, 0.0f));
}

float UFlareEngine::GetMaxThrust() const
{
	if (Spacecraft)
	{
		return MaxThrust * GetDamageRatio() * (IsPowered() ? 1 : 0)  * (Spacecraft->GetDamageSystem()->HasPowerOutage() ? 0 : 1);
	}
	else
	{
		return MaxThrust;
	}
}

float UFlareEngine::GetInitialMaxThrust() const
{
	return MaxThrust;
}

float UFlareEngine::GetHeatProduction() const
{
	return Super::GetHeatProduction() * GetEffectiveAlpha();
}

void UFlareEngine::ApplyHeatDamage(float OverheatEnergy, float BurnEnergy)
{
	Super::ApplyHeatDamage(OverheatEnergy, BurnEnergy);

	// Apply damage only on usage
	if (GetEffectiveAlpha() > 0)
	{
		ApplyDamage(OverheatEnergy * ExhaustAlpha);
	}
}
