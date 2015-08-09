#pragma once

#include "FlareSpacecraftDockingSystemInterface.h"
#include "FlareSpacecraftDockingSystem.generated.h"

class AFlareSpacecraft;



/** Spacecraft docking system class */
UCLASS()
class FLARE_API UFlareSpacecraftDockingSystem : public UObject, public IFlareSpacecraftDockingSystemInterface
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
	virtual TArray<IFlareSpacecraftInterface*> GetDockedShips();

	/** Request a docking point */
	virtual FFlareDockingInfo RequestDock(IFlareSpacecraftInterface* Ship, FVector PreferredLocation);

	/** Cancel docking */
	virtual void ReleaseDock(IFlareSpacecraftInterface* Ship, int32 DockId);

	/** Confirm the docking from external ship */
	virtual void Dock(IFlareSpacecraftInterface* Ship, int32 DockId);

	/** Get infos for a specific docking port */
	virtual FFlareDockingInfo GetDockInfo(int32 DockId);

	virtual bool HasAvailableDock(IFlareSpacecraftInterface* Ship) const;

	virtual bool HasCompatibleDock(IFlareSpacecraftInterface* Ship) const;

	virtual int GetDockCount() const;

	virtual bool IsGrantedShip(IFlareSpacecraftInterface* ShipCanditate) const;

	virtual bool IsDockedShip(IFlareSpacecraftInterface* ShipCanditate) const;

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
