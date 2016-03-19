;
#include "../Flare.h"
#include "FlareSpacecraft.h"
#include "Subsystems/FlareSpacecraftDamageSystem.h"
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
	if (!Ship->IsPresentationMode() && Ship->GetDamageSystem()->IsAlive())
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
				FRotator SunRotator = Planetarium->GetSunDirection().Rotation();
				FRotator SpinnerRotator = GetComponentRotation();
				FRotator Delta = SunRotator - SpinnerRotator;

				// Does not work, because Roll axis if zero when extracted from a direction... :) 

				if (RotationAxisRoll)
				{
					Delta = FRotator(0, 0, Delta.Roll);
				}
				else if (RotationAxisYaw)
				{
					Delta = FRotator(0, Delta.Yaw, 0);
				}
				else
				{
					Delta = FRotator(Delta.Pitch, 0, 0);
				}
				
				AddLocalRotation(RotationSpeed * DeltaSeconds * Delta);
			}
		}

		// Simple spinner
		else
		{
			AddLocalRotation(RotationSpeed * DeltaSeconds * Axis);
		}
	}
}


