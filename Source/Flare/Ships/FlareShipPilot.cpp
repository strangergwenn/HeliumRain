
#include "../Flare.h"

#include "FlareShipPilot.h"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareShipPilot::UFlareShipPilot(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	TimeUntilNextChange = 0;
	PilotTargetLocation = FVector::ZeroVector;
	PilotTargetShip = NULL;
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void UFlareShipPilot::TickPilot(float DeltaSeconds)
{
	TimeUntilNextChange -= DeltaSeconds;
	if(TimeUntilNextChange <= 0)
	{

		TimeUntilNextChange = FMath::FRandRange(10, 40);
		PilotTargetLocation = FVector(FMath::FRandRange(-4000, 4000), FMath::FRandRange(-1000, 1000), FMath::FRandRange(-1000, 1000));
		//FLOGV("Pilot change destination to %s", *PilotTargetLocation.ToString());
		//FLOGV("New change in %fs", TimeUntilNextChange);
	}

	LinearTargetVelocity = (PilotTargetLocation - Ship->GetActorLocation()/100);
	LinearTargetVelocity = FVector::ZeroVector;

	if(Ship->GetTemperature() < 600)
	{
		UseOrbitalBoost = true;
	}

	if(Ship->GetTemperature() > 780)
	{
		UseOrbitalBoost = false;
	}

	bool AllowOrbitalBoost = false;
	WantFire = false;

	// Begin to find a new target only if the pilot has currently no alive target or the target is too far
	if(!PilotTargetShip || !PilotTargetShip->IsAlive() || (PilotTargetShip->GetActorLocation() - Ship->GetActorLocation()).Size() > 50000)
	{
		PilotTargetShip = GetNearestHostileShip();
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
		AngularTargetVelocity = GetAngularVelocityToAlignAxis(BulletDirection, FireTargetAxis, DeltaSeconds);


		/*FLOGV("DeltaLocation=%s", *DeltaLocation.ToString());
		FLOGV("TargetAxis=%s", *TargetAxis.ToString());
		FLOGV("FireTargetAxis=%s", *FireTargetAxis.ToString());
		FLOGV("BulletVelocity=%s", *BulletVelocity.ToString());
		FLOGV("BulletDirection=%s", *BulletDirection.ToString());
		FLOGV("AngularTargetVelocity=%s", *AngularTargetVelocity.ToString());
*/

		// If is near, match speed, else go to target with boost
		if(Distance < 200.f)
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
		if(Distance < 600.f)
		{
			//FLOGV("is at fire range=%f", Distance);
			// TODO Use BulletDirection instead of LocalNose
			FVector WorldShipAxis = Ship->Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalNose);
			float AngularPrecision = FVector::DotProduct(FireTargetAxis, WorldShipAxis);

			//FLOGV("WorldShipAxis=%s", *WorldShipAxis.ToString());

			//FLOGV("AngularPrecision=%f", AngularPrecision);

			if(AngularPrecision > 0.997f)
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
		float Distance = DeltaLocation.Size() / 100.f; // Distance in meters

		if(Distance < 200.f)
		{
			// Below 200m begin avoidance maneuver
			float Alpha = 1 - Distance/200.f;
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

	if(Ship->GetTemperature() > 800)
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

AFlareShip* UFlareShipPilot::GetNearestHostileShip() const
{
	// For now an host ship is a the nearest host ship with the following critera:
	// - Alive
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

	float AngleToStop = (DeltaVelocity.Size() / 2) * (TimeToFinalVelocity + DeltaSeconds);

	FVector RelativeResultSpeed;

	if (AngleToStop > angle) {
		RelativeResultSpeed = FVector::ZeroVector;
	}
	else {

		float MaxPreciseSpeed = FMath::Min((angle - AngleToStop) / DeltaSeconds, Ship->GetAngularMaxVelocity());

		RelativeResultSpeed = RotationDirection;
		RelativeResultSpeed *= MaxPreciseSpeed;
	}

	return RelativeResultSpeed;
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