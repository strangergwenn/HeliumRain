
#include "../Flare.h"

#include "FlareShipPilot.h"
#include "FlareSpacecraft.h"
#include "FlarePilotHelper.h"

#include "../Game/FlareCompany.h"
#include "../Game/FlareGame.h"
#include "../Game/AI/FlareCompanyAI.h"

#include "../Player/FlarePlayerController.h"

#include "../Spacecrafts/FlareEngine.h"
#include "../Spacecrafts/FlareRCS.h"


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
	UseOrbitalBoost = true;

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
	if (Ship->GetNavigationSystem()->IsDocked())
	{
		// Let's undock
		Ship->GetNavigationSystem()->Undock();
		return;
	}
	else if (Ship->GetNavigationSystem()->IsAutoPilot())
	{
		// Wait manoeuver
		return;
	}

	EFlareCombatGroup::Type CombatGroup;

	if(Ship->GetSize() == EFlarePartSize::L)
	{
		CombatGroup = EFlareCombatGroup::Capitals;
	}
	else
	{
		CombatGroup = EFlareCombatGroup::Fighters;
	}

	EFlareCombatTactic::Type Tactic = Ship->GetCompany()->GetTacticManager()->GetCurrentTacticForShipGroup(CombatGroup);

	if (!PilotTargetShip // No target
			|| !PilotTargetShip->GetParent()->GetDamageSystem()->IsAlive() // Target dead
			|| (PilotTargetShip->GetActorLocation() - Ship->GetActorLocation()).Size() > MaxFollowDistance * 100 // Target too far
			|| (Ship->GetSize() == EFlarePartSize::S && SelectedWeaponGroupIndex == -1)  // No selected weapon
			|| (Ship->GetSize() == EFlarePartSize::S && !LockTarget && Ship->GetDamageSystem()->GetWeaponGroupHealth(SelectedWeaponGroupIndex, true) <=0)
			|| Tactic != CurrentTactic)  // Selected weapon not usable
	{
// 		if (PilotTargetShip == NULL)
// 		{
// 			FLOG("Switch target because no current target");
// 		}
// 		else if (!PilotTargetShip->GetDamageSystem()->IsAlive())
// 		{
// 			FLOG("Switch target because current target is dead");
// 		}
// 		else if ((PilotTargetShip->GetActorLocation() - Ship->GetActorLocation()).Size() > MaxFollowDistance * 100)
// 		{
// 			FLOGV("Switch target because current target is too far %f > %f", (PilotTargetShip->GetActorLocation() - Ship->GetActorLocation()).Size(), (MaxFollowDistance * 100));
// 		}
// 		else if (SelectedWeaponGroupIndex == -1)
// 		{
// 			FLOG("Switch target because no selected weapon");
// 		}
// 		else if (( !LockTarget && Ship->GetDamageSystem()->GetWeaponGroupHealth(SelectedWeaponGroupIndex, false, true) <=0))
// 		{
// 			FLOG("Switch target because selected weapon is not usable");
// 		}

		PilotTargetShip = NULL;
		SelectedWeaponGroupIndex = -1;
		CurrentTactic = Tactic;
		FindBestHostileTarget(CurrentTactic);
	}

	bool Idle = true;

	TimeUntilNextComponentSwitch-=ReactionTime;


	if (Ship->GetSize() == EFlarePartSize::S && PilotTargetShip && SelectedWeaponGroupIndex >= 0)
	{
		if (TimeUntilNextComponentSwitch <= 0 && !LockTarget)
		{
			PilotTargetComponent = NULL;
		}
		else if (PilotTargetComponent)
		{
			if (PilotTargetComponent->GetSpacecraft() != PilotTargetShip)
			{
				PilotTargetComponent = NULL;
			}
			else if (PilotTargetComponent->GetUsableRatio() <=0)
			{
				PilotTargetComponent = NULL;
			}
		}

		if (!PilotTargetComponent)
		{
			PilotTargetComponent = PilotHelper::GetBestTargetComponent(PilotTargetShip);
			TimeUntilNextComponentSwitch = 5;
		}

		if(PilotTargetComponent)
		{
			EFlareWeaponGroupType::Type WeaponType = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Type;
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
		else
		{
			PilotTargetShip = NULL;
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
			if(Ship->GetNavigationSystem()->IsDocked())
			{
				Ship->GetNavigationSystem()->Undock();
			}
			LinearTargetVelocity = -DeltaLocation.GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity() * 100;




			UseOrbitalBoost = true;
		}
		else
		{
			PilotTargetShip = NULL;
		}

	}

	if(PilotTargetShip)
	{
		// Already done
	}
	else if (Ship->GetNavigationSystem()->IsDocked())
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
	else if (Ship->GetNavigationSystem()->IsAutoPilot())
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


	// Exit avoidance
	LinearTargetVelocity = ExitAvoidance(Ship, LinearTargetVelocity);

	AlignToTargetVelocityWithThrust(DeltaSeconds);

	// Anticollision
	LinearTargetVelocity = PilotHelper::AnticollisionCorrection(Ship, LinearTargetVelocity);

	//FLOGV("%s Location = %s LinearTargetVelocity = %s",  *Ship->GetImmatriculation().ToString(), * Ship->GetActorLocation().ToString(),	 *LinearTargetVelocity.ToString());
}

void UFlareShipPilot::FighterPilot(float DeltaSeconds)
{
	float AmmoVelocity = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Weapons[0]->GetAmmoVelocity() * 100;

	bool DangerousTarget = PilotHelper::IsShipDangerous(PilotTargetShip);

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

	// TargetAngularVelocity = FVector(0,0,0);

	// First allow align nose to target bullet interception point
	// TODO Use BulletDirection instead of LocalNose
	// AngularTargetVelocity = GetAngularVelocityToAlignAxis(LocalNose, FireTargetAxis, DeltaSeconds);
	// TODO find target angular velocity

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
	// TODO increase tolerance if target is near
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
					if (!PilotHelper::CheckFriendlyFire(Ship->GetGame()->GetActiveSector(), PlayerCompany, MuzzleLocation, ShipVelocity, AmmoVelocity, GunFireTargetAxis, GunAmmoIntersectionTime, 0))
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

	if (Ship->GetParent()->GetDamageSystem()->GetTemperature() > Ship->GetParent()->GetDamageSystem()->GetOverheatTemperature() * (DangerousTarget ? 1.1f : 0.90f))
	{
		// TODO Fire on dangerous target
		WantFire = false;
	}


	// Manage orbital boost
	if (Ship->GetParent()->GetDamageSystem()->GetTemperature() > Ship->GetParent()->GetDamageSystem()->GetOverheatTemperature() * 0.75)
	{
		//UseOrbitalBoost = false;
	}

	// Find friend barycenter
	// Go to friend barycenter
	// If near
		// Turn to opposite from barycentre
	// else
		// Turn to direction

	float TimeSinceDamage = Ship->GetDamageSystem()->GetTimeSinceLastExternalDamage();

	if (TimeSinceDamage < 5.)
	{
		UseOrbitalBoost = true;
		FVector NoseAxis = Ship->Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalNose);
		LinearTargetVelocity = LinearTargetVelocity.GetUnsafeNormal() * 0.5 + NoseAxis * 0.5 *  Ship->GetNavigationSystem()->GetLinearMaxVelocity() * 3.0;
	}

	// Anticollision
	LinearTargetVelocity = PilotHelper::AnticollisionCorrection(Ship, LinearTargetVelocity);


	if (ClearTarget)
	{
		PilotTargetShip = NULL;
	}
}

void UFlareShipPilot::BomberPilot(float DeltaSeconds)
{
	LinearTargetVelocity = FVector::ZeroVector;


	FVector DeltaLocation = (PilotTargetComponent->GetComponentLocation() - Ship->GetActorLocation()) / 100.f;
	FVector TargetAxis = DeltaLocation.GetUnsafeNormal();
	float Distance = DeltaLocation.Size(); // Distance in meters

	// Attack Phases
	// 0 - Prepare attack : change velocity to approch the target
	// 1 - Attacking : target is approching with boost
	// 3 - Drop : Drop util its not safe to stay
	// 2 - Withdraw : target is passed, wait a security distance to attack again

	float WeigthCoef = FMath::Sqrt(Ship->GetSpacecraftMass()) / FMath::Sqrt(5425.f) * (2-Ship->GetParent()->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_RCS)) ; // 1 for ghoul at 100%

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

	if (AttackPhase == 0)
	{
		if (Distance < ChargeDistance)
		{
			// Target is approching, prepare attack
			AttackPhase = 1;
			LockTarget = true;
		}
		else
		{
			LinearTargetVelocity = TargetAxis * Ship->GetNavigationSystem()->GetLinearMaxVelocity();
			AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), TargetAxis, FVector::ZeroVector, DeltaSeconds);
			UseOrbitalBoost = true;
		}
	}

	if (AttackPhase == 1)
	{
		FVector AmmoIntersectionLocation;
		float AmmoVelocity = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Weapons[0]->GetAmmoVelocity() * 100;
		float AmmoIntersectionTime = SpacecraftHelper::GetIntersectionPosition(PilotTargetComponent->GetComponentLocation(), PilotTargetShip->Airframe->GetPhysicsLinearVelocity(), Ship->GetActorLocation(), Ship->Airframe->GetPhysicsLinearVelocity(), AmmoVelocity, 0.0, &AmmoIntersectionLocation);

		AlignToSpeed = true;

		if (AmmoIntersectionTime > 0 && AmmoIntersectionTime < DropTime)
		{
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
		}
		else
		{
			LinearTargetVelocity = TargetAxis * Ship->GetNavigationSystem()->GetLinearMaxVelocity();
		}

		LastTargetDistance = Distance;
	}

	if (AttackPhase == 2)
	{
		FVector AmmoIntersectionLocation;
		float AmmoVelocity = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Weapons[0]->GetAmmoVelocity() * 100;
		float AmmoIntersectionTime = SpacecraftHelper::GetIntersectionPosition(PilotTargetComponent->GetComponentLocation(), PilotTargetShip->Airframe->GetPhysicsLinearVelocity(), Ship->GetActorLocation(), Ship->Airframe->GetPhysicsLinearVelocity(), AmmoVelocity, 0.0, &AmmoIntersectionLocation);
		FVector FrontVector = Ship->GetFrontVector();
		FVector ChargeAxis = (AmmoIntersectionLocation - Ship->GetActorLocation()).GetUnsafeNormal();
		// DrawDebugLine(Ship->GetWorld(), Ship->GetActorLocation(), AmmoIntersectionLocation, FColor::Blue, false, ReactionTime);

		Anticollision = false;
		AlignToSpeed = true;

		if (AmmoIntersectionTime < EvadeTime || FVector::DotProduct(FrontVector, ChargeAxis) < 0.6 || AmmoIntersectionTime > AlignTime)
		{
			// Security distance reach
			AttackPhase = 3;
		}
		else if (FVector::DotProduct(FrontVector, ChargeAxis) > 0.9 && AmmoIntersectionTime < DropTime)
		{
			if (TimeBeforeNextDrop > 0)
			{
				TimeBeforeNextDrop -= ReactionTime;
			}
			else
			{

				WantFire = !LastWantFire;
				LastWantFire = WantFire;
				if (WantFire)
				{
					TimeBeforeNextDrop = TimeBetweenDrop;
				}
			}

			LinearTargetVelocity = ChargeAxis * Ship->GetNavigationSystem()->GetLinearMaxVelocity();
		}
	}

	if (AttackPhase == 3)
	{
		FVector DeltaVelocity = (PilotTargetShip->GetLinearVelocity() - Ship->GetLinearVelocity()) / 100.;
		if (Distance > SecurityDistance)
		{
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
			UseOrbitalBoost = true;
			HardBoost = true;
		}
		else
		{
			UseOrbitalBoost = true;
			HardBoost = true;
			LinearTargetVelocity = -TargetAxis * Ship->GetNavigationSystem()->GetLinearMaxVelocity() * 2;
			AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), -TargetAxis, FVector::ZeroVector, DeltaSeconds);
		}
	}


	// DrawDebugLine(Ship->GetWorld(), Ship->GetActorLocation(), Ship->GetActorLocation() + LinearTargetVelocity * 100, FColor::Red, false, ReactionTime);

	if (AlignToSpeed)
	{
		if (Ship->GetLinearVelocity().IsNearlyZero())
		{
			AngularTargetVelocity = FVector::ZeroVector;
		}
		else
		{
			FVector LinearVelocityAxis = Ship->GetLinearVelocity().GetUnsafeNormal();
			AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), LinearVelocityAxis, FVector::ZeroVector, DeltaSeconds);
		}
	}
	else
	{
		AlignToTargetVelocityWithThrust(DeltaSeconds);
	}

	// Anticollision
	if (Anticollision)
	{
		LinearTargetVelocity = PilotHelper::AnticollisionCorrection(Ship, LinearTargetVelocity, PilotTargetShip);
	}


	if (ClearTarget)
	{
		PilotTargetShip = NULL;
	}
	// TODO ignore target


	// Manage orbital boost
	if (Ship->GetParent()->GetDamageSystem()->GetTemperature() > Ship->GetParent()->GetDamageSystem()->GetOverheatTemperature() * 0.75)
	{
		//UseOrbitalBoost = false;
	}


	// Manage orbital boost

}

void UFlareShipPilot::IdlePilot(float DeltaSeconds)
{
	// TODO find better
	//UseOrbitalBoost = false;


	// If damaged repair
	if (Ship->GetParent()->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon, true) < 1
		|| Ship->GetParent()->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_LifeSupport, true) < 1
		|| Ship->GetParent()->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Propulsion, true) < 1
		|| Ship->GetParent()->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_RCS, true) < 1
		|| Ship->GetParent()->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Power, true) < 1)
	{
		// Go repair or refill ammo
		AFlareSpacecraft* TargetStation  = GetNearestAvailableStation(false);
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
		if (Distance < 10000) // 10 km
		{
			Ship->ForceManual(); // TODO make independant command channel
			LinearTargetVelocity = -DeltaLocation.GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity();

			UseOrbitalBoost = true;
		}

		if (Distance > 2000 && Ship->GetParent()->GetDamageSystem()->GetTemperature() > Ship->GetParent()->GetDamageSystem()->GetOverheatTemperature() * 0.95)
		{
			// Too hot and no imminent danger
			//UseOrbitalBoost = false;
		}
	}
	else
	{
		// If not, find a leader
		AFlareSpacecraft* LeaderShip = Ship;

		TArray<AFlareSpacecraft*> Spacecrafts = Ship->GetGame()->GetActiveSector()->GetCompanySpacecrafts(Ship->GetCompany());
		for (int ShipIndex = 0; ShipIndex < Spacecrafts.Num() ; ShipIndex++)
		{
			AFlareSpacecraft* CandidateShip = Spacecrafts[ShipIndex];
			float LeaderMass = LeaderShip->GetSpacecraftMass();
			float CandidateMass = CandidateShip->GetSpacecraftMass();

			if (Ship == CandidateShip)
			{
				continue;
			}

			if (!CandidateShip->IsMilitary())
			{
				continue;
			}

            if (LeaderMass == CandidateMass)
			{
                if (LeaderShip->GetImmatriculation() < CandidateShip->GetImmatriculation())
                {
                    continue;
                }
			}
            else if (LeaderMass > CandidateMass)
            {
                continue;
            }

            LeaderShip = CandidateShip;
		}

		// If is the leader, find a location in a 10 km radius and patrol
		if (LeaderShip == Ship)
		{
			// Go to a random point at 10000 m from the center

			// If at less than 500 m from this point, get another random point

			float TargetLocationToShipDistance = (PilotTargetLocation - Ship->GetActorLocation()).Size();

			if (TargetLocationToShipDistance < 50000 || PilotTargetLocation.IsZero())
			{
				FVector PatrolCenter = FVector::ZeroVector;

				// Use random station
				AFlareSpacecraft* NearestStation =  GetNearestAvailableStation(true);

				if (NearestStation)
				{
					PatrolCenter = NearestStation->GetActorLocation();
				}

				PilotTargetLocation = PatrolCenter + FMath::VRand() * FMath::FRand() * 400000;
			}
			LinearTargetVelocity = (PilotTargetLocation - Ship->GetActorLocation()).GetUnsafeNormal()  * Ship->GetNavigationSystem()->GetLinearMaxVelocity() * 0.8;
		}
		else
		{
			// Follow the leader
			float FollowRadius = 50000 + FMath::Pow(Ship->GetCompany()->GetCompanyShips().Num() * 3 * FMath::Pow(10000, 3),1/3.);
			if ((LeaderShip->GetActorLocation() - Ship->GetActorLocation()).Size() < FollowRadius)
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


	// Exit avoidance
	LinearTargetVelocity = ExitAvoidance(Ship, LinearTargetVelocity);

	AlignToTargetVelocityWithThrust(DeltaSeconds);

	// Anticollision
	LinearTargetVelocity = PilotHelper::AnticollisionCorrection(Ship, LinearTargetVelocity);

    //FLOGV("%s Leader ship LinearTargetVelocity=%s", *LinearTargetVelocity.ToString());
}

void UFlareShipPilot::FlagShipPilot(float DeltaSeconds)
{
	// Go to a random point at 800 m from the target

	// If at less than 50 m from this point, get another random point

	// If the target to farther than 2000 to the target point, change point

	float TargetLocationToTargetShipDistance = (PilotTargetLocation - PilotTargetShip->GetActorLocation()).Size();
	float TargetLocationToShipDistance = (PilotTargetLocation - Ship->GetActorLocation()).Size();

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


	if (NewTargetLocation || PilotTargetLocation.IsZero())
	{

		PilotTargetLocation = FMath::VRand() * FMath::FRand() * 80000 + PilotTargetShip->GetActorLocation();
	}


	AngularTargetVelocity = FVector::ZeroVector;
	LinearTargetVelocity = (PilotTargetLocation - Ship->GetActorLocation()).GetUnsafeNormal()  * Ship->GetNavigationSystem()->GetLinearMaxVelocity();

	// TODO Bomb avoid

	AlignToTargetVelocityWithThrust(DeltaSeconds);

	// Anticollision
	LinearTargetVelocity = PilotHelper::AnticollisionCorrection(Ship, LinearTargetVelocity);

	FVector FrontAxis = Ship->Airframe->GetComponentToWorld().GetRotation().RotateVector(FVector(1,0,0));

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
	if (Ship->GetParent()->GetDamageSystem()->GetTemperature() > Ship->GetParent()->GetDamageSystem()->GetOverheatTemperature() * 0.9)
	{
		//UseOrbitalBoost = false;
	}
}



void UFlareShipPilot::FindBestHostileTarget(EFlareCombatTactic::Type Tactic)
{
	AFlareSpacecraft* TargetCandidate = NULL;

	struct PilotHelper::TargetPreferences TargetPreferences;
	TargetPreferences.IsLarge = 1;
	TargetPreferences.IsSmall = 1;
	TargetPreferences.IsStation = 1;
	TargetPreferences.IsNotStation = 1;
	TargetPreferences.IsMilitary = 1;
	TargetPreferences.IsNotMilitary = 0.1;
	TargetPreferences.IsDangerous = 1;
	TargetPreferences.IsNotDangerous = 0.01;
	TargetPreferences.IsStranded = 1;
	TargetPreferences.IsNotStranded = 0.5;
	TargetPreferences.IsUncontrolable = 0.1;
	TargetPreferences.IsNotUncontrolable = 1;
	TargetPreferences.TargetStateWeight = 1;
	TargetPreferences.MaxDistance = 1000000;
	TargetPreferences.DistanceWeight = 0.5;
	TargetPreferences.AttackTarget = NULL;
	TargetPreferences.AttackTargetWeight = 1;
	TargetPreferences.PreferredDirection = Ship->GetFrontVector();
	TargetPreferences.MinAlignement = -1;
	TargetPreferences.AlignementWeight = 0.1;
	TargetPreferences.BaseLocation = Ship->GetActorLocation();

	Ship->GetWeaponsSystem()->GetTargetSizePreference(&TargetPreferences.IsSmall, &TargetPreferences.IsLarge);

	if (Tactic == EFlareCombatTactic::AttackStations)
	{
		TargetPreferences.IsStation = 10;
	}
	else if (Tactic == EFlareCombatTactic::AttackMilitary)
	{
		TargetPreferences.IsStation = 0.1;
	}
	else if (Tactic == EFlareCombatTactic::AttackCivilians)
	{
		TargetPreferences.IsMilitary = 0.1;
		TargetPreferences.IsNotMilitary = 1.0;
		TargetPreferences.IsNotDangerous = 1.0;
	}
	else if (Tactic == EFlareCombatTactic::ProtectMe)
	{
		// Protect me is only available for player ship
		if (Ship->GetCompany() == Ship->GetGame()->GetPC()->GetCompany())
		{
			TargetPreferences.AttackTarget = Ship->GetGame()->GetPC()->GetShipPawn();
			TargetPreferences.AttackTargetWeight = 1.0;
		}
	}

	TargetCandidate = PilotHelper::GetBestTarget(Ship, TargetPreferences);

	if (TargetCandidate)
	{
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

		for (int WeaponGroupIndex=0; WeaponGroupIndex < Ship->GetWeaponsSystem()->GetWeaponGroupCount(); WeaponGroupIndex++)
		{
			float Score = Ship->GetDamageSystem()->GetWeaponGroupHealth(WeaponGroupIndex, true);
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
	}
}



int32 UFlareShipPilot::GetPreferedWeaponGroup() const
{
	return SelectedWeaponGroupIndex;
}

/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

FVector UFlareShipPilot::ExitAvoidance(AFlareSpacecraft* TargetShip, FVector InitialVelocityTarget) const
{

	/*if(TargetShip->GetImmatriculation() != "PIRSOL-0111")
	{
		return InitialVelocityTarget;
	}


	DrawDebugLine(TargetShip->GetWorld(), TargetShip->GetActorLocation(), TargetShip->GetGame()->GetPC()->GetPlayerShip()->GetActive()->GetActorLocation(), FColor::Cyan, false, 1.0);
	DrawDebugLine(TargetShip->GetWorld(),  FVector::ZeroVector, TargetShip->GetGame()->GetPC()->GetPlayerShip()->GetActive()->GetActorLocation(), FColor::Cyan, false, 1.0);
	DrawDebugLine(TargetShip->GetWorld(), FVector::ZeroVector, TargetShip->GetActorLocation(), FColor::Cyan, false, 1.0);


	DrawDebugLine(TargetShip->GetWorld(), TargetShip->GetActorLocation(), TargetShip->GetActorLocation() + TargetShip->GetVelocity() * 1000, FColor::Red, false, 1.0);
	DrawDebugLine(TargetShip->GetWorld(), TargetShip->GetActorLocation(), TargetShip->GetActorLocation() + InitialVelocityTarget * 1000, FColor::Blue, false, 1.0);
*/
	float CurveTrajectoryLimit = 0.3f;
	float ShipCenterDistance = TargetShip->GetActorLocation().Size();
	float SectorLimits = TargetShip->GetGame()->GetActiveSector()->GetSectorLimits() / 3;
	/*FLOGV("%s ExitAvoidance ShipCenterDistance=%f SectorLimits=%f InitialVelocityTarget=%s",
		  *TargetShip->GetImmatriculation().ToString(),
		  ShipCenterDistance /100, SectorLimits /100, *InitialVelocityTarget.ToString());*/

	if (ShipCenterDistance > SectorLimits * CurveTrajectoryLimit)
	{
		// Curve the trajectory to avoid exit
		float CurveRatio = FMath::Min(1.f, (ShipCenterDistance - SectorLimits * CurveTrajectoryLimit) / (SectorLimits * (1-CurveTrajectoryLimit)));


		FVector CenterDirection = (TargetShip->GetGame()->GetActiveSector()->GetSectorCenter() - TargetShip->GetActorLocation()).GetUnsafeNormal();


		//DrawDebugLine(TargetShip->GetWorld(), TargetShip->GetActorLocation(), TargetShip->GetActorLocation() + CenterDirection * 100000, FColor::White, false, 1.0);

		//FLOGV("CenterDirection %s", *CenterDirection.ToCompactString());


		if (InitialVelocityTarget.IsNearlyZero()) {
			//FLOGV("TargetShip->GetNavigationSystem()->GetLinearMaxVelocity() %f", TargetShip->GetNavigationSystem()->GetLinearMaxVelocity());
			//DrawDebugLine(TargetShip->GetWorld(), TargetShip->GetActorLocation(), TargetShip->GetActorLocation() + CenterDirection * TargetShip->GetNavigationSystem()->GetLinearMaxVelocity() * 10000, FColor::Green, false, 1.0);

			return CenterDirection * TargetShip->GetNavigationSystem()->GetLinearMaxVelocity();
		}
		else
		{
			FVector InitialVelocityTargetDirection = InitialVelocityTarget.GetUnsafeNormal();

			FVector ExitAvoidanceDirection = (CurveRatio * CenterDirection + (1 - CurveRatio) * InitialVelocityTargetDirection).GetUnsafeNormal();
			FVector ExitAvoidanceVelocity = ExitAvoidanceDirection *  InitialVelocityTarget.Size();

		/*	FLOGV("CurveRatio %f", CurveRatio);

			FLOGV("InitialVelocityTargetDirection %s", *InitialVelocityTargetDirection.ToCompactString());
			FLOGV("ExitAvoidanceDirection %s", *ExitAvoidanceDirection.ToCompactString());
			FLOGV("ExitAvoidanceVelocity %s", *ExitAvoidanceVelocity.ToCompactString());
			DrawDebugLine(TargetShip->GetWorld(), TargetShip->GetActorLocation(), TargetShip->GetActorLocation() + ExitAvoidanceVelocity * 10000, FColor::Green, false, 1.0);
			*/
			return ExitAvoidanceVelocity;
		}

	}
	else
	{
		return InitialVelocityTarget;
	}
}

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

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Ship->GetGame()->GetActiveSector()->GetSpacecrafts().Num(); SpacecraftIndex++)
	{
		AFlareSpacecraft* ShipCandidate = Ship->GetGame()->GetActiveSector()->GetSpacecrafts()[SpacecraftIndex];

		if (!ShipCandidate->GetParent()->GetDamageSystem()->IsAlive())
		{
			continue;
		}

		if (ShipCandidate->GetSize() != Size)
		{
			continue;
		}

		if (DangerousOnly && ! PilotHelper::IsShipDangerous(ShipCandidate))
		{
			continue;
		}

		if (Ship->GetCompany()->GetWarState(ShipCandidate->GetCompany()) != EFlareHostility::Hostile)
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

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Ship->GetGame()->GetActiveSector()->GetSpacecrafts().Num(); SpacecraftIndex++)
	{
		AFlareSpacecraft* ShipCandidate = Ship->GetGame()->GetActiveSector()->GetSpacecrafts()[SpacecraftIndex];

		if (ShipCandidate == Ship)
		{
			continue;
		}

		if (IgnoreDockingShip && Ship->GetDockingSystem()->IsGrantedShip(ShipCandidate) && !ShipCandidate->GetParent()->GetDamageSystem()->IsUncontrollable())
		{
			// Constrollable ship are not dangerous for collision
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


AFlareSpacecraft* UFlareShipPilot::GetNearestAvailableStation(bool RealStation) const
{
	FVector PilotLocation = Ship->GetActorLocation();
	float MinDistanceSquared = -1;
	AFlareSpacecraft* NearestStation = NULL;

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Ship->GetGame()->GetActiveSector()->GetStations().Num(); SpacecraftIndex++)
	{
		AFlareSpacecraft* StationCandidate = Ship->GetGame()->GetActiveSector()->GetStations()[SpacecraftIndex];

		if (StationCandidate == Ship)
		{
			continue;
		}

		if (!StationCandidate->GetDockingSystem()->HasAvailableDock(Ship))
		{
			continue;
		}

		if (RealStation && !StationCandidate->IsStation())
		{
			continue;
		}

		if (StationCandidate->GetCompany() != Ship->GetCompany())
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
	return NearestStation;
}

TArray<AFlareSpacecraft*> UFlareShipPilot::GetFriendlyStations() const
{
	TArray<AFlareSpacecraft*> FriendlyStations;

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Ship->GetGame()->GetActiveSector()->GetStations().Num(); SpacecraftIndex++)
	{
		AFlareSpacecraft* StationCandidate = Ship->GetGame()->GetActiveSector()->GetStations()[SpacecraftIndex];

		if (StationCandidate->GetDockingSystem()->GetDockCount() > 0)
		{

			if (StationCandidate->GetWarState(Ship->GetCompany()) == EFlareHostility::Hostile)
			{
				continue;
			}

			FriendlyStations.Add(StationCandidate);
		}
	}
	return FriendlyStations;
}

void UFlareShipPilot::AlignToTargetVelocityWithThrust(float DeltaSeconds)
{
	if(!LinearTargetVelocity.IsNearlyZero()) {
		FVector LinearTargetVelocityAxis = LinearTargetVelocity.GetUnsafeNormal();
		FVector ThrustVector = (LinearTargetVelocity - 100 * Ship->GetLinearVelocity());
		FVector ThrustAxis;
		if(ThrustVector.IsNearlyZero())
		{
			 ThrustAxis = LinearTargetVelocityAxis;
		}
		else
		{
			ThrustAxis = ThrustVector.GetUnsafeNormal();
		}

		FVector OrientationVector = (ThrustAxis * 0.3  + LinearTargetVelocityAxis * 0.7).GetUnsafeNormal();

		AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), OrientationVector, FVector::ZeroVector, DeltaSeconds);
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
