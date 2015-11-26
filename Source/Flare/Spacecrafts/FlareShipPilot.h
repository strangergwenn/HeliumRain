#pragma once

#include "FlareSpacecraftTypes.h"
#include "../Game/FlareCompany.h"
#include "FlareShipPilot.generated.h"

class AFlareSpacecraft;
class IFlareSpacecraftInterface;
class UFlareSpacecraftComponent;


/** Ship pilot class */
UCLASS()
class FLARE_API UFlareShipPilot : public UObject
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

	/** Return true if the ship is dangerous */
	virtual bool IsShipDangerous(AFlareSpacecraft* ShipCandidate) const;

	virtual void FindBestHostileTarget();

	virtual UFlareSpacecraftComponent* GetRandomTargetComponent(AFlareSpacecraft* TargetSpacecraft);

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
	AFlareSpacecraft*                               Ship;

	UPROPERTY()
	UFlareCompany*                            PlayerCompany;

	// Component description
	FFlareShipPilotSave                       ShipPilotData;

	// Output commands
	bool                                      UseOrbitalBoost;
	bool                                      WantFire;
	FVector                                   LinearTargetVelocity;
	FVector                                   AngularTargetVelocity;


	// Pilot brain TODO save in save
	float                                ReactionTime;
	float                                TimeUntilNextReaction;
	FVector                              PilotTargetLocation;
	float								 WaitTime;
	UPROPERTY()
	AFlareSpacecraft*                          PilotTargetShip;
	UPROPERTY()
	AFlareSpacecraft*                          PilotTargetStation;
	UPROPERTY()
	AFlareSpacecraft*                          PilotLastTargetStation;
	UPROPERTY()
	UFlareSpacecraftComponent*			 PilotTargetComponent;

	float AttackAngle;
	float AttackDistance;
	float MaxFollowDistance;
	int32 AttackPhase;
	float LastTargetDistance;
	int32 SelectedWeaponGroupIndex;
	bool LockTarget;
	bool LastWantFire;
	float TimeBeforeNextDrop;
	float                                TimeUntilNextComponentSwitch;

};
