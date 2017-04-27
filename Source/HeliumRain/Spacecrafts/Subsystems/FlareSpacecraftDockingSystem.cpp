
#include "FlareSpacecraftDockingSystem.h"
#include "../../Flare.h"

#include "../FlareStationDock.h"
#include "../FlareSpacecraft.h"
#include "../../Game/FlareGame.h"
#include "../../Quests/FlareQuestManager.h"

DECLARE_CYCLE_STAT(TEXT("FlareDockingSystem Tick"), STAT_FlareDockingSystem_Tick, STATGROUP_Flare);

#define LOCTEXT_NAMESPACE "FlareDockingSystem"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftDockingSystem::UFlareSpacecraftDockingSystem(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, Spacecraft(NULL)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareDockingSystem_Tick);
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
	Description = Spacecraft->GetParent()->GetDescription();
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
			Info.LocalTopAxis = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(DockRotation.RotateVector(FVector(0,1,0)));
			Info.LocalLocation = Spacecraft->Airframe->GetComponentToWorld().Inverse().TransformPosition(DockLocation);
			Info.DockId = Count;
			Info.DockSize = Component->DockSize;
			Info.Station = Spacecraft;
			Info.Granted = false;
			Info.Occupied = false;

			// Push this slot
			DockingSlots.Add(Info);
			Count++;
		}
	}
}

bool UFlareSpacecraftDockingSystem::HasCompatibleDock(AFlareSpacecraft* Ship) const
{
	for (int32 i = 0; i < DockingSlots.Num(); i++)
	{
		if (DockingSlots[i].DockSize == Ship->GetSize())
		{
			return true;
		}
	}
	return false;
}

FFlareDockingInfo UFlareSpacecraftDockingSystem::RequestDock(AFlareSpacecraft* Ship, FVector PreferredLocation)
{
	int32 BestIndex = -1;
	float BestDistance = 0;

	// Looking for nearest available slot
	for (int32 i = 0; i < DockingSlots.Num(); i++)
	{
		if (!DockingSlots[i].Granted && DockingSlots[i].DockSize == Ship->GetSize())
		{
			float DockDistance = (Spacecraft->Airframe->GetComponentToWorld().TransformPosition(DockingSlots[i].LocalLocation) - PreferredLocation).Size();
			if (BestIndex < 0 || DockDistance < BestDistance)
			{
				BestDistance = DockDistance;
				BestIndex = i;
			}
		}
	}

	// Granted
	if (BestIndex >= 0)
	{
		DockingSlots[BestIndex].Granted = true;
		DockingSlots[BestIndex].Ship = Ship;
		return DockingSlots[BestIndex];
	}

	// Denied, but player ship, so undock an AI ship
	else if (Ship->IsPlayerShip())
	{
		// Look again without constraint
		for (int32 i = 0; i < DockingSlots.Num(); i++)
		{
			if (DockingSlots[i].DockSize == Ship->GetSize())
			{
				float DockDistance = (Spacecraft->Airframe->GetComponentToWorld().TransformPosition(DockingSlots[i].LocalLocation) - PreferredLocation).Size();
				if (BestIndex < 0 || DockDistance < BestDistance)
				{
					BestDistance = DockDistance;
					BestIndex = i;
				}
			}
		}

		// Undock previous owner
		FCHECK(BestIndex >= 0);
		if (DockingSlots[BestIndex].Ship && DockingSlots[BestIndex].Ship->IsValidLowLevel())
		{
			if (DockingSlots[BestIndex].Ship->GetNavigationSystem()->IsDocked())
			{
				DockingSlots[BestIndex].Ship->GetNavigationSystem()->Undock();
			}
			else
			{
				DockingSlots[BestIndex].Ship->GetNavigationSystem()->AbortAllCommands();
			}
		}

		// Grant dock
		DockingSlots[BestIndex].Granted = true;
		DockingSlots[BestIndex].Ship = Ship;
		return DockingSlots[BestIndex];
	}

	// Denied
	else
	{
		FFlareDockingInfo Info;
		Info.Granted = false;
		Info.Station = Spacecraft;
		return Info;
	}
}

void UFlareSpacecraftDockingSystem::ReleaseDock(AFlareSpacecraft* Ship, int32 DockId)
{
	FLOGV("UFlareSpacecraftDockingSystem::ReleaseDock %d ('%s')", DockId, *Ship->GetParent()->GetImmatriculation().ToString());
	DockingSlots[DockId].Granted = false;
	DockingSlots[DockId].Occupied = false;
	DockingSlots[DockId].Ship = NULL;
}

void UFlareSpacecraftDockingSystem::Dock(AFlareSpacecraft* Ship, int32 DockId)
{
	FLOGV("UFlareSpacecraftDockingSystem::Dock %d ('%s')", DockId, *Ship->GetParent()->GetImmatriculation().ToString());
	DockingSlots[DockId].Granted = true;
	DockingSlots[DockId].Occupied = true;
	DockingSlots[DockId].Ship = Ship;

	Spacecraft->GetGame()->GetQuestManager()->OnShipDocked(Spacecraft->GetParent(), Ship->GetParent());
}

TArray<AFlareSpacecraft*> UFlareSpacecraftDockingSystem::GetDockedShips()
{
	TArray<AFlareSpacecraft*> Result;

	for (int32 i = 0; i < DockingSlots.Num(); i++)
	{
		if (DockingSlots[i].Granted && DockingSlots[i].Occupied)
		{
			//FLOGV("UFlareSpacecraftDockingSystem::GetDockedShips : found valid dock %d", i);
			Result.AddUnique(DockingSlots[i].Ship);
		}
	}

	return Result;
}

bool UFlareSpacecraftDockingSystem::HasAvailableDock(AFlareSpacecraft* Ship) const
{
	// Looking for slot
	for (int32 i = 0; i < DockingSlots.Num(); i++)
	{
		if (DockingSlots[i].DockSize != Ship->GetSize())
		{
			continue;
		}

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

bool UFlareSpacecraftDockingSystem::IsGrantedShip(AFlareSpacecraft* ShipCanditate) const
{
	for (int32 i = 0; i < DockingSlots.Num(); i++)
	{
		if (DockingSlots[i].Granted && DockingSlots[i].Ship == ShipCanditate)
		{
			return true;
		}
	}

	return false;
}


bool UFlareSpacecraftDockingSystem::IsDockedShip(AFlareSpacecraft* ShipCanditate) const
{
	for (int32 i = 0; i < DockingSlots.Num(); i++)
	{
		if (DockingSlots[i].Occupied && DockingSlots[i].Ship == ShipCanditate)
		{
			return true;
		}
	}

	return false;
}


#undef LOCTEXT_NAMESPACE
