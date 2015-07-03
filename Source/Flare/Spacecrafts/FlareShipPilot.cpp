
#include "../Flare.h"

#include "FlareShipPilot.h"
#include "FlareSpacecraft.h"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareShipPilot::UFlareShipPilot(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	ReactionTime = FMath::FRandRange(0.2, 0.3);
	TimeUntilNextReaction = 0;
	WaitTime = 0;
	PilotTargetLocation = FVector::ZeroVector;
	PilotTargetShip = NULL;
	PilotTargetStation = NULL;
	PilotLastTargetStation = NULL;
	SelectedWeaponGroupIndex = -1;
	MaxFollowDistance = 0;
	LockTarget = false;
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void UFlareShipPilot::TickPilot(float DeltaSeconds)
{
	if (Ship->IsStation())
	{
		// No pilot for stations
		return;
	}

	if (TimeUntilNextReaction > 0)
	{
		TimeUntilNextReaction -= DeltaSeconds;
		return;
	}
	else
	{
		TimeUntilNextReaction = ReactionTime;
	}

	LinearTargetVelocity = FVector::ZeroVector;
	AngularTargetVelocity = FVector::ZeroVector;
	WantFire = false;
	UseOrbitalBoost = false;

	if (Ship->IsMilitary())
	{
		MilitaryPilot(DeltaSeconds);
	}
	else
	{
		CargoPilot(DeltaSeconds);
	}

}

void UFlareShipPilot::Initialize(const FFlareShipPilotSave* Data, UFlareCompany* Company, AFlareSpacecraft* OwnerShip)
{
	// Main data
	Ship = OwnerShip;
	PlayerCompany = Company;

	// Setup properties
	if (Data)
	{
		ShipPilotData = *Data;
	}
	AttackAngle = FMath::FRandRange(0, 360);
}

/*----------------------------------------------------
	Pilot functions
----------------------------------------------------*/
void UFlareShipPilot::MilitaryPilot(float DeltaSeconds)
{
//	FLOGV("%s MilitaryPilot",  *Ship->GetImmatriculation());

	if (Ship->GetNavigationSystem()->GetStatus() == EFlareShipStatus::SS_Docked)
	{
		// Let's undock
		Ship->GetNavigationSystem()->Undock();
		return;
	}
	else if (Ship->GetNavigationSystem()->GetStatus() == EFlareShipStatus::SS_AutoPilot)
	{
		// Wait manoeuver
		return;
	}

	// Ship is hs, go repair and refill
	if (Ship->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon, false, true) <= 0)
	{
		// Go repair or refill ammo
		AFlareSpacecraft* TargetStation  = GetNearestAvailableStation();
		if (TargetStation)
		{
			if (Ship->GetNavigationSystem()->DockAt(TargetStation))
			{
				PilotTargetShip = NULL;
				// Ok let dock
				return;
			}
		}
	}


	if (!PilotTargetShip // No target
			|| !PilotTargetShip->GetDamageSystem()->IsAlive() // Target dead
			|| (PilotTargetShip->GetActorLocation() - Ship->GetActorLocation()).Size() > MaxFollowDistance * 100 // Target too far
			|| (Ship->GetSize() == EFlarePartSize::S && SelectedWeaponGroupIndex == -1)  // No selected weapon
			|| (Ship->GetSize() == EFlarePartSize::S && !LockTarget && Ship->GetDamageSystem()->GetWeaponGroupHealth(SelectedWeaponGroupIndex, false, true) <=0))  // Selected weapon not usable
	{
//		if (PilotTargetShip == NULL)
//		{
//			FLOG("Switch target because no current target");
//		}
//		else if (!PilotTargetShip->GetDamageSystem()->IsAlive())
//		{
//			FLOG("Switch target because current target is dead");
//		}
//		else if ((PilotTargetShip->GetActorLocation() - Ship->GetActorLocation()).Size() > MaxFollowDistance * 100)
//		{
//			FLOGV("Switch target because current target is too far %f > %f", (PilotTargetShip->GetActorLocation() - Ship->GetActorLocation()).Size(), (MaxFollowDistance * 100));
//		}
//		else if (SelectedWeaponGroupIndex == -1)
//		{
//			FLOG("Switch target because no selected weapon");
//		}
//		else if (( !LockTarget && Ship->GetDamageSystem()->GetWeaponGroupHealth(SelectedWeaponGroupIndex, false, true) <=0))
//		{
//			FLOG("Switch target because selected weapon is not usable");
//		}

		PilotTargetShip = NULL;
		SelectedWeaponGroupIndex = -1;
		FindBestHostileTarget();
	}

//	FLOGV("%s Target %x",  *Ship->GetImmatriculation(), PilotTargetShip);
	bool Idle = true;

	TimeUntilNextComponentSwitch-=ReactionTime;


	if (Ship->GetSize() == EFlarePartSize::S && PilotTargetShip && SelectedWeaponGroupIndex >= 0)
	{
		if (TimeUntilNextComponentSwitch <= 0 && !LockTarget)
		{
			//FLOGV("%s Switch because of timeout", *Ship->GetImmatriculation());
			PilotTargetComponent = NULL;
		}
		else if (PilotTargetComponent)
		{
			if (PilotTargetComponent->GetSpacecraft() != PilotTargetShip)
			{
				//FLOGV("%s Switch because the component %s is not in the target ship", *Ship->GetImmatriculation(), *PilotTargetComponent->GetReadableName());
				PilotTargetComponent = NULL;
			}
			else if (PilotTargetComponent->GetDamageRatio() <=0)
			{
				//FLOGV("%s Switch because the component %s is destroyed", *Ship->GetImmatriculation(), *PilotTargetComponent->GetReadableName());
				PilotTargetComponent = NULL;
			}
		}

		if (!PilotTargetComponent)
		{
			PilotTargetComponent = GetRandomTargetComponent(PilotTargetShip);
			TimeUntilNextComponentSwitch = 5;
			//FLOGV("%s Select new target component %s ", *Ship->GetImmatriculation(), *PilotTargetComponent->GetReadableName());
		}


		//FLOGV("%s target %s",  *Ship->GetImmatriculation(),  *PilotTargetShip->GetImmatriculation());
		EFlareWeaponGroupType::Type WeaponType = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Type;
		//FLOGV("%s WeaponType %d",  *Ship->GetImmatriculation(), (WeaponType - EFlareWeaponGroupType::WG_NONE));
		if (WeaponType == EFlareWeaponGroupType::WG_GUN)
		{
			FighterPilot(DeltaSeconds);
			Idle = false;
		}
		else if (WeaponType == EFlareWeaponGroupType::WG_BOMB)
		{
			BomberPilot(DeltaSeconds);
			Idle = false;
		}	
	}
	else if (Ship->GetSize() == EFlarePartSize::L && PilotTargetShip)
	{
		FlagShipPilot(DeltaSeconds);
		Idle = false;
	}


	if (Idle)
	{
		IdlePilot(DeltaSeconds);
	}


	// TODO S or L ship dispatch


}

void UFlareShipPilot::CargoPilot(float DeltaSeconds)
{

	if (Ship->GetNavigationSystem()->GetStatus() == EFlareShipStatus::SS_Docked)
	{
		if (WaitTime < 10)
		{
			WaitTime += ReactionTime;
		}
		else
		{
			// Let's undock
			Ship->GetNavigationSystem()->Undock();

			// Swap target station
			PilotLastTargetStation = PilotTargetStation;
			PilotTargetStation = NULL;
			WaitTime = 0;
		}

		return;
	}
	else if (Ship->GetNavigationSystem()->GetStatus() == EFlareShipStatus::SS_AutoPilot)
	{
		// Wait manoeuver
		return;
	} else {
		// If no station target, find a target : a random friendly station different from the last station
		if (!PilotTargetStation)
		{
			TArray<AFlareSpacecraft*> FriendlyStations = GetFriendlyStations();
			if (FriendlyStations.Num() > 0)
			{
				int32 Index = FMath::RandHelper(FriendlyStations.Num());

				if (PilotLastTargetStation != FriendlyStations[Index])
				{
						PilotTargetStation = FriendlyStations[Index];
				}
			}
		}

		if (PilotTargetStation)
		{
			FVector DeltaLocation = (PilotTargetStation->GetActorLocation() - Ship->GetActorLocation()) / 100.f;
			float Distance = DeltaLocation.Size(); // Distance in meters


			if (Distance < 1000)
			{
				if (!Ship->GetNavigationSystem()->DockAt(PilotTargetStation))
				{
					LinearTargetVelocity = -DeltaLocation.GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity();
				}
			}
			else
			{
				LinearTargetVelocity = DeltaLocation.GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity();
			}
		}
	}

	PilotTargetShip = GetNearestHostileShip(true, EFlarePartSize::S);
	if (!PilotTargetShip)
	{
		PilotTargetShip = GetNearestHostileShip(true, EFlarePartSize::L);
	}

	// If enemy near, run away !
	if (PilotTargetShip)
	{

		FVector DeltaLocation = (PilotTargetShip->GetActorLocation() - Ship->GetActorLocation()) / 100.f;
		float Distance = DeltaLocation.Size(); // Distance in meters

		// There is at least one hostile enemy
		if (Distance < 4000)
		{
			Ship->ForceManual(); // TODO make independant command channel
			LinearTargetVelocity = -DeltaLocation.GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity();

			UseOrbitalBoost = true;
		}

		if (Distance > 1000 && Ship->GetDamageSystem()->GetTemperature() > Ship->GetDamageSystem()->GetOverheatTemperature() * 0.95)
		{
			// Too hot and no imminent danger
			UseOrbitalBoost = false;
		}
	}

	// Anticollision
	LinearTargetVelocity = PilotHelper::AnticollisionCorrection(Ship, LinearTargetVelocity);

	// Turn to destination
	if (! LinearTargetVelocity.IsZero())
	{
		AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1.f, 0.f, 0.f) , LinearTargetVelocity.GetUnsafeNormal(),FVector(0.f, 0.f, 0.f), DeltaSeconds);
	}
}

void UFlareShipPilot::FighterPilot(float DeltaSeconds)
{
	float AmmoVelocity = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Weapons[0]->GetAmmoVelocity() * 100;
	//FLOGV("%s FighterPilot AmmoVelocity %f",  *Ship->GetImmatriculation(), AmmoVelocity);

	bool DangerousTarget = IsShipDangerous(PilotTargetShip);

	//FLOGV("%s target %s",  *Ship->GetHumanReadableName(),  *PilotTargetShip->GetHumanReadableName());
	// The pilot have a target, track and kill it

	FVector LocalNose = FVector(1.f, 0.f, 0.f);
	FVector DeltaLocation = (PilotTargetComponent->GetComponentLocation() - Ship->GetActorLocation()) / 100.f;
	float Distance = DeltaLocation.Size(); // Distance in meters
	float TargetSize = PilotTargetShip->GetMeshScale() / 100.f; // Radius in meters
	FVector TargetAxis = DeltaLocation.GetUnsafeNormal();
	FVector ShipVelocity = 100 * Ship->GetLinearVelocity();
	FVector PilotTargetShipVelocity = 100 * PilotTargetShip->GetLinearVelocity();

	// Use position prediction
	float PredictionDelay = ReactionTime - DeltaSeconds;
	FVector PredictedShipLocation = Ship->GetActorLocation() + ShipVelocity * PredictionDelay;
	FVector PredictedPilotTargetShipLocation = PilotTargetComponent->GetComponentLocation() + PilotTargetShipVelocity * PredictionDelay;
	FVector PredictedDeltaLocation = (PredictedPilotTargetShipLocation - PredictedShipLocation) / 100.f;
	FVector PredictedTargetAxis = PredictedDeltaLocation.GetUnsafeNormal();
	float PredictedDistance = PredictedDeltaLocation.Size(); // Distance in meters

	FVector AmmoIntersectionLocation;
	float AmmoIntersectionTime = SpacecraftHelper::GetIntersectionPosition(PilotTargetComponent->GetComponentLocation(), PilotTargetShip->Airframe->GetPhysicsLinearVelocity(), Ship->GetActorLocation(), ShipVelocity, AmmoVelocity, 0, &AmmoIntersectionLocation);

	FVector FireTargetAxis;
	if (AmmoIntersectionTime > 0)
	{
		FireTargetAxis = (AmmoIntersectionLocation - Ship->GetActorLocation()).GetUnsafeNormal();
	}
	else
	{
		FireTargetAxis = (PilotTargetComponent->GetComponentLocation() - Ship->GetActorLocation()).GetUnsafeNormal();
	}


	FVector AmmoIntersectionPredictedLocation;
	float AmmoIntersectionPredictedTime = SpacecraftHelper::GetIntersectionPosition(PilotTargetComponent->GetComponentLocation(), PilotTargetShip->Airframe->GetPhysicsLinearVelocity(), Ship->GetActorLocation(), ShipVelocity, AmmoVelocity, PredictionDelay, &AmmoIntersectionPredictedLocation);
	FVector PredictedFireTargetAxis;
	if (AmmoIntersectionPredictedTime > 0)
	{
		PredictedFireTargetAxis = (AmmoIntersectionPredictedLocation - PredictedShipLocation).GetUnsafeNormal();
	}
	else
	{
		PredictedFireTargetAxis = (PredictedDeltaLocation* 100.f - PredictedShipLocation).GetUnsafeNormal();
	}

	FRotator ShipAttitude = Ship->GetActorRotation();


	// Bullet velocity
	FVector BulletVelocity = ShipAttitude.Vector();
	BulletVelocity.Normalize();
	BulletVelocity *= AmmoVelocity;

	FVector BulletDirection = Ship->Airframe->GetComponentToWorld().GetRotation().Inverse().RotateVector((ShipVelocity + BulletVelocity)).GetUnsafeNormal();


	FVector DeltaVelocity = PilotTargetShip->GetLinearVelocity() - ShipVelocity / 100.;

	FVector PredictedTargetAngularVelocity = - 180 / (PI * PredictedDistance) * FVector::CrossProduct(DeltaVelocity, PredictedTargetAxis);

	//TargetAngularVelocity = FVector(0,0,0);

	// First allow align nose to target bullet interception point
	// TODO Use BulletDirection instead of LocalNose
	//AngularTargetVelocity = GetAngularVelocityToAlignAxis(LocalNose, FireTargetAxis, DeltaSeconds);
	//TODO find target angular velocity

	AngularTargetVelocity = GetAngularVelocityToAlignAxis(BulletDirection, PredictedFireTargetAxis, PredictedTargetAngularVelocity, DeltaSeconds);

	/*FLOGV("Distance=%f", Distance);
	FLOGV("PilotTargetShip->GetLinearVelocity()=%s", *(PilotTargetShip->GetLinearVelocity().ToString()));
	FLOGV("TargetAxis=%s", *TargetAxis.ToString());
	FLOGV("TargetAngularVelocity=%s", *TargetAngularVelocity.ToString());
	FLOGV("AngularTargetVelocity=%s", *AngularTargetVelocity.ToString());
	FLOGV("Ship->Airframe->GetPhysicsAngularVelocity()=%s", *(Ship->Airframe->GetPhysicsAngularVelocity().ToString()));
*/
	/*FLOGV("DeltaLocation=%s", *DeltaLocation.ToString());
	FLOGV("TargetAxis=%s", *TargetAxis.ToString());
	FLOGV("FireTargetAxis=%s", *FireTargetAxis.ToString());
	FLOGV("BulletVelocity=%s", *BulletVelocity.ToString());
	FLOGV("BulletDirection=%s", *BulletDirection.ToString());

*/

	// Attack Phases
	// 0 - Prepare attack : change velocity to approch the target
	// 1 - Attacking : target is approching
	// 2 - Withdraw : target is passed, wait a security distance to attack again
	float SecurityDistance = (DangerousTarget ? 1200: 800) + TargetSize * 4;
	bool ClearTarget = false;
	if (AttackPhase == 0)
	{
		if (FVector::DotProduct(DeltaLocation, DeltaVelocity) < 0)
		{
			// Target is approching, prepare attack
			AttackPhase = 1;
			LastTargetDistance = Distance;
		}
		else
		{
			LinearTargetVelocity = PredictedFireTargetAxis * Ship->GetNavigationSystem()->GetLinearMaxVelocity();
			UseOrbitalBoost = true;
		}

		if (Distance < SecurityDistance)
		{
			AttackPhase = 1;
		}

	}

	if (AttackPhase == 1)
	{
		if (LastTargetDistance < Distance) // TODO : check if I can't use dot product
		{
			// Target is passed
			AttackPhase = 2;
		}
		else
		{
			FQuat AttackDistanceQuat = FQuat(TargetAxis, AttackAngle);
			FVector TopVector = Ship->GetActorRotation().RotateVector(FVector(0,0,AttackDistance));
			FVector AttackMargin =  AttackDistanceQuat.RotateVector(TopVector);


			LinearTargetVelocity = (AttackMargin + DeltaLocation).GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity();

			if (Distance > SecurityDistance || DangerousTarget)
			{
				UseOrbitalBoost = true;
			}
		}

		LastTargetDistance = Distance;
	}

	if (AttackPhase == 2)
	{
		if (Distance > SecurityDistance)
		{
			// Security distance reach
			AttackPhase = 0;
			ClearTarget = true;
		}
		else
		{
			LinearTargetVelocity = -DeltaLocation.GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity();
			if (DangerousTarget)
			{
				UseOrbitalBoost = true;
			}
		}
	}

	// If at range and aligned fire on the target
	//TODO increase tolerance if target is near
	if (AmmoIntersectionTime > 0 && AmmoIntersectionTime < 1.5)
	{
		FVector FireAxis = Ship->Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalNose);
		TArray <UFlareWeapon*> Weapons = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Weapons;
		for (int WeaponIndex = 0; WeaponIndex < Weapons.Num(); WeaponIndex++)
		{
			UFlareWeapon* Weapon = Weapons[WeaponIndex];
			if (Weapon->GetUsableRatio() <= 0)
			{
				continue;
			}

			for (int GunIndex = 0; GunIndex < Weapon->GetGunCount(); GunIndex++)
			{
				FVector MuzzleLocation = Weapon->GetMuzzleLocation(GunIndex);

				// Compute target Axis for each gun
				FVector GunAmmoIntersectionLocation;
				float GunAmmoIntersectionTime = SpacecraftHelper::GetIntersectionPosition(PilotTargetComponent->GetComponentLocation(), PilotTargetShip->Airframe->GetPhysicsLinearVelocity(), MuzzleLocation, ShipVelocity, AmmoVelocity, 0, &GunAmmoIntersectionLocation);
				if (GunAmmoIntersectionTime < 0)
				{
					// No ammo intersection, don't fire
					continue;
				}
				FVector GunFireTargetAxis = (GunAmmoIntersectionLocation - MuzzleLocation - AmmoIntersectionPredictedTime * ShipVelocity).GetUnsafeNormal();
				/*FLOGV("Gun %d FireAxis=%s", GunIndex, *FireAxis.ToString());
				FLOGV("Gun %d GunFireTargetAxis=%s", GunIndex, *GunFireTargetAxis.ToString());
	*/
				float AngularPrecisionDot = FVector::DotProduct(GunFireTargetAxis, FireAxis);
				float AngularPrecision = FMath::Acos(AngularPrecisionDot);
				float AngularSize = FMath::Atan(TargetSize / Distance);

			/*	FLOGV("Gun %d Distance=%f", GunIndex, Distance);
				FLOGV("Gun %d TargetSize=%f", GunIndex, TargetSize);
				FLOGV("Gun %d AngularSize=%f", GunIndex, AngularSize);
				FLOGV("Gun %d AngularPrecision=%f", GunIndex, AngularPrecision);*/
				if (AngularPrecision < (DangerousTarget ? AngularSize * 0.25 : AngularSize * 0.2))
				{
					if(!PilotHelper::CheckFriendlyFire(Ship->GetWorld(), PlayerCompany, MuzzleLocation, ShipVelocity, AmmoVelocity, GunFireTargetAxis, GunAmmoIntersectionTime, 0))
					{
						Weapon->SetTarget(PilotTargetShip);
						/*FLOG("Want Fire");*/
						WantFire = true;
						break;
					}
					else
					{
						FLOG("Friendly fire avoidance");
					}
				}
			}
			if (WantFire)
			{
				break;
			}
		}
	}

	if (Ship->GetDamageSystem()->GetTemperature() > Ship->GetDamageSystem()->GetOverheatTemperature() * (DangerousTarget ? 1.1f : 0.90f))
	{
		// TODO Fire on dangerous target
		WantFire = false;
	}


	// Anticollision
	LinearTargetVelocity = PilotHelper::AnticollisionCorrection(Ship, LinearTargetVelocity);



	// Find friend barycenter
	// Go to friend barycenter
	// If near
		// Turn to opposite from barycentre
	// else
		// Turn to direction


	// Manage orbital boost
	if (Ship->GetDamageSystem()->GetTemperature() > Ship->GetDamageSystem()->GetOverheatTemperature() * 0.75)
	{
		UseOrbitalBoost = false;
	}

	if (ClearTarget)
	{
		PilotTargetShip = NULL;
	}
}

void UFlareShipPilot::BomberPilot(float DeltaSeconds)
{
	FLOGV("%s BomberPilot",  *Ship->GetImmatriculation());
	// TODO

	FLOGV("%s GetLinearVelocity %s",  *Ship->GetImmatriculation(), *(Ship->GetLinearVelocity()).ToString());

	//DrawDebugLine(Ship->GetWorld(), Ship->GetActorLocation(), Ship->GetActorLocation() + Ship->GetLinearVelocity() * 100, FColor::Green, false, ReactionTime);




	LinearTargetVelocity = FVector::ZeroVector;


	FVector DeltaLocation = (PilotTargetComponent->GetComponentLocation() - Ship->GetActorLocation()) / 100.f;
	FVector TargetAxis = DeltaLocation.GetUnsafeNormal();
	float Distance = DeltaLocation.Size(); // Distance in meters

	//FLOGV("%s DeltaLocation %s",  *Ship->GetImmatriculation(), *DeltaLocation.ToString());
	//FLOGV("%s TargetAxis %s",  *Ship->GetImmatriculation(), *TargetAxis.ToString());
	//FLOGV("%s Distance %f",  *Ship->GetImmatriculation(), Distance);

	// Attack Phases
	// 0 - Prepare attack : change velocity to approch the target
	// 1 - Attacking : target is approching with boost
	// 3 - Drop : Drop util its not safe to stay
	// 2 - Withdraw : target is passed, wait a security distance to attack again

	float WeigthCoef = FMath::Sqrt(Ship->Airframe->GetMass()) / FMath::Sqrt(5425.f) * (2-Ship->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_RCS)) ; // 1 for ghoul at 100%

	float ChargeDistance = 15 * Ship->GetNavigationSystem()->GetLinearMaxVelocity() * WeigthCoef ;
	float AlignTime = 12 * WeigthCoef;
	float DropTime = 5 * WeigthCoef ;
	float EvadeTime = 2.5 * WeigthCoef;
	float TimeBetweenDrop = 0.50 * WeigthCoef;
	float SecurityDistance = 1500;
	UseOrbitalBoost = false;
	bool ClearTarget = false;
	bool AlignToSpeed = false;
	bool HardBoost = false;
	bool Anticollision = true;

	//FLOGV("%s AttackPhase %d",  *Ship->GetImmatriculation(), AttackPhase);
	if (AttackPhase == 0)
	{
		if (Distance < ChargeDistance)
		{
			//FLOGV("%s Distance < ChargeDistance => phase 1",  *Ship->GetImmatriculation());
			// Target is approching, prepare attack
			AttackPhase = 1;
			LockTarget = true;
		}
		else
		{
			LinearTargetVelocity = TargetAxis * Ship->GetNavigationSystem()->GetLinearMaxVelocity();
			AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), TargetAxis, FVector::ZeroVector, DeltaSeconds);
			UseOrbitalBoost = true;
			FLOGV("%s Goto target %s",  *Ship->GetImmatriculation(), *LinearTargetVelocity.ToString());
		}
	}

	if (AttackPhase == 1)
	{
		FVector AmmoIntersectionLocation;
		float AmmoVelocity = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Weapons[0]->GetAmmoVelocity() * 100;
		float AmmoIntersectionTime = SpacecraftHelper::GetIntersectionPosition(PilotTargetComponent->GetComponentLocation(), PilotTargetShip->Airframe->GetPhysicsLinearVelocity(), Ship->GetActorLocation(), Ship->Airframe->GetPhysicsLinearVelocity(), AmmoVelocity, 0.0, &AmmoIntersectionLocation);

//		FLOGV("%s AmmoIntersectionLocation %s",  *Ship->GetImmatriculation(), *AmmoIntersectionLocation.ToString());
//		FLOGV("%s AmmoIntersectionTime %f",  *Ship->GetImmatriculation(), AmmoIntersectionTime);
//		FLOGV("%s AmmoVelocity %f",  *Ship->GetImmatriculation(), AmmoVelocity);

		//DrawDebugLine(Ship->GetWorld(), Ship->GetActorLocation(), AmmoIntersectionLocation, FColor::Blue, false, ReactionTime);

		AlignToSpeed = true;

		if (AmmoIntersectionTime > 0 && AmmoIntersectionTime < DropTime)
		{
//			FLOGV("%s AmmoIntersectionTime < DropTime => phase 2",  *Ship->GetImmatriculation());
			// Near enougt
			AttackPhase = 2;
			LastWantFire = false;
			TimeBeforeNextDrop = 0;
		}
		else if (AmmoIntersectionTime > 0 && AmmoIntersectionTime < AlignTime)
		{
			FVector ChargeAxis = (AmmoIntersectionLocation - Ship->GetActorLocation()).GetUnsafeNormal();
			LinearTargetVelocity = ChargeAxis * Ship->GetNavigationSystem()->GetLinearMaxVelocity();
			UseOrbitalBoost = true;
			HardBoost = true;
			Anticollision = false;
//			FLOGV("%s ChargeAxis %s",  *Ship->GetImmatriculation(), *ChargeAxis.ToString());
//			FLOGV("%s Charge target %s",  *Ship->GetImmatriculation(), *LinearTargetVelocity.ToString());
		}
		else
		{
			LinearTargetVelocity = TargetAxis * Ship->GetNavigationSystem()->GetLinearMaxVelocity();

//			FLOGV("%s Goto target %s",  *Ship->GetImmatriculation(), *LinearTargetVelocity.ToString());
		}

		LastTargetDistance = Distance;
	}

	if (AttackPhase == 2)
	{
		FVector AmmoIntersectionLocation;
		float AmmoVelocity = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Weapons[0]->GetAmmoVelocity() * 100;
		float AmmoIntersectionTime = SpacecraftHelper::GetIntersectionPosition(PilotTargetComponent->GetComponentLocation(), PilotTargetShip->Airframe->GetPhysicsLinearVelocity(), Ship->GetActorLocation(), Ship->Airframe->GetPhysicsLinearVelocity(), AmmoVelocity, 0.0, &AmmoIntersectionLocation);
		FVector FrontVector = Ship->GetActorRotation().RotateVector(FVector(1,0,0));
		FVector ChargeAxis = (AmmoIntersectionLocation - Ship->GetActorLocation()).GetUnsafeNormal();
		//DrawDebugLine(Ship->GetWorld(), Ship->GetActorLocation(), AmmoIntersectionLocation, FColor::Blue, false, ReactionTime);

		Anticollision = false;
		AlignToSpeed = true;

//		FLOGV("%s AmmoIntersectionLocation %s",  *Ship->GetImmatriculation(), *AmmoIntersectionLocation.ToString());
//		FLOGV("%s AmmoIntersectionTime %f",  *Ship->GetImmatriculation(), AmmoIntersectionTime);
//		FLOGV("%s AmmoVelocity %f",  *Ship->GetImmatriculation(), AmmoVelocity);


		if (AmmoIntersectionTime < EvadeTime || FVector::DotProduct(FrontVector, ChargeAxis) < 0.6 || AmmoIntersectionTime > AlignTime)
		{
			// Security distance reach
//			FLOGV("%s AmmoIntersectionTime < EvadeTime => phase3",  *Ship->GetImmatriculation());
			AttackPhase = 3;
		}
		else if (FVector::DotProduct(FrontVector, ChargeAxis) > 0.9 && AmmoIntersectionTime < DropTime)
		{
//			FLOGV("%s TimeBeforeNextDrop %f", *Ship->GetImmatriculation(), TimeBeforeNextDrop);
			if (TimeBeforeNextDrop > 0)
			{
				TimeBeforeNextDrop -= ReactionTime;
//				FLOGV("%s Reduce time to  %f",*Ship->GetImmatriculation(),  TimeBeforeNextDrop);
			}
			else
			{

				WantFire = !LastWantFire;
//				FLOGV("%s WantFire=%d LastWantFire=%d",*Ship->GetImmatriculation(),  WantFire, LastWantFire);
				LastWantFire = WantFire;
				if (WantFire)
				{
					TimeBeforeNextDrop = TimeBetweenDrop;
				}
			}


//			FLOGV("%s WantFire %d",  *Ship->GetImmatriculation(), WantFire);

			LinearTargetVelocity = ChargeAxis * Ship->GetNavigationSystem()->GetLinearMaxVelocity();
//			FLOGV("%s ChargeAxis %s",  *Ship->GetImmatriculation(), *ChargeAxis.ToString());
//			FLOGV("%s Charge target %s",  *Ship->GetImmatriculation(), *LinearTargetVelocity.ToString());
		}
	}

	if (AttackPhase == 3)
	{
		FVector DeltaVelocity = (PilotTargetShip->GetLinearVelocity() - Ship->GetLinearVelocity()) / 100.;
//		FLOGV("%s DeltaVelocity %s",  *Ship->GetImmatriculation(), *DeltaVelocity.ToString());
		if (Distance > SecurityDistance)
		{
//			FLOGV("%s Distance > SecurityDistance => phase0",  *Ship->GetImmatriculation());
			// Security distance reach
			AttackPhase = 0;
			ClearTarget = true;
			LockTarget = false;
		}
		else if (FVector::DotProduct(DeltaLocation, DeltaVelocity) < 0)
		{
			AlignToSpeed = true;
			FQuat AvoidQuat = FQuat(DeltaLocation.GetUnsafeNormal(), AttackAngle);
			FVector TopVector = Ship->GetActorRotation().RotateVector(FVector(0,0,PilotTargetShip->GetMeshScale()));
			FVector Avoid =  AvoidQuat.RotateVector(TopVector);

			LinearTargetVelocity = Avoid.GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity();
//			FLOGV("%s AttackAngle %f",  *Ship->GetImmatriculation(), AttackAngle);
//			FLOGV("%s TopVector %s",  *Ship->GetImmatriculation(), *TopVector.ToString());
//			FLOGV("%s Avoid %s",  *Ship->GetImmatriculation(), *Avoid.ToString());
//			FLOGV("%s Escape target %s",  *Ship->GetImmatriculation(), *LinearTargetVelocity.ToString());
			UseOrbitalBoost = true;
			HardBoost = true;
		}
		else
		{
			UseOrbitalBoost = true;
			HardBoost = true;
			LinearTargetVelocity = -TargetAxis * Ship->GetNavigationSystem()->GetLinearMaxVelocity() * 2;
			AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), -TargetAxis, FVector::ZeroVector, DeltaSeconds);
//			FLOGV("%s Run from target %s",  *Ship->GetImmatriculation(), *LinearTargetVelocity.ToString());
		}
	}

	// Anticollision
	if (Anticollision)
	{
		LinearTargetVelocity = PilotHelper::AnticollisionCorrection(Ship, LinearTargetVelocity, PilotTargetShip);
	}

	//DrawDebugLine(Ship->GetWorld(), Ship->GetActorLocation(), Ship->GetActorLocation() + LinearTargetVelocity * 100, FColor::Red, false, ReactionTime);

	if (AlignToSpeed)
	{
		if (Ship->GetLinearVelocity().IsNearlyZero())
		{
			AngularTargetVelocity = FVector::ZeroVector;
		}
		else
		{
			FVector LinearVelocityAxis = Ship->GetLinearVelocity().GetUnsafeNormal();
//			FLOGV("%s LinearVelocityAxis %s",  *Ship->GetImmatriculation(), *LinearVelocityAxis.ToString());
			AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), LinearVelocityAxis, FVector::ZeroVector, DeltaSeconds);
//			FLOGV("%s AngularTargetVelocity %s",  *Ship->GetImmatriculation(), *AngularTargetVelocity.ToString());
		}
	}
	else
	{
		if (LinearTargetVelocity.IsNearlyZero())
		{
			AngularTargetVelocity = FVector::ZeroVector;
		}
		else
		{
			FVector LinearTargetVelocityAxis = LinearTargetVelocity.GetUnsafeNormal();
//			FLOGV("%s LinearTargetVelocityAxis %s",  *Ship->GetImmatriculation(), *LinearTargetVelocityAxis.ToString());
			AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), LinearTargetVelocityAxis, FVector::ZeroVector, DeltaSeconds);
//			FLOGV("%s AngularTargetVelocity %s",  *Ship->GetImmatriculation(), *AngularTargetVelocity.ToString());
		}
	}

	if (ClearTarget)
	{
		PilotTargetShip = NULL;
	}
	// TODO ignore target


	// Manage orbital boost
	if (Ship->GetDamageSystem()->GetTemperature() > Ship->GetDamageSystem()->GetOverheatTemperature() * 0.75)
	{
		UseOrbitalBoost = false;
	}


	// Manage orbital boost

}

void UFlareShipPilot::IdlePilot(float DeltaSeconds)
{
	//FLOGV("%s IdlePilot",  *Ship->GetImmatriculation());
	// TODO find better
	UseOrbitalBoost = false;

	// If there is ennemy fly away
	PilotTargetShip = GetNearestHostileShip(true, EFlarePartSize::S);
	if (!PilotTargetShip)
	{
		PilotTargetShip = GetNearestHostileShip(true, EFlarePartSize::L);
	}

	// If enemy near, run away !
	if (PilotTargetShip)
	{

		FVector DeltaLocation = (PilotTargetShip->GetActorLocation() - Ship->GetActorLocation()) / 100.f;
		float Distance = DeltaLocation.Size(); // Distance in meters

		// There is at least one hostile enemy
		if (Distance < 10000)
		{
			Ship->ForceManual(); // TODO make independant command channel
			LinearTargetVelocity = -DeltaLocation.GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity();

			UseOrbitalBoost = true;
		}

		if (Distance > 2000 && Ship->GetDamageSystem()->GetTemperature() > Ship->GetDamageSystem()->GetOverheatTemperature() * 0.95)
		{
			// Too hot and no imminent danger
			UseOrbitalBoost = false;
		}
	}
	else
	{
		// If not, find a leader
		AFlareSpacecraft* LeaderShip = Ship;

		for(int ShipIndex = 0; ShipIndex < Ship->GetCompany()->GetCompanyShips().Num() ; ShipIndex++)
		{
			AFlareSpacecraft* CandidateShip = Cast<AFlareSpacecraft>(Ship->GetCompany()->GetCompanyShips()[ShipIndex]);

			if(!CandidateShip || Ship == CandidateShip)
			{
				continue;
			}

			if(LeaderShip->Airframe->GetMass() == CandidateShip->Airframe->GetMass())
			{
				if(LeaderShip->GetImmatriculation() > CandidateShip->GetImmatriculation())
				{
					LeaderShip = CandidateShip;
					continue;
				}
			}



			else if(CandidateShip->GetSize() == EFlarePartSize::L)
			{
				LeaderShip = CandidateShip;
				continue;
			}
		}

		// If is the leader, find a location in a 10 km radius and patrol
		if(LeaderShip == Ship)
		{
			// Go to a random point at 10000 m from the center

			// If at less than 500 m from this point, get another random point

			float TargetLocationToShipDistance = (PilotTargetLocation - Ship->GetActorLocation()).Size();

			if (TargetLocationToShipDistance < 50000 || PilotTargetLocation.IsZero())
			{
				PilotTargetLocation = FMath::VRand() * FMath::FRand() * 400000;
			}
			LinearTargetVelocity = (PilotTargetLocation - Ship->GetActorLocation()).GetUnsafeNormal()  * Ship->GetNavigationSystem()->GetLinearMaxVelocity() * 0.8;

		}
		else
		{
			// Follow the leader
			float FollowRadius = 50000 + FMath::Pow(Ship->GetCompany()->GetCompanyShips().Num() * 3 * FMath::Pow(20000, 3),1/3.);
			if((LeaderShip->GetActorLocation() - Ship->GetActorLocation()).Size() < FollowRadius)
			{
				LinearTargetVelocity = LeaderShip->GetLinearVelocity();
			}
			else
			{
				LinearTargetVelocity = (LeaderShip->GetActorLocation() - Ship->GetActorLocation()).GetUnsafeNormal()  * Ship->GetNavigationSystem()->GetLinearMaxVelocity();

			}
		}

	}

	AngularTargetVelocity = FVector::ZeroVector;




	// Turn to destination
	if (! LinearTargetVelocity.IsZero())
	{
		AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), LinearTargetVelocity, FVector::ZeroVector, DeltaSeconds);
	}

	// Anticollision
	LinearTargetVelocity = PilotHelper::AnticollisionCorrection(Ship, LinearTargetVelocity);


}

void UFlareShipPilot::FlagShipPilot(float DeltaSeconds)
{
	//FLOGV("%s FlagShipPilot",  *Ship->GetImmatriculation());

	// Go to a random point at 1000 m from the target

	// If at less than 50 m from this point, get another random point

	// If the target to farther than 2000 to the target point, change point

	float TargetLocationToTargetShipDistance = (PilotTargetLocation - PilotTargetShip->GetActorLocation()).Size();
	float TargetLocationToShipDistance = (PilotTargetLocation - Ship->GetActorLocation()).Size();

	//FLOGV("%s FlagShipPilot PilotTargetLocation %s",  *Ship->GetImmatriculation(), *PilotTargetLocation.ToString());
	//FLOGV("%s FlagShipPilot TargetLocationToTargetShipDistance %f",  *Ship->GetImmatriculation(), TargetLocationToTargetShipDistance);
	//FLOGV("%s FlagShipPilot TargetLocationToShipDistance %f",  *Ship->GetImmatriculation(), TargetLocationToShipDistance);

	bool NewTargetLocation = false;
	if (TargetLocationToTargetShipDistance > 200000)
	{
		// Target location too far from target ship
		NewTargetLocation = true;
	}
	else if (TargetLocationToShipDistance < 50000)
	{
		// Near to target location
		NewTargetLocation = true;
	}


	//FLOGV("%s FlagShipPilot NewTargetLocation %d",  *Ship->GetImmatriculation(), NewTargetLocation);
	if (NewTargetLocation || PilotTargetLocation.IsZero())
	{

		PilotTargetLocation = FMath::VRand() * FMath::FRand() * 100000 + PilotTargetShip->GetActorLocation();
		//FLOGV("%s FlagShipPilot NewTargetLocation %d",  *Ship->GetImmatriculation(), NewTargetLocation);
	}


	AngularTargetVelocity = FVector::ZeroVector;
	LinearTargetVelocity = (PilotTargetLocation - Ship->GetActorLocation()).GetUnsafeNormal()  * Ship->GetNavigationSystem()->GetLinearMaxVelocity();

	// TODO Bomb avoid

	// Anticollision
	LinearTargetVelocity = PilotHelper::AnticollisionCorrection(Ship, LinearTargetVelocity);


	AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), LinearTargetVelocity, FVector::ZeroVector, DeltaSeconds);


	//FLOGV("%s FlagShipPilot LinearTargetVelocity %s",  *Ship->GetImmatriculation(), *LinearTargetVelocity.ToString());


	FVector FrontAxis = Ship->Airframe->GetComponentToWorld().GetRotation().RotateVector(FVector(1,0,0));

	//FLOGV("%s FlagShipPilot FVector::DotProduct(FrontAxis, LinearTargetVelocity.GetUnsafeNormal()) %f",  *Ship->GetImmatriculation(), FVector::DotProduct(FrontAxis, LinearTargetVelocity.GetUnsafeNormal()));


	if (FVector::DotProduct(FrontAxis, LinearTargetVelocity.GetUnsafeNormal()) > 0.9 && (LinearTargetVelocity - Ship->Airframe->GetPhysicsLinearVelocity()).Size() > 500)
	{
		UseOrbitalBoost = true;
	}




	// Find friend barycenter
	// Go to friend barycenter
	// If near
		// Turn to opposite from barycentre
	// else
		// Turn to direction

	WantFire = false;
	// Manage orbital boost
	if (Ship->GetDamageSystem()->GetTemperature() > Ship->GetDamageSystem()->GetOverheatTemperature() * 0.9)
	{
		UseOrbitalBoost = false;
	}
}



void UFlareShipPilot::FindBestHostileTarget()
{

	//FLOGV("%s FindBestHostileTarget",  *Ship->GetImmatriculation());
	// TODO S or L ship dispatch

	AFlareSpacecraft* TargetCandidate = NULL;


	// First search dangerous target

	if (Ship->GetWeaponsSystem()->HasUsableWeaponType(EFlareWeaponGroupType::WG_BOMB))
	{
//		FLOGV("%s Has Bomb",  *Ship->GetImmatriculation());
		TargetCandidate = GetNearestHostileShip(true, EFlarePartSize::L);
	}

	if (!TargetCandidate && Ship->GetWeaponsSystem()->HasUsableWeaponType(EFlareWeaponGroupType::WG_GUN))
	{
//		FLOGV("%s Has Gun",  *Ship->GetImmatriculation());
		TargetCandidate = GetNearestHostileShip(true, EFlarePartSize::S);
		if (!TargetCandidate)
		{
//			FLOGV("%s no S target search L",  *Ship->GetImmatriculation());
			TargetCandidate = GetNearestHostileShip(true, EFlarePartSize::L);
		}
	}

	if (!TargetCandidate && Ship->GetWeaponsSystem()->HasUsableWeaponType(EFlareWeaponGroupType::WG_TURRET))
	{
		//TODO if has AA turret, follow S
		//FLOGV("%s Has turret",  *Ship->GetImmatriculation());
		TargetCandidate = GetNearestHostileShip(true, EFlarePartSize::L);
		if (!TargetCandidate)
		{
			//FLOGV("%s no S target search L",  *Ship->GetImmatriculation());
			TargetCandidate = GetNearestHostileShip(true, EFlarePartSize::S);
		}
	}

	// Then search non dangerous target

	if (!TargetCandidate && Ship->GetWeaponsSystem()->HasUsableWeaponType(EFlareWeaponGroupType::WG_BOMB))
	{
		TargetCandidate = GetNearestHostileShip(false, EFlarePartSize::L);
	}

	if (!TargetCandidate && Ship->GetWeaponsSystem()->HasUsableWeaponType(EFlareWeaponGroupType::WG_GUN))
	{
		TargetCandidate = GetNearestHostileShip(false, EFlarePartSize::S);
		if (!TargetCandidate)
		{
			TargetCandidate = GetNearestHostileShip(false, EFlarePartSize::L);
		}
	}

	if (TargetCandidate)
	{
		//FLOGV("%s Candidate found %s",  *Ship->GetImmatriculation(), *TargetCandidate->GetImmatriculation());
		PilotTargetShip = TargetCandidate;
		AttackPhase = 0;
		AttackAngle = FMath::FRandRange(0, 360);
		float TargetSize = PilotTargetShip->GetMeshScale() / 100.f; // Radius in meters
		AttackDistance = FMath::FRandRange(50, 100) + TargetSize;
		MaxFollowDistance = TargetSize * 60; // Distance in meters
		LockTarget = false;

		SelectedWeaponGroupIndex = -1;
		// Find best weapon

		float BestScore = 0;

		for(int WeaponGroupIndex=0; WeaponGroupIndex < Ship->GetWeaponsSystem()->GetWeaponGroupCount(); WeaponGroupIndex++)
		{
			float Score = Ship->GetDamageSystem()->GetWeaponGroupHealth(WeaponGroupIndex, false, true);
			switch(Ship->GetWeaponsSystem()->GetWeaponGroup(WeaponGroupIndex)->Type)
			{
				case EFlareWeaponGroupType::WG_BOMB:
					Score *= (PilotTargetShip->GetSize() == EFlarePartSize::L ? 1.f : 0.f);
					break;
				case EFlareWeaponGroupType::WG_GUN:
					Score *= (PilotTargetShip->GetSize() == EFlarePartSize::L ? 0.01f : 1.f);
					break;
				default:
					Score *= 0.f;
			}

			if (Score > 0 && Score > BestScore)
			{
				SelectedWeaponGroupIndex = WeaponGroupIndex;
				BestScore = Score;
			}
		}
//		FLOGV("%s Candidate SelectedWeaponGroupIndex %d",  *Ship->GetImmatriculation(), SelectedWeaponGroupIndex);
	}
}



int32 UFlareShipPilot::GetPreferedWeaponGroup() const
{
	return SelectedWeaponGroupIndex;
}

/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

AFlareSpacecraft* UFlareShipPilot::GetNearestHostileShip(bool DangerousOnly, EFlarePartSize::Type Size) const
{
	// For now an host ship is a the nearest host ship with the following critera:
	// - Alive
	// - Is dangerous if needed
	// - From another company
	// - Is the nearest

	FVector PilotLocation = Ship->GetActorLocation();
	float MinDistanceSquared = -1;
	AFlareSpacecraft* NearestHostileShip = NULL;

	for (TActorIterator<AActor> ActorItr(Ship->GetWorld()); ActorItr; ++ActorItr)
	{
		// Ship
		AFlareSpacecraft* ShipCandidate = Cast<AFlareSpacecraft>(*ActorItr);
		if (ShipCandidate)
		{
			if (!ShipCandidate->GetDamageSystem()->IsAlive())
			{
				continue;
			}

			if (ShipCandidate->GetSize() != Size)
			{
				continue;
			}

			if (DangerousOnly && !IsShipDangerous(ShipCandidate))
			{
				continue;
			}

			if (Ship->GetCompany()->GetHostility(ShipCandidate->GetCompany()) != EFlareHostility::Hostile)
			{
				continue;
			}

			float DistanceSquared = (PilotLocation - ShipCandidate->GetActorLocation()).SizeSquared();
			if (NearestHostileShip == NULL || DistanceSquared < MinDistanceSquared)
			{
				MinDistanceSquared = DistanceSquared;
				NearestHostileShip = ShipCandidate;
			}
		}
	}
	return NearestHostileShip;
}

AFlareSpacecraft* UFlareShipPilot::GetNearestShip(bool IgnoreDockingShip) const
{
	// For now an host ship is a the nearest host ship with the following critera:
	// - Alive or not
	// - From any company
	// - Is the nearest
	// - Is not me

	FVector PilotLocation = Ship->GetActorLocation();
	float MinDistanceSquared = -1;
	AFlareSpacecraft* NearestShip = NULL;

	for (TActorIterator<AActor> ActorItr(Ship->GetWorld()); ActorItr; ++ActorItr)
	{
		// Ship
		AFlareSpacecraft* ShipCandidate = Cast<AFlareSpacecraft>(*ActorItr);
		if (ShipCandidate && ShipCandidate != Ship)
		{
			if (IgnoreDockingShip && Ship->GetDockingSystem()->IsGrantedShip(ShipCandidate) && ShipCandidate->GetDamageSystem()->IsAlive() && ShipCandidate->GetDamageSystem()->IsPowered())
			{
				// Alive and powered granted ship are not dangerous for collision
				continue;
			}

			if (IgnoreDockingShip && Ship->GetDockingSystem()->IsDockedShip(ShipCandidate))
			{
				// Docked shipship are not dangerous for collision, even if they are dead or offlline
				continue;
			}

			float DistanceSquared = (PilotLocation - ShipCandidate->GetActorLocation()).SizeSquared();
			if (NearestShip == NULL || DistanceSquared < MinDistanceSquared)
			{
				MinDistanceSquared = DistanceSquared;
				NearestShip = ShipCandidate;
			}
		}
	}
	return NearestShip;
}

FVector UFlareShipPilot::GetAngularVelocityToAlignAxis(FVector LocalShipAxis, FVector TargetAxis, FVector TargetAngularVelocity, float DeltaSeconds) const
{
	TArray<UActorComponent*> Engines = Ship->GetComponentsByClass(UFlareEngine::StaticClass());

	FVector AngularVelocity = Ship->Airframe->GetPhysicsAngularVelocity();
	FVector WorldShipAxis = Ship->Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalShipAxis);

	WorldShipAxis.Normalize();
	TargetAxis.Normalize();

	FVector RotationDirection = FVector::CrossProduct(WorldShipAxis, TargetAxis);
	RotationDirection.Normalize();
	float Dot = FVector::DotProduct(WorldShipAxis, TargetAxis);
	float angle = FMath::RadiansToDegrees(FMath::Acos(Dot));

	FVector DeltaVelocity = TargetAngularVelocity - AngularVelocity;
	FVector DeltaVelocityAxis = DeltaVelocity;
	DeltaVelocityAxis.Normalize();

	float TimeToFinalVelocity;

	if (FMath::IsNearlyZero(DeltaVelocity.SizeSquared()))
	{
		TimeToFinalVelocity = 0;
	}
	else {
		FVector SimpleAcceleration = DeltaVelocityAxis * Ship->GetNavigationSystem()->GetAngularAccelerationRate();
	    // Scale with damages
		float DamageRatio = Ship->GetNavigationSystem()->GetTotalMaxTorqueInAxis(Engines, DeltaVelocityAxis, true) / Ship->GetNavigationSystem()->GetTotalMaxTorqueInAxis(Engines, DeltaVelocityAxis, false);
	    FVector DamagedSimpleAcceleration = SimpleAcceleration * DamageRatio;

	    FVector Acceleration = DamagedSimpleAcceleration;
	    float AccelerationInAngleAxis =  FMath::Abs(FVector::DotProduct(DamagedSimpleAcceleration, RotationDirection));

	    TimeToFinalVelocity = (DeltaVelocity.Size() / AccelerationInAngleAxis);
	}

	float AngleToStop = (DeltaVelocity.Size() / 2) * (FMath::Max(TimeToFinalVelocity,ReactionTime));

	FVector RelativeResultSpeed;

	if (AngleToStop > angle) {
		RelativeResultSpeed = TargetAngularVelocity;
	}
	else
	{
		float MaxPreciseSpeed = FMath::Min((angle - AngleToStop) / (ReactionTime * 0.75f), Ship->GetNavigationSystem()->GetAngularMaxVelocity());

		RelativeResultSpeed = RotationDirection;
		RelativeResultSpeed *= MaxPreciseSpeed;
	}

	return RelativeResultSpeed;
}


AFlareSpacecraft* UFlareShipPilot::GetNearestAvailableStation() const
{
	FVector PilotLocation = Ship->GetActorLocation();
	float MinDistanceSquared = -1;
	AFlareSpacecraft* NearestStation = NULL;

	for (TActorIterator<AActor> ActorItr(Ship->GetWorld()); ActorItr; ++ActorItr)
	{
		// Ship
		AFlareSpacecraft* StationCandidate = Cast<AFlareSpacecraft>(*ActorItr);
		if (StationCandidate && StationCandidate->IsStation())
		{

			if (StationCandidate->GetCompany() != Ship->GetCompany())
			{
				continue;
			}

			if (!StationCandidate->GetDockingSystem()->HasAvailableDock(Ship))
			{
				continue;
			}

			float DistanceSquared = (PilotLocation - StationCandidate->GetActorLocation()).SizeSquared();
			if (NearestStation == NULL || DistanceSquared < MinDistanceSquared)
			{
				MinDistanceSquared = DistanceSquared;
				NearestStation = StationCandidate;
			}
		}
	}
	return NearestStation;
}

TArray<AFlareSpacecraft*> UFlareShipPilot::GetFriendlyStations() const
{
	TArray<AFlareSpacecraft*> FriendlyStations;

	for (TActorIterator<AActor> ActorItr(Ship->GetWorld()); ActorItr; ++ActorItr)
	{
		// Ship
		AFlareSpacecraft* StationCandidate = Cast<AFlareSpacecraft>(*ActorItr);
		if (StationCandidate && StationCandidate->GetDockingSystem()->GetDockCount() > 0)
		{

			if (StationCandidate->GetCompany() != Ship->GetCompany())
			{
				continue;
			}

			FriendlyStations.Add(StationCandidate);
		}
	}
	return FriendlyStations;
}


bool UFlareShipPilot::IsShipDangerous(AFlareSpacecraft* ShipCandidate) const
{
	return ShipCandidate->IsMilitary() && ShipCandidate->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon) > 0;
}

UFlareSpacecraftComponent* UFlareShipPilot::GetRandomTargetComponent(AFlareSpacecraft* TargetSpacecraft)
{
	TArray<UFlareSpacecraftComponent*> ComponentSelection;

	TArray<UActorComponent*> Components = TargetSpacecraft->GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);

		if (Component->GetDescription() && Component->GetDamageRatio() > 0)
		{
			ComponentSelection.Add(Component);
		}
	}

	if (ComponentSelection.Num() == 0)
	{
		return TargetSpacecraft->GetCockpit();
	}
	else
	{
		while(true)
		{
			UFlareSpacecraftComponent* Component = ComponentSelection[FMath::RandRange(0, ComponentSelection.Num()-1)];

			UFlareRCS* RCS = Cast<UFlareRCS>(Component);
			if (RCS)
			{
				if (FMath::FRand() > 0.25)
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

FVector UFlareShipPilot::GetLinearTargetVelocity() const
{
	return LinearTargetVelocity;
}

FVector UFlareShipPilot::GetAngularTargetVelocity() const
{
	return AngularTargetVelocity;
}

bool UFlareShipPilot::IsUseOrbitalBoost() const
{
	return UseOrbitalBoost;
}

bool UFlareShipPilot::IsWantFire() const
{
	return WantFire;
}
