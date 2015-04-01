#pragma once

#include "FlareShipPilot.generated.h"

class AFlareShip;

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
	FVector                                   LinearTargetVelocity;
	FVector                                   AngularTargetVelocity;

	// Pilot brain
	float                                TimeUntilNextChange;
	FVector                              PilotTarget;
};
