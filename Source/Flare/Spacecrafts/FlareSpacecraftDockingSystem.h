#pragma once

#include "FlareSpacecraftDockingSystem.generated.h"

class AFlareSpacecraft;

struct FFlareDockingInfo
{
	bool                      Granted;
	bool                      Occupied;
	int32                     DockId;
	IFlareSpacecraftInterface*   Station;
	IFlareSpacecraftInterface*      Ship;

	FRotator                  Rotation;
	FVector                   StartPoint;
	FVector                   EndPoint;

	FFlareDockingInfo()
		: Granted(false)
		, Occupied(false)
		, DockId(-1)
		, Station(NULL)
	{}
};

/** Spacecraft docking system class */
UCLASS()
class FLARE_API UFlareSpacecraftDockingSystem : public UObject
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
	virtual FFlareDockingInfo RequestDock(IFlareSpacecraftInterface* Ship);

	/** Cancel docking */
	virtual void ReleaseDock(IFlareSpacecraftInterface* Ship, int32 DockId);

	/** Confirm the docking from external ship */
	virtual void Dock(IFlareSpacecraftInterface* Ship, int32 DockId);

	virtual bool HasAvailableDock(IFlareSpacecraftInterface* Ship) const;

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UPROPERTY()
	AFlareSpacecraft*                               Spacecraft;
	FFlareSpacecraftSave*                           Data;
	FFlareSpacecraftDescription*                    Description;
	TArray<UActorComponent*>                        Components;
};
