
#include "../Flare.h"

#include "FlareSpacecraftDockingSystem.h"
#include "FlareSpacecraft.h"

#define LOCTEXT_NAMESPACE "FlareSpacecraftDockingSystem"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftDockingSystem::UFlareSpacecraftDockingSystem(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, Spacecraft(NULL)
{
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void UFlareSpacecraftDockingSystem::TickSystem(float DeltaSeconds)
{
}

void UFlareSpacecraftDockingSystem::Initialize(AFlareSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData)
{
	Spacecraft = OwnerSpacecraft;
	Components = Spacecraft->GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	Description = Spacecraft->GetDescription();
	Data = OwnerData;

}

void UFlareSpacecraftDockingSystem::Start()
{

}



FFlareDockingInfo UFlareSpacecraftDockingSystem::RequestDock(IFlareSpacecraftInterface* Ship)
{
	FLOGV("AFlareSpacecraft::RequestDock ('%s')", *Ship->_getUObject()->GetName());

	// Looking for slot
	/*for (int32 i = 0; i < DockingSlots.Num(); i++)
	{
		if (!DockingSlots[i].Granted)
		{
			FLOGV("AFlareSpacecraft::RequestDock : found valid dock %d", i);
			DockingSlots[i].Granted = true;
			DockingSlots[i].Ship = Ship;
			return DockingSlots[i];
		}
	}*/
	// TODO Fix

	// Default values
	FFlareDockingInfo Info;
	Info.Granted = false;
	Info.Station = Spacecraft;
	return Info;
}

void UFlareSpacecraftDockingSystem::ReleaseDock(IFlareSpacecraftInterface* Ship, int32 DockId)
{
	FLOGV("AFlareSpacecraft::ReleaseDock %d ('%s')", DockId, *Ship->_getUObject()->GetName());
	/*DockingSlots[DockId].Granted = false;
	DockingSlots[DockId].Occupied = false;
	DockingSlots[DockId].Ship = NULL;*/
	// TODO Fix
}

void UFlareSpacecraftDockingSystem::Dock(IFlareSpacecraftInterface* Ship, int32 DockId)
{
	FLOGV("AFlareSpacecraft::Dock %d ('%s')", DockId, *Ship->_getUObject()->GetName());
	/*DockingSlots[DockId].Granted = true;
	DockingSlots[DockId].Occupied = true;
	DockingSlots[DockId].Ship = Ship;*/
	// TODO Fix
}

TArray<IFlareSpacecraftInterface*> UFlareSpacecraftDockingSystem::GetDockedShips()
{
	TArray<IFlareSpacecraftInterface*> Result;

	/*for (int32 i = 0; i < DockingSlots.Num(); i++)
	{
		if (DockingSlots[i].Granted)
		{
			FLOGV("AFlareSpacecraft::GetDockedShips : found valid dock %d", i);
			Result.AddUnique(DockingSlots[i].Ship);
		}
	}*/
	//TODO Externalize

	return Result;
}

bool UFlareSpacecraftDockingSystem::HasAvailableDock(IFlareSpacecraftInterface* Ship) const
{
	// Looking for slot
	/*for (int32 i = 0; i < DockingSlots.Num(); i++)
	{
		if (!DockingSlots[i].Granted)
		{
			return true;
		}
	}*/

	//TODO Externalize

	return false;
}



#undef LOCTEXT_NAMESPACE
