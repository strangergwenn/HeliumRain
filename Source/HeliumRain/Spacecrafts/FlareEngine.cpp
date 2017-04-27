
#include "FlareEngine.h"
#include "../Flare.h"
#include "FlareSpacecraft.h"
#include "FlareOrbitalEngine.h"


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

void UFlareEngine::Initialize(FFlareSpacecraftComponentSave* Data, UFlareCompany* Company, AFlareSpacecraftPawn* OwnerShip, bool IsInMenu)
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

	if(this->IsA(UFlareOrbitalEngine::StaticClass()))
	{
		AverageCoeff = 1 * DeltaTime;
	}

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
		return ExhaustAlpha * GetUsableRatio();
	}
	else
	{
		return 1.f;
	}
}

void UFlareEngine::UpdateEffects()
{
	// Apply the glow value
	if (EffectMaterial && SpacecraftPawn && IsComponentVisible())
	{
		float Opacity = ExhaustAccumulator;

		if (IsA(UFlareOrbitalEngine::StaticClass()))
		{
			Opacity = FMath::Square(ExhaustAccumulator);
		}

		if (!SpacecraftPawn->IsPresentationMode())
		{
			EffectMaterial->SetScalarParameterValue(TEXT("Opacity"), Opacity);
		}
		else
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
		return MaxThrust * GetUsableRatio();
	}
	else
	{
		return MaxThrust;
	}
}

float UFlareEngine::GetUsableRatio() const
{
	float BaseUsableRatio = UFlareSpacecraftComponent::GetUsableRatio();

	if (Spacecraft)
	{
		return BaseUsableRatio * (1.0f - Spacecraft->GetDamageSystem()->GetOverheatRatio(0.05));
	}
	else
	{
		return BaseUsableRatio;
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

