#pragma once

#include "Engine.h"

class UFlareCompany;
class UFlareSector;

struct PilotHelper
{
	static bool CheckFriendlyFire(UFlareSector* Sector, UFlareCompany* MyCompany, FVector FireBaseLocation, FVector FireBaseVelocity , float AmmoVelocity, FVector FireAxis, float MaxDelay, float AimRadius);

	static FVector AnticollisionCorrection(AFlareSpacecraft* Ship, FVector InitialVelocity, AFlareSpacecraft* SpacecraftToIgnore = NULL);

private:
	static void CheckRelativeDangerosity(AActor* CandidateActor, FVector CurrentLocation, float CurrentSize, UStaticMeshComponent* StaticMeshComponent, FVector CurrentVelocity, AActor** MostDangerousCandidateActor, FVector*MostDangerousLocation, float* MostDangerousHitTime, float* MostDangerousInterCollisionTravelTime);

};
