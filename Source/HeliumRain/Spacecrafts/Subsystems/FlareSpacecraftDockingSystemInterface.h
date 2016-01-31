#pragma once

#include "FlareSpacecraftDockingSystemInterface.generated.h"


class IFlareSpacecraftInterface;

/** Docking data */
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
		virtual void ReleaseDock(IFlareSpacecraftInterface* Ship, int32 DockId) = 0;

		/** Confirm the docking from external ship */
		virtual void Dock(IFlareSpacecraftInterface* Ship, int32 DockId) = 0;

		virtual int GetDockCount() const = 0;

		virtual bool HasCompatibleDock(IFlareSpacecraftInterface* Ship) const = 0;

		virtual bool IsDockedShip(IFlareSpacecraftInterface* ShipCanditate) const = 0;


};
