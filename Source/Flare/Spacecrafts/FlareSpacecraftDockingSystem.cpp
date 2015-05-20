
#include "../Flare.h"

#include "FlareSpacecraftDockingSystem.h"
#include "FlareStationDock.h"
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
	// Dock data
	int32 Count = 0;
	TArray<UActorComponent*> ActorComponents;
	Spacecraft->GetComponents(ActorComponents);

	// Fill all dock slots
	for (TArray<UActorComponent*>::TIterator ComponentIt(ActorComponents); ComponentIt; ++ComponentIt)
	{
		UFlareStationDock* Component = Cast<UFlareStationDock>(*ComponentIt);
		if (Component)
		{
			// Get data
			FVector DockLocation;
			FRotator DockRotation;
			Component->GetSocketWorldLocationAndRotation(FName("dock"), DockLocation, DockRotation);

			// Fill info
			FFlareDockingInfo Info;
			Info.LocalAxis = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(DockRotation.RotateVector(FVector(1,0,0)));
			Info.LocalLocation = Spacecraft->Airframe->GetComponentToWorld().Inverse().TransformPosition(DockLocation);
			Info.DockId = Count;
			Info.Station = Spacecraft;
			Info.Granted = false;
			Info.Occupied = false;

			// Push this slot
			DockingSlots.Add(Info);
			Count++;
		}
	}
}

FFlareDockingInfo UFlareSpacecraftDockingSystem::RequestDock(IFlareSpacecraftInterface* Ship)
{
	FLOGV("UFlareSpacecraftDockingSystem::RequestDock ('%s')", *Ship->_getUObject()->GetName());

	// Looking for slot
	for (int32 i = 0; i < DockingSlots.Num(); i++)
	{
		if (!DockingSlots[i].Granted)
		{
			FLOGV("UFlareSpacecraftDockingSystem::RequestDock : found valid dock %d", i);
			DockingSlots[i].Granted = true;
			DockingSlots[i].Ship = Ship;
			return DockingSlots[i];
		}
	}
	// Default values
	FFlareDockingInfo Info;
	Info.Granted = false;
	Info.Station = Spacecraft;
	return Info;
}

void UFlareSpacecraftDockingSystem::ReleaseDock(IFlareSpacecraftInterface* Ship, int32 DockId)
{
	FLOGV("UFlareSpacecraftDockingSystem::ReleaseDock %d ('%s')", DockId, *Ship->_getUObject()->GetName());
	DockingSlots[DockId].Granted = false;
	DockingSlots[DockId].Occupied = false;
	DockingSlots[DockId].Ship = NULL;
}

void UFlareSpacecraftDockingSystem::Dock(IFlareSpacecraftInterface* Ship, int32 DockId)
{
	FLOGV("UFlareSpacecraftDockingSystem::Dock %d ('%s')", DockId, *Ship->_getUObject()->GetName());
	DockingSlots[DockId].Granted = true;
	DockingSlots[DockId].Occupied = true;
	DockingSlots[DockId].Ship = Ship;
}

TArray<IFlareSpacecraftInterface*> UFlareSpacecraftDockingSystem::GetDockedShips()
{
	TArray<IFlareSpacecraftInterface*> Result;

	for (int32 i = 0; i < DockingSlots.Num(); i++)
	{
		if (DockingSlots[i].Granted)
		{
			FLOGV("UFlareSpacecraftDockingSystem::GetDockedShips : found valid dock %d", i);
			Result.AddUnique(DockingSlots[i].Ship);
		}
	}

	return Result;
}

bool UFlareSpacecraftDockingSystem::HasAvailableDock(IFlareSpacecraftInterface* Ship) const
{
	// Looking for slot
	for (int32 i = 0; i < DockingSlots.Num(); i++)
	{
		if (!DockingSlots[i].Granted)
		{
			return true;
		}
	}

	return false;
}

int UFlareSpacecraftDockingSystem::GetDockCount() const
{
	return DockingSlots.Num();
}

FFlareDockingInfo UFlareSpacecraftDockingSystem::GetDockInfo(int32 DockId)
{
	return DockingSlots[DockId];
}


#undef LOCTEXT_NAMESPACE
