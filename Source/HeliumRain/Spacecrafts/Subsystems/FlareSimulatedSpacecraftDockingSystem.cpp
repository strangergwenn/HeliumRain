
#include "../../Flare.h"

#include "../FlareSimulatedSpacecraft.h"
#include "FlareSimulatedSpacecraftDockingSystem.h"

#define LOCTEXT_NAMESPACE "FlareSimulatedSpacecraftDockingSystem"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSimulatedSpacecraftDockingSystem::UFlareSimulatedSpacecraftDockingSystem(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, Spacecraft(NULL)
{
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/


void UFlareSimulatedSpacecraftDockingSystem::Initialize(UFlareSimulatedSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData)
{
	Spacecraft = OwnerSpacecraft;
	Description = Spacecraft->GetDescription();
	Data = OwnerData;

}

/*----------------------------------------------------
	System API
----------------------------------------------------*/

TArray<IFlareSpacecraftInterface*> UFlareSimulatedSpacecraftDockingSystem::GetDockedShips()
{
	// TODO replace mock
	TArray<IFlareSpacecraftInterface*> MockArray;
	return MockArray;
}

FFlareDockingInfo UFlareSimulatedSpacecraftDockingSystem::RequestDock(IFlareSpacecraftInterface* Ship, FVector PreferredLocation)
{
	// TODO replace mock
	FFlareDockingInfo MockInfo;
	return MockInfo;
}

void UFlareSimulatedSpacecraftDockingSystem::ReleaseDock(IFlareSpacecraftInterface* Ship, int32 DockId)
{
	// TODO replace mock
}

void UFlareSimulatedSpacecraftDockingSystem::Dock(IFlareSpacecraftInterface* Ship, int32 DockId)
{
	// TODO replace mock
}

int UFlareSimulatedSpacecraftDockingSystem::GetDockCount() const
{
	// TODO replace mock
	return 0;
}

bool UFlareSimulatedSpacecraftDockingSystem::HasCompatibleDock(IFlareSpacecraftInterface* Ship) const
{
	// TODO replace mock
	return false;
}

bool UFlareSimulatedSpacecraftDockingSystem::IsDockedShip(IFlareSpacecraftInterface* ShipCanditate) const
{
	// TODO replace mock
	return false;
}


#undef LOCTEXT_NAMESPACE
