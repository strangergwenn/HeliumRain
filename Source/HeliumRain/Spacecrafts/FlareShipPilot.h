#pragma once

#include "FlareSpacecraftTypes.h"
#include "../Game/FlareGameTypes.h"
#include "FlareShipPilot.generated.h"

class UFlareCompany;
class AFlareSpacecraft;
class UFlareSpacecraftComponent;


/** Ship pilot class */
UCLASS()
class HELIUMRAIN_API UFlareShipPilot : public UObject
{

public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void TickPilot(float DeltaSeconds);

	/** Initialize this pilot and register the master ship object */
	virtual void Initialize(const FFlareShipPilotSave* Data, UFlareCompany* Company, AFlareSpacecraft* OwnerShip);

protected:
	/*----------------------------------------------------
		Pilot functions
	----------------------------------------------------*/
	virtual void MilitaryPilot(float DeltaSeconds);

	virtual void CargoPilot(float DeltaSeconds);

	virtual void FighterPilot(float DeltaSeconds);

	virtual void BomberPilot(float DeltaSeconds);

	virtual void FlagShipPilot(float DeltaSeconds);

	virtual void IdlePilot(float DeltaSeconds);

public:
	/*----------------------------------------------------
		Helpers
	----------------------------------------------------*/

	/**
	 * Return the curved trajectory to avoid exiting sector
	 */
	FVector ExitAvoidance(AFlareSpacecraft* Ship, FVector InitialVelocityTarget,float CurveTrajectoryLimit) const;

	/**
	 * Return the nearest hostile alive ship
	 */
	virtual AFlareSpacecraft* GetNearestHostileShip(bool DangerousOnly, EFlarePartSize::Type Size) const;

	/**
	 * Return the nearest ship, alive or not
	 */
	virtual AFlareSpacecraft* GetNearestShip(bool IgnoreDockingShip) const;

	/**
	* Return the nearest station where docking is available
	*/
	virtual AFlareSpacecraft* GetNearestAvailableStation(bool RealStation) const;

	/** Return all friendly station in the sector */
	virtual TArray<AFlareSpacecraft*> GetFriendlyStations() const;

	/**
	 * Return the angular velocity need to align the local ship axis to the target axis
	 */
	virtual FVector GetAngularVelocityToAlignAxis(FVector LocalShipAxis, FVector TargetAxis, FVector TargetAngularVelocity, float DeltaSeconds) const;

	virtual void FindBestHostileTarget(EFlareCombatTactic::Type Tactic);

	void AlignToTargetVelocityWithThrust(float DeltaSeconds);

public:

	/*----------------------------------------------------
		Pilot Output
	----------------------------------------------------*/

	/** Linear target velocity */
	virtual FVector GetLinearTargetVelocity() const;

	/** Angular target velocity */
	virtual FVector GetAngularTargetVelocity() const;

	/** Is pilot want to use orbital boost */
	virtual bool IsUseOrbitalBoost() const;

	/** Is pilot want to fire */
	virtual bool IsWantFire() const;

	/** Pilot weapon selection */
	virtual int32 GetPreferedWeaponGroup() const;

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UPROPERTY()
	AFlareSpacecraft*                            Ship;

	UPROPERTY()
	UFlareCompany*                               PlayerCompany;

	// Component description
	FFlareShipPilotSave                          ShipPilotData;

	// Output commands
	bool                                         UseOrbitalBoost;
	bool                                         WantFire;
	FVector                                      LinearTargetVelocity;
	FVector                                      AngularTargetVelocity;


	// Pilot brain TODO save in save
	float                                        ReactionTime;
	float                                        TimeUntilNextReaction;
	FVector                                      PilotTargetLocation;
	float								         WaitTime;

	// Pilot targets
	UPROPERTY()
	AFlareSpacecraft*                            PilotTargetShip;
	UPROPERTY()
	AFlareSpacecraft*                            PilotTargetStation;
	UPROPERTY()
	AFlareSpacecraft*                            PilotLastTargetStation;
	UPROPERTY()
	UFlareSpacecraftComponent*			         PilotTargetComponent;

	float                                        AttackAngle;
	float                                        AttackDistance;
	float                                        MaxFollowDistance;
	int32                                        AttackPhase;
	float                                        LastTargetDistance;
	int32                                        SelectedWeaponGroupIndex;
	bool                                         LockTarget;
	bool                                         LastWantFire;

	float                                        TimeBeforeNextDrop;
	float                                        TimeSinceAiming;
	float                                        TimeUntilNextComponentSwitch;
	float                                        TimeSinceLastDockingAttempt;
	float                                        TimeUntilNextDockingAttempt;
	float                                        MaxTimeBetweenDockingAttempt;

	EFlareCombatTactic::Type			         CurrentTactic;


	/*----------------------------------------------------
		Helper
	----------------------------------------------------*/

public:

	inline AFlareSpacecraft* GetTargetShip()
	{
		return PilotTargetShip;
	}
};
