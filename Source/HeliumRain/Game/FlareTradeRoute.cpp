
#include "../Flare.h"
#include "FlareTradeRoute.h"
#include "FlareCompany.h"
#include "FlareSimulatedSector.h"
#include "FlareFleet.h"
#include "FlareGame.h"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareTradeRoute::UFlareTradeRoute(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareTradeRoute::Load(const FFlareTradeRouteSave& Data)
{
	TradeRouteCompany = Cast<UFlareCompany>(GetOuter());
	Game = TradeRouteCompany->GetGame();
	TradeRouteData = Data;
	IsFleetListLoaded = false;

    InitFleetList();
}

FFlareTradeRouteSave* UFlareTradeRoute::Save()
{
	return &TradeRouteData;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareTradeRoute::Simulate()
{
    if(TradeRouteData.Sectors.Num() == 0)
    {
        // Nothing to do
        return;
    }

	FLOG("Trade route simulate");
	for(int32 FleetIndex = 0; FleetIndex < TradeRouteFleets.Num() ; FleetIndex++)
	{
		UFlareFleet* Fleet = TradeRouteFleets[FleetIndex];

        FLOGV("  - Fleet %d", FleetIndex);
		if(Fleet->IsTraveling())
		{
            FLOG("  -> is travelling");
			continue;
		}

		UFlareSimulatedSector* CurrentSector = Fleet->GetCurrentSector();
		FFlareTradeRouteSectorSave* SectorOrder = GetSectorOrders(CurrentSector);

		if(SectorOrder)
		{
			// Load and unload ships
			for (int ShipIndex = 0; ShipIndex < Fleet->GetShips().Num(); ShipIndex++)
			{
				UFlareSimulatedSpacecraft* Ship = Fleet->GetShips()[ShipIndex];
				UFlareCompany* Company = Ship->GetCompany();

				// Unload
				for(int ResourceIndex = 0; ResourceIndex < SectorOrder->ResourcesToUnload.Num(); ResourceIndex++)
				{
					FFlareCargoSave* ResourceToUnload = &SectorOrder->ResourcesToUnload[ResourceIndex];
					FFlareResourceDescription* Resource = Game->GetResourceCatalog()->Get(ResourceToUnload->ResourceIdentifier);


					uint32 SectorResourceCount = CurrentSector->GetResourceCount(Company, Resource, false, true);

					if(ResourceToUnload->Quantity == 0 || SectorResourceCount < ResourceToUnload->Quantity)
					{
						uint32 AvailableResourceCount = Ship->GetCargoBay()->GetResourceQuantity(Resource);

						uint32 ResourceToGive = AvailableResourceCount;
						if(ResourceToUnload->Quantity != 0)
						{
							ResourceToGive = FMath::Min(ResourceToGive, ResourceToUnload->Quantity - SectorResourceCount);
						}

						uint32 GivenResources = CurrentSector->GiveResources(Company, Resource, ResourceToGive, true);
						Ship->GetCargoBay()->TakeResources(Resource, GivenResources);
					}
				}

				// Load
				for(int ResourceIndex = 0; ResourceIndex < SectorOrder->ResourcesToLoad.Num(); ResourceIndex++)
				{
					FFlareCargoSave* ResourceToLoad = &SectorOrder->ResourcesToLoad[ResourceIndex];
					FFlareResourceDescription* Resource = Game->GetResourceCatalog()->Get(ResourceToLoad->ResourceIdentifier);

					uint32 AvailableResourceCount = CurrentSector->GetResourceCount(Company, Resource, false, true);

					if(AvailableResourceCount > ResourceToLoad->Quantity)
					{
						uint32 ResourceToBuy = AvailableResourceCount - ResourceToLoad->Quantity;
						uint32 FreeSpace = Ship->GetCargoBay()->GetFreeSpaceForResource(Resource);

						uint32 ResourcesToTake = FMath::Min(FreeSpace, ResourceToBuy);

						uint32 TakenResources = CurrentSector->TakeUselessResources(Company, Resource, ResourcesToTake, true);

						uint32 GivenResources = Ship->GetCargoBay()->GiveResources(Resource, TakenResources);

						if(GivenResources != TakenResources)
						{
							FLOGV("WARNING: Trade route take %d resource but can give only %d resource in the cargo", TakenResources, GivenResources);
						}
					}
				}
			}
		}

		UFlareSimulatedSector* NextTradeSector = GetNextTradeSector(CurrentSector);

		if(NextTradeSector == CurrentSector)
		{
			continue;
		}
        if(NextTradeSector)
        {
            FLOGV("  start travel to %s", *NextTradeSector->GetSectorName().ToString());
            // Travel to next sector
            Game->GetGameWorld()->StartTravel(Fleet, NextTradeSector);
        }
        else
        {
            FLOG("No next sector");
        }


	}

}


void UFlareTradeRoute::AddFleet(UFlareFleet* Fleet)
{
	UFlareTradeRoute* OldTradeRoute = Fleet->GetCurrentTradeRoute();
	if (OldTradeRoute)
	{
		OldTradeRoute->RemoveFleet(Fleet);
	}

	TradeRouteData.FleetIdentifiers.Add(Fleet->GetIdentifier());
	TradeRouteFleets.AddUnique(Fleet);
	Fleet->SetCurrentTradeRoute(this);
}

void UFlareTradeRoute::RemoveFleet(UFlareFleet* Fleet)
{
	TradeRouteData.FleetIdentifiers.Remove(Fleet->GetIdentifier());
	TradeRouteFleets.Remove(Fleet);
	Fleet->SetCurrentTradeRoute(NULL);
}

void UFlareTradeRoute::AddSector(UFlareSimulatedSector* Sector)
{
    if(IsVisiting(Sector))
    {
        FLOG("Warning: try to add a sector already visited by the trade route");
        return;
    }
	FFlareTradeRouteSectorSave TradeRouteSector;
	TradeRouteSector.SectorIdentifier = Sector->GetIdentifier();

	TradeRouteData.Sectors.Add(TradeRouteSector);
}

void UFlareTradeRoute::RemoveSector(UFlareSimulatedSector* Sector)
{
	for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
	{
		if (TradeRouteData.Sectors[SectorIndex].SectorIdentifier == Sector->GetIdentifier())
		{
			TradeRouteData.Sectors.RemoveAt(SectorIndex);
			return;
		}
	}
}

void UFlareTradeRoute::SetSectorLoadOrder(int32 SectorIndex, FFlareResourceDescription* Resource, uint32 QuantityToLeft)
{
	if(SectorIndex >= TradeRouteData.Sectors.Num())
	{
		FLOGV("Fail to configure trade route '%s', sector %d: only %d sector present.", *GetTradeRouteName().ToString(), SectorIndex, TradeRouteData.Sectors.Num());
		return;
	}

	 FFlareTradeRouteSectorSave* Sector = &TradeRouteData.Sectors[SectorIndex];

	 // First check if there is a previous order for this resource
	 for(int ResourceIndex = 0; ResourceIndex < Sector->ResourcesToLoad.Num(); ResourceIndex++)
	 {
		 if(Sector->ResourcesToLoad[ResourceIndex].ResourceIdentifier == Resource->Identifier)
		 {
			 Sector->ResourcesToLoad[ResourceIndex].Quantity = QuantityToLeft;
			 return;
		 }
	 }

	 // Not found
	 FFlareCargoSave ResourceOrder;
	 ResourceOrder.ResourceIdentifier = Resource->Identifier;
	 ResourceOrder.Quantity = QuantityToLeft;
	 Sector->ResourcesToLoad.Add(ResourceOrder);
}

void UFlareTradeRoute::ClearSectorLoadOrder(int32 SectorIndex, FFlareResourceDescription* Resource)
{
	if(SectorIndex >= TradeRouteData.Sectors.Num())
	{
		FLOGV("Fail to configure trade route '%s', sector %d: only %d sector present.", *GetTradeRouteName().ToString(), SectorIndex, TradeRouteData.Sectors.Num());
		return;
	}

	FFlareTradeRouteSectorSave* Sector = &TradeRouteData.Sectors[SectorIndex];

	for(int ResourceIndex = 0; ResourceIndex < Sector->ResourcesToLoad.Num(); ResourceIndex++)
	{
		if(Sector->ResourcesToLoad[ResourceIndex].ResourceIdentifier == Resource->Identifier)
		{
			Sector->ResourcesToLoad.RemoveAt(ResourceIndex);
			break;
		}
	}
}

void UFlareTradeRoute::SetSectorUnloadOrder(int32 SectorIndex, FFlareResourceDescription* Resource, uint32 LimitQuantity)
{
	if(SectorIndex >= TradeRouteData.Sectors.Num())
	{
		FLOGV("Fail to configure trade route '%s', sector %d: only %d sector present.", *GetTradeRouteName().ToString(), SectorIndex, TradeRouteData.Sectors.Num());
		return;
	}

	FFlareTradeRouteSectorSave* Sector = &TradeRouteData.Sectors[SectorIndex];

	// First check if there is a previous order for this resource
	for(int ResourceIndex = 0; ResourceIndex < Sector->ResourcesToUnload.Num(); ResourceIndex++)
	{
		if(Sector->ResourcesToUnload[ResourceIndex].ResourceIdentifier == Resource->Identifier)
		{
			Sector->ResourcesToUnload[ResourceIndex].Quantity = LimitQuantity;
			return;
		}
	}

	// Not found
	FFlareCargoSave ResourceOrder;
	ResourceOrder.ResourceIdentifier = Resource->Identifier;
	ResourceOrder.Quantity = LimitQuantity;
	Sector->ResourcesToUnload.Add(ResourceOrder);
}

void UFlareTradeRoute::ClearSectorUnloadOrder(int32 SectorIndex, FFlareResourceDescription* Resource)
{
	if(SectorIndex >= TradeRouteData.Sectors.Num())
	{
		FLOGV("Fail to configure trade route '%s', sector %d: only %d sector present.", *GetTradeRouteName().ToString(), SectorIndex, TradeRouteData.Sectors.Num());
		return;
	}

	FFlareTradeRouteSectorSave* Sector = &TradeRouteData.Sectors[SectorIndex];

	for(int ResourceIndex = 0; ResourceIndex < Sector->ResourcesToUnload.Num(); ResourceIndex++)
	{
		if(Sector->ResourcesToUnload[ResourceIndex].ResourceIdentifier == Resource->Identifier)
		{
			Sector->ResourcesToUnload.RemoveAt(ResourceIndex);
			break;
		}
	}
}

/** Remove all ship from the trade route and delete it.*/
void UFlareTradeRoute::Dissolve()
{
	for(int32 FleetIndex = 0; FleetIndex < TradeRouteFleets.Num() ; FleetIndex++)
	{
		UFlareFleet* Fleet = TradeRouteFleets[FleetIndex];
		Fleet->SetCurrentTradeRoute(NULL);
	}

	TradeRouteCompany->RemoveTradeRoute(this);
}

void UFlareTradeRoute::InitFleetList()
{
	if (!IsFleetListLoaded)
	{
		IsFleetListLoaded = true;
		TradeRouteFleets.Empty();
		for (int FleetIndex = 0; FleetIndex < TradeRouteData.FleetIdentifiers.Num(); FleetIndex++)
		{
			UFlareFleet* Fleet= TradeRouteCompany->FindFleet(TradeRouteData.FleetIdentifiers[FleetIndex]);
			if (Fleet)
			{
				Fleet->SetCurrentTradeRoute(this);
				TradeRouteFleets.Add(Fleet);
			}
			else
			{
				FLOGV("WARNING: Fail to find fleet '%s'. Save corrupted", *TradeRouteData.FleetIdentifiers[FleetIndex].ToString());
				TradeRouteData.FleetIdentifiers.Empty();
				break;
			}
		}
	}
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

TArray<UFlareFleet*>& UFlareTradeRoute::GetFleets()
{
	InitFleetList();

	return TradeRouteFleets;
}

FFlareTradeRouteSectorSave* UFlareTradeRoute::GetSectorOrders(UFlareSimulatedSector* Sector)
{
	for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
	{
		if(TradeRouteData.Sectors[SectorIndex].SectorIdentifier == Sector->GetIdentifier())
		{
			return &TradeRouteData.Sectors[SectorIndex];
		}
	}

	return NULL;
}

UFlareSimulatedSector* UFlareTradeRoute::GetNextTradeSector(UFlareSimulatedSector* Sector)
{
	int32 NextSectorId = -1;
	for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
	{
		if(TradeRouteData.Sectors[SectorIndex].SectorIdentifier == Sector->GetIdentifier())
		{
			NextSectorId = SectorIndex + 1;
			break;
		}
	}

	if(NextSectorId < 0)
	{
        NextSectorId = 0;
	}

	if(NextSectorId >= TradeRouteData.Sectors.Num())
	{
		NextSectorId = 0;
	}

	return Game->GetGameWorld()->FindSector(TradeRouteData.Sectors[NextSectorId].SectorIdentifier);
}


bool UFlareTradeRoute::IsVisiting(UFlareSimulatedSector *Sector)
{
    for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
    {
        if(TradeRouteData.Sectors[SectorIndex].SectorIdentifier == Sector->GetIdentifier())
        {
            return true;
        }
    }
    return false;
}

int32 UFlareTradeRoute::GetSectorIndex(UFlareSimulatedSector *Sector)
{
    for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
    {
        if(TradeRouteData.Sectors[SectorIndex].SectorIdentifier == Sector->GetIdentifier())
        {
            return SectorIndex;
        }
    }
    return -1;
}
