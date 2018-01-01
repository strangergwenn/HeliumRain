#pragma once

#include "EngineMinimal.h"

class UFlareCompany;
class UFlareSeOctor;
class UFlareSpacecraftComponent;
class AFlareSpacecraft;
class AFlareMeteorite;
class AFlareBomb;

struct PilotHelper
{
	struct PilotTarget
	{
		PilotTarget()
			: SpacecraftTarget(nullptr)
			, MeteoriteTarget(nullptr)
			, BombTarget(nullptr) {}

		PilotTarget(AFlareSpacecraft* Spacecraft)
			: SpacecraftTarget(Spacecraft)
			, MeteoriteTarget(nullptr)
			, BombTarget(nullptr) {}

		PilotTarget(AFlareBomb* Bomb)
			: SpacecraftTarget(nullptr)
			, MeteoriteTarget(nullptr)
			, BombTarget(Bomb) {}

		PilotTarget(AFlareMeteorite* Meteorite)
			: SpacecraftTarget(nullptr)
			, MeteoriteTarget(Meteorite)
			, BombTarget(nullptr) {}


		bool IsValid() const;
		bool IsEmpty() const;
		bool Is(AFlareSpacecraft* Spacecraft) const;
		bool Is(AFlareMeteorite* Meteorite) const;
		bool Is(AFlareBomb* Bomb) const;

		void Clear();
		void SetSpacecraft(AFlareSpacecraft* Spacecraft);
		void SetMeteorite(AFlareMeteorite* Meteorite);
		void SetBomb(AFlareBomb* Bomb);
		FVector GetActorLocation() const;
		FVector GetLinearVelocity() const;
		float GetMeshScale();

		AActor* GetActor();

		/** Extrapolate the position of a ship for a given targeting ship. Return time before intersect. If time is negative, no intersection. */
		float GetAimPosition(AFlareSpacecraft* TargettingShip, float BulletSpeed, float PredictionDelay, FVector* ResultPosition) const;

		bool operator==(const PilotTarget& rhs) const
		{
			return (SpacecraftTarget == rhs.SpacecraftTarget) &&
					(MeteoriteTarget == rhs.MeteoriteTarget) &&
					(BombTarget == rhs.BombTarget);
		}

		bool operator!=(const PilotTarget& rhs) const
		{
			return (SpacecraftTarget != rhs.SpacecraftTarget) ||
					(MeteoriteTarget != rhs.MeteoriteTarget) ||
					(BombTarget != rhs.BombTarget);
		}

		AFlareSpacecraft* SpacecraftTarget;
		AFlareMeteorite* MeteoriteTarget;
		AFlareBomb* BombTarget;

	};

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
		float AttackMeWeight;
		PilotTarget LastTarget;
		float LastTargetWeight;
		FVector PreferredDirection;
		float MinAlignement;
		float AlignementWeight;
		FVector BaseLocation;
		float IsBomb;
		float MaxBombDistance;
		float IsMeteorite;
		TArray<PilotTarget> IgnoreList;
	};

	static bool CheckFriendlyFire(UFlareSector* Sector, UFlareCompany* MyCompany, FVector FireBaseLocation, FVector FireBaseVelocity , float AmmoVelocity, FVector FireAxis, float MaxDelay, float AimRadius);

	struct AnticollisionConfig
	{
		AnticollisionConfig(): SpacecraftToIgnore(nullptr), IgnoreAllStations(false), SpeedCorrectionOnly(false){}
		AFlareSpacecraft* SpacecraftToIgnore;
		bool IgnoreAllStations;
		bool SpeedCorrectionOnly;
	};



	/** Correct trajectory to avoid incoming ships */
	static FVector AnticollisionCorrection(AFlareSpacecraft* Ship, FVector InitialVelocity, float PreventionDuration, AnticollisionConfig IgnoreConfig, float SpeedLimit);

	static bool FindMostDangerousCollision(AActor*& MostDangerousCandidateActor, FVector& MostDangerousLocation, float& MostDangerousTimeToHit, float& MostDangerousInterseptDepth,
										   AFlareSpacecraft* Ship, AnticollisionConfig IgnoreConfig, float SpeedLimit);

	static bool IsAnticollisionImminent(AFlareSpacecraft* Ship, float PreventionDuration, float SpeedLimit);
	static bool IsSectorExitImminent(AFlareSpacecraft* Ship, float PreventionDuration);

	static PilotTarget GetBestTarget(AFlareSpacecraft* Ship, struct TargetPreferences Preferences);

	static UFlareSpacecraftComponent* GetBestTargetComponent(AFlareSpacecraft* TargetSpacecraft);

	/** Return true if the ship is dangerous */
	static bool IsTargetDangerous(PilotHelper::PilotTarget const& Target);

private:

	/** Check if CandidateActor is dangerous on the player's trajectory */
	static bool CheckRelativeDangerosity(AActor*& MostDangerousCandidateActor, FVector& MostDangerousLocation, float& MostDangerousTimeToHit, float& MostDangerousInterseptDepth, AActor* CandidateActor, FVector CurrentLocation, float CurrentSize, FVector TargetVelocity, FVector CurrentVelocity, float SpeedLimit);

};
