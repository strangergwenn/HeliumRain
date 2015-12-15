
#include "../Flare.h"
#include "FlareTradeRoute.h"
#include "FlareCompany.h"
#include "FlareSimulatedSector.h"

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
	IsShipListLoaded = false;
}

FFlareTradeRouteSave* UFlareTradeRoute::Save()
{
	return &TradeRouteData;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareTradeRoute::AddShip(UFlareSimulatedSpacecraft* Ship)
{
	UFlareTradeRoute* OldTradeRoute = Ship->GetCurrentTradeRoute();
	if (OldTradeRoute)
	{
		OldTradeRoute->RemoveShip(Ship);
	}

	TradeRouteData.ShipImmatriculations.Add(Ship->GetImmatriculation());
	TradeRouteShips.AddUnique(Ship);
	Ship->SetCurrentTradeRoute(this);
}

void UFlareTradeRoute::RemoveShip(UFlareSimulatedSpacecraft* Ship)
{
	TradeRouteData.ShipImmatriculations.Remove(Ship->GetImmatriculation());
	TradeRouteShips.Remove(Ship);
	Ship->SetCurrentTradeRoute(NULL);
}

void UFlareTradeRoute::AddSector(UFlareSimulatedSector* Sector)
{
	FFlareTradeRouteSectorSave TradeRouteSector;
	TradeRouteSector.SectorIdentifier = Sector->GetIdentifier();

	TradeRouteData.Sectors.Add(TradeRouteSector);
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
	TradeRouteCompany->RemoveTradeRoute(this);
}

void UFlareTradeRoute::InitShipList()
{
	if (!IsShipListLoaded)
	{
		IsShipListLoaded = true;
		TradeRouteShips.Empty();
		for (int ShipIndex = 0; ShipIndex < TradeRouteData.ShipImmatriculations.Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = TradeRouteCompany->FindSpacecraft(TradeRouteData.ShipImmatriculations[ShipIndex]);
			Ship->SetCurrentTradeRoute(this);
			TradeRouteShips.Add(Ship);
		}
	}
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

TArray<UFlareSimulatedSpacecraft*>& UFlareTradeRoute::GetShips()
{
	InitShipList();

	return TradeRouteShips;
}

