#pragma once

#include "FlareShipPilot.generated.h"

class AFlareShip;
class IFlareSpacecraftInterface;

/** Ship component save data */
USTRUCT()
struct FFlareShipPilotSave
{
	GENERATED_USTRUCT_BODY()

	/** Pilot identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	/** Pilot name */
	UPROPERTY(EditAnywhere, Category = Save)
	FString Name;

};

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
	virtual void Initialize(const FFlareShipPilotSave* Data, UFlareCompany* Company, AFlareShip* OwnerShip);

protected:
	/*----------------------------------------------------
		Pilot functions
	----------------------------------------------------*/
	virtual void MilitaryPilot(float DeltaSeconds);

	virtual void CargoPilot(float DeltaSeconds);

	/*----------------------------------------------------
		Helpers
	----------------------------------------------------*/

	/**
	 * Return the nearest hostile alive ship
	 */
	virtual AFlareShip* GetNearestHostileShip(bool DangerousOnly) const;

	/**
	 * Return the nearest ship, alive or not
	 */
	virtual AFlareShip* GetNearestShip() const;

	/**
	* Return the nearest station where docking is available
	*/
	virtual AFlareShip* GetNearestAvailableStation() const;

	/** Return all friendly station in the sector */
	virtual TArray<AFlareShip*> GetFriendlyStations() const;

	/**
	 * Return the angular velocity need to align the local ship axis to the target axis
	 */
	virtual FVector GetAngularVelocityToAlignAxis(FVector LocalShipAxis, FVector TargetAxis, FVector TargetAngularVelocity, float DeltaSeconds) const;

	/** Return true if the ship is dangerous */
	virtual bool IsShipDangerous(AFlareShip* ShipCandidate) const;

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

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UPROPERTY()
	AFlareShip*                               Ship;

	UPROPERTY()
	UFlareCompany*                            PlayerCompany;

	// Component description
	FFlareShipPilotSave                       ShipPilotData;

	//Output commands
	bool                                      UseOrbitalBoost;
	bool                                      WantFire;
	FVector                                   LinearTargetVelocity;
	FVector                                   AngularTargetVelocity;


	// Pilot brain TODO save in save
	float                                ReactionTime;
	float                                TimeUntilNextReaction;
	FVector                              PilotTargetLocation;
	AFlareShip*                          PilotTargetShip;
	AFlareShip*                          PilotTargetStation;
	AFlareShip*                          PilotLastTargetStation;

	float AttackAngle;
	float AttackDistance;
	float AttackPhase;
	float LastTargetDistance;
};
