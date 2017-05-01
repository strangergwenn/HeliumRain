#pragma once

#include "EngineMinimal.h"

class UFlareCompany;
class UFlareSector;
class UFlareSpacecraftComponent;
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
		float IsUncontrollableSmallMilitary;
		float IsUncontrollableLargeMilitary;
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
	static FVector AnticollisionCorrection(AFlareSpacecraft* Ship, FVector InitialVelocity, float PreventionDuration, AFlareSpacecraft* SpacecraftToIgnore = NULL);

	static bool FindMostDangerousCollision(AActor*& MostDangerousCandidateActor, FVector& MostDangerousLocation, float& MostDangerousTimeToHit, float& MostDangerousInterseptDepth,
										   AFlareSpacecraft* Ship, AFlareSpacecraft* SpacecraftToIgnore);

	static bool IsAnticollisionImminent(AFlareSpacecraft* Ship, float PreventionDuration);
	static bool IsSectorExitImminent(AFlareSpacecraft* Ship, float PreventionDuration);

	static AFlareSpacecraft* GetBestTarget(AFlareSpacecraft* Ship, struct TargetPreferences Preferences);

	static UFlareSpacecraftComponent* GetBestTargetComponent(AFlareSpacecraft* TargetSpacecraft);

	/** Return true if the ship is dangerous */
	static bool IsShipDangerous(AFlareSpacecraft* ShipCandidate);

private:

	/** Check if CandidateActor is dangerous on the player's trajectory */
	static bool CheckRelativeDangerosity(AActor*& MostDangerousCandidateActor, FVector& MostDangerousLocation, float& MostDangerousTimeToHit, float& MostDangerousInterseptDepth, AActor* CandidateActor, FVector CurrentLocation, float CurrentSize, FVector TargetVelocity, FVector CurrentVelocity);

};
