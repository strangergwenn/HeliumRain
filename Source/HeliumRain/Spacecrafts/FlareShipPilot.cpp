
#include "FlareShipPilot.h"
#include "../Flare.h"

#include "FlareSpacecraft.h"
#include "FlarePilotHelper.h"

#include "../Data/FlareSpacecraftComponentsCatalog.h"

#include "../Game/FlareCompany.h"
#include "../Game/FlareGame.h"
#include "../Game/AI/FlareCompanyAI.h"
#include "../Quests/FlareQuest.h"
#include "../Quests/FlareQuestStep.h"

#include "../Player/FlarePlayerController.h"

#include "../Spacecrafts/FlareEngine.h"
#include "../Spacecrafts/FlareRCS.h"

DECLARE_CYCLE_STAT(TEXT("FlareShipPilot Tick"), STAT_FlareShipPilot_Tick, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot Military"), STAT_FlareShipPilot_Military, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot Cargo"), STAT_FlareShipPilot_Cargo, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot Fighter"), STAT_FlareShipPilot_Fighter, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot Bomber"), STAT_FlareShipPilot_Bomber, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot Missile"), STAT_FlareShipPilot_Missile, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot Idle"), STAT_FlareShipPilot_Idle, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot Flagship"), STAT_FlareShipPilot_Flagship, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot FindBestHostileTarget"), STAT_FlareShipPilot_FindBestHostileTarget, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot ExitAvoidance"), STAT_FlareShipPilot_ExitAvoidance, STATGROUP_Flare);


#define PILOT_ANTICOLLISION_SPEED -200.f

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareShipPilot::UFlareShipPilot(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	ReactionTime = FMath::FRandRange(0.4, 0.7);
	TimeUntilNextReaction = 0;
	CurrentWaitTime = 0;
	DockWaitTime = FMath::FRandRange(30, 45);
	PilotTargetLocation = FVector::ZeroVector;
	PilotTargetShip = NULL;
	LastPilotTargetShip = NULL;
	PilotTargetStation = NULL;
	PilotLastTargetStation = NULL;
	SelectedWeaponGroupIndex = -1;
	MaxFollowDistance = 0;
	LockTarget = false;
	WantFire = false;

	TimeSinceLastDockingAttempt = 0.0f;
	TimeUntilNextDockingAttempt = 0.0f;
	MaxTimeBetweenDockingAttempt = 60.0f;
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void UFlareShipPilot::TickPilot(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_Tick);

	if (Ship->IsStation())
	{
		// No pilot for stations
		return;
	}

	TimeUntilNextReaction -= DeltaSeconds;


	LinearTargetVelocity = FVector::ZeroVector;
	AngularTargetVelocity = FVector::ZeroVector;
	UseOrbitalBoost = true;

	if(!Ship->GetParent()->GetDamageSystem()->IsUncontrollable())
	{
		if (Ship->IsMilitary())
		{
			MilitaryPilot(DeltaSeconds);
		}
		else
		{
			CargoPilot(DeltaSeconds);
		}
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
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_Military);

	if (Ship->GetNavigationSystem()->IsDocked() && ( Ship->GetParent()->GetCompany() != PlayerCompany || Ship->GetParent()->GetCurrentSector()->GetSectorBattleState(PlayerCompany).HasDanger))
	{
		WantFire = false;
		// Let's undock
		Ship->GetNavigationSystem()->Undock();
		return;
	}
	else if (Ship->GetNavigationSystem()->IsAutoPilot())
	{
		WantFire = false;
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

	CurrentTactic = Ship->GetCompany()->GetTacticManager()->GetCurrentTacticForShipGroup(CombatGroup);
	FindBestHostileTarget(CurrentTactic);

	bool Idle = true;

	TimeUntilNextComponentSwitch-=DeltaSeconds;

	if(Ship->GetParent()->GetDamageSystem()->IsDisarmed())
	{
		// Idle
	}
	else if (Ship->GetSize() == EFlarePartSize::S && PilotTargetShip && SelectedWeaponGroupIndex >= 0)
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
			else if (WeaponType == EFlareWeaponGroupType::WG_MISSILE)
			{
				MissilePilot(DeltaSeconds);
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
		WantFire = false;
		FlagShipPilot(DeltaSeconds);
		Idle = false;
	}



	if (Idle)
	{
		PilotTargetShip = NULL;
		WantFire = false;
		IdlePilot(DeltaSeconds);
	}


	// TODO S or L ship dispatch


}

void UFlareShipPilot::CargoPilot(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_Cargo);

	TimeSinceLastDockingAttempt += DeltaSeconds;

	AFlareSpacecraft* PilotAvoidShip = GetNearestHostileShip(true, EFlarePartSize::S);
	if (!PilotAvoidShip)
	{
		PilotAvoidShip = GetNearestHostileShip(true, EFlarePartSize::L);
	}

	// If enemy near, run away !
	if (PilotAvoidShip)
	{
		FVector DeltaLocation = (PilotAvoidShip->GetActorLocation() - Ship->GetActorLocation()) / 100.f;
		float Distance = DeltaLocation.Size(); // Distance in meters

		// There is at least one hostile enemy
		if (Distance < 4000)
		{
			Ship->GetNavigationSystem()->AbortAllCommands();
			if(Ship->GetNavigationSystem()->IsDocked())
			{
				Ship->GetNavigationSystem()->Undock();
			}
			LinearTargetVelocity = -DeltaLocation.GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity() * 100;
			
			UseOrbitalBoost = true;
		}
		else
		{
			PilotAvoidShip = NULL;
		}
	}

	// Docking wait time
	if (!PilotAvoidShip && Ship->GetNavigationSystem()->IsDocked() && Ship->GetParent()->GetCompany() != Ship->GetGame()->GetPC()->GetCompany())
	{
		// Dock
		if (CurrentWaitTime <= 0)
		{
			CurrentWaitTime = DockWaitTime;
		}

		// Undock
		else if (CurrentWaitTime < 0.1)
		{
			Ship->GetNavigationSystem()->Undock();

			// Swap target station
			PilotLastTargetStation = PilotTargetStation;
			PilotTargetStation = NULL;
			CurrentWaitTime = 0;
		}

		// Wait
		else
		{
			CurrentWaitTime -= DeltaSeconds;
		}

		return;
	}

	// Wait maneuver
	else if (Ship->GetNavigationSystem()->IsAutoPilot())
	{
		return;
	}

	// If no station target, find a target : a random friendly station different from the last station
	else if(Ship->GetParent()->GetCompany() != Ship->GetGame()->GetPC()->GetCompany())
	{
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
			
			if (Distance < 1000 && TimeSinceLastDockingAttempt > TimeUntilNextDockingAttempt)
			{
				if (!Ship->GetNavigationSystem()->DockAt(PilotTargetStation))
				{
					TimeSinceLastDockingAttempt = 0;
					TimeUntilNextDockingAttempt = FMath::FRand() * MaxTimeBetweenDockingAttempt;
					LinearTargetVelocity = -DeltaLocation.GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity();
				}
			}
			else if (Distance < 200)
			{
				LinearTargetVelocity = FVector::ZeroVector;
			}
			else
			{
				LinearTargetVelocity = DeltaLocation.GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity();
			}
		}
	}

	// Exit avoidance

	//FLOGV("%s LinearTargetVelocity before exit avoidance = %s",  *Ship->GetImmatriculation().ToString(), *LinearTargetVelocity.ToString());

	LinearTargetVelocity = ExitAvoidance(Ship, LinearTargetVelocity, 0.4);
	AlignToTargetVelocityWithThrust(DeltaSeconds);

	//FLOGV("%s LinearTargetVelocity before anticollision = %s",  *Ship->GetImmatriculation().ToString(), *LinearTargetVelocity.ToString());

	// Anticollision
	LinearTargetVelocity = PilotHelper::AnticollisionCorrection(Ship, LinearTargetVelocity, Ship->GetPreferedAnticollisionTime(), PilotHelper::AnticollisionConfig(), PILOT_ANTICOLLISION_SPEED);

	//FLOGV("%s Location = %s Velocity = %s LinearTargetVelocity = %s",  *Ship->GetImmatriculation().ToString(), * Ship->GetActorLocation().ToString(), *Ship->GetLinearVelocity().ToString(), *LinearTargetVelocity.ToString());
}

void UFlareShipPilot::FighterPilot(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_Fighter);

	float AmmoVelocity = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Weapons[0]->GetAmmoVelocity() * 100;

	FVector BaseLocation = Ship->GetCamera()->GetComponentLocation();

	bool DangerousTarget = PilotHelper::IsShipDangerous(PilotTargetShip);

	//float PreferedVelocity = FMath::Max(PilotTargetShip->GetLinearVelocity().Size() * 2.0f, Ship->GetNavigationSystem()->GetLinearMaxVelocity());
	float PreferedVelocity = Ship->GetNavigationSystem()->GetLinearMaxVelocity() * 2;

	//FLOGV("%s target %s",  *Ship->GetImmatriculation().ToString(),  *PilotTargetShip->GetImmatriculation().ToString());
	// The pilot have a target, track and kill it

	FVector LocalNose = FVector(1.f, 0.f, 0.f);
	FVector DeltaLocation = (PilotTargetComponent->GetComponentLocation() - BaseLocation) / 100.f;
	float Distance = DeltaLocation.Size(); // Distance in meters
	float TargetSize = PilotTargetShip->GetMeshScale() / 100.f; // Radius in meters
	FVector TargetAxis = DeltaLocation.GetUnsafeNormal();
	FVector ShipVelocity = 100 * Ship->GetLinearVelocity();
	FVector PilotTargetShipVelocity = 100 * PilotTargetShip->GetLinearVelocity();

	// Use position prediction
	float PredictionDelay = DeltaSeconds;
	FVector PredictedShipLocation = BaseLocation + ShipVelocity * PredictionDelay;
	FVector PredictedPilotTargetShipLocation = PilotTargetComponent->GetComponentLocation() + PilotTargetShipVelocity * PredictionDelay;
	FVector PredictedDeltaLocation = (PredictedPilotTargetShipLocation - PredictedShipLocation) / 100.f;
	FVector PredictedTargetAxis = PredictedDeltaLocation.GetUnsafeNormal();
	float PredictedDistance = PredictedDeltaLocation.Size(); // Distance in meters

	FVector AmmoIntersectionLocation;
	float AmmoIntersectionTime = SpacecraftHelper::GetIntersectionPosition(PilotTargetComponent->GetComponentLocation(), PilotTargetShip->Airframe->GetPhysicsLinearVelocity(), BaseLocation, ShipVelocity, AmmoVelocity, 0, &AmmoIntersectionLocation);



	FVector FireTargetAxis;
	if (AmmoIntersectionTime > 0)
	{
		FireTargetAxis = (AmmoIntersectionLocation - BaseLocation).GetUnsafeNormal();
	}
	else
	{
		FireTargetAxis = (PilotTargetComponent->GetComponentLocation() - BaseLocation).GetUnsafeNormal();
	}


	FVector AmmoIntersectionPredictedLocation;
	float AmmoIntersectionPredictedTime = SpacecraftHelper::GetIntersectionPosition(PilotTargetComponent->GetComponentLocation(), PilotTargetShip->Airframe->GetPhysicsLinearVelocity(), BaseLocation, ShipVelocity, AmmoVelocity, PredictionDelay, &AmmoIntersectionPredictedLocation);

	/*if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
	{
		DrawDebugSphere(Ship->GetWorld(), AmmoIntersectionPredictedLocation, 100, 12, FColor::White, true);
	}*/



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

	FVector FireAxis = Ship->Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalNose);

	float TargetAxisAngularPrecisionDot = FVector::DotProduct(PredictedFireTargetAxis, FireAxis);
	float TargetAxisAngularPrecision = FMath::Acos(TargetAxisAngularPrecisionDot);
	float AngularNoise;

	float Experience = 0.5;

	if(WantFire)
	{
		TimeSinceAiming -= 10 * DeltaSeconds / Experience;
		TimeSinceAiming = FMath::Max(0.f, TimeSinceAiming);
	}
	if(FMath::RadiansToDegrees(TargetAxisAngularPrecision) < 30)
	{
		TimeSinceAiming += DeltaSeconds;
		AngularNoise = 20 / (1+ Experience * FMath::Square(TimeSinceAiming)) ; // 10 degree
	}
	else
	{
		TimeSinceAiming = 0;
		AngularNoise = 20; // 1 degree
	}


	/*if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
	{
		FLOGV("TargetAxisAngularPrecisionDot=%f", TargetAxisAngularPrecisionDot);
		FLOGV("TargetAxisAngularPrecision=%f", TargetAxisAngularPrecision);
		FLOGV("TimeSinceAiming=%f", TimeSinceAiming);
		FLOGV("AngularNoise=%f", AngularNoise);
		FLOGV("Distance=%f", Distance);
		FLOGV("AttackPhase=%d", AttackPhase);
		AngularNoise = 0;
	}*/

	FVector PredictedFireTargetAxisWithError = FMath::VRandCone(PredictedFireTargetAxis, FMath::DegreesToRadians(AngularNoise));


	/*if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
	{
		DrawDebugLine(Ship->GetWorld(), Ship->GetActorLocation(), BaseLocation + PredictedFireTargetAxis *200000, FColor::Green, true);
	}*/

	AngularTargetVelocity = GetAngularVelocityToAlignAxis(LocalNose, PredictedFireTargetAxisWithError, PredictedTargetAngularVelocity, DeltaSeconds);

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
			if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
			{
				FLOG("Pass to phase 1");
			}
			LastTargetDistance = Distance;
		}
		else
		{
			LinearTargetVelocity = PredictedFireTargetAxis * PreferedVelocity;
			LinearTargetVelocity += LinearTargetVelocity.GetUnsafeNormal() * FVector::DotProduct(PilotTargetShip->GetLinearVelocity(),LinearTargetVelocity.GetUnsafeNormal());

			UseOrbitalBoost = true;
		}
	}

	if (AttackPhase == 1)
	{
		if (FVector::DotProduct(DeltaLocation, DeltaVelocity) > 0) // TODO : check if I can't use dot product
		{
			// Target is passed
			AttackPhase = 2;
			if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
			{
				FLOG("Pass to phase 2");
			}
		}
		else
		{
			FQuat AttackDistanceQuat = FQuat(TargetAxis, AttackAngle);
			FVector TopVector = Ship->GetActorRotation().RotateVector(FVector(0,0,AttackDistance));
			FVector AttackMargin =  AttackDistanceQuat.RotateVector(TopVector);

			if(Distance < SecurityDistance)
			{
				PreferedVelocity /=2;
			}


			LinearTargetVelocity = (AttackMargin + DeltaLocation).GetUnsafeNormal() * PreferedVelocity;
			LinearTargetVelocity += LinearTargetVelocity.GetUnsafeNormal() * FVector::DotProduct(PilotTargetShip->GetLinearVelocity(),LinearTargetVelocity.GetUnsafeNormal());

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
			LinearTargetVelocity = PredictedFireTargetAxis * PreferedVelocity;
			LinearTargetVelocity += LinearTargetVelocity.GetUnsafeNormal() * FVector::DotProduct(PilotTargetShip->GetLinearVelocity(),LinearTargetVelocity.GetUnsafeNormal());

			AttackPhase = 0;
			if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
			{
				FLOG("Pass to phase 0");
			}
			ClearTarget = true;
		}
		else
		{
			LinearTargetVelocity = -DeltaLocation.GetUnsafeNormal() * PreferedVelocity;
			LinearTargetVelocity += LinearTargetVelocity.GetUnsafeNormal() * FVector::DotProduct(PilotTargetShip->GetLinearVelocity(),LinearTargetVelocity.GetUnsafeNormal());

			if (DangerousTarget)
			{
				UseOrbitalBoost = true;
			}
		}
	}

	if (TimeUntilNextReaction <= 0)
	{
		TimeUntilNextReaction = ReactionTime;

		WantFire = false;

		/*if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
		{
			FLOGV("FireAxis=%s", *FireAxis.ToString());

			DrawDebugSphere(Ship->GetWorld(), BaseLocation, 50, 12, FColor::Green, true);
			//DrawDebugSphere(Ship->GetWorld(), AmmoIntersectionLocation, 100, 12, FColor::Red, true);
		}*/

		if(Ship->GetCurrentTarget() != PilotTargetShip)
		{
			Ship->SetCurrentTarget(PilotTargetShip);
		}

		// If at range and aligned fire on the target
		// TODO increase tolerance if target is near
		if (AmmoIntersectionTime > 0 && AmmoIntersectionTime < 1.5)
		{
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
					FVector GunAmmoIntersectionLocation = AmmoIntersectionLocation;
					float GunAmmoIntersectionTime = AmmoIntersectionTime;



					//FVector GunFireTargetAxis = (GunAmmoIntersectionLocation - MuzzleLocation - AmmoIntersectionPredictedTime * ShipVelocity).GetUnsafeNormal();
					FVector GunFireTargetAxis = (GunAmmoIntersectionLocation - MuzzleLocation).GetUnsafeNormal();
					/*FLOGV("Gun %d FireAxis=%s", GunIndex, *FireAxis.ToString());
					FLOGV("Gun %d GunFireTargetAxis=%s", GunIndex, *GunFireTargetAxis.ToString());
		*/
					float AngularPrecisionDot = FVector::DotProduct(GunFireTargetAxis, Weapon->ComputeParallaxCorrection(GunIndex));
					float AngularPrecision = FMath::Acos(AngularPrecisionDot);
					float AngularSize = FMath::Atan(TargetSize / Distance);


					/*if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
					{

						DrawDebugSphere(Ship->GetWorld(), GunAmmoIntersectionLocation, 50, 12, FColor::Red, true);
						DrawDebugLine(Ship->GetWorld(), MuzzleLocation, GunAmmoIntersectionLocation, FColor::Red, true);
						DrawDebugLine(Ship->GetWorld(), MuzzleLocation, MuzzleLocation + GunFireTargetAxis *100000, FColor::Blue, true);
						//DrawDebugLine(Ship->GetWorld(), MuzzleLocation, MuzzleLocation + GunFireTargetAxis2 *100000, FColor::Green, true);
						DrawDebugLine(Ship->GetWorld(), MuzzleLocation, MuzzleLocation + Weapon->ComputeParallaxCorrection(GunIndex) *100000, FColor::White, true);
					}*/

					/*if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
					{
						FLOGV("GunFireTargetAxis=%s", *GunFireTargetAxis.ToString());
						FLOGV("GunFireTargetAxis=%s", *GunFireTargetAxis.ToString());
						FLOGV("ComputeParallaxCorrection=%s", *Weapon->ComputeParallaxCorrection(GunIndex).ToString());

						FLOGV("Gun %d Distance=%f", GunIndex, Distance);
						FLOGV("Gun %d TargetSize=%f", GunIndex, TargetSize);
						FLOGV("Gun %d AngularSize=%f", GunIndex, AngularSize);
						FLOGV("Gun %d AngularPrecision=%f", GunIndex, AngularPrecision);
						FLOGV("Gun %d limit=%f", GunIndex, (DangerousTarget ? AngularSize * 0.5 : AngularSize * 0.2));
					}*/
					if (AngularPrecision < (DangerousTarget ? AngularSize * 0.5 : AngularSize * 0.2))
					{
						/*if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
						{
							FLOG("Gun precision ok, check FF");
						}*/
						if (!PilotHelper::CheckFriendlyFire(Ship->GetGame()->GetActiveSector(), PlayerCompany, MuzzleLocation, ShipVelocity, AmmoVelocity, GunFireTargetAxis, GunAmmoIntersectionTime, 0))
						{
							FVector Location = PilotTargetShip->GetActorLocation();
							FVector Velocity = Cast<UPrimitiveComponent>(PilotTargetShip->GetRootComponent())->GetPhysicsLinearVelocity() / 100;
							Weapon->SetTarget(Location, Velocity);
							WantFire = true;
							/*if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
							{
								FLOG("Gun fire");
								//DrawDebugSphere(Ship->GetWorld(), PilotTargetShip->GetRootComponent()->Get, 50, 12, FColor::Cyan, true);
							}*/


							break;
						}
					}
				}
				if (WantFire)
				{
					break;
				}
			}
		}

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

	// Exit avoidance
	if (!PilotTargetShip
			||
			(!PilotTargetShip->GetParent()->GetDamageSystem()->IsUncontrollable()
			&& PilotTargetShip != Ship->GetGame()->GetPC()->GetShipPawn()))
	{
		LinearTargetVelocity = ExitAvoidance(Ship, LinearTargetVelocity, 0.8);
	}

	// Anticollision
	LinearTargetVelocity = PilotHelper::AnticollisionCorrection(Ship, LinearTargetVelocity, Ship->GetPreferedAnticollisionTime(), PilotHelper::AnticollisionConfig(), PILOT_ANTICOLLISION_SPEED);


	if (ClearTarget)
	{
		PilotTargetShip = NULL;
	}
}

void UFlareShipPilot::BomberPilot(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_Bomber);

	// Weapon info
	UFlareWeapon* CurrentWeapon = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Weapons[0];
	bool IsSalvage = CurrentWeapon->GetDescription()->WeaponCharacteristics.DamageType == EFlareShellDamageType::HeavySalvage
		|| CurrentWeapon->GetDescription()->WeaponCharacteristics.DamageType == EFlareShellDamageType::LightSalvage;

	// Get speed and location data
	LinearTargetVelocity = FVector::ZeroVector;
	FVector DeltaLocation = (PilotTargetComponent->GetComponentLocation() - Ship->GetActorLocation()) / 100.f;
	FVector TargetAxis = DeltaLocation.GetUnsafeNormal();
	float Distance = DeltaLocation.Size(); // Distance in meters

	// Attack Phases
	// 0 - Prepare attack : change velocity to approch the target
	// 1 - Attacking : target is approching with boost
	// 3 - Drop : Drop util its not safe to stay
	// 2 - Withdraw : target is passed, wait a security distance to attack again

	// Get mass coefficient for use as a reference
	float WeigthCoef = FMath::Sqrt(Ship->GetSpacecraftMass()) / FMath::Sqrt(5425.f) * (2-Ship->GetParent()->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_RCS)) ; // 1 for ghoul at 100%
	float PreferedVelocity = FMath::Max(PilotTargetShip->GetLinearVelocity().Size() * 3.0f, Ship->GetNavigationSystem()->GetLinearMaxVelocity());
	TimeUntilNextReaction /= 5;

	// Compute distances and reaction times
	float ApproachDistance = 15 * PreferedVelocity * WeigthCoef ;
	float AlignTime = 12 * WeigthCoef;
	float DropTime = (IsSalvage ? 3 : 5) * WeigthCoef;
	float EvadeTime = 2 * WeigthCoef;
	float TimeBetweenDrop = (IsSalvage ? 5.0 : 0.5) * WeigthCoef;
	float SecurityDistance = 1500;
	
	// Setup behaviour flags
	UseOrbitalBoost = false;
	bool ClearTarget = false;
	bool AlignToSpeed = false;
	bool HardBoost = false;
	bool Anticollision = true;

	// Approach phase
	if (AttackPhase == 0)
	{
		// Target is approching, prepare attack
		if (Distance < ApproachDistance)
		{
			AttackPhase = 1;
			LockTarget = true;
		}
		else
		{
			LinearTargetVelocity = TargetAxis * PreferedVelocity;
			AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), TargetAxis, FVector::ZeroVector, DeltaSeconds);
			UseOrbitalBoost = true;
		}
	}

	// Attack phase
	if (AttackPhase == 1)
	{
		FVector AmmoIntersectionLocation;
		float AmmoVelocity = CurrentWeapon->GetAmmoVelocity() * 100;
		float AmmoIntersectionTime = SpacecraftHelper::GetIntersectionPosition(PilotTargetComponent->GetComponentLocation(), PilotTargetShip->Airframe->GetPhysicsLinearVelocity(), Ship->GetActorLocation(), Ship->Airframe->GetPhysicsLinearVelocity(), AmmoVelocity, 0.0, &AmmoIntersectionLocation);

		AlignToSpeed = true;

		// Near enough, start bombing
		if (AmmoIntersectionTime > 0 && AmmoIntersectionTime < DropTime)
		{
			AttackPhase = 2;
			LastWantFire = false;
			TimeBeforeNextDrop = 0;
		}
		else if (AmmoIntersectionTime > 0 && AmmoIntersectionTime < AlignTime)
		{
			FVector ChargeAxis = (AmmoIntersectionLocation - Ship->GetActorLocation()).GetUnsafeNormal();
			LinearTargetVelocity = ChargeAxis * PreferedVelocity;
			UseOrbitalBoost = true;
			HardBoost = true;
			Anticollision = false;
		}
		else
		{
			LinearTargetVelocity = TargetAxis * PreferedVelocity;
		}

		LastTargetDistance = Distance;
	}

	// Bombing phase
	if (AttackPhase == 2)
	{
		FVector AmmoIntersectionLocation;
		float AmmoVelocity = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Weapons[0]->GetAmmoVelocity() * 100;
		float AmmoIntersectionTime = SpacecraftHelper::GetIntersectionPosition(PilotTargetComponent->GetComponentLocation(), PilotTargetShip->Airframe->GetPhysicsLinearVelocity(), Ship->GetActorLocation(), Ship->Airframe->GetPhysicsLinearVelocity(), AmmoVelocity, 0.0, &AmmoIntersectionLocation);
		FVector FrontVector = Ship->GetFrontVector();
		FVector ChargeAxis = (AmmoIntersectionLocation - Ship->GetActorLocation()).GetUnsafeNormal();

		Anticollision = false;
		AlignToSpeed = true;

		// Security distance reached
		if (AmmoIntersectionTime < EvadeTime || FVector::DotProduct(FrontVector, ChargeAxis) < 0.6 || AmmoIntersectionTime > AlignTime)
		{
			AttackPhase = 3;
		}
		else if (FVector::DotProduct(FrontVector, ChargeAxis) > 0.9 && AmmoIntersectionTime < DropTime)
		{
			if (TimeBeforeNextDrop > 0)
			{
				TimeBeforeNextDrop -= DeltaSeconds;
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

			LinearTargetVelocity = ChargeAxis * PreferedVelocity;
		}
	}

	// Evasion phase
	if (AttackPhase == 3)
	{
		// Security distance reached
		FVector DeltaVelocity = (PilotTargetShip->GetLinearVelocity() - Ship->GetLinearVelocity()) / 100.;
		if (Distance > SecurityDistance)
		{
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

			LinearTargetVelocity = Avoid.GetUnsafeNormal() * PreferedVelocity;
			UseOrbitalBoost = true;
			HardBoost = true;
		}
		else
		{
			UseOrbitalBoost = true;
			HardBoost = true;
			LinearTargetVelocity = -TargetAxis * PreferedVelocity * 2;
			AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), -TargetAxis, FVector::ZeroVector, DeltaSeconds);
		}
	}
	
	// Compute speed
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
		PilotHelper::AnticollisionConfig IgnoreConfig;
		IgnoreConfig.SpacecraftToIgnore = PilotTargetShip;
		LinearTargetVelocity = PilotHelper::AnticollisionCorrection(Ship, LinearTargetVelocity, Ship->GetPreferedAnticollisionTime(), IgnoreConfig, PILOT_ANTICOLLISION_SPEED);
	}
	
	if (ClearTarget)
	{
		PilotTargetShip = NULL;
	}
}

void UFlareShipPilot::MissilePilot(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_Missile);

	// Weapon info
	UFlareWeapon* CurrentWeapon = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Weapons[0];
	bool IsSalvage = CurrentWeapon->GetDescription()->WeaponCharacteristics.DamageType == EFlareShellDamageType::HeavySalvage
		|| CurrentWeapon->GetDescription()->WeaponCharacteristics.DamageType == EFlareShellDamageType::LightSalvage;

	// Get speed and location data
	LinearTargetVelocity = FVector::ZeroVector;
	FVector DeltaLocation = (PilotTargetComponent->GetComponentLocation() - Ship->GetActorLocation()) / 100.f;
	FVector TargetAxis = DeltaLocation.GetUnsafeNormal();
	float Distance = DeltaLocation.Size(); // Distance in meters

	// Attack Phases
	// 0 - Prepare attack : change velocity to approch the target
	// 1 - Attacking : target is approching with boost
	// 3 - Drop : Drop util its not safe to stay
	// 2 - Withdraw : target is passed, wait a security distance to attack again

	// Get mass coefficient for use as a reference
	float WeigthCoef = FMath::Sqrt(Ship->GetSpacecraftMass()) / FMath::Sqrt(5425.f) * (2-Ship->GetParent()->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_RCS)) ; // 1 for ghoul at 100%
	float PreferedVelocity = FMath::Max(PilotTargetShip->GetLinearVelocity().Size() * 3.0f, Ship->GetNavigationSystem()->GetLinearMaxVelocity());
	TimeUntilNextReaction /= 5;

	// Compute distances and reaction times
	float ApproachDistance = 500000; // 5 km
	float TimeBetweenDrop = (IsSalvage ? 5.0 : 0.5) * WeigthCoef;

	// Setup behaviour flags
	UseOrbitalBoost = false;

	if (Distance < ApproachDistance)
	{
		LinearTargetVelocity = -TargetAxis * PreferedVelocity * 0.2;
	}
	else
	{
		LinearTargetVelocity = TargetAxis * PreferedVelocity;
	}

	AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), TargetAxis, FVector::ZeroVector, DeltaSeconds);
	UseOrbitalBoost = true;
	FVector FrontVector = Ship->GetFrontVector();

	if (Distance < ApproachDistance && FVector::DotProduct(FrontVector, TargetAxis) > 0.8)
	{
		// Attack
		if (TimeBeforeNextDrop > 0)
		{
			TimeBeforeNextDrop -= DeltaSeconds;
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
	}

	// Anticollision
	PilotHelper::AnticollisionConfig IgnoreConfig;
	IgnoreConfig.SpacecraftToIgnore = PilotTargetShip;
	LinearTargetVelocity = PilotHelper::AnticollisionCorrection(Ship, LinearTargetVelocity, Ship->GetPreferedAnticollisionTime(), IgnoreConfig, PILOT_ANTICOLLISION_SPEED);
}


void UFlareShipPilot::IdlePilot(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_Idle);

	// TODO find better
	//UseOrbitalBoost = false;

	// If there is ennemy fly away
	AFlareSpacecraft* PilotAvoidShip = GetNearestHostileShip(true, EFlarePartSize::S);
	if (!PilotAvoidShip)
	{
		PilotAvoidShip = GetNearestHostileShip(true, EFlarePartSize::L);
	}

	// If enemy near, run away !
	if (PilotAvoidShip)
	{
		FVector DeltaLocation = (PilotAvoidShip->GetActorLocation() - Ship->GetActorLocation()) / 100.f;
		float Distance = DeltaLocation.Size(); // Distance in meters

		// There is at least one hostile enemy
		if (Distance < 10000) // 10 km
		{
			Ship->GetNavigationSystem()->AbortAllCommands();
			LinearTargetVelocity = -DeltaLocation.GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity();

			UseOrbitalBoost = true;
		}

		if (Distance > 2000 && Ship->GetParent()->GetDamageSystem()->GetTemperature() > Ship->GetParent()->GetDamageSystem()->GetOverheatTemperature() * 0.95)
		{
			// Too hot and no imminent danger
			//UseOrbitalBoost = false;
		}
	}

	// If not, and outside the player fleet, find a leader and follow it
	else if (Ship->GetCompany() != Ship->GetGame()->GetPC()->GetCompany())
	{
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
	LinearTargetVelocity = ExitAvoidance(Ship, LinearTargetVelocity, 0.5);

	AlignToTargetVelocityWithThrust(DeltaSeconds);

	// Anticollision
	LinearTargetVelocity = PilotHelper::AnticollisionCorrection(Ship, LinearTargetVelocity, Ship->GetPreferedAnticollisionTime(), PilotHelper::AnticollisionConfig(), PILOT_ANTICOLLISION_SPEED);

    //FLOGV("%s Leader ship LinearTargetVelocity=%s", *LinearTargetVelocity.ToString());
}

void UFlareShipPilot::FlagShipPilot(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_Flagship);

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
	LinearTargetVelocity = (PilotTargetLocation - Ship->GetActorLocation()).GetUnsafeNormal()  * Ship->GetNavigationSystem()->GetLinearMaxVelocity() * 2;

	LinearTargetVelocity += LinearTargetVelocity.GetUnsafeNormal() * FVector::DotProduct(PilotTargetShip->GetLinearVelocity(),LinearTargetVelocity.GetUnsafeNormal());

	// TODO Bomb avoid

	AlignToTargetVelocityWithThrust(DeltaSeconds);

	AngularTargetVelocity = AngularTargetVelocity.GetClampedToMaxSize(0.1f);

	// Exit avoidance
	if(!PilotTargetShip || PilotTargetShip != Ship->GetGame()->GetPC()->GetShipPawn())
	{
		LinearTargetVelocity = ExitAvoidance(Ship, LinearTargetVelocity, 0.5);
	}

	// Anticollision
	LinearTargetVelocity = PilotHelper::AnticollisionCorrection(Ship, LinearTargetVelocity, Ship->GetPreferedAnticollisionTime(), PilotHelper::AnticollisionConfig(), PILOT_ANTICOLLISION_SPEED);

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
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_FindBestHostileTarget);

	AFlareSpacecraft* TargetCandidate = NULL;

	struct PilotHelper::TargetPreferences TargetPreferences;
	TargetPreferences.IsLarge = 1;
	TargetPreferences.IsSmall = 1;
	TargetPreferences.IsStation = 0;
	TargetPreferences.IsNotStation = 1;
	TargetPreferences.IsMilitary = 1;
	TargetPreferences.IsNotMilitary = 0.1;
	TargetPreferences.IsDangerous = 1;
	TargetPreferences.IsNotDangerous = 0.01;
	TargetPreferences.IsStranded = 1;
	TargetPreferences.IsNotStranded = 0.5;
	TargetPreferences.IsUncontrollableCivil = 0.0;
	TargetPreferences.IsUncontrollableSmallMilitary = 0.0;
	TargetPreferences.IsUncontrollableLargeMilitary = 0.0;
	TargetPreferences.IsNotUncontrollable = 1;
	TargetPreferences.IsHarpooned = 0;
	TargetPreferences.TargetStateWeight = 1;
	TargetPreferences.MaxDistance = 1000000;
	TargetPreferences.DistanceWeight = 0.5;
	TargetPreferences.AttackTarget = NULL;
	TargetPreferences.AttackTargetWeight = 1;
	TargetPreferences.LastTarget = PilotTargetShip;
	TargetPreferences.LastTargetWeight = 20;
	TargetPreferences.PreferredDirection = Ship->GetFrontVector();
	TargetPreferences.MinAlignement = -1;
	TargetPreferences.AlignementWeight = 0.5;
	TargetPreferences.BaseLocation = Ship->GetActorLocation();

	Ship->GetWeaponsSystem()->GetTargetPreference(&TargetPreferences.IsSmall, &TargetPreferences.IsLarge, &TargetPreferences.IsUncontrollableCivil , &TargetPreferences.IsUncontrollableSmallMilitary , &TargetPreferences.IsUncontrollableLargeMilitary, &TargetPreferences.IsNotUncontrollable, &TargetPreferences.IsStation, &TargetPreferences.IsHarpooned);


	float MinAmmoRatio = 1.f;


	UFlareSpacecraftComponentsCatalog* Catalog = Ship->GetGame()->GetShipPartsCatalog();
	for (int32 ComponentIndex = 0; ComponentIndex < Ship->GetData().Components.Num(); ComponentIndex++)
	{
		FFlareSpacecraftComponentSave* ComponentData = &Ship->GetData().Components[ComponentIndex];
		FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

		if (ComponentDescription->Type == EFlarePartType::Weapon)
		{
			float AmmoRatio = float(ComponentDescription->WeaponCharacteristics.AmmoCapacity - ComponentData->Weapon.FiredAmmo) /  ComponentDescription->WeaponCharacteristics.AmmoCapacity;
			if(AmmoRatio < MinAmmoRatio)
			{
				MinAmmoRatio = AmmoRatio;
			}
		}
	}
	//FLOGV("%s MinAmmoRatio=%f", *Ship->GetImmatriculation().ToString(), MinAmmoRatio);

	if(MinAmmoRatio <0.9)
	{
		TargetPreferences.IsUncontrollableSmallMilitary = 0.0;
	}

	if(MinAmmoRatio < 0.5)
	{
		TargetPreferences.IsNotMilitary = 0.0;
	}

	if (Tactic == EFlareCombatTactic::AttackStations)
	{
		TargetPreferences.IsStation = 10.0f;
	}
	else if (Tactic == EFlareCombatTactic::AttackMilitary)
	{
		TargetPreferences.IsStation = 0.0f;
	}
	else if (Tactic == EFlareCombatTactic::AttackCivilians)
	{
		TargetPreferences.IsStation = 0.0f;
		TargetPreferences.IsMilitary = 0.1;
		TargetPreferences.IsNotMilitary = 1.0;
		TargetPreferences.IsNotDangerous = 1.0;
	}
	else if (Tactic == EFlareCombatTactic::ProtectMe)
	{
		TargetPreferences.IsStation = 0.0f;
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
		bool NewTarget = false;
		bool NewWeapon = false;

		if(PilotTargetShip != TargetCandidate)
		{
			if(LastPilotTargetShip != PilotTargetShip && LastPilotTargetShip != TargetCandidate)
			{
				TimeSinceAiming = 0;
			}

			PilotTargetShip = TargetCandidate;
			Ship->SetCurrentTarget(PilotTargetShip);
			LastPilotTargetShip = TargetCandidate;

			NewTarget = true;
		}

		// Find best weapon
		int32 WeaponGroupIndex = Ship->GetWeaponsSystem()->FindBestWeaponGroup(PilotTargetShip);
		if (SelectedWeaponGroupIndex != WeaponGroupIndex)
		{
			SelectedWeaponGroupIndex = WeaponGroupIndex;
			NewWeapon = true;
		}

		if (NewTarget || NewWeapon)
		{
			AttackPhase = 0;
			AttackAngle = FMath::FRandRange(0, 360);
			float TargetSize = PilotTargetShip->GetMeshScale() / 100.f; // Radius in meters
			AttackDistance = FMath::FRandRange(100, 200) + TargetSize;
			MaxFollowDistance = TargetSize * 60; // Distance in meters
			LockTarget = false;
		}

	}
	else
	{
		PilotTargetShip = NULL;
		SelectedWeaponGroupIndex = -1;
	}
}



int32 UFlareShipPilot::GetPreferedWeaponGroup() const
{
	return SelectedWeaponGroupIndex;
}

/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

FVector UFlareShipPilot::ExitAvoidance(AFlareSpacecraft* TargetShip, FVector InitialVelocityTarget, float CurveTrajectoryLimit) const
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_ExitAvoidance);

	// DEBUG
	/*if(TargetShip->GetImmatriculation() != "PIRSPX-Arrow")
	{

		return InitialVelocityTarget;
	}*/


	float TimeToStop;


	FVector CurrentVelocity = Ship->GetLinearVelocity();

	if (FMath::IsNearlyZero(CurrentVelocity.SizeSquared()))
	{
		TimeToStop = 0;
	}
	else
	{

		FVector CurrentVelocityAxis = CurrentVelocity.GetUnsafeNormal();

		// TODO Cache
		TArray<UActorComponent*> Engines = Ship->GetComponentsByClass(UFlareEngine::StaticClass());


		FVector Acceleration = Ship->GetNavigationSystem()->GetTotalMaxThrustInAxis(Engines, CurrentVelocityAxis, false) / Ship->GetSpacecraftMass();
		float AccelerationInAngleAxis =  FMath::Abs(FVector::DotProduct(Acceleration, CurrentVelocityAxis));

		TimeToStop= (CurrentVelocity.Size() / (AccelerationInAngleAxis));
	}

	float DistanceToStop = (CurrentVelocity.Size() / (2)) * (Ship->GetPreferedAnticollisionTime());


	FVector SectorCenter = TargetShip->GetGame()->GetActiveSector()->GetSectorCenter();


/*	DrawDebugLine(TargetShip->GetWorld(), TargetShip->GetActorLocation(), TargetShip->GetGame()->GetPC()->GetPlayerShip()->GetActive()->GetActorLocation(), FColor::Cyan, false, 1.0);
	DrawDebugLine(TargetShip->GetWorld(),  FVector::ZeroVector, TargetShip->GetGame()->GetPC()->GetPlayerShip()->GetActive()->GetActorLocation(), FColor::Cyan, false, 1.0);
	DrawDebugLine(TargetShip->GetWorld(), FVector::ZeroVector, TargetShip->GetActorLocation(), FColor::Cyan, false, 1.0);


	DrawDebugLine(TargetShip->GetWorld(), TargetShip->GetActorLocation(), TargetShip->GetActorLocation() + TargetShip->GetVelocity() * 1000, FColor::Red, false, 1.0);
	DrawDebugLine(TargetShip->GetWorld(), TargetShip->GetActorLocation(), TargetShip->GetActorLocation() + InitialVelocityTarget * 1000, FColor::Blue, false, 1.0);
*/

	float ShipCenterDistance = (TargetShip->GetActorLocation() - SectorCenter).Size();
	float SectorLimits = TargetShip->GetGame()->GetActiveSector()->GetSectorLimits() / 3;
	/*FLOGV("%s ExitAvoidance ShipCenterDistance=%f SectorLimits=%f InitialVelocityTarget=%s",
		  *TargetShip->GetImmatriculation().ToString(),
		  ShipCenterDistance /100, SectorLimits /100, *InitialVelocityTarget.ToString());


	FLOGV("CurrentVelocity %s", *CurrentVelocity.ToString());
	FLOGV("TimeToFinalVelocity %f", TimeToStop);
	FLOGV("DistanceToStop %f", DistanceToStop);
	FLOGV("SectorLimits * CurveTrajectoryLimit %f", SectorLimits * CurveTrajectoryLimit);
*/
	if(DistanceToStop * 100 > SectorLimits * CurveTrajectoryLimit)
	{
		float OverflowRatio = (DistanceToStop * 100) / (SectorLimits * CurveTrajectoryLimit);
		// Clamp target velocity size
		InitialVelocityTarget = InitialVelocityTarget.GetClampedToSize(0, (CurrentVelocity.Size()) / OverflowRatio);

	//	FLOGV("Clamp target velocity %s (%f) OverflowRatio %f", *InitialVelocityTarget.ToString(), InitialVelocityTarget.Size(), OverflowRatio);
	}


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

			/*FLOGV("CurveRatio %f", CurveRatio);

			FLOGV("InitialVelocityTargetDirection %s", *InitialVelocityTargetDirection.ToCompactString());
			FLOGV("ExitAvoidanceDirection %s", *ExitAvoidanceDirection.ToCompactString());
			FLOGV("ExitAvoidanceVelocity %s %f", *ExitAvoidanceVelocity.ToCompactString(), ExitAvoidanceVelocity.Size());
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

	if (!Ship || !Ship->GetGame()->GetActiveSector())
	{
		return NULL;
	}

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

		// Tutorial execption
		if(ShipCandidate->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip()
				&& ShipCandidate->GetCurrentTarget() == Ship
				&& Ship->GetGame()->GetQuestManager()->FindQuest("tutorial-fighter")->GetStatus() == EFlareQuestStatus::ONGOING
				&& Ship->GetGame()->GetQuestManager()->FindQuest("tutorial-fighter")->GetCurrentStep()->GetIdentifier() == "hit-cargo")
		{
			// don't skip
		}
		else if (Ship->GetCompany()->GetWarState(ShipCandidate->GetCompany()) != EFlareHostility::Hostile)
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

	FVector AngularVelocity = Ship->Airframe->GetPhysicsAngularVelocityInDegrees();
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

	float AngleToStop = (DeltaVelocity.Size() / 2) * (FMath::Max(TimeToFinalVelocity,DeltaSeconds));

	FVector RelativeResultSpeed;

	if (AngleToStop > angle) {
		RelativeResultSpeed = TargetAngularVelocity;
	}
	else
	{
		float MaxPreciseSpeed = FMath::Min((angle - AngleToStop) / (DeltaSeconds * 0.75f), Ship->GetNavigationSystem()->GetAngularMaxVelocity());

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
