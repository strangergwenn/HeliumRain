#pragma once

#include "FlareSpacecraftDockingSystemInterface.h"
#include "FlareSimulatedSpacecraftDockingSystem.generated.h"



/** Spacecraft docking system class */
UCLASS()
class FLARE_API UFlareSimulatedSpacecraftDockingSystem : public UObject, public IFlareSpacecraftDockingSystemInterface
{

public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Initialize this system */
	virtual void Initialize(UFlareSimulatedSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData);

public:

	/*----------------------------------------------------
		System API
	----------------------------------------------------*/

	/** Get the list of docked ships */
	virtual TArray<IFlareSpacecraftInterface*> GetDockedShips();

	/** Request a docking point */
	virtual FFlareDockingInfo RequestDock(IFlareSpacecraftInterface* Ship, FVector PreferredLocation);

	/** Cancel docking */
	virtual void ReleaseDock(IFlareSpacecraftInterface* Ship, int32 DockId);

	/** Confirm the docking from external ship */
	virtual void Dock(IFlareSpacecraftInterface* Ship, int32 DockId);

	virtual int GetDockCount() const;

	virtual bool HasCompatibleDock(IFlareSpacecraftInterface* Ship) const;

	virtual bool IsDockedShip(IFlareSpacecraftInterface* ShipCanditate) const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UPROPERTY()
	UFlareSimulatedSpacecraft*                               Spacecraft;
	FFlareSpacecraftSave*                           Data;
	FFlareSpacecraftDescription*                    Description;

};
