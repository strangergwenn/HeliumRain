#pragma once
#include "FlareTurretPilot.generated.h"

class UFlareTurret;
class UFlareCompany;
class AFlareSpacecraft;
class UFlareSpacecraftComponent;

/** Turret pilot save data */
USTRUCT()
struct FFlareTurretPilotSave
{
	GENERATED_USTRUCT_BODY()

	/** Pilot identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	/** Pilot name */
	UPROPERTY(EditAnywhere, Category = Save)
	FString Name;

};

/** Turret pilot class */
UCLASS()
class FLARE_API UFlareTurretPilot : public UObject
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

	AFlareSpacecraft* GetNearestHostileShip(bool DangerousOnly, bool ReachableOnly, float MaxDistance, EFlarePartSize::Type PreferredType) const;

	virtual UFlareSpacecraftComponent* GetRandomTargetComponent(AFlareSpacecraft* TargetSpacecraft);

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
	float                                ReactionTime;
	float                                TimeUntilNextReaction;
	float                                TimeUntilNextComponentSwitch;
	AFlareSpacecraft*                    PilotTargetShip;
	UFlareSpacecraftComponent*			 PilotTargetComponent;
};
