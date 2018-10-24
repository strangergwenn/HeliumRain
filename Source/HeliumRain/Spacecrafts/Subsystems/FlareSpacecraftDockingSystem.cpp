
#include "FlareSpacecraftDockingSystem.h"
#include "../../Flare.h"

#include "../FlareStationDock.h"
#include "../FlareStationConnector.h"
#include "../FlareSpacecraft.h"

#include "../../Player/FlarePlayerController.h"

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

	auto CheckDockingSlot = [&Ship, &PreferredLocation](FFlareDockingInfo const& Slot, float& DockDistance, bool OnlyFreeSlot)
	{
		if(OnlyFreeSlot && Slot.Granted)
		{
			return false;
		}

		if (Slot.DockSize == Ship->GetSize())
		{
			DockDistance = (Slot.Station->Airframe->GetComponentToWorld().TransformPosition(Slot.LocalLocation) - PreferredLocation).Size();
			return true;
		}

		return false;
	};

	auto CheckDockingSlots = [&CheckDockingSlot](FFlareDockingInfo*& BestDockingSlot, float& BestDistance, TArray<FFlareDockingInfo>& Slots, bool OnlyFreeSlot)
	{
		for(FFlareDockingInfo& Slot : Slots)
		{
			float DockDistance = 0;
			if(CheckDockingSlot(Slot, DockDistance, OnlyFreeSlot))
			{
				if(BestDockingSlot == nullptr || DockDistance < BestDistance)
				{
					BestDockingSlot = &Slot;
					BestDistance = DockDistance;
				}
			}
		}
	};

	auto FindBestDockingSlot = [this, &CheckDockingSlots](bool OnlyFreeSlot)
	{
		FFlareDockingInfo* BestDockingSlot = nullptr;
		float BestDistance = 0;

		CheckDockingSlots(BestDockingSlot, BestDistance, DockingSlots, OnlyFreeSlot);

		if(Spacecraft->GetParent()->IsComplex())
		{
			for (FFlareDockingInfo& MasterConnector : Spacecraft->GetParent()->GetStationConnectors())
			{
				if(MasterConnector.Occupied)
				{
					AFlareSpacecraft* ChildStation = NULL;
					for(AFlareSpacecraft* StationCandidate : Spacecraft->GetGame()->GetActiveSector()->GetSpacecrafts())
					{
						if (StationCandidate->GetImmatriculation() == MasterConnector.ConnectedStationName)
						{
							ChildStation = StationCandidate;
							break;
						}
					}

					if(ChildStation)
					{
						CheckDockingSlots(BestDockingSlot, BestDistance, ChildStation->GetDockingSystem()->GetDockingSlots(), OnlyFreeSlot);
					}
				}
			}

		}

		return BestDockingSlot;
	};


	// Looking for nearest available slot
	FFlareDockingInfo* BestDockingSlot = FindBestDockingSlot(true);

	auto GrantSlot = [](FFlareDockingInfo* Slot, AFlareSpacecraft* Ship)
	{
		Slot->Granted = true;
		Slot->Ship = Ship;
	};


	if(BestDockingSlot)
	{
		// Granted
		GrantSlot(BestDockingSlot, Ship);
		return *BestDockingSlot;
	}
	// Denied, but player ship, so undock an AI ship
	else if (Ship->IsPlayerShip())
	{
		BestDockingSlot = FindBestDockingSlot(false);

		if(BestDockingSlot)
		{
			// Undock previous owner
			if ((BestDockingSlot->Granted || BestDockingSlot->Occupied) && BestDockingSlot->Ship && BestDockingSlot->Ship->IsValidLowLevel())
			{
				if (BestDockingSlot->Ship->GetNavigationSystem()->IsDocked())
				{
					BestDockingSlot->Ship->GetNavigationSystem()->Undock();
				}
				else
				{
					BestDockingSlot->Ship->GetNavigationSystem()->AbortAllCommands();
				}
			}

			// Grant dock
			GrantSlot(BestDockingSlot, Ship);
			return *BestDockingSlot;
		}
	}

	// Denied
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
