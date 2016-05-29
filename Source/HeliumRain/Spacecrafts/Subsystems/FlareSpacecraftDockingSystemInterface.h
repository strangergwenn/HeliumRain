#pragma once

#include "../FlareSpacecraftTypes.h"
#include "FlareSpacecraftDockingSystemInterface.generated.h"

/** Docking data */
struct FFlareDockingInfo
{
	bool                      Granted;
	bool                      Occupied;
	int32                     DockId;
	EFlarePartSize::Type      DockSize;
	UFlareSimulatedSpacecraft*   Station;
	UFlareSimulatedSpacecraft*      Ship;

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
		virtual TArray<UFlareSimulatedSpacecraft*> GetDockedShips() = 0;

		/** Request a docking point */
		virtual FFlareDockingInfo RequestDock(UFlareSimulatedSpacecraft* Ship, FVector PreferredLocation) = 0;

		/** Cancel docking */
		virtual void ReleaseDock(UFlareSimulatedSpacecraft* Ship, int32 DockId) = 0;

		/** Confirm the docking from external ship */
		virtual void Dock(UFlareSimulatedSpacecraft* Ship, int32 DockId) = 0;

		virtual int GetDockCount() const = 0;

		virtual bool HasCompatibleDock(UFlareSimulatedSpacecraft* Ship) const = 0;

		virtual bool IsDockedShip(UFlareSimulatedSpacecraft* ShipCanditate) const = 0;


};
