
#include "../Flare.h"

#include "FlareTurretPilot.h"
#include "FlareSpacecraft.h"
#include "FlareSpacecraftComponent.h"

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

	EFlarePartSize::Type PreferredShipSize;
	EFlarePartSize::Type SecondaryShipSize;
	if(Turret->GetDescription()->WeaponCharacteristics.DamageType == EFlareShellDamageType::HEAT)
	{
		PreferredShipSize = EFlarePartSize::L;
		SecondaryShipSize = EFlarePartSize::S;
	}
	else
	{
		PreferredShipSize = EFlarePartSize::S;
		SecondaryShipSize = EFlarePartSize::L;
	}


	PilotTargetShip = GetNearestHostileShip(true, true, 1200000,PreferredShipSize);

	if (!PilotTargetShip)
	{
		PilotTargetShip = GetNearestHostileShip(true, false, 5000000, PreferredShipSize);
	}

	if (!PilotTargetShip)
	{
		PilotTargetShip = GetNearestHostileShip(true, true, 1200000,SecondaryShipSize);
	}

	if (!PilotTargetShip)
	{
		PilotTargetShip = GetNearestHostileShip(true, false, 5000000, SecondaryShipSize);
	}

	// No dangerous ship, try not dangerous ships
	if (!PilotTargetShip)
	{
		PilotTargetShip = GetNearestHostileShip(false, false, 1200000, PreferredShipSize);
	}

	if (!PilotTargetShip)
	{
		PilotTargetShip = GetNearestHostileShip(false, false, 5000000, PreferredShipSize);
	}

	if (!PilotTargetShip)
	{
		PilotTargetShip = GetNearestHostileShip(false, false, 1200000, SecondaryShipSize);
	}

	if (!PilotTargetShip)
	{
		PilotTargetShip = GetNearestHostileShip(false, false, 5000000, SecondaryShipSize);
	}

	TimeUntilNextComponentSwitch-=ReactionTime;

	if (PilotTargetShip)
	{
		if(TimeUntilNextComponentSwitch <= 0)
		{
			//FLOGV("%s Switch because of timeout", *Turret->GetReadableName());
			PilotTargetComponent = NULL;
		}
		else if(PilotTargetComponent)
		{
			if(PilotTargetComponent->GetSpacecraft() != PilotTargetShip)
			{
				//FLOGV("%s Switch because the component %s is not in the target ship", *Turret->GetReadableName(), *PilotTargetComponent->GetReadableName());
				PilotTargetComponent = NULL;
			}
			else if(PilotTargetComponent->GetDamageRatio() <=0)
			{
				//FLOGV("%s Switch because the component %s is destroyed", *Turret->GetReadableName(), *PilotTargetComponent->GetReadableName());
				PilotTargetComponent = NULL;

			}
		}

		if(!PilotTargetComponent)
		{
			PilotTargetComponent = GetRandomTargetComponent(PilotTargetShip);
			TimeUntilNextComponentSwitch = 10;
			//FLOGV("%s Select new target component %s ", *Turret->GetReadableName(), *PilotTargetComponent->GetReadableName());
		}


		bool DangerousTarget = IsShipDangerous(PilotTargetShip);

		//float PredictionDelay = ReactionTime - DeltaSeconds;
		float PredictionDelay = 0;
		float AmmoVelocity = Turret->GetAmmoVelocity() * 100;
		FVector TurretVelocity = 100 * Turret->GetSpacecraft()->GetLinearVelocity();


		FVector AmmoIntersectionPredictedLocation;



		float AmmoIntersectionPredictedTime = SpacecraftHelper::GetIntersectionPosition(PilotTargetComponent->GetComponentLocation(), PilotTargetShip->Airframe->GetPhysicsLinearVelocity(), TurretLocation, TurretVelocity, AmmoVelocity, PredictionDelay, &AmmoIntersectionPredictedLocation);
		FVector PredictedFireTargetLocation;
		if (AmmoIntersectionPredictedTime > 0)
		{
			PredictedFireTargetLocation = AmmoIntersectionPredictedLocation - AmmoIntersectionPredictedTime * TurretVelocity;
		}
		else
		{
			PredictedFireTargetLocation = PilotTargetComponent->GetComponentLocation();
		}


		AimAxis = (PredictedFireTargetLocation - TurretLocation).GetUnsafeNormal();
		/*FLOGV("%s Have target AimAxis=%s",*Turret->GetReadableName(),  * AimAxis.ToString());
		FLOGV("%s AmmoIntersectionPredictedTime=%f",*Turret->GetReadableName(),  AmmoIntersectionPredictedTime);
		FLOGV("%s AmmoVelocity=%f",*Turret->GetReadableName(),  AmmoVelocity);*/



		float TargetSize = PilotTargetShip->GetMeshScale() / 100.f + Turret->GetAimRadius() * 2; // Radius in meters
		FVector DeltaLocation = (PilotTargetComponent->GetComponentLocation()-TurretLocation) / 100.f;
		float Distance = DeltaLocation.Size(); // Distance in meters

		//FLOGV("%s Distance=%f",*Turret->GetReadableName(),  Distance);

		// If at range and aligned fire on the target
		//TODO increase tolerance if target is near


		if (AmmoIntersectionPredictedTime > 0 && AmmoIntersectionPredictedTime < 10.f)
		{
			//FLOG("Near enough");
			FVector FireAxis = Turret->GetFireAxis();


			for (int GunIndex = 0; GunIndex < Turret->GetGunCount(); GunIndex++)
			{
				FVector MuzzleLocation = Turret->GetMuzzleLocation(GunIndex);

				// Compute target Axis for each gun
				FVector AmmoIntersectionLocation;
				float AmmoIntersectionTime = SpacecraftHelper::GetIntersectionPosition(PilotTargetComponent->GetComponentLocation(), PilotTargetShip->Airframe->GetPhysicsLinearVelocity(), MuzzleLocation, TurretVelocity , AmmoVelocity, 0, &AmmoIntersectionLocation);
				if (AmmoIntersectionTime < 0)
				{
					// No ammo intersection, don't fire
					continue;
				}
				FVector FireTargetAxis = (AmmoIntersectionLocation - MuzzleLocation - AmmoIntersectionPredictedTime * TurretVelocity).GetUnsafeNormal();
				/*FLOGV("Gun %d FireAxis=%s", GunIndex, *FireAxis.ToString());
				FLOGV("Gun %d FireTargetAxis=%s", GunIndex, *FireTargetAxis.ToString());*/

				float AngularPrecisionDot = FVector::DotProduct(FireTargetAxis, FireAxis);
				float AngularPrecision = FMath::Acos(AngularPrecisionDot);
				float AngularSize = FMath::Atan(TargetSize / Distance);

				/*FLOGV("Gun %d Distance=%f", GunIndex, Distance);
				FLOGV("Gun %d TargetSize=%f", GunIndex, TargetSize);
				FLOGV("Gun %d AngularSize=%f", GunIndex, AngularSize);
				FLOGV("Gun %d AngularPrecision=%f", GunIndex, AngularPrecision);*/
				if (AngularPrecision < (DangerousTarget ? AngularSize * 0.25 : AngularSize * 0.2))
				{
					Turret->SetTarget(PilotTargetShip);
					//FLOG("Want Fire");
					WantFire = true;
					break;
				}
			}
		}

		if (Turret->GetSpacecraft()->GetDamageSystem()->GetTemperature() > Turret->GetSpacecraft()->GetDamageSystem()->GetOverheatTemperature() * (DangerousTarget ? 1.1f : 0.90f))
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

AFlareSpacecraft* UFlareTurretPilot::GetNearestHostileShip(bool DangerousOnly, bool ReachableOnly, float MaxDistance, EFlarePartSize::Type PreferredType) const
{
	// For now an host ship is a the nearest host ship with the following critera:
	// - Alive
	// - Is dangerous if needed
	// - From another company
	// - Is the nearest

	float SecurityRadius = 0;

	if (Turret->GetDescription()->WeaponCharacteristics.FuzeType == EFlareShellFuzeType::Proximity)
	{
		 SecurityRadius = Turret->GetDescription()->WeaponCharacteristics.AmmoExplosionRadius + Turret->GetSpacecraft()->GetMeshScale() / 100;
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
			if (ShipCandidate->GetSize() != PreferredType)
			{
				continue;
			}



			if (PlayerCompany->GetHostility(ShipCandidate->GetCompany()) != EFlareHostility::Hostile)
			{
				continue;
			}

			float Distance = (PilotLocation - ShipCandidate->GetActorLocation()).Size();
			if (Distance < SecurityRadius * 100)
			{
				continue;
			}

			if (Distance > MaxDistance)
			{
				continue;
			}

			FVector TargetAxis = (ShipCandidate->GetActorLocation()- PilotLocation).GetUnsafeNormal();

			if (ReachableOnly && !Turret->IsReacheableAxis(TargetAxis))
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

UFlareSpacecraftComponent* UFlareTurretPilot::GetRandomTargetComponent(AFlareSpacecraft* TargetSpacecraft)
{
	TArray<UFlareSpacecraftComponent*> ComponentSelection;

	TArray<UActorComponent*> Components = TargetSpacecraft->GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);

		if(Component->GetDescription() && Component->GetDamageRatio() > 0)
		{
			ComponentSelection.Add(Component);
		}
	}

	if(ComponentSelection.Num() == 0)
	{
		return TargetSpacecraft->GetCockpit();
	}
	else
	{
		while(true)
		{
			UFlareSpacecraftComponent* Component = ComponentSelection[FMath::RandRange(0, ComponentSelection.Num()-1)];

			UFlareRCS* RCS = Cast<UFlareRCS>(Component);
			if(RCS)
			{
				if(FMath::FRand() > 0.25)
				{
					continue;
				}
			}
			return Component;
		}

	}
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
