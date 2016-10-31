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

	virtual void TickPilot(float DeltaSeconds);

	/** Initialize this pilot and register the master ship object */
	virtual void Initialize(const FFlareTurretPilotSave* Data, UFlareCompany* Company, UFlareTurret* OwnerTurret);

protected:
	void ProcessTurretTargetSelection();

	AFlareSpacecraft* GetNearestHostileShip(bool ReachableOnly, EFlareCombatTactic::Type Tactic) const;

public:

	/*----------------------------------------------------
		Pilot Output
	----------------------------------------------------*/

	/** Linear target velocity */
	virtual FVector GetTargetAimAxis() const;

	/** Is pilot want to fire */
	virtual bool IsWantFire() const;

	/** Return true if the ship is dangerous */
	virtual bool IsShipDangerous(AFlareSpacecraft* ShipCandidate) const;

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UPROPERTY()
	UFlareTurret*                               Turret;

	UPROPERTY()
	UFlareCompany*                            PlayerCompany;

	// Component description
	FFlareTurretPilotSave                       TurretPilotData;

	// Output commands
	bool                                      WantFire;
	FVector                                   AimAxis;


	// Pilot brain TODO save in save
	float                                TargetSelectionReactionTime;
	float                                FireReactionTime;
	float                                TimeUntilNextTargetSelectionReaction;
	float                                TimeUntilFireReaction;
	float                                TimeUntilNextComponentSwitch;
	AFlareSpacecraft*                    PilotTargetShip;
	UFlareSpacecraftComponent*			 PilotTargetComponent;
};
