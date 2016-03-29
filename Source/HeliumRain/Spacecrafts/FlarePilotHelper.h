#pragma once

#include "Engine.h"

class UFlareCompany;
class UFlareSector;
class AFlareSpacecraft;


struct PilotHelper
{
	struct TargetPreferences
	{
		float IsLarge;
		float IsSmall;
		float IsStation;
		float IsNotStation;
		float IsMilitary;
		float IsNotMilitary;
		float IsDangerous;
		float IsNotDangerous;
		float TargetStateWeight;
		float MaxDistance;
		float DistanceWeight;
		AFlareSpacecraft* AttackTarget;
		float AttackTargetWeight;
		FVector PreferredDirection;
		float MinAlignement;
		float AlignementWeight;
		FVector BaseLocation;
		TArray<AFlareSpacecraft*> IgnoreList;
	};

	static bool CheckFriendlyFire(UFlareSector* Sector, UFlareCompany* MyCompany, FVector FireBaseLocation, FVector FireBaseVelocity , float AmmoVelocity, FVector FireAxis, float MaxDelay, float AimRadius);

	static FVector AnticollisionCorrection(AFlareSpacecraft* Ship, FVector InitialVelocity, AFlareSpacecraft* SpacecraftToIgnore = NULL);

	static AFlareSpacecraft* GetBestTarget(AFlareSpacecraft* Ship, struct TargetPreferences Preferences);

	/** Return true if the ship is dangerous */
	static bool IsShipDangerous(AFlareSpacecraft* ShipCandidate);

private:


	static void CheckRelativeDangerosity(AActor* CandidateActor, FVector CurrentLocation, float CurrentSize, UStaticMeshComponent* StaticMeshComponent, FVector CurrentVelocity, AActor** MostDangerousCandidateActor, FVector*MostDangerousLocation, float* MostDangerousHitTime, float* MostDangerousInterCollisionTravelTime);

};
