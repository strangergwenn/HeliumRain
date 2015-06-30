#include "../Flare.h"

#include "../Game/FlareCompany.h"
#include "FlarePilotHelper.h"


bool PilotHelper::CheckFriendlyFire(UWorld* World, UFlareCompany* MyCompany, FVector FireBaseLocation, FVector FireBaseVelocity , float AmmoVelocity, FVector FireAxis, float MaxDelay, float AimRadius)
{
	//FLOG("CheckFriendlyFire");
	for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		// Ship
		AFlareSpacecraft* ShipCandidate = Cast<AFlareSpacecraft>(*ActorItr);
		if (ShipCandidate)
		{

			//FLOGV("  check %s", *ShipCandidate->GetName());

			if (MyCompany->GetHostility(ShipCandidate->GetCompany()) == EFlareHostility::Hostile)
			{
				//FLOG("  hostile, skip");
				continue;
			}

			// Compute target Axis for each gun
			FVector AmmoIntersectionLocation;
			float AmmoIntersectionTime = SpacecraftHelper::GetIntersectionPosition(ShipCandidate->GetActorLocation(), ShipCandidate->Airframe->GetPhysicsLinearVelocity(), FireBaseLocation, FireBaseVelocity , AmmoVelocity, 0, &AmmoIntersectionLocation);
			if (AmmoIntersectionTime < 0 || AmmoIntersectionTime > MaxDelay)
			{
				// No ammo intersection, or too far, don't alert
				//FLOGV("  Too far (%f > %f)", AmmoIntersectionTime , MaxDelay);
				continue;
			}

			float TargetSize = ShipCandidate->GetMeshScale() / 100.f + AimRadius * 2; // Radius in meters
			FVector DeltaLocation = (ShipCandidate->GetActorLocation()-FireBaseLocation) / 100.f;
			float Distance = DeltaLocation.Size();

			FVector FireTargetAxis = (AmmoIntersectionLocation - FireBaseLocation - AmmoIntersectionTime * FireBaseVelocity).GetUnsafeNormal();

			float AngularPrecisionDot = FVector::DotProduct(FireTargetAxis, FireAxis);
			float AngularPrecision = FMath::Acos(AngularPrecisionDot);
			float AngularSize = FMath::Atan(TargetSize / Distance);



			/*FLOGV("  TargetSize %f", TargetSize);
			FLOGV("  Distance %f", Distance);
			FLOGV("  AngularPrecision %f", AngularPrecision);
			FLOGV("  AngularSize %f", AngularSize);*/
			if (AngularPrecision < AngularSize)
			{
				//FLOG("  Dangerous");
				// Dangerous !
				return true;
			}
		}
	}
	return false;

}
