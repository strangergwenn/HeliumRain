#include "../Flare.h"

#include "../Game/FlareCompany.h"
#include "../Game/FlareSector.h"
#include "FlarePilotHelper.h"


bool PilotHelper::CheckFriendlyFire(UFlareSector* Sector, UFlareCompany* MyCompany, FVector FireBaseLocation, FVector FireBaseVelocity , float AmmoVelocity, FVector FireAxis, float MaxDelay, float AimRadius)
{
	//FLOG("CheckFriendlyFire");
	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSpacecrafts().Num(); SpacecraftIndex++)
	{
		AFlareSpacecraft* SpacecraftCandidate = Sector->GetSpacecrafts()[SpacecraftIndex];

		if (SpacecraftCandidate)
		{


			if (MyCompany->GetHostility(SpacecraftCandidate->GetCompany()) == EFlareHostility::Hostile)
			{
				//FLOG("  hostile, skip");
				continue;
			}

			// Compute target Axis for each gun
			FVector AmmoIntersectionLocation;
			float AmmoIntersectionTime = SpacecraftHelper::GetIntersectionPosition(SpacecraftCandidate->GetActorLocation(), SpacecraftCandidate->Airframe->GetPhysicsLinearVelocity(), FireBaseLocation, FireBaseVelocity , AmmoVelocity, 0, &AmmoIntersectionLocation);
			if (AmmoIntersectionTime < 0 || AmmoIntersectionTime > MaxDelay)
			{
				// No ammo intersection, or too far, don't alert
				//FLOGV("  Too far (%f > %f)", AmmoIntersectionTime , MaxDelay);
				continue;
			}

			float TargetSize = SpacecraftCandidate->GetMeshScale() / 100.f + AimRadius * 2; // Radius in meters
			FVector DeltaLocation = (SpacecraftCandidate->GetActorLocation()-FireBaseLocation) / 100.f;
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

FVector PilotHelper::AnticollisionCorrection(AFlareSpacecraft* Ship, FVector InitialVelocity, AFlareSpacecraft* SpacecraftToIgnore)

{
	FBox ShipBox = Ship->GetComponentsBoundingBox();
	float CurrentSize = FMath::Max(ShipBox.GetExtent().Size(), 1.0f);
	FVector CurrentVelocity = Ship->GetLinearVelocity() * 100;
	FVector CurrentLocation = (ShipBox.Max + ShipBox.Min) / 2.0;


	AActor* MostDangerousCandidateActor = NULL;
	FVector MostDangerousLocation;
	float MostDangerousHitTime = 0;
	float MostDangerousInterCollisionTravelTime = 0;

	UFlareSector* ActiveSector = Ship->GetGame()->GetActiveSector();

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < ActiveSector->GetSpacecrafts().Num(); SpacecraftIndex++)
	{
		AFlareSpacecraft* SpacecraftCandidate = ActiveSector->GetSpacecrafts()[SpacecraftIndex];

		if (SpacecraftCandidate != Ship
				&& SpacecraftCandidate != SpacecraftToIgnore
				&& !Ship->GetDockingSystem()->IsGrantedShip(SpacecraftCandidate)
				&& !Ship->GetDockingSystem()->IsDockedShip(SpacecraftCandidate))
		{
			CheckRelativeDangerosity(SpacecraftCandidate, CurrentLocation, CurrentSize, SpacecraftCandidate->Airframe, CurrentVelocity, &MostDangerousCandidateActor, &MostDangerousLocation, &MostDangerousHitTime, &MostDangerousInterCollisionTravelTime);
		}
	}

	for (int32 AsteroidIndex = 0; AsteroidIndex < ActiveSector->GetAsteroids().Num(); AsteroidIndex++)
	{
		AFlareAsteroid* AsteroidCandidate = ActiveSector->GetAsteroids()[AsteroidIndex];
		//FLOGV("  -> test AsteroidCandidate %s", *AsteroidCandidate->GetName());
		CheckRelativeDangerosity(AsteroidCandidate, CurrentLocation, CurrentSize, AsteroidCandidate->GetStaticMeshComponent(), CurrentVelocity, &MostDangerousCandidateActor, &MostDangerousLocation, &MostDangerousHitTime, &MostDangerousInterCollisionTravelTime);
	}




	if(MostDangerousCandidateActor)
	{
		UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MostDangerousCandidateActor->GetRootComponent());

		//FVector DeltaVelocity = CurrentVelocity - StaticMeshComponent->GetPhysicsLinearVelocity();

		FVector MinDistancePoint = CurrentLocation + Ship->Airframe->GetPhysicsLinearVelocity() * MostDangerousHitTime;

		FVector FutureTargetLocation =  MostDangerousLocation + StaticMeshComponent->GetPhysicsLinearVelocity() * MostDangerousHitTime;

		FVector AvoidanceVector = MinDistancePoint - FutureTargetLocation;

		FVector AvoidanceAxis;

		if(AvoidanceAxis.IsNearlyZero())
		{
			AvoidanceAxis = FMath::VRand();
		}
		else
		{
			AvoidanceAxis = AvoidanceVector.GetUnsafeNormal();
		}

		//DrawDebugLine(Ship->GetWorld(), CurrentLocation, MinDistancePoint , FColor::Yellow, true);

		//DrawDebugLine(Ship->GetWorld(), CurrentLocation, CurrentLocation + Ship->Airframe->GetPhysicsLinearVelocity(), FColor::White, true);


		//DrawDebugLine(Ship->GetWorld(), CurrentLocation, CurrentLocation + AvoidanceAxis *1000 , FColor::Green, true);

		//DrawDebugLine(Ship->GetWorld(), CurrentLocation, CurrentLocation + InitialVelocity *10 , FColor::Blue, true);

		// Below 5s  begin avoidance maneuver
		float Alpha = 1 - FMath::Max(0.0f, MostDangerousHitTime - MostDangerousInterCollisionTravelTime)/5.f;

		FVector Temp = InitialVelocity * (1.f - Alpha) + Alpha * AvoidanceAxis * Ship->GetNavigationSystem()->GetLinearMaxVelocity();

		//DrawDebugLine(Ship->GetWorld(), CurrentLocation, CurrentLocation + Temp * 10, FColor::Magenta, true);

		return Temp;
	}

	return InitialVelocity;
}




void PilotHelper::CheckRelativeDangerosity(AActor* CandidateActor, FVector CurrentLocation, float CurrentSize, UStaticMeshComponent* StaticMeshComponent, FVector CurrentVelocity, AActor** MostDangerousCandidateActor, FVector*MostDangerousLocation, float* MostDangerousHitTime, float* MostDangerousInterCollisionTravelTime)
{

	if(!StaticMeshComponent)
	{
		// Not physical shape : not dangerous
		return;
	}


	FVector DeltaVelocity = StaticMeshComponent->GetPhysicsLinearVelocity() - CurrentVelocity;
	//FLOGV("  DeltaVelocity=%s", *DeltaVelocity.ToString());


	if(DeltaVelocity.IsNearlyZero())
	{
		//FLOG("  !! DeltaVelocity.IsNearlyZero()");
		// Relative velocity is near zero : not dangerous
		return;
	}

	FBox CandidateBox = CandidateActor->GetComponentsBoundingBox();
	float CandidateSize = FMath::Max(CandidateBox.GetExtent().Size(), 1.0f);

	FVector CandidateLocation= (CandidateBox.Max + CandidateBox.Min) / 2.0;
	FVector DeltaLocation = CandidateLocation - CurrentLocation;
	//FLOGV("  CandidateLocation=%s", *CandidateLocation.ToString());
	//FLOGV("  DeltaLocation=%s", *DeltaLocation.ToString());

	//FLOGV("  FVector::DotProduct(DeltaLocation, DeltaVelocity)=%f", FVector::DotProduct(DeltaLocation, DeltaVelocity));

	if(FVector::DotProduct(DeltaLocation, DeltaVelocity) > 0)
	{
		//FLOG("  !! Go away from candidate");
		// Go away from candidate : not dangerous
		return;
	}

	FVector RelativeVelocity = -DeltaVelocity;

	// Min distance
	float MinDistance = FVector::CrossProduct(DeltaLocation, RelativeVelocity).Size() / RelativeVelocity.Size();
	float SizeSum = CurrentSize + CandidateSize;

	if (SizeSum < MinDistance)
	{
		//FLOG("  !! Minimum distance highter than object size sum");
		// Minimum distance highter than object size sum : not Dangerous
		return;
	}


	// There will be a hit.
	float DistanceToMinDistancePoint = FMath::Sqrt(DeltaLocation.SizeSquared() - FMath::Square(MinDistance));

	float TimeToMinDistance = DistanceToMinDistancePoint / RelativeVelocity.Size();

	//FLOGV("DistanceToMinDistancePoint %f", DistanceToMinDistancePoint)
	//FLOGV("TimeToMinDistance %f", TimeToMinDistance)




	float InterCollisionTravelTime = SizeSum / RelativeVelocity.Size();
	//FLOGV("InterCollisionTravelTime %f", InterCollisionTravelTime)

	if(TimeToMinDistance > 5.f + InterCollisionTravelTime)
	{
		//FLOG("  !! Time to minimum distance hight");
		// Time to minimum distance hight : not Dangerous
		return;
	}

	//DrawDebugLine(Ship->GetWorld(), CandidateLocation, Box.Max , FColor::Yellow, true);
	//DrawDebugLine(Ship->GetWorld(), CandidateLocation, Box.Min , FColor::Green, true);

	/*DrawDebugSphere(Ship->GetWorld(), CandidateLocation, 10, 12, FColor::White, true);


	DrawDebugBox(Ship->GetWorld(),
				 BoxLocation,
				 BoxSize,
				 FQuat(FRotator::ZeroRotator),
				 FColor::Blue,
				 1.f);
	DrawDebugBox(Ship->GetWorld(),
				 Box2Location,
				 Box2Size,
				 FQuat(FRotator::ZeroRotator),
				 FColor::Cyan,
				 1.f);*/

	//DrawDebugSphere(Ship->GetWorld(), CandidateLocation, CandidateSize, 12, FColor::Red, true);
	//DrawDebugSphere(Ship->GetWorld(), CurrentLocation, Ship->GetMeshScale(), 12, FColor::Green, true);

	//DrawDebugSphere(Ship->GetWorld(), Box2Location, CandidateSize * 2.f, 12, FColor::Magenta, true);


	// Keep only most imminent hit
	if(!MostDangerousCandidateActor || *MostDangerousHitTime > TimeToMinDistance)
	{
		//FLOG("  > Canditade is the most dangerous candidate");
		*MostDangerousCandidateActor = CandidateActor;
		*MostDangerousHitTime = TimeToMinDistance;
		*MostDangerousInterCollisionTravelTime = InterCollisionTravelTime;
		*MostDangerousLocation = CandidateLocation;
	}
}
