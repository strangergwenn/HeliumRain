
#include "../Flare.h"

#include "FlareShipPilot.h"
#include "FlareShip.h"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareShipPilot::UFlareShipPilot(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	ReactionTime = FMath::FRandRange(0.2, 0.5);
	TimeUntilNextReaction = 0;
	PilotTargetLocation = FVector::ZeroVector;
	PilotTargetShip = NULL;
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

	bool DangerousTarget = true;
	bool AllowOrbitalBoost = false;
	WantFire = false;


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



	// Begin to find a new target only if the pilot has currently no alive target or the target is too far or not dangerous
	if(!PilotTargetShip || !PilotTargetShip->IsAlive() || (PilotTargetShip->GetActorLocation() - Ship->GetActorLocation()).Size() > 50000 || PilotTargetShip->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon) <=0  )
	{
		PilotTargetShip = GetNearestHostileShip(true);
	}

	// No dangerous ship, try not dangerous ships
	if(!PilotTargetShip)
	{
		PilotTargetShip = GetNearestHostileShip(false);
		DangerousTarget = false;
	}


	if(PilotTargetShip)
	{
		//FLOGV("%s target %s",  *Ship->GetHumanReadableName(),  *PilotTargetShip->GetHumanReadableName());
		// The pilot have a target, track and kill it

		FVector LocalNose = FVector(1.f, 0.f, 0.f);
		FVector DeltaLocation = PilotTargetShip->GetActorLocation() - Ship->GetActorLocation();
		float Distance = DeltaLocation.Size() / 100.f; // Distance in meters
		FVector TargetAxis = DeltaLocation.GetUnsafeNormal();


		FVector FireTargetAxis = (PilotTargetShip->GetAimPosition(Ship, 50000) - Ship->GetActorLocation()).GetUnsafeNormal(); // TODO et weapon velocity

		FRotator ShipAttitude = Ship->GetActorRotation();
		FVector ShipVelocity = 100 * Ship->GetLinearVelocity();

		// Bullet velocity
		FVector BulletVelocity = ShipAttitude.Vector();
		BulletVelocity.Normalize();
		BulletVelocity *= 50000; // TODO get from projectile

		FVector BulletDirection = Ship->Airframe->GetComponentToWorld().GetRotation().Inverse().RotateVector((ShipVelocity + BulletVelocity)).GetUnsafeNormal();


		// First allow align nose to target bullet interception point
		// TODO Use BulletDirection instead of LocalNose
		//AngularTargetVelocity = GetAngularVelocityToAlignAxis(LocalNose, FireTargetAxis, DeltaSeconds);
		//TODO find target angular velocity
		AngularTargetVelocity = GetAngularVelocityToAlignAxis(BulletDirection, FireTargetAxis, DeltaSeconds);


		/*FLOGV("DeltaLocation=%s", *DeltaLocation.ToString());
		FLOGV("TargetAxis=%s", *TargetAxis.ToString());
		FLOGV("FireTargetAxis=%s", *FireTargetAxis.ToString());
		FLOGV("BulletVelocity=%s", *BulletVelocity.ToString());
		FLOGV("BulletDirection=%s", *BulletDirection.ToString());
		FLOGV("AngularTargetVelocity=%s", *AngularTargetVelocity.ToString());
*/

		// If is near, match speed, else go to target with boost
		if(Distance < (DangerousTarget ? 200.f : 100.f))
		{
			//FLOGV("is near distance=%f", Distance);
			LinearTargetVelocity = PilotTargetShip->GetLinearVelocity();
		}
		else
		{
			//FLOGV("is far distance=%f", Distance);
			LinearTargetVelocity = DeltaLocation.GetClampedToMaxSize(Ship->GetLinearMaxVelocity());
			AllowOrbitalBoost = true;
		}

		// If at range and aligned fire on the target
		//TODO increase tolerance if target is near
		if(Distance < (DangerousTarget ? 600.f : 300.f))
		{
			//FLOGV("is at fire range=%f", Distance);
			// TODO Use BulletDirection instead of LocalNose
			FVector WorldShipAxis = Ship->Airframe->GetComponentToWorld().GetRotation().RotateVector(BulletDirection);
			float AngularPrecision = FVector::DotProduct(FireTargetAxis, WorldShipAxis);

			//FLOGV("WorldShipAxis=%s", *WorldShipAxis.ToString());

			//FLOGV("AngularPrecision=%f", AngularPrecision);

			if(AngularPrecision > (DangerousTarget ? 0.999f : 0.9995f))
			{
				//FLOG("Fire");
				WantFire = true;
			}
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
		float Distance = FMath::Abs(DeltaLocation.Size() - NearestShip->GetMeshScale()) / 100.f; // Distance in meters

		if (Distance < 100.f)
		{
			// Below 100m begin avoidance maneuver
			float Alpha = 1 - Distance/100.f;
			LinearTargetVelocity = LinearTargetVelocity * (1.f - Alpha) + Alpha * (-DeltaLocation.GetUnsafeNormal() * Ship->GetLinearMaxVelocity());
		}
	}


	// Find friend barycenter
	// Go to friend barycenter
	// If near
		// Turn to opposite from barycentre
	// else
		// Turn to direction


	// Manage orbital boost
	if(Ship->GetTemperature() < 680)
	{
		UseOrbitalBoost = true;
	}
	else
	{
		UseOrbitalBoost = false;
	}

	if(Ship->GetTemperature() > (DangerousTarget ? 900.f : 780.f))
	{
		// TODO Fire on dangerous target
		WantFire = false;
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
			if(!ShipCandidate->IsAlive())
			{
				continue;
			}

			if(DangerousOnly && ShipCandidate->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon) <= 0)
			{
				continue;
			}

			if(ShipCandidate->GetCompany() == Ship->GetCompany())
			{
				continue;
			}




			float DistanceSquared = (PilotLocation - ShipCandidate->GetActorLocation()).SizeSquared();
			if(NearestHostileShip == NULL || DistanceSquared < MinDistanceSquared)
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

FVector UFlareShipPilot::GetAngularVelocityToAlignAxis(FVector LocalShipAxis, FVector TargetAxis, float DeltaSeconds) const
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

	FVector DeltaVelocity = -AngularVelocity;
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

	float AngleToStop = (DeltaVelocity.Size() / 2) * (TimeToFinalVelocity + ReactionTime);

	FVector RelativeResultSpeed;

	if (AngleToStop > angle) {
		RelativeResultSpeed = FVector::ZeroVector;
	}
	else {

		float MaxPreciseSpeed = FMath::Min((angle - AngleToStop) / ReactionTime, Ship->GetAngularMaxVelocity());

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