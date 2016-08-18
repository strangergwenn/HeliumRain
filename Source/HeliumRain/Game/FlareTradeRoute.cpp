
#include "../Flare.h"
#include "FlareTradeRoute.h"
#include "FlareCompany.h"
#include "FlareSimulatedSector.h"
#include "FlareFleet.h"
#include "../Economy/FlareCargoBay.h"
#include "FlareGame.h"
#include "FlareSectorHelper.h"

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

	UpdateTargetSector();

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
	FLOG("Trade route simulate");

	if(TradeRouteData.IsPaused)
	{
		// Trade route paused. To nothing
		return;
	}

	if (TradeRouteData.Sectors.Num() == 0 || TradeRouteFleet == NULL)
    {
		FLOG("  -> no sector or assigned fleet");
        // Nothing to do
        return;
    }

	if (TradeRouteFleet->IsTraveling())
	{
		FLOG("  -> is travelling");
		return;
	}

	UFlareSimulatedSector* TargetSector = UpdateTargetSector();

	// Not travelling, check if the fleet is in a trade route sector
	UFlareSimulatedSector* CurrentSector = TradeRouteFleet->GetCurrentSector();


	if (TargetSector == CurrentSector)
	{
		// In the target sector
		FFlareTradeRouteSectorSave* SectorOrder = GetSectorOrders(CurrentSector);

		while (TradeRouteData.CurrentOperationIndex < SectorOrder->Operations.Num())
		{
			if (ProcessCurrentOperation(&SectorOrder->Operations[TradeRouteData.CurrentOperationIndex]))
			{
				// Operation finish
				TradeRouteData.CurrentOperationDuration = 0;
				TradeRouteData.CurrentOperationProgress = 0;
				TradeRouteData.CurrentOperationIndex++;
			}
			else
			{
				TradeRouteData.CurrentOperationDuration++;
				break;
			}
		}

		if (TradeRouteData.CurrentOperationIndex >= SectorOrder->Operations.Num())
		{
			// Sector operations finished
			TargetSector = GetNextTradeSector(TargetSector);
			SetTargetSector(TargetSector);
		}
	}


	if (TargetSector && TargetSector != CurrentSector)
	{
		FLOGV("  start travel to %s", *TargetSector->GetSectorName().ToString());
		// Travel to next sector
		Game->GetGameWorld()->StartTravel(TradeRouteFleet, TargetSector);
	}
}

UFlareSimulatedSector* UFlareTradeRoute::UpdateTargetSector()
{
	UFlareSimulatedSector* TargetSector = Game->GetGameWorld()->FindSector(TradeRouteData.TargetSectorIdentifier);
	if (!TargetSector || GetSectorOrders(TargetSector) == NULL)
	{
		TargetSector = GetNextTradeSector(NULL);
		if(TargetSector)
		{
			FLOGV("Has TargetSector %s", *TargetSector->GetIdentifier().ToString());
		}
		else
		{
			FLOG("Has no TargetSector");
		}
		SetTargetSector(TargetSector);
	}

	return TargetSector;
}

bool UFlareTradeRoute::ProcessCurrentOperation(FFlareTradeRouteSectorOperationSave* Operation)
{
	if (Operation->MaxWait != -1 && TradeRouteData.CurrentOperationDuration >= Operation->MaxWait)
	{
		FLOGV("Max wait duration reach (%d)", Operation->MaxWait);
		return true;
	}


	// Return true if : limit reach or all ship full/empty or buy and no money or sell and nobody has money

	switch (Operation->Type) {
	case EFlareTradeRouteOperation::Buy:
	case EFlareTradeRouteOperation::Load:
			return ProcessLoadOperation(Operation);
		break;
	case EFlareTradeRouteOperation::Sell:
	case EFlareTradeRouteOperation::Unload:
			return ProcessUnloadOperation(Operation);
		break;
	default:
		FLOGV("ERROR: Unknown trade route operation (%d)", (Operation->Type + 0));
		break;
	}
	return true;
}

bool UFlareTradeRoute::ProcessLoadOperation(FFlareTradeRouteSectorOperationSave* Operation)
{

	FFlareResourceDescription* Resource = Game->GetResourceCatalog()->Get(Operation->ResourceIdentifier);

	TArray<UFlareSimulatedSpacecraft*> UsefullShips;

	TArray<UFlareSimulatedSpacecraft*>&  RouteShips = TradeRouteFleet->GetShips();
	int32 FleetFreeSpace = 0;
	//
	for (int ShipIndex = 0; ShipIndex < RouteShips.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = RouteShips[ShipIndex];

		// Keep trading ship as they will be usefull later
		int32 FreeSpace = Ship->GetCargoBay()->GetFreeSpaceForResource(Resource);
		if (FreeSpace > 0)
		{
			FleetFreeSpace += FreeSpace;
			UsefullShips.Add(Ship);
		}

		// TODO sort by most pertinent
	}

	if (FleetFreeSpace == 0)
	{
		// Fleet full: operation done
		return true;
	}

	SectorHelper::FlareTradeRequest Request;
	Request.Resource = Resource;
	Request.Operation = Operation->Type;

	for (int ShipIndex = 0; ShipIndex < UsefullShips.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = RouteShips[ShipIndex];

		if (Ship->IsTrading())
		{
			// Skip trading ships
			continue;
		}

		Request.Client = Ship;
		Request.MaxQuantity = Ship->GetCargoBay()->GetFreeSpaceForResource(Resource);
		if (Operation->MaxQuantity !=-1)
		{
			Request.MaxQuantity = FMath::Min(Request.MaxQuantity, GetOperationRemainingQuantity(Operation));
		}

		UFlareSimulatedSpacecraft* StationCandidate = SectorHelper::FindTradeStation(Request);


		if (StationCandidate)
		{
			TradeRouteData.CurrentOperationProgress += SectorHelper::Trade(StationCandidate, Ship, Resource, Request.MaxQuantity);
		}

		if (IsOperationQuantityLimitReach(Operation))
		{
			// Operation limit reach : operation done
			return true;
		}
	}

	// Limit not reach and useful ship present. Operation not finished
	return false;
}

bool UFlareTradeRoute::ProcessUnloadOperation(FFlareTradeRouteSectorOperationSave* Operation)
{

	FFlareResourceDescription* Resource = Game->GetResourceCatalog()->Get(Operation->ResourceIdentifier);

	TArray<UFlareSimulatedSpacecraft*> UsefullShips;

	TArray<UFlareSimulatedSpacecraft*>&  RouteShips = TradeRouteFleet->GetShips();
	int32 FleetQuantity = 0;

	for (int ShipIndex = 0; ShipIndex < RouteShips.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = RouteShips[ShipIndex];

		// Keep trading ship as they will be usefull later
		int32 Quantity = Ship->GetCargoBay()->GetResourceQuantity(Resource);
		if (Quantity > 0)
		{
			FleetQuantity += Quantity;
			UsefullShips.Add(Ship);
		}

		// TODO sort by most pertinent
	}

	if (FleetQuantity == 0)
	{
		// Fleet empty: operation done
		return true;
	}

	SectorHelper::FlareTradeRequest Request;
	Request.Resource = Resource;
	Request.Operation = Operation->Type;

	for (int ShipIndex = 0; ShipIndex < UsefullShips.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = RouteShips[ShipIndex];

		if (Ship->IsTrading())
		{
			// Skip trading ships
			continue;
		}

		Request.Client = Ship;
		Request.MaxQuantity = Ship->GetCargoBay()->GetResourceQuantity(Resource);
		if (Operation->MaxQuantity !=-1)
		{
			Request.MaxQuantity = FMath::Min(Request.MaxQuantity, GetOperationRemainingQuantity(Operation));
		}

		UFlareSimulatedSpacecraft* StationCandidate = SectorHelper::FindTradeStation(Request);

		if (StationCandidate)
		{
			TradeRouteData.CurrentOperationProgress += SectorHelper::Trade(Ship, StationCandidate, Resource, Request.MaxQuantity);
		}

		if (IsOperationQuantityLimitReach(Operation))
		{
			// Operation limit reach : operation done
			return true;
		}
	}

	// Limit not reach and useful ship present. Operation not finished
	return false;
}

int32 UFlareTradeRoute::GetOperationRemainingQuantity(FFlareTradeRouteSectorOperationSave* Operation)
{
	if (Operation->MaxQuantity != -1)
	{
		return MAX_int32;
	}

	return Operation->MaxQuantity - TradeRouteData.CurrentOperationProgress;
}

bool UFlareTradeRoute::IsOperationQuantityLimitReach(FFlareTradeRouteSectorOperationSave* Operation)
{
	if (Operation->MaxQuantity != -1 && TradeRouteData.CurrentOperationProgress >= Operation->MaxQuantity)
	{
		return true;
	}
	return false;
}

void UFlareTradeRoute::SetTargetSector(UFlareSimulatedSector* Sector)
{
	if (Sector)
	{
		TradeRouteData.TargetSectorIdentifier = Sector->GetIdentifier();
	}
	else
	{
		TradeRouteData.TargetSectorIdentifier = NAME_None;
	}
	TradeRouteData.CurrentOperationDuration = 0;
	TradeRouteData.CurrentOperationIndex = 0;
	TradeRouteData.CurrentOperationProgress = 0;

}


void UFlareTradeRoute::AssignFleet(UFlareFleet* Fleet)
{
	UFlareTradeRoute* OldTradeRoute = Fleet->GetCurrentTradeRoute();
	if (OldTradeRoute)
	{
		OldTradeRoute->RemoveFleet(Fleet);
	}

	TradeRouteData.FleetIdentifier = Fleet->GetIdentifier();
	TradeRouteFleet = Fleet;
	Fleet->SetCurrentTradeRoute(this);
}

void UFlareTradeRoute::RemoveFleet(UFlareFleet* Fleet)
{
	TradeRouteData.FleetIdentifier = NAME_None;
	TradeRouteFleet = NULL;
	Fleet->SetCurrentTradeRoute(NULL);
}

void UFlareTradeRoute::AddSector(UFlareSimulatedSector* Sector)
{
    if (IsVisiting(Sector))
    {
        FLOG("Warning: try to add a sector already visited by the trade route");
        return;
    }
	FFlareTradeRouteSectorSave TradeRouteSector;
	TradeRouteSector.SectorIdentifier = Sector->GetIdentifier();

	TradeRouteData.Sectors.Add(TradeRouteSector);
	if(TradeRouteData.Sectors.Num() == 1)
	{
		SetTargetSector(Sector);
	}
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

void UFlareTradeRoute::AddSectorOperation(int32 SectorIndex, EFlareTradeRouteOperation::Type Type, FFlareResourceDescription* Resource)
{
	if (SectorIndex >= TradeRouteData.Sectors.Num())
	{
		FLOGV("Fail to configure trade route '%s', sector %d: only %d sector present.", *GetTradeRouteName().ToString(), SectorIndex, TradeRouteData.Sectors.Num());
		return;
	}

	FFlareTradeRouteSectorSave* Sector = &TradeRouteData.Sectors[SectorIndex];

	FFlareTradeRouteSectorOperationSave Operation;
	Operation.Type = Type;
	Operation.ResourceIdentifier = Resource->Identifier;
	Operation.MaxQuantity = -1;
	Operation.MaxWait = -1;

	Sector->Operations.Add(Operation);
}

void UFlareTradeRoute::RemoveSectorOperation(int32 SectorIndex, int32 OperationIndex)
{
	if (SectorIndex >= TradeRouteData.Sectors.Num())
	{
		FLOGV("Fail to configure trade route '%s', sector %d: only %d sector present.", *GetTradeRouteName().ToString(), SectorIndex, TradeRouteData.Sectors.Num());
		return;
	}

	FFlareTradeRouteSectorSave* Sector = &TradeRouteData.Sectors[SectorIndex];

	Sector->Operations.RemoveAt(OperationIndex);
}

void UFlareTradeRoute::DeleteOperation(FFlareTradeRouteSectorOperationSave* Operation)
{
	for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
	{
		FFlareTradeRouteSectorSave* Sector = &TradeRouteData.Sectors[SectorIndex];

		for (int32 OperationIndex = 0; OperationIndex < Sector->Operations.Num(); OperationIndex++)
		{
			if(&Sector->Operations[OperationIndex] == Operation)
			{
				if(Operation == GetActiveOperation())
				{
					// Active operation, reset progress
					TradeRouteData.CurrentOperationDuration = 0;
					TradeRouteData.CurrentOperationProgress = 0;
				}
				else if(TradeRouteData.TargetSectorIdentifier == Sector->SectorIdentifier && TradeRouteData.CurrentOperationIndex > OperationIndex)
				{
					// Modify the current operation index
					TradeRouteData.CurrentOperationIndex--;
				}
				Sector->Operations.RemoveAt(OperationIndex);
				return;
			}
		}
	}
}

FFlareTradeRouteSectorOperationSave* UFlareTradeRoute::MoveOperationUp(FFlareTradeRouteSectorOperationSave* Operation)
{
	for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
	{
		FFlareTradeRouteSectorSave* Sector = &TradeRouteData.Sectors[SectorIndex];

		for (int32 OperationIndex = 0; OperationIndex < Sector->Operations.Num(); OperationIndex++)
		{
			if(&Sector->Operations[OperationIndex] == Operation)
			{
				if(OperationIndex == 0)
				{
					//Already on top
					return Operation;
				}

				if(Operation == GetActiveOperation())
				{
					// Active operation, modify the current operation index
					TradeRouteData.CurrentOperationIndex--;
				}
				else if(TradeRouteData.TargetSectorIdentifier == Sector->SectorIdentifier && TradeRouteData.CurrentOperationIndex == OperationIndex-1)
				{
					TradeRouteData.CurrentOperationIndex++;
				}

				FFlareTradeRouteSectorOperationSave NewOperation = *Operation;

				Sector->Operations.RemoveAt(OperationIndex);
				Sector->Operations.Insert(NewOperation, OperationIndex-1);
				return &Sector->Operations[OperationIndex-1];
			}
		}
	}

	return Operation;
}

FFlareTradeRouteSectorOperationSave* UFlareTradeRoute::MoveOperationDown(FFlareTradeRouteSectorOperationSave* Operation)
{
	for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
	{
		FFlareTradeRouteSectorSave* Sector = &TradeRouteData.Sectors[SectorIndex];

		for (int32 OperationIndex = 0; OperationIndex < Sector->Operations.Num(); OperationIndex++)
		{
			if(&Sector->Operations[OperationIndex] == Operation)
			{
				if(OperationIndex == Sector->Operations.Num()-1)
				{
					//Already on bottom
					return Operation;
				}

				if(Operation == GetActiveOperation())
				{
					// Active operation, modify the current operation index
					TradeRouteData.CurrentOperationIndex++;
				}
				else if(TradeRouteData.TargetSectorIdentifier == Sector->SectorIdentifier && TradeRouteData.CurrentOperationIndex == OperationIndex+1)
				{
					TradeRouteData.CurrentOperationIndex--;
				}

				FFlareTradeRouteSectorOperationSave NewOperation = *Operation;

				Sector->Operations.RemoveAt(OperationIndex);
				Sector->Operations.Insert(NewOperation, OperationIndex+1);
				return &Sector->Operations[OperationIndex+1];
			}
		}
	}

	return Operation;
}

/** Remove all ship from the trade route and delete it.*/
void UFlareTradeRoute::Dissolve()
{
	if (TradeRouteFleet)
	{
		TradeRouteFleet->SetCurrentTradeRoute(NULL);
	}

	TradeRouteCompany->RemoveTradeRoute(this);
}

void UFlareTradeRoute::InitFleetList()
{
	if (!IsFleetListLoaded)
	{
		IsFleetListLoaded = true;
		TradeRouteFleet = NULL;

		if (TradeRouteData.FleetIdentifier != NAME_None)
		{
			TradeRouteFleet = TradeRouteCompany->FindFleet(TradeRouteData.FleetIdentifier);
			if (TradeRouteFleet)
			{
				TradeRouteFleet->SetCurrentTradeRoute(this);
			}
			else
			{
				FLOGV("WARNING: Fail to find fleet '%s'. Save corrupted", *TradeRouteData.FleetIdentifier.ToString());
				TradeRouteData.FleetIdentifier = NAME_None;
			}
		}
	}
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

UFlareFleet* UFlareTradeRoute::GetFleet()
{
	InitFleetList();

	return TradeRouteFleet;
}

FFlareTradeRouteSectorSave* UFlareTradeRoute::GetSectorOrders(UFlareSimulatedSector* Sector)
{
	for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
	{
		if (TradeRouteData.Sectors[SectorIndex].SectorIdentifier == Sector->GetIdentifier())
		{
			return &TradeRouteData.Sectors[SectorIndex];
		}
	}

	return NULL;
}

UFlareSimulatedSector* UFlareTradeRoute::GetNextTradeSector(UFlareSimulatedSector* Sector)
{
	if(TradeRouteData.Sectors.Num() == 0)
	{
		return NULL;
	}

	int32 NextSectorId = -1;
	if(Sector)
	{
		for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
		{
			if(TradeRouteData.Sectors[SectorIndex].SectorIdentifier == Sector->GetIdentifier())
			{
				NextSectorId = SectorIndex + 1;
				break;
			}
		}
	}

	if (NextSectorId < 0)
	{
        NextSectorId = 0;
	}


	if (NextSectorId >= TradeRouteData.Sectors.Num())
	{
		NextSectorId = 0;
	}
	return Game->GetGameWorld()->FindSector(TradeRouteData.Sectors[NextSectorId].SectorIdentifier);
}


bool UFlareTradeRoute::IsVisiting(UFlareSimulatedSector *Sector)
{
    for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
    {
        if (TradeRouteData.Sectors[SectorIndex].SectorIdentifier == Sector->GetIdentifier())
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
        if (TradeRouteData.Sectors[SectorIndex].SectorIdentifier == Sector->GetIdentifier())
        {
            return SectorIndex;
        }
    }
    return -1;
}

UFlareSimulatedSector* UFlareTradeRoute::GetTargetSector() const
{
	return Game->GetGameWorld()->FindSector(TradeRouteData.TargetSectorIdentifier);
}

FFlareTradeRouteSectorOperationSave* UFlareTradeRoute::GetActiveOperation()
{
	UFlareSimulatedSector* TargetSector = Game->GetGameWorld()->FindSector(TradeRouteData.TargetSectorIdentifier);

	if(TargetSector == NULL)
	{
		return NULL;
	}

	FFlareTradeRouteSectorSave* SectorOrder = GetSectorOrders(TargetSector);

	if(SectorOrder == NULL)
	{
		return NULL;
	}

	if(SectorOrder->Operations.Num() == 0)
	{
		return NULL;
	}

	if(TradeRouteData.CurrentOperationIndex >= SectorOrder->Operations.Num())
	{
		return NULL;
	}

	return &SectorOrder->Operations[TradeRouteData.CurrentOperationIndex];
}

void UFlareTradeRoute::SkipCurrentOperation()
{
	UFlareSimulatedSector* TargetSector = UpdateTargetSector();
	TradeRouteData.CurrentOperationDuration = 0;
	TradeRouteData.CurrentOperationProgress = 0;
	TradeRouteData.CurrentOperationIndex++;

	FFlareTradeRouteSectorSave* SectorOrder = GetSectorOrders(TargetSector);

	if (SectorOrder && TradeRouteData.CurrentOperationIndex >= SectorOrder->Operations.Num())
	{
		TargetSector = GetNextTradeSector(TargetSector);
		SetTargetSector(TargetSector);
	}
}
