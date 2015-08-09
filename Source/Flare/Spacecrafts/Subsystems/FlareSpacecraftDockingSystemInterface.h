#pragma once

#include "FlareSpacecraftDockingSystemInterface.generated.h"

struct FFlareDockingInfo
{
	bool                      Granted;
	bool                      Occupied;
	int32                     DockId;
	EFlarePartSize::Type      DockSize;
	IFlareSpacecraftInterface*   Station;
	IFlareSpacecraftInterface*      Ship;

	FVector                   LocalAxis;
	FVector                   LocalLocation;

	FFlareDockingInfo()
		: Granted(false)
		, Occupied(false)
		, DockId(-1)
		, Station(NULL)
	{}
};

/** Interface wrapper */
UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UFlareSpacecraftDockingSystemInterface  : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};


/** Actual interface */
class IFlareSpacecraftDockingSystemInterface
{
	GENERATED_IINTERFACE_BODY()

public:

		/*----------------------------------------------------
			System Interface
		----------------------------------------------------*/

		/** Get the list of docked ships */
		virtual TArray<IFlareSpacecraftInterface*> GetDockedShips() = 0;

		/** Request a docking point */
		virtual FFlareDockingInfo RequestDock(IFlareSpacecraftInterface* Ship, FVector PreferredLocation) = 0;

		/** Cancel docking */
		virtual void ReleaseDock(IFlareSpacecraftInterface* Ship, int32 DockId);

		/** Confirm the docking from external ship */
		virtual void Dock(IFlareSpacecraftInterface* Ship, int32 DockId);

		virtual int GetDockCount() const;

		virtual bool HasCompatibleDock(IFlareSpacecraftInterface* Ship) const;

		virtual bool IsDockedShip(IFlareSpacecraftInterface* ShipCanditate) const;


};
