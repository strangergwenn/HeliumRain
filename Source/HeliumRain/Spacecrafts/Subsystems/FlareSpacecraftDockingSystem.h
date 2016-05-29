#pragma once

#include "FlareSpacecraftDockingSystemInterface.h"
#include "FlareSpacecraftDockingSystem.generated.h"

class AFlareSpacecraft;



/** Spacecraft docking system class */
UCLASS()
class HELIUMRAIN_API UFlareSpacecraftDockingSystem : public UObject, public IFlareSpacecraftDockingSystemInterface
{

public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void TickSystem(float DeltaSeconds);

	/** Initialize this system */
	virtual void Initialize(AFlareSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData);

	virtual void Start();

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

	/** Get infos for a specific docking port */
	virtual FFlareDockingInfo GetDockInfo(int32 DockId);

	virtual bool HasAvailableDock(UFlareSimulatedSpacecraft* Ship) const;

	virtual bool HasCompatibleDock(UFlareSimulatedSpacecraft* Ship) const;

	virtual int GetDockCount() const;

	virtual bool IsGrantedShip(UFlareSimulatedSpacecraft* ShipCanditate) const;

	virtual bool IsDockedShip(UFlareSimulatedSpacecraft* ShipCanditate) const;

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UPROPERTY()
	AFlareSpacecraft*                               Spacecraft;
	FFlareSpacecraftSave*                           Data;
	FFlareSpacecraftDescription*                    Description;
	TArray<UActorComponent*>                        Components;

	// Dock data
	TArray <FFlareDockingInfo>       DockingSlots;

};
