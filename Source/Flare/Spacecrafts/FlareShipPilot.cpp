
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
	FLOGV("%s MilitaryPilot",  *Ship->GetName());

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
			|| SelectedWeaponGroupIndex == -1  // No selected weapon
			|| ( !LockTarget && Ship->GetDamageSystem()->GetWeaponGroupHealth(SelectedWeaponGroupIndex, false, true) <=0))  // Selected weapon not usable
	{
		if(PilotTargetShip == NULL)
		{
			FLOG("Switch target because no current target");
		}
		else if(!PilotTargetShip->GetDamageSystem()->IsAlive())
		{
			FLOG("Switch target because current target is dead");
		}
		else if((PilotTargetShip->GetActorLocation() - Ship->GetActorLocation()).Size() > MaxFollowDistance * 100)
		{
			FLOGV("Switch target because current target is too far %f > %f", (PilotTargetShip->GetActorLocation() - Ship->GetActorLocation()).Size(), (MaxFollowDistance * 100));
		}
		else if(SelectedWeaponGroupIndex == -1)
		{
			FLOG("Switch target because no selected weapon");
		}
		else if(( !LockTarget && Ship->GetDamageSystem()->GetWeaponGroupHealth(SelectedWeaponGroupIndex, false, true) <=0))
		{
			FLOG("Switch target because selected weapon is not usable");
		}

		PilotTargetShip = NULL;
		SelectedWeaponGroupIndex = -1;
		FindBestHostileTarget();
	}

	FLOGV("%s Target %x",  *Ship->GetName(), PilotTargetShip);
	bool Idle = true;
	if (PilotTargetShip && SelectedWeaponGroupIndex >= 0)
	{
		FLOGV("%s target %s",  *Ship->GetName(),  *PilotTargetShip->GetName());
		EFlareWeaponGroupType::Type WeaponType = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Type;
		FLOGV("%s WeaponType %d",  *Ship->GetName(),  WeaponType);
		if(WeaponType == EFlareWeaponGroupType::WG_GUN)
		{
			FighterPilot(DeltaSeconds);
			Idle = false;
		}
		else if(WeaponType == EFlareWeaponGroupType::WG_BOMB)
		{
			BomberPilot(DeltaSeconds);
			Idle = false;
		}
	}

	if(Idle)
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
	if(!PilotTargetShip)
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
	LinearTargetVelocity = AnticollisionCorrection(LinearTargetVelocity, AttackAngle);

	// Turn to destination
	if (! LinearTargetVelocity.IsZero())
	{
		AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1.f, 0.f, 0.f) , LinearTargetVelocity.GetUnsafeNormal(),FVector(0.f, 0.f, 0.f), DeltaSeconds);
	}
}

void UFlareShipPilot::FighterPilot(float DeltaSeconds)
{
	float AmmoVelocity = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Weapons[0]->GetAmmoVelocity();
	FLOGV("%s FighterPilot AmmoVelocity %f",  *Ship->GetName(), AmmoVelocity);

	bool DangerousTarget = IsShipDangerous(PilotTargetShip);

	//FLOGV("%s target %s",  *Ship->GetHumanReadableName(),  *PilotTargetShip->GetHumanReadableName());
	// The pilot have a target, track and kill it

	FVector LocalNose = FVector(1.f, 0.f, 0.f);
	FVector DeltaLocation = (PilotTargetShip->GetActorLocation() - Ship->GetActorLocation()) / 100.f;
	float Distance = DeltaLocation.Size(); // Distance in meters
	float TargetSize = PilotTargetShip->GetMeshScale() / 100.f; // Radius in meters
	FVector TargetAxis = DeltaLocation.GetUnsafeNormal();
	FVector ShipVelocity = 100 * Ship->GetLinearVelocity();
	FVector PilotTargetShipVelocity = 100 * PilotTargetShip->GetLinearVelocity();

	// Use position prediction
	float PredictionDelay = ReactionTime - DeltaSeconds;
	FVector PredictedShipLocation = Ship->GetActorLocation() + ShipVelocity * PredictionDelay;
	FVector PredictedPilotTargetShipLocation = PilotTargetShip->GetActorLocation() + PilotTargetShipVelocity * PredictionDelay;
	FVector PredictedDeltaLocation = (PredictedPilotTargetShipLocation - PredictedShipLocation) / 100.f;
	FVector PredictedTargetAxis = PredictedDeltaLocation.GetUnsafeNormal();
	float PredictedDistance = PredictedDeltaLocation.Size(); // Distance in meters

	FVector AmmoIntersectionLocation;
	float AmmoIntersectionTime = PilotTargetShip->GetAimPosition(Ship, AmmoVelocity, 0, &AmmoIntersectionLocation);
	FVector FireTargetAxis;
	if (AmmoIntersectionTime > 0)
	{
		FireTargetAxis = (AmmoIntersectionLocation - Ship->GetActorLocation()).GetUnsafeNormal();
	}
	else
	{
		FireTargetAxis = (PilotTargetShip->GetActorLocation() - Ship->GetActorLocation()).GetUnsafeNormal();
	}


	FVector AmmoIntersectionPredictedLocation;
	float AmmoIntersectionPredictedTime = PilotTargetShip->GetAimPosition(Ship, AmmoVelocity, PredictionDelay, &AmmoIntersectionPredictedLocation);
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
	BulletVelocity *= 100 * AmmoVelocity;

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
	float SecurityDistance = (DangerousTarget ? 600: 300) + TargetSize * 4;
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
		}

		if (Distance < SecurityDistance)
		{
			AttackPhase = 1;
		}

	}

	if (AttackPhase == 1)
	{
		if (LastTargetDistance < Distance)
		{
			// Target is passed
			AttackPhase = 2;
		}
		else
		{
			FQuat AttackDistanceQuat = FQuat(TargetAxis, AttackAngle);
			FVector TopVector = Ship->GetActorRotation().RotateVector(FVector(0,0,AttackDistance));
			FVector AttackMargin =  AttackDistanceQuat.RotateVector(TopVector);

			if (Distance > SecurityDistance || DangerousTarget)
			{
				LinearTargetVelocity = (AttackMargin + DeltaLocation).GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity();
			}
			else
			{
				LinearTargetVelocity = PilotTargetShip->GetLinearVelocity() + (AttackMargin + DeltaLocation).GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity() / 4.0;
			}
			UseOrbitalBoost = true;
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
			if (DangerousTarget)
			{
				LinearTargetVelocity = -DeltaLocation.GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity();
				UseOrbitalBoost = true;
			}
			else
			{
				LinearTargetVelocity = PilotTargetShip->GetLinearVelocity() - DeltaLocation.GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity() / 4.0 ;
			}
		}
	}

	// If at range and aligned fire on the target
	//TODO increase tolerance if target is near
	if (AmmoIntersectionTime > 0 && AmmoIntersectionTime < 1.5)
	{
		//FLOGV("is at fire range=%f", Distance);
		// TODO Use BulletDirection instead of LocalNose
		FVector WorldShipAxis = Ship->Airframe->GetComponentToWorld().GetRotation().RotateVector(BulletDirection);
		float AngularPrecisionDot = FVector::DotProduct(FireTargetAxis, WorldShipAxis);

		float AngularPrecision = FMath::Acos(AngularPrecisionDot);
		float AngularSize = FMath::Atan(TargetSize / Distance);

	/*	FLOGV("WorldShipAxis=%s", *WorldShipAxis.ToString());
		FLOGV("FireTargetAxis=%s", *FireTargetAxis.ToString());
		FLOGV("TargetSize=%f", TargetSize);
		FLOGV("Distance=%f", Distance);

		FLOGV("AngularPrecisionDot=%f", AngularPrecisionDot);

		*/

		if (AngularPrecision < (DangerousTarget ? AngularSize * 0.25 : AngularSize * 0.2))
		{
			WantFire = true;
		}
	}

	if (Ship->GetDamageSystem()->GetTemperature() > Ship->GetDamageSystem()->GetOverheatTemperature() * (DangerousTarget ? 1.1f : 0.90f))
	{
		// TODO Fire on dangerous target
		WantFire = false;
	}


	// Anticollision
	LinearTargetVelocity = AnticollisionCorrection(LinearTargetVelocity, AttackAngle);



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

	if(ClearTarget)
	{
		PilotTargetShip = NULL;
	}
}

void UFlareShipPilot::BomberPilot(float DeltaSeconds)
{
	FLOGV("%s BomberPilot",  *Ship->GetName());
	// TODO

	FLOGV("%s GetLinearVelocity %s",  *Ship->GetName(), *(Ship->GetLinearVelocity()).ToString());

	DrawDebugLine(Ship->GetWorld(), Ship->GetActorLocation(), Ship->GetActorLocation() + Ship->GetLinearVelocity() * 100, FColor::Green, false, ReactionTime);

	if(Ship->GetLinearVelocity().IsNearlyZero())
	{
		AngularTargetVelocity = FVector::ZeroVector;
	}
	else
	{
		FVector LinearVelocityAxis = Ship->GetLinearVelocity().GetUnsafeNormal();
		FLOGV("%s LinearVelocityAxis %s",  *Ship->GetName(), *LinearVelocityAxis.ToString());
		AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), LinearVelocityAxis, FVector::ZeroVector, DeltaSeconds);
		FLOGV("%s AngularTargetVelocity %s",  *Ship->GetName(), *AngularTargetVelocity.ToString());
	}


	LinearTargetVelocity = FVector::ZeroVector;


	FVector DeltaLocation = (PilotTargetShip->GetActorLocation() - Ship->GetActorLocation()) / 100.f;
	FVector TargetAxis = DeltaLocation.GetUnsafeNormal();
	float Distance = DeltaLocation.Size(); // Distance in meters

	FLOGV("%s DeltaLocation %s",  *Ship->GetName(), *DeltaLocation.ToString());
	FLOGV("%s TargetAxis %s",  *Ship->GetName(), *TargetAxis.ToString());
	FLOGV("%s Distance %f",  *Ship->GetName(), Distance);

	// Attack Phases
	// 0 - Prepare attack : change velocity to approch the target
	// 1 - Attacking : target is approching with boost
	// 3 - Drop : Drop util its not safe to stay
	// 2 - Withdraw : target is passed, wait a security distance to attack again

	float ChargeDistance = 1000;
	float DropTime = 4;
	float EvadeTime = 2;
	float SecurityDistance = 1500;
	UseOrbitalBoost = false;
	bool ClearTarget = false;

	FLOGV("%s AttackPhase %d",  *Ship->GetName(), AttackPhase);
	if (AttackPhase == 0)
	{
		if (Distance < ChargeDistance)
		{
			FLOGV("%s Distance < ChargeDistance => phase 1",  *Ship->GetName());
			// Target is approching, prepare attack
			AttackPhase = 1;
			LockTarget = true;
		}
		else
		{
			LinearTargetVelocity = TargetAxis * Ship->GetNavigationSystem()->GetLinearMaxVelocity();
			AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), TargetAxis, FVector::ZeroVector, DeltaSeconds);
			FLOGV("%s Goto target %s",  *Ship->GetName(), *LinearTargetVelocity.ToString());
		}
	}

	if (AttackPhase == 1)
	{
		FVector AmmoIntersectionLocation;
		float AmmoVelocity = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Weapons[0]->GetAmmoVelocity();
		float AmmoIntersectionTime = PilotTargetShip->GetAimPosition(Ship, AmmoVelocity, 0.0, &AmmoIntersectionLocation);

		FLOGV("%s AmmoIntersectionLocation %s",  *Ship->GetName(), *AmmoIntersectionLocation.ToString());
		FLOGV("%s AmmoIntersectionTime %f",  *Ship->GetName(), AmmoIntersectionTime);
		FLOGV("%s AmmoVelocity %f",  *Ship->GetName(), AmmoVelocity);

		DrawDebugLine(Ship->GetWorld(), Ship->GetActorLocation(), AmmoIntersectionLocation, FColor::Blue, false, ReactionTime);

		if (AmmoIntersectionTime > 0 && AmmoIntersectionTime < DropTime)
		{
			FLOGV("%s AmmoIntersectionTime < DropTime => phase 2",  *Ship->GetName());
			// Near enougt
			AttackPhase = 2;
			LastWantFire = false;
			TimeBeforeNextDrop = 0;
		}
		else if(AmmoIntersectionTime > 0 && AmmoIntersectionTime < DropTime * 2)
		{
			FVector ChargeAxis = (AmmoIntersectionLocation - Ship->GetActorLocation()).GetUnsafeNormal();
			LinearTargetVelocity = ChargeAxis * Ship->GetLinearVelocity().Size() * 1.2;
			UseOrbitalBoost = true;
			FLOGV("%s ChargeAxis %s",  *Ship->GetName(), *ChargeAxis.ToString());
			FLOGV("%s Charge target %s",  *Ship->GetName(), *LinearTargetVelocity.ToString());
		}
		else
		{
			LinearTargetVelocity = TargetAxis * Ship->GetNavigationSystem()->GetLinearMaxVelocity();

			FLOGV("%s Goto target %s",  *Ship->GetName(), *LinearTargetVelocity.ToString());
		}

		LastTargetDistance = Distance;
	}

	if (AttackPhase == 2)
	{
		FVector AmmoIntersectionLocation;
		float AmmoVelocity = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Weapons[0]->GetAmmoVelocity();
		float AmmoIntersectionTime = PilotTargetShip->GetAimPosition(Ship, AmmoVelocity, 0.0, &AmmoIntersectionLocation);
		FVector FrontVector = Ship->GetActorRotation().RotateVector(FVector(1,0,0));
		FVector ChargeAxis = (AmmoIntersectionLocation - Ship->GetActorLocation()).GetUnsafeNormal();
		DrawDebugLine(Ship->GetWorld(), Ship->GetActorLocation(), AmmoIntersectionLocation, FColor::Blue, false, ReactionTime);


		FLOGV("%s AmmoIntersectionLocation %s",  *Ship->GetName(), *AmmoIntersectionLocation.ToString());
		FLOGV("%s AmmoIntersectionTime %f",  *Ship->GetName(), AmmoIntersectionTime);
		FLOGV("%s AmmoVelocity %f",  *Ship->GetName(), AmmoVelocity);


		if (AmmoIntersectionTime < EvadeTime || FVector::DotProduct(FrontVector, ChargeAxis) < 0.6)
		{
			// Security distance reach
			FLOGV("%s AmmoIntersectionTime < EvadeTime => phase3",  *Ship->GetName());
			AttackPhase = 3;
		}
		else if(FVector::DotProduct(FrontVector, ChargeAxis) > 0.9)
		{
			FLOGV("%s TimeBeforeNextDrop %f", *Ship->GetName(), TimeBeforeNextDrop);
			if(TimeBeforeNextDrop > 0)
			{
				TimeBeforeNextDrop -= ReactionTime;
				FLOGV("%s Reduce time to  %f",*Ship->GetName(),  TimeBeforeNextDrop);
			}
			else
			{

				WantFire = !LastWantFire;
				FLOGV("%s WantFire=%d LastWantFire=%d",*Ship->GetName(),  WantFire, LastWantFire);
				LastWantFire = WantFire;
				if(WantFire)
				{
					TimeBeforeNextDrop = 0.30;
				}
			}


			FLOGV("%s WantFire %d",  *Ship->GetName(), WantFire);

			LinearTargetVelocity = ChargeAxis * Ship->GetNavigationSystem()->GetLinearMaxVelocity() * 1.2;
			FLOGV("%s ChargeAxis %s",  *Ship->GetName(), *ChargeAxis.ToString());
			FLOGV("%s Charge target %s",  *Ship->GetName(), *LinearTargetVelocity.ToString());
		}
	}

	if (AttackPhase == 3)
	{
		FVector DeltaVelocity = PilotTargetShip->GetLinearVelocity() - Ship->GetLinearVelocity() / 100.;

		if (Distance > SecurityDistance)
		{
			FLOGV("%s Distance > SecurityDistance => phase0",  *Ship->GetName());
			// Security distance reach
			AttackPhase = 0;
			ClearTarget = true;
			LockTarget = false;
		}
		else if (FVector::DotProduct(DeltaLocation, DeltaVelocity) < 0)
		{

			FQuat AvoidQuat = FQuat(DeltaLocation.GetUnsafeNormal(), AttackAngle);
			FVector TopVector = Ship->GetActorRotation().RotateVector(FVector(0,0,PilotTargetShip->GetMeshScale()));
			FVector Avoid =  AvoidQuat.RotateVector(TopVector);

			LinearTargetVelocity = Avoid.GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity();
			FLOGV("%s AttackAngle %f",  *Ship->GetName(), AttackAngle);
			FLOGV("%s TopVector %s",  *Ship->GetName(), *TopVector.ToString());
			FLOGV("%s Avoid %s",  *Ship->GetName(), *Avoid.ToString());
			FLOGV("%s Escape target %s",  *Ship->GetName(), *LinearTargetVelocity.ToString());
		}
		else
		{
			UseOrbitalBoost = true;
			LinearTargetVelocity = -TargetAxis * Ship->GetNavigationSystem()->GetLinearMaxVelocity() * 2;
			AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), -TargetAxis, FVector::ZeroVector, DeltaSeconds);
			FLOGV("%s Run from target %s",  *Ship->GetName(), *LinearTargetVelocity.ToString());
		}
	}

	// Anticollision
	LinearTargetVelocity = AnticollisionCorrection(LinearTargetVelocity, AttackAngle, PilotTargetShip);

	DrawDebugLine(Ship->GetWorld(), Ship->GetActorLocation(), Ship->GetActorLocation() + LinearTargetVelocity * 100, FColor::Red, false, ReactionTime);

	if(ClearTarget)
	{
		PilotTargetShip = NULL;
	}
	// TODO ignore target





	// Manage orbital boost

}

void UFlareShipPilot::IdlePilot(float DeltaSeconds)
{
	FLOGV("%s IdlePilot",  *Ship->GetName());
	// TODO find better

	AngularTargetVelocity = FVector::ZeroVector;
	LinearTargetVelocity = - Ship->GetActorLocation().GetClampedToMaxSize(Ship->GetNavigationSystem()->GetLinearMaxVelocity());


	// Anticollision
	LinearTargetVelocity = AnticollisionCorrection(LinearTargetVelocity, AttackAngle);



	// Find friend barycenter
	// Go to friend barycenter
	// If near
		// Turn to opposite from barycentre
	// else
		// Turn to direction

	WantFire = false;
	// Manage orbital boost
	UseOrbitalBoost = false;
}

void UFlareShipPilot::FindBestHostileTarget()
{

	FLOGV("%s FindBestHostileTarget",  *Ship->GetName());
	// TODO S or L ship dispatch

	AFlareSpacecraft* TargetCandidate = NULL;


	// First search dangerous target

	if(Ship->GetWeaponsSystem()->HasUsableWeaponType(EFlareWeaponGroupType::WG_BOMB))
	{
		FLOGV("%s Has Bomb",  *Ship->GetName());
		TargetCandidate = GetNearestHostileShip(true, EFlarePartSize::L);
	}

	if(!TargetCandidate && Ship->GetWeaponsSystem()->HasUsableWeaponType(EFlareWeaponGroupType::WG_GUN))
	{
		FLOGV("%s Has Gun",  *Ship->GetName());
		TargetCandidate = GetNearestHostileShip(true, EFlarePartSize::S);
		if(!TargetCandidate)
		{
			FLOGV("%s no S target search L",  *Ship->GetName());
			TargetCandidate = GetNearestHostileShip(true, EFlarePartSize::L);
		}
	}

	// Then search non dangerous target

	if(!TargetCandidate && Ship->GetWeaponsSystem()->HasUsableWeaponType(EFlareWeaponGroupType::WG_BOMB))
	{
		TargetCandidate = GetNearestHostileShip(false, EFlarePartSize::L);
	}

	if(!TargetCandidate && Ship->GetWeaponsSystem()->HasUsableWeaponType(EFlareWeaponGroupType::WG_GUN))
	{
		TargetCandidate = GetNearestHostileShip(false, EFlarePartSize::S);
		if(!TargetCandidate)
		{
			TargetCandidate = GetNearestHostileShip(false, EFlarePartSize::L);
		}
	}

	if (TargetCandidate)
	{
		FLOGV("%s Candidate found %s",  *Ship->GetName(), *TargetCandidate->GetName());
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

			if(Score > 0 && Score > BestScore)
			{
				SelectedWeaponGroupIndex = WeaponGroupIndex;
				BestScore = Score;
			}
		}
		FLOGV("%s Candidate SelectedWeaponGroupIndex %d",  *Ship->GetName(), SelectedWeaponGroupIndex);
	}
}

FVector UFlareShipPilot::AnticollisionCorrection(FVector InitialVelocity, float PreferedAttackAngle, AFlareSpacecraft* SpacecraftToIgnore) const
{
	AFlareSpacecraft* NearestShip = GetNearestShip(true);

	if (NearestShip && NearestShip != SpacecraftToIgnore)
	{
		FVector DeltaLocation = NearestShip->GetActorLocation() - Ship->GetActorLocation();
		float Distance = FMath::Max(0.0f, (DeltaLocation.Size() - NearestShip->GetMeshScale() *4) / 100.f); // Distance in meters

		if (Distance < 100.f)
		{
			FQuat AvoidQuat = FQuat(DeltaLocation.GetUnsafeNormal(), PreferedAttackAngle);
			FVector TopVector = Ship->GetActorRotation().RotateVector(FVector(0,0,NearestShip->GetMeshScale()));
			FVector Avoid =  AvoidQuat.RotateVector(TopVector);

			// Below 100m begin avoidance maneuver
			float Alpha = 1 - Distance/100.f;
			return InitialVelocity * (1.f - Alpha) + Alpha * ((4* (1.f - Alpha) * Avoid.GetUnsafeNormal() - DeltaLocation.GetUnsafeNormal()).GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity());
		}
	}

	return InitialVelocity;
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
