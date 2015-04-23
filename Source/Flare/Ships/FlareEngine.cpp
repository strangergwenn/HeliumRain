
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
	MaxThrust = 0.0;
}


void UFlareEngine::Initialize(const FFlareShipComponentSave* Data, UFlareCompany* Company, AFlareShipBase* OwnerShip, bool IsInMenu)
{
	Super::Initialize(Data, Company, OwnerShip, IsInMenu);

	if (ComponentDescription)
	{
		for (int32 i = 0; i < ComponentDescription->Characteristics.Num(); i++)
		{
			const FFlareShipComponentCharacteristic& Characteristic = ComponentDescription->Characteristics[i];

			// Calculate the engine linear thrust force in N (data value in kN)
			if (Characteristic.CharacteristicType == EFlarePartCharacteristicType::EnginePower)
			{
				MaxThrust = 1000 * Characteristic.CharacteristicValue;
			}
		}
	}
}

/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

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
	return ExhaustAlpha * GetDamageRatio() * (IsPowered() ? 1 : 0)  * (Ship->HasPowerOutage() ? 0 : 1);
}

void UFlareEngine::UpdateEffects()
{
	// Apply the glow value
	if (EffectMaterial && IsVisibleByPlayer())
	{
		EffectMaterial->SetScalarParameterValue(TEXT("Opacity"), ExhaustAccumulator);
	}
}

FVector UFlareEngine::GetThrustAxis() const
{
	// Return the front vector in world space
	return GetComponentToWorld().GetRotation().RotateVector(FVector(1.0f, 0.0f, 0.0f));
}

float UFlareEngine::GetMaxThrust() const
{
	return MaxThrust * GetDamageRatio() * (IsPowered() ? 1 : 0)  * (Ship->HasPowerOutage() ? 0 : 1);
}

float UFlareEngine::GetInitialMaxThrust() const
{
	return MaxThrust;
}

float UFlareEngine::GetHeatProduction() const
{
	return Super::GetHeatProduction() * GetEffectiveAlpha();
}

void UFlareEngine::ApplyHeatDamage(float Energy)
{
	// Apply damage only on usage
	if (GetEffectiveAlpha() > 0)
	{
		ApplyDamage(Energy * ExhaustAlpha);
	}
}
