
#include "../Flare.h"

#include "FlareShipPilot.h"
#include "FlareShip.h"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareShipPilot::UFlareShipPilot(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	ReactionTime = FMath::FRandRange(0.2, 0.3);
	TimeUntilNextReaction = 0;
	PilotTargetLocation = FVector::ZeroVector;
	PilotTargetShip = NULL;
	PilotTargetStation = NULL;
	PilotLastTargetStation = NULL;
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void UFlareShipPilot::TickPilot(float DeltaSeconds)
{
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

void UFlareShipPilot::Initialize(const FFlareShipPilotSave* Data, UFlareCompany* Company, AFlareShip* OwnerShip)
{
	// Main data
	Ship = OwnerShip;
	PlayerCompany = Company;

	// Setup properties
	if (Data)
	{
		ShipPilotData = *Data;
	}
}

/*----------------------------------------------------
	Pilot functions
----------------------------------------------------*/
void UFlareShipPilot::MilitaryPilot(float DeltaSeconds)
{

	if (Ship->GetStatus() == EFlareShipStatus::SS_Docked)
	{
		// Let's undock
		Ship->Undock();
		return;
	}
	else if (Ship->GetStatus() == EFlareShipStatus::SS_AutoPilot)
	{
		// Wait manoeuver
		return;
	}

	TArray<UFlareWeapon*> Weapons = Ship->GetWeaponList();
	float AmmoVelocity = 100;
	if (Weapons.Num() > 0)
	{
		AmmoVelocity = Weapons[0]->GetAmmoVelocity();
	}

	if (Ship->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon) <= 0)
	{
		// Go repair or refill ammo
		AFlareStation* TargetStation  = GetNearestAvailableStation();
		if (TargetStation)
		{
			if (Ship->DockAt(TargetStation))
			{
				// Ok let dock
				return;
			}
		}
	}

	AFlareShip* OldPilotTargetShip = PilotTargetShip;


	// Begin to find a new target only if the pilot has currently no alive target or the target is too far or not dangerous
	if(!PilotTargetShip || !PilotTargetShip->IsAlive() || (PilotTargetShip->GetActorLocation() - Ship->GetActorLocation()).Size() > 60000 || PilotTargetShip->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon) <=0  )
	{
		PilotTargetShip = GetNearestHostileShip(true);
	}

	// No dangerous ship, try not dangerous ships
	if(!PilotTargetShip)
	{
		PilotTargetShip = GetNearestHostileShip(false);
	}

	if(PilotTargetShip && OldPilotTargetShip != PilotTargetShip)
	{
		AttackPhase = 0;
		AttackAngle = FMath::FRandRange(0, 360);
		float TargetSize = PilotTargetShip->GetMeshScale() / 100.f; // Radius in meters
		AttackDistance = FMath::FRandRange(50, 100) + TargetSize;
	}

	if(PilotTargetShip)
	{
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

		FVector FireTargetAxis = (PilotTargetShip->GetAimPosition(Ship, AmmoVelocity, 0) - Ship->GetActorLocation()).GetUnsafeNormal();
		FVector PredictedFireTargetAxis = (PilotTargetShip->GetAimPosition(Ship, AmmoVelocity, PredictionDelay) - PredictedShipLocation).GetUnsafeNormal();

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

		if (AttackPhase == 0)
		{
			if(FVector::DotProduct(DeltaLocation, DeltaVelocity) < 0)
			{
				// Target is approching, prepare attack
				AttackPhase = 1;
				LastTargetDistance = Distance;
			}
			else
			{
				LinearTargetVelocity = PredictedFireTargetAxis * Ship->GetLinearMaxVelocity();
			}

			if(Distance < SecurityDistance) {
				AttackPhase = 1;
			}

		}

		if (AttackPhase == 1)
		{
			if(LastTargetDistance < Distance) {
				// Target is passed
				AttackPhase = 2;
			}
			else
			{
				FQuat AttackDistanceQuat = FQuat(TargetAxis, AttackAngle);
				FVector AttackMargin =  AttackDistanceQuat.RotateVector(FVector(0,0,AttackDistance));

				if (Distance > SecurityDistance || DangerousTarget)
				{
					LinearTargetVelocity = (AttackMargin + DeltaLocation).GetUnsafeNormal() * Ship->GetLinearMaxVelocity();
				}
				else
				{
					LinearTargetVelocity = PilotTargetShip->GetLinearVelocity() + (AttackMargin + DeltaLocation).GetUnsafeNormal() * Ship->GetLinearMaxVelocity() / 4.0;
				}
				UseOrbitalBoost = true;
			}

			LastTargetDistance = Distance;
		}

		if (AttackPhase == 2)
		{
			if(Distance > SecurityDistance) {
				// Security distance reach
				AttackPhase = 0;
			} else {
				if (DangerousTarget)
				{
					LinearTargetVelocity = -DeltaLocation.GetUnsafeNormal() * Ship->GetLinearMaxVelocity();
					UseOrbitalBoost = true;
				}
				else
				{
					LinearTargetVelocity = PilotTargetShip->GetLinearVelocity() - DeltaLocation.GetUnsafeNormal() * Ship->GetLinearMaxVelocity() / 4.0 ;
				}
			}
		}

		// If at range and aligned fire on the target
		//TODO increase tolerance if target is near
		if(Distance < (DangerousTarget ? 600.f : 300.f) + 4 * TargetSize)
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

			if(AngularPrecision < (DangerousTarget ? AngularSize * 0.25 : AngularSize * 0.2))
			{
				WantFire = true;
			}
		}

		if(Ship->GetTemperature() > Ship->GetOverheatTemperature() * (DangerousTarget ? 1.1f : 0.90f))
		{
			// TODO Fire on dangerous target
			WantFire = false;
		}
	}
	else
	{
		AngularTargetVelocity = FVector::ZeroVector;
		LinearTargetVelocity = - Ship->GetActorLocation().GetClampedToMaxSize(Ship->GetLinearMaxVelocity());
	}

	// Anticollision
	AFlareShip* NearestShip = GetNearestShip();

	if(NearestShip)
	{
		FVector DeltaLocation = NearestShip->GetActorLocation() - Ship->GetActorLocation();
		float Distance = FMath::Abs(DeltaLocation.Size() - NearestShip->GetMeshScale() *4) / 100.f; // Distance in meters




		if (Distance < 100.f)
		{

			FQuat AvoidQuat = FQuat(DeltaLocation.GetUnsafeNormal(), AttackAngle);
			FVector Avoid =  AvoidQuat.RotateVector(FVector(0,0,NearestShip->GetMeshScale() *4. / 100. ));



			// Below 100m begin avoidance maneuver
			float Alpha = 1 - Distance/100.f;
			LinearTargetVelocity = LinearTargetVelocity * (1.f - Alpha) + Alpha * ((Avoid - DeltaLocation) .GetUnsafeNormal() * Ship->GetLinearMaxVelocity());
		}
	}


	// Find friend barycenter
	// Go to friend barycenter
	// If near
		// Turn to opposite from barycentre
	// else
		// Turn to direction


	// Manage orbital boost
	if(Ship->GetTemperature() > Ship->GetOverheatTemperature() * 0.75)
	{
		UseOrbitalBoost = false;
	}
}

void UFlareShipPilot::CargoPilot(float DeltaSeconds)
{
	if (Ship->GetStatus() == EFlareShipStatus::SS_Docked)
	{
		// Let's undock
		Ship->Undock();

		// Swap target station
		PilotLastTargetStation = PilotTargetStation;
		PilotTargetStation = NULL;

		return;
	}
	else if (Ship->GetStatus() == EFlareShipStatus::SS_AutoPilot)
	{
		// Wait manoeuver
	} else {
		// If no station target, find a target : a random friendly station different from the last station
		if (!PilotTargetStation)
		{
			TArray<AFlareStation*> FriendlyStations = GetFriendlyStations();
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
				if(!Ship->DockAt(PilotTargetStation))
				{
					LinearTargetVelocity = -DeltaLocation.GetUnsafeNormal() * Ship->GetLinearMaxVelocity();
				}
			}
			else
			{
				LinearTargetVelocity = DeltaLocation.GetUnsafeNormal() * Ship->GetLinearMaxVelocity();
			}
		}
	}

	PilotTargetShip = GetNearestHostileShip(true);
	// If enemy near, run away !
	if (PilotTargetShip)
	{

		FVector DeltaLocation = (PilotTargetShip->GetActorLocation() - Ship->GetActorLocation()) / 100.f;
		float Distance = DeltaLocation.Size(); // Distance in meters

		// There is at least one hostile enemy
		if (Distance < 4000)
		{
			Ship->ForceManual();
			LinearTargetVelocity = -DeltaLocation.GetUnsafeNormal() * Ship->GetLinearMaxVelocity();

			UseOrbitalBoost = true;
		}

		if(Distance > 1000 && Ship->GetTemperature() > Ship->GetOverheatTemperature() * 0.95)
		{
			// Too hot and no imminent danger
			UseOrbitalBoost = false;
		}
	}

	// Turn to destination
	if (! LinearTargetVelocity.IsZero())
	{
		AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1.f, 0.f, 0.f) , LinearTargetVelocity.GetUnsafeNormal(),FVector(0.f, 0.f, 0.f), DeltaSeconds);
	}
}

/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

AFlareShip* UFlareShipPilot::GetNearestHostileShip(bool DangerousOnly) const
{
	// For now an host ship is a the nearest host ship with the following critera:
	// - Alive
	// - Is dangerous if needed
	// - From another company
	// - Is the nearest

	FVector PilotLocation = Ship->GetActorLocation();
	float MinDistanceSquared = -1;
	AFlareShip* NearestHostileShip = NULL;

	for (TActorIterator<AActor> ActorItr(Ship->GetWorld()); ActorItr; ++ActorItr)
	{
		// Ship
		AFlareShip* ShipCandidate = Cast<AFlareShip>(*ActorItr);
		if (ShipCandidate)
		{
			if (!ShipCandidate->IsAlive())
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

AFlareShip* UFlareShipPilot::GetNearestShip() const
{
	// For now an host ship is a the nearest host ship with the following critera:
	// - Alive or not
	// - From any company
	// - Is the nearest
	// - Is not me

	FVector PilotLocation = Ship->GetActorLocation();
	float MinDistanceSquared = -1;
	AFlareShip* NearestShip = NULL;

	for (TActorIterator<AActor> ActorItr(Ship->GetWorld()); ActorItr; ++ActorItr)
	{
		// Ship
		AFlareShip* ShipCandidate = Cast<AFlareShip>(*ActorItr);
		if (ShipCandidate && ShipCandidate != Ship)
		{
			float DistanceSquared = (PilotLocation - ShipCandidate->GetActorLocation()).SizeSquared();
			if(NearestShip == NULL || DistanceSquared < MinDistanceSquared)
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
	    FVector SimpleAcceleration = DeltaVelocityAxis * Ship->GetAngularAccelerationRate();
	    // Scale with damages
	    float DamageRatio = Ship->GetTotalMaxTorqueInAxis(Engines, DeltaVelocityAxis, true) / Ship->GetTotalMaxTorqueInAxis(Engines, DeltaVelocityAxis, false);
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
		float MaxPreciseSpeed = FMath::Min((angle - AngleToStop) / (ReactionTime * 0.75f), Ship->GetAngularMaxVelocity());

		RelativeResultSpeed = RotationDirection;
		RelativeResultSpeed *= MaxPreciseSpeed;
	}

	return RelativeResultSpeed;
}


AFlareStation* UFlareShipPilot::GetNearestAvailableStation() const
{
	FVector PilotLocation = Ship->GetActorLocation();
	float MinDistanceSquared = -1;
	AFlareStation* NearestStation = NULL;

	for (TActorIterator<AActor> ActorItr(Ship->GetWorld()); ActorItr; ++ActorItr)
	{
		// Ship
		AFlareStation* StationCandidate = Cast<AFlareStation>(*ActorItr);
		if (StationCandidate)
		{

			if (StationCandidate->GetCompany() != Ship->GetCompany())
			{
				continue;
			}

			if (!StationCandidate->HasAvailableDock(Ship))
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

TArray<AFlareStation*> UFlareShipPilot::GetFriendlyStations() const
{
	TArray<AFlareStation*> FriendlyStations;

	for (TActorIterator<AActor> ActorItr(Ship->GetWorld()); ActorItr; ++ActorItr)
	{
		// Ship
		AFlareStation* StationCandidate = Cast<AFlareStation>(*ActorItr);
		if (StationCandidate)
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


bool UFlareShipPilot::IsShipDangerous(AFlareShip* ShipCandidate) const
{
	return ShipCandidate->IsMilitary() && ShipCandidate->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon) > 0;
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
