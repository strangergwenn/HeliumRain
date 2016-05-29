
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

TArray<UFlareSimulatedSpacecraft*> UFlareSimulatedSpacecraftDockingSystem::GetDockedShips()
{
	// TODO replace mock
	TArray<UFlareSimulatedSpacecraft*> MockArray;
	return MockArray;
}

FFlareDockingInfo UFlareSimulatedSpacecraftDockingSystem::RequestDock(UFlareSimulatedSpacecraft* Ship, FVector PreferredLocation)
{
	// TODO replace mock
	FFlareDockingInfo MockInfo;
	return MockInfo;
}

void UFlareSimulatedSpacecraftDockingSystem::ReleaseDock(UFlareSimulatedSpacecraft* Ship, int32 DockId)
{
	// TODO replace mock
}

void UFlareSimulatedSpacecraftDockingSystem::Dock(UFlareSimulatedSpacecraft* Ship, int32 DockId)
{
	// TODO replace mock
}

int UFlareSimulatedSpacecraftDockingSystem::GetDockCount() const
{
	// TODO replace mock
	return 0;
}

bool UFlareSimulatedSpacecraftDockingSystem::HasCompatibleDock(UFlareSimulatedSpacecraft* Ship) const
{
	// TODO replace mock
	return false;
}

bool UFlareSimulatedSpacecraftDockingSystem::IsDockedShip(UFlareSimulatedSpacecraft* ShipCanditate) const
{
	// TODO replace mock
	return false;
}


#undef LOCTEXT_NAMESPACE
