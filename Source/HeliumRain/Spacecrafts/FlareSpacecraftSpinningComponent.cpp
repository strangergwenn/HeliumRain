;
#include "../Flare.h"
#include "FlareSpacecraft.h"
#include "Subsystems/FlareSpacecraftDamageSystem.h"
#include "../Game/FlareGame.h"
#include "../Game/FlarePlanetarium.h"
#include "FlareSpacecraftSpinningComponent.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftSpinningComponent::UFlareSpacecraftSpinningComponent(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, RotationAxisRoll(true)
	, RotationAxisYaw(false)
	, RotationSpeed(20)
{
	GetBodyInstance()->bAutoWeld = false;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareSpacecraftSpinningComponent::BeginPlay()
{
	Super::BeginPlay();
	SetCollisionProfileName("BlockAllDynamic");
}

void UFlareSpacecraftSpinningComponent::TickComponent(float DeltaSeconds, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaSeconds, TickType, ThisTickFunction);
	AFlareSpacecraft* Ship = GetSpacecraft();
	if (!Ship)
	{
		return;
	}

	// Update
	if (!Ship->IsPresentationMode() && Ship->GetParent()->GetDamageSystem()->IsAlive())
	{
		// Rotation axis
		FRotator Axis;
		if (RotationAxisRoll)
		{
			Axis = FRotator(0, 0, 1);
		}
		else if (RotationAxisYaw)
		{
			Axis = FRotator(0, 1, 0);
		}
		else
		{
			Axis = FRotator(1, 0, 0);
		}

		// Sun-looker
		if (LookForSun)
		{
			AFlarePlanetarium* Planetarium = Ship->GetGame()->GetPlanetarium();
			if (Planetarium)
			{
				FVector X(1, 0, 0);
				FVector Y(0, 1, 0);
				FVector Z(0, 0, 1);

				FRotator SunRotator = Planetarium->GetSunDirection().Rotation();
				FRotator SpinnerRotator = GetComponentRotation();
				FRotator Delta;

				// Compute the angle difference
				SunRotator.Normalize();
				SpinnerRotator.Normalize();
				float Angle = FMath::Acos(FVector::DotProduct(SunRotator.RotateVector(X), SpinnerRotator.RotateVector(Z)));
				// TODO : angle has a 50% chance of being the wrong way depending on orientation...

				// Use the correct axis
				if (RotationAxisRoll)
				{
					Delta = FQuat(X, Angle).Rotator();
				}
				else if (RotationAxisYaw)
				{
					Delta = FQuat(Z, Angle).Rotator();
				}
				else
				{
					Delta = FQuat(Y, Angle).Rotator();
				}

				// Update
				float DegreesAngle = FMath::RadiansToDegrees(Angle);
				if (DegreesAngle > 1)
				{
					AddLocalRotation(RotationSpeed * DeltaSeconds * Delta);
				}
			}
		}

		// Simple spinner
		else
		{
			AddLocalRotation(RotationSpeed * DeltaSeconds * Axis);
		}
	}
}


