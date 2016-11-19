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
		float IsStranded;
		float IsNotStranded;
		float IsUncontrollableCivil;
		float IsUncontrollableMilitary;
		float IsNotUncontrollable;
		float IsHarpooned;
		float TargetStateWeight;
		float MaxDistance;
		float DistanceWeight;
		AFlareSpacecraft* AttackTarget;
		float AttackTargetWeight;
		AFlareSpacecraft* LastTarget;
		float LastTargetWeight;
		FVector PreferredDirection;
		float MinAlignement;
		float AlignementWeight;
		FVector BaseLocation;
		TArray<AFlareSpacecraft*> IgnoreList;
	};

	static bool CheckFriendlyFire(UFlareSector* Sector, UFlareCompany* MyCompany, FVector FireBaseLocation, FVector FireBaseVelocity , float AmmoVelocity, FVector FireAxis, float MaxDelay, float AimRadius);

	/** Correct trajectory to avoid incoming ships */
	static FVector AnticollisionCorrection(AFlareSpacecraft* Ship, FVector InitialVelocity, AFlareSpacecraft* SpacecraftToIgnore = NULL);

	static AFlareSpacecraft* GetBestTarget(AFlareSpacecraft* Ship, struct TargetPreferences Preferences);

	static UFlareSpacecraftComponent* GetBestTargetComponent(AFlareSpacecraft* TargetSpacecraft);

	/** Return true if the ship is dangerous */
	static bool IsShipDangerous(AFlareSpacecraft* ShipCandidate);

private:

	/** Check if CandidateActor is dangerous on the player's trajectory */
	static bool CheckRelativeDangerosity(AActor* CandidateActor, FVector CurrentLocation, float CurrentSize, FVector TargetVelocity, FVector CurrentVelocity,
		AActor** MostDangerousCandidateActor, FVector*MostDangerousLocation, float* MostDangerousHitTime, float* MostDangerousInterCollisionTravelTime);

};
