#pragma once

#include "../Game/FlareGameTypes.h"
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
	void PlayerSetAim(FVector AimDirection);

	/** Fire the turret if ready */
	void PlayerStartFire();

	/** Stop firing */
	void PlayerStopFire();


	/*----------------------------------------------------
		Pilot output
	----------------------------------------------------*/

	/** Linear target velocity */
	virtual FVector GetTargetAimAxis() const;

	/** Is pilot want to fire */
	virtual bool IsWantFire() const;

	/** Return true if the ship is dangerous */
	virtual bool IsShipDangerous(AFlareSpacecraft* ShipCandidate) const;


protected:

	/*----------------------------------------------------
		Internal
	----------------------------------------------------*/

	void ProcessTurretTargetSelection();

	AFlareSpacecraft* GetNearestHostileShip(bool ReachableOnly, EFlareCombatTactic::Type Tactic) const;


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
	
	// Pilot brain TODO save in save
	float                                TargetSelectionReactionTime;
	float                                FireReactionTime;
	float                                TimeUntilNextTargetSelectionReaction;
	float                                TimeUntilFireReaction;
	float                                TimeUntilNextComponentSwitch;
	AFlareSpacecraft*                    PilotTargetShip;
	UFlareSpacecraftComponent*			 PilotTargetComponent;


	/*----------------------------------------------------
		Helper
	----------------------------------------------------*/

public:

	inline AFlareSpacecraft* GetTargetShip()
	{
		return PilotTargetShip;
	}

};
