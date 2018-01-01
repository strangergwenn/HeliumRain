#pragma once

#include "../Game/FlareGameTypes.h"
#include "FlarePilotHelper.h"
#include "FlareTurretPilot.generated.h"

class UFlareTurret;
class UFlareCompany;
class AFlareSpacecraft;
class UFlareSpacecraftComponent;

/** Turret pilot class */
UCLASS()
class HELIUMRAIN_API UFlareTurretPilot : public UObject
{

public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Initialize this pilot and register the master ship object */
	void Initialize(const FFlareTurretPilotSave* Data, UFlareCompany* Company, UFlareTurret* OwnerTurret);

	/** Update this system */
	void TickPilot(float DeltaSeconds);

	/** Indicate the direction the player is aiming */
	void PlayerSetAim(FVector AimDirection, float AimDistance);

	/** Fire the turret if ready */
	void PlayerStartFire();

	/** Stop firing */
	void PlayerStopFire();

	void ClearInvalidTarget(PilotHelper::PilotTarget InvalidTarget);


	/*----------------------------------------------------
		Pilot output
	----------------------------------------------------*/

	/** Linear target velocity */
	virtual FVector GetTargetAimAxis() const;

	/** Is pilot want to fire */
	virtual bool IsWantFire() const;

	/** Return true if the ship is dangerous */
	virtual bool IsTargetDangerous(PilotHelper::PilotTarget const& Target) const;


protected:

	/*----------------------------------------------------
		Internal
	----------------------------------------------------*/

	void ProcessTurretTargetSelection();

	PilotHelper::PilotTarget GetNearestHostileTarget(bool ReachableOnly, EFlareCombatTactic::Type Tactic) const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UPROPERTY()
	UFlareTurret*                        Turret;

	UPROPERTY()
	UFlareCompany*                       PlayerCompany;

	// Component description
	FFlareTurretPilotSave                TurretPilotData;

	// Local data
	bool                                 WantFire;
	FVector                              AimAxis;
	FVector                              ManualAimDirection;
	float                                ManualAimDistance;
	
	// Pilot brain TODO save in save
	float                                TargetSelectionReactionTime;
	float                                FireReactionTime;
	float                                TimeUntilNextTargetSelectionReaction;
	float                                TimeUntilFireReaction;
	float                                TimeUntilNextComponentSwitch;
	PilotHelper::PilotTarget             PilotTarget;
	UFlareSpacecraftComponent*			 PilotTargetShipComponent;


	/*----------------------------------------------------
		Helper
	----------------------------------------------------*/

public:

	inline PilotHelper::PilotTarget GetTurretTarget()
	{
		return PilotTarget;
	}

	float GetFireDistance() const
	{
		return ManualAimDistance;
	}

};
