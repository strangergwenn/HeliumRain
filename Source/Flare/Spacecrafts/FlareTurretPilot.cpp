
#include "../Flare.h"

#include "FlareTurretPilot.h"
#include "FlareSpacecraft.h"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareTurretPilot::UFlareTurretPilot(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	ReactionTime = FMath::FRandRange(0.005, 0.01);
	TimeUntilNextReaction = 0;
	PilotTargetShip = NULL;
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/


void UFlareTurretPilot::Initialize(const FFlareTurretPilotSave* Data, UFlareCompany* Company, UFlareTurret* OwnerTurret)
{
	// Main data
	Turret = OwnerTurret;
	PlayerCompany = Company;

	// Setup properties
	if (Data)
	{
		TurretPilotData = *Data;
	}
}

void UFlareTurretPilot::TickPilot(float DeltaSeconds)
{
	/*if (TimeUntilNextReaction > 0)
	{
		TimeUntilNextReaction -= DeltaSeconds;
		return;
	}
	else
	{
		TimeUntilNextReaction = ReactionTime;
	}*/


	AimAxis = FVector::ZeroVector;
	WantFire = false;


	AFlareSpacecraft* OldPilotTargetShip = PilotTargetShip;

	FVector TurretLocation = Turret->GetTurretBaseLocation();

	PilotTargetShip = GetNearestHostileShip(true, true, 1200000);

	if(!PilotTargetShip)
	{
		PilotTargetShip = GetNearestHostileShip(true, false, 5000000);
	}

	// No dangerous ship, try not dangerous ships
	if(!PilotTargetShip)
	{
		PilotTargetShip = GetNearestHostileShip(false, false, 1200000);
	}

	if(!PilotTargetShip)
	{
		PilotTargetShip = GetNearestHostileShip(false, false, 5000000);
	}

	if(PilotTargetShip)
	{

		bool DangerousTarget = IsShipDangerous(PilotTargetShip);

		//float PredictionDelay = ReactionTime - DeltaSeconds;
		float PredictionDelay = 0;
		float AmmoVelocity = Turret->GetAmmoVelocity();
		FVector TurretVelocity = 100 * Turret->GetSpacecraft()->GetLinearVelocity();
		FVector PredictedFireTargetLocation = (PilotTargetShip->GetAimPosition(TurretLocation, TurretVelocity / 100, AmmoVelocity, PredictionDelay));


		AimAxis = (PredictedFireTargetLocation - TurretLocation).GetUnsafeNormal();
		/*FLOGV("%s Have target AimAxis=%s",*Turret->GetReadableName(),  * AimAxis.ToString());
*/

		float TargetSize = PilotTargetShip->GetMeshScale() / 100.f + Turret->GetAimRadius() * 2; // Radius in meters
		FVector DeltaLocation = (PilotTargetShip->GetActorLocation()-TurretLocation) / 100.f;
		float Distance = DeltaLocation.Size(); // Distance in meters

		// If at range and aligned fire on the target
		//TODO increase tolerance if target is near
		if(Distance < (DangerousTarget ? 10000.f : 5000.f) + 4 * TargetSize)
		{
			//FLOG("Near enough");
			FVector FireAxis = Turret->GetFireAxis();


			for(int GunIndex = 0; GunIndex < Turret->GetGunCount(); GunIndex++)
			{
				FVector MuzzleLocation = Turret->GetMuzzleLocation(GunIndex);

				// Compute target Axis for each gun
				FVector FireTargetAxis = (PilotTargetShip->GetAimPosition(MuzzleLocation, TurretVelocity , AmmoVelocity, 0) - MuzzleLocation).GetUnsafeNormal();
				/*FLOGV("Gun %d FireAxis=%s", GunIndex, *FireAxis.ToString());
				FLOGV("Gun %d FireTargetAxis=%s", GunIndex, *FireTargetAxis.ToString());
*/
				float AngularPrecisionDot = FVector::DotProduct(FireTargetAxis, FireAxis);
				float AngularPrecision = FMath::Acos(AngularPrecisionDot);
				float AngularSize = FMath::Atan(TargetSize / Distance);

			/*	FLOGV("Gun %d Distance=%f", GunIndex, Distance);
				FLOGV("Gun %d TargetSize=%f", GunIndex, TargetSize);
				FLOGV("Gun %d AngularSize=%f", GunIndex, AngularSize);
				FLOGV("Gun %d AngularPrecision=%f", GunIndex, AngularPrecision);*/
				if(AngularPrecision < (DangerousTarget ? AngularSize * 0.25 : AngularSize * 0.2))
				{
					Turret->SetTarget(PilotTargetShip);
					/*FLOG("Want Fire");*/
					WantFire = true;
					break;
				}
			}
		}

		if(Turret->GetSpacecraft()->GetDamageSystem()->GetTemperature() > Turret->GetSpacecraft()->GetDamageSystem()->GetOverheatTemperature() * (DangerousTarget ? 1.1f : 0.90f))
		{
			// TODO Fire on dangerous target
			WantFire = false;
			/*FLOG("Want Fire but too hot");*/
		}
	}
}

/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

AFlareSpacecraft* UFlareTurretPilot::GetNearestHostileShip(bool DangerousOnly, bool ReachableOnly, float MaxDistance) const
{
	// For now an host ship is a the nearest host ship with the following critera:
	// - Alive
	// - Is dangerous if needed
	// - From another company
	// - Is the nearest

	float SecurityRadius = 0;

	if(Turret->GetDescription()->GunCharacteristics.FuzeType == EFlareShellFuzeType::Proximity)
	{
		 SecurityRadius = Turret->GetDescription()->GunCharacteristics.AmmoExplosionRadius + Turret->GetSpacecraft()->GetMeshScale() / 100;
	}

	FVector PilotLocation = Turret->GetTurretBaseLocation();
	float MaxDot = 0;
	AFlareSpacecraft* NearestHostileShip = NULL;
	FVector FireAxis = Turret->GetFireAxis();


	for (TActorIterator<AActor> ActorItr(Turret->GetSpacecraft()->GetWorld()); ActorItr; ++ActorItr)
	{
		// Ship
		AFlareSpacecraft* ShipCandidate = Cast<AFlareSpacecraft>(*ActorItr);
		if (ShipCandidate)
		{
			if (!ShipCandidate->GetDamageSystem()->IsAlive())
			{
				continue;
			}

			if (DangerousOnly && !IsShipDangerous(ShipCandidate))
			{
				continue;
			}

			if (PlayerCompany->GetHostility(ShipCandidate->GetCompany()) != EFlareHostility::Hostile)
			{
				continue;
			}

			float Distance = (PilotLocation - ShipCandidate->GetActorLocation()).Size();
			if(Distance < SecurityRadius * 100)
			{
				continue;
			}

			if (Distance > MaxDistance)
			{
				continue;
			}

			FVector TargetAxis = (ShipCandidate->GetActorLocation()- PilotLocation).GetUnsafeNormal();

			if(ReachableOnly && !Turret->IsReacheableAxis(TargetAxis))
			{
				continue;
			}
			float Dot = FVector::DotProduct(TargetAxis, FireAxis);

			if (NearestHostileShip == NULL || Dot > MaxDot)
			{
				MaxDot = Dot;
				NearestHostileShip = ShipCandidate;
			}
		}
	}
	return NearestHostileShip;
}

bool UFlareTurretPilot::IsShipDangerous(AFlareSpacecraft* ShipCandidate) const
{
	return ShipCandidate->IsMilitary() && ShipCandidate->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon) > 0;
}

/*----------------------------------------------------
	Pilot Output
----------------------------------------------------*/

FVector UFlareTurretPilot::GetTargetAimAxis() const
{
	return AimAxis;
}

bool UFlareTurretPilot::IsWantFire() const
{
	return WantFire;
}
