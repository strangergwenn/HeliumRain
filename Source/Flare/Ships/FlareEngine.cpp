
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
	ThrustAxis = FVector(-1,0,0);
}


void UFlareEngine::Initialize(const FFlareShipModuleDescription* Description, UFlareCompany* Company, AFlareShipBase* OwnerShip, bool IsInMenu)
{
	Super::Initialize(Description, Company, OwnerShip, IsInMenu);
	for (int32 i = 0; i < Description->Characteristics.Num(); i++)
	{
		const FFlarePartCharacteristic& Characteristic = Description->Characteristics[i];

		// Calculate the engine linear thrust force in N (data value in kN)
		if (Characteristic.CharacteristicType == EFlarePartAttributeType::EnginePower)
		{
			MaxThrust = 1000 * Characteristic.CharacteristicValue;
		}
	}
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
	ExhaustAlpha = FMath::Max(CurrentLinearThrust, CurrentAngularThrust) / MaxThrust;
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

void UFlareEngine::SetTargetLinearThrustRatio(float Ratio)
{
	if (Ratio > 0)
	{
		TargetLinearThrust = Ratio * MaxThrust;
	}
	else
	{
		TargetLinearThrust = 0;
	}
}

void UFlareEngine::SetTargetAngularThrustRatio(float Ratio)
{
	if (Ratio > 0)
	{
		TargetAngularThrust = Ratio * MaxThrust;
	}
	else
	{
		TargetAngularThrust = 0;
	}
}

FVector UFlareEngine::GetThurstAxis() const
{
	  return GetComponentToWorld().GetRotation().RotateVector(ThrustAxis);
}

float UFlareEngine::GetMaxThrust() const
{
	return MaxThrust; // TODO include damages
}

float UFlareEngine::GetInitialMaxThrust() const
{
	return MaxThrust;
}

void UFlareEngine::TickModule(float DeltaTime)
{
	Super::TickModule(DeltaTime);
	AFlareShip* OwnerShip = Cast<AFlareShip>(Ship);

	CurrentLinearThrust = TargetLinearThrust;
	CurrentLinearThrust = FMath::Clamp(CurrentLinearThrust, 0.f, MaxThrust);
	FVector LocalLinearThrust = ThrustAxis * CurrentLinearThrust;
	FVector WorldLinearThrust = GetComponentToWorld().GetRotation().RotateVector(LocalLinearThrust);
	
	CurrentAngularThrust = TargetAngularThrust;
	CurrentAngularThrust = FMath::Clamp(CurrentAngularThrust, 0.f, MaxThrust);
	FVector LocalAngularThrust = ThrustAxis * CurrentAngularThrust;
	FVector WorldAngularThrust = GetComponentToWorld().GetRotation().RotateVector(LocalAngularThrust);
	

	OwnerShip->AddForceAtLocation(WorldLinearThrust, WorldAngularThrust, GetComponentLocation());
}
