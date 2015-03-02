
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
	MaxThrust = 600; // TODO init
	ThrustAxis = FVector(-1,0,0);
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
	ExhaustAlpha = CurrentThrust / MaxThrust;
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

void UFlareEngine::SetTargetThrustRatio(float Ratio)
{
	if (Ratio > 0)
	{
		TargetThrust = Ratio * MaxThrust;
	}
	else
	{
		TargetThrust = 0;
	}
}

FVector UFlareEngine::GetThurstAxis() const
{
	  return GetComponentToWorld().GetRotation().RotateVector(ThrustAxis);
}

float UFlareEngine::GetMaxThrust() const
{
	return MaxThrust;
}


void UFlareEngine::TickModule(float DeltaTime)
{
	Super::TickModule(DeltaTime);
	AFlareShip* OwnerShip = Cast<AFlareShip>(Ship);

	CurrentThrust = TargetThrust;
	CurrentThrust = FMath::Clamp(CurrentThrust, 0.f, MaxThrust);



	FVector ThrustNoise; // Little random because engine are not perfectly precise
	ThrustNoise.X = FMath::FRandRange(-0.01, 0.01) * CurrentThrust;
	ThrustNoise.Y = FMath::FRandRange(-0.01, 0.01) * CurrentThrust;
	ThrustNoise.Z = FMath::FRandRange(-0.05, 0.05) * CurrentThrust;
	FVector LocalThrust = ThrustAxis * CurrentThrust + ThrustNoise * 0.0;

	FVector WorldThrust = GetComponentToWorld().GetRotation().RotateVector(LocalThrust);

	UE_LOG(LogTemp, Warning, TEXT("1 - WorldThrust: %s TargetThrust=%f"), *WorldThrust.ToString(), TargetThrust);


	OwnerShip->AddForceAtLocation(WorldThrust, GetComponentLocation());
}
