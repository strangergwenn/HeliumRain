
#include "FlareSpacecraftDockingSystem.h"
#include "../../Flare.h"

#include "../FlareStationDock.h"
#include "../FlareStationConnector.h"
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
	int32 DockCount = 0;
	int32 ConnectorCount = 0;
	TArray<UActorComponent*> ActorComponents;
	Spacecraft->GetComponents(ActorComponents);

	for (TArray<UActorComponent*>::TIterator ComponentIt(ActorComponents); ComponentIt; ++ComponentIt)
	{
		FVector DockLocation;
		FRotator DockRotation;
		UFlareStationDock* DockComponent = Cast<UFlareStationDock>(*ComponentIt);
		UFlareStationConnector* ConnectorComponent = Cast<UFlareStationConnector>(*ComponentIt);

		// Fill dock slots
		if (DockComponent)
		{
			DockComponent->GetSocketWorldLocationAndRotation(FName("dock"), DockLocation, DockRotation);

			// Fill info
			FFlareDockingInfo Info;
			Info.Ship = NULL;
			Info.LocalAxis = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(DockRotation.RotateVector(FVector(1,0,0)));
			Info.LocalTopAxis = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(DockRotation.RotateVector(FVector(0,1,0)));
			Info.LocalLocation = Spacecraft->Airframe->GetComponentToWorld().Inverse().TransformPosition(DockLocation);
			Info.DockId = DockCount;
			Info.DockSize = DockComponent->DockSize;
			Info.Station = Spacecraft;
			Info.Granted = false;
			Info.Occupied = false;

			// Push this slot
			DockingSlots.Add(Info);
			DockCount++;
		}

		// Fill connector data to the parent
		if (ConnectorComponent)
		{
			FCHECK(ConnectorCount < Spacecraft->GetDescription()->StationConnectorCount);
			FCHECK(ConnectorCount < Spacecraft->GetParent()->GetStationConnectors().Num());
			
			// Fetch connector data
			FFlareDockingInfo* StationConnection = Spacecraft->GetParent()->GetStationConnector(ConnectorComponent->SlotIdentifier);
			FCHECK(StationConnection != NULL);
			
			// Fill info
			if (StationConnection)
			{
				ConnectorComponent->GetSocketWorldLocationAndRotation(FName("dock"), DockLocation, DockRotation);

				StationConnection->Ship = NULL;
				StationConnection->Station = Spacecraft;
				StationConnection->LocalAxis = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(DockRotation.RotateVector(FVector(1, 0, 0)));
				StationConnection->LocalTopAxis = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(DockRotation.RotateVector(FVector(0, 1, 0)));
				StationConnection->LocalLocation = Spacecraft->Airframe->GetComponentToWorld().Inverse().TransformPosition(DockLocation);
				StationConnection->Occupied = true;
			}
			else
			{
				FLOGV("UFlareSpacecraftDockingSystem::Start : couldn't find station connector '%s' in parent", *ConnectorComponent->SlotIdentifier.ToString());
			}

			ConnectorCount++;
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

	// Player docking data
	FText PlayerDockInfo;
	AFlareSpacecraft* PlayerDockSpacecraft;
	FFlareDockingParameters PlayerDockingParameters;
	AFlareSpacecraft* PlayerShip = Ship->GetGame()->GetPC()->GetShipPawn();

	// Deny all docking requests if the player is docking
	if (PlayerShip)
	{
		bool DockingInProgress = PlayerShip->GetManualDockingProgress(PlayerDockSpacecraft, PlayerDockingParameters, PlayerDockInfo);
		if (DockingInProgress && PlayerDockSpacecraft == Spacecraft && PlayerShip != Ship)
		{
			FLOG("UFlareSpacecraftDockingSystem::RequestDock : denied, player is docking");
			FFlareDockingInfo Info;
			Info.Granted = false;
			Info.Station = Spacecraft;
			return Info;
		}
	}

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
		BestIndex = 0;

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
		if ((DockingSlots[BestIndex].Granted || DockingSlots[BestIndex].Occupied) && DockingSlots[BestIndex].Ship && DockingSlots[BestIndex].Ship->IsValidLowLevel())
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
