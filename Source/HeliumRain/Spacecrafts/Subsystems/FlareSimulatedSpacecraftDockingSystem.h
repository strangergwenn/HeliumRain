#pragma once

#include "FlareSpacecraftDockingSystemInterface.h"
#include "FlareSimulatedSpacecraftDockingSystem.generated.h"

class UFlareSimulatedSpacecraft;
struct FFlareSpacecraftDescription;

/** Spacecraft docking system class */
UCLASS()
class HELIUMRAIN_API UFlareSimulatedSpacecraftDockingSystem : public UObject, public IFlareSpacecraftDockingSystemInterface
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
	virtual TArray<UFlareSimulatedSpacecraft*> GetDockedShips();

	/** Request a docking point */
	virtual FFlareDockingInfo RequestDock(UFlareSimulatedSpacecraft* Ship, FVector PreferredLocation);

	/** Cancel docking */
	virtual void ReleaseDock(UFlareSimulatedSpacecraft* Ship, int32 DockId);

	/** Confirm the docking from external ship */
	virtual void Dock(UFlareSimulatedSpacecraft* Ship, int32 DockId);

	virtual int GetDockCount() const;

	virtual bool HasCompatibleDock(UFlareSimulatedSpacecraft* Ship) const;

	virtual bool IsDockedShip(UFlareSimulatedSpacecraft* ShipCanditate) const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UPROPERTY()
	UFlareSimulatedSpacecraft*                               Spacecraft;
	FFlareSpacecraftSave*                           Data;
	FFlareSpacecraftDescription*                    Description;

};
