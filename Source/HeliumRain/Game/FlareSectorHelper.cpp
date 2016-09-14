#include "../Flare.h"
#include "FlareSectorHelper.h"

#include "FlareCompany.h"
#include "../Game/FlareGame.h"
#include "../Economy/FlareCargoBay.h"
#include "../Player/FlarePlayerController.h"

UFlareSimulatedSpacecraft*  SectorHelper::FindTradeStation(FlareTradeRequest Request)
{
	if(!Request.Client || !Request.Client->GetCurrentSector())
	{
		FLOG("Invalid find trade query");
		return NULL;
	}

	UFlareSimulatedSector* Sector = Request.Client->GetCurrentSector();
	TArray<UFlareSimulatedSpacecraft*>& SectorStations = Sector->GetSectorStations();

	float UnloadQuantityScoreMultiplier = 0;
	float LoadQuantityScoreMultiplier = 0;
	float SellQuantityScoreMultiplier = 0;
	float BuyQuantityScoreMultiplier = 0;
	float FullRatioBonus = 0;
	float EmptyRatioBonus = 0;
	bool  NeedInput = false;
	bool  NeedOutput = false;

	switch(Request.Operation)
	{
		case EFlareTradeRouteOperation::Buy:
			BuyQuantityScoreMultiplier = 10.f;
			FullRatioBonus = 0.1;
			NeedOutput = true;
		break;
		case EFlareTradeRouteOperation::Sell:
			SellQuantityScoreMultiplier = 10.f;
			EmptyRatioBonus = 0.1;
			NeedInput = true;
		break;
		case EFlareTradeRouteOperation::Load:
			LoadQuantityScoreMultiplier = 10.f;
			FullRatioBonus = 0.1;
			NeedOutput = true;
		break;
		case EFlareTradeRouteOperation::Unload:
			UnloadQuantityScoreMultiplier = 10.f;
			EmptyRatioBonus = 0.1;
			NeedInput = true;
		break;
		case EFlareTradeRouteOperation::LoadOrBuy:
			LoadQuantityScoreMultiplier = 10.f;
			BuyQuantityScoreMultiplier = 1.f;
			FullRatioBonus = 0.1;
			NeedOutput = true;
		break;
		case EFlareTradeRouteOperation::UnloadOrSell:
			UnloadQuantityScoreMultiplier = 10.f;
			SellQuantityScoreMultiplier = 1.f;
			EmptyRatioBonus = 0.1;
			NeedInput = true;
		break;
	}

	float BestScore = 0;
	UFlareSimulatedSpacecraft* BestStation = NULL;
	uint32 AvailableQuantity = Request.Client->GetCargoBay()->GetResourceQuantity(Request.Resource, Request.Client->GetCompany());
	uint32 FreeSpace = Request.Client->GetCargoBay()->GetFreeSpaceForResource(Request.Resource, Request.Client->GetCompany());

	for (int32 StationIndex = 0; StationIndex < SectorStations.Num(); StationIndex++)
	{
		UFlareSimulatedSpacecraft* Station = SectorStations[StationIndex];

		if(!Request.Client->CanTradeWith(Station))
		{
			continue;
		}
		EFlareResourcePriceContext::Type StationResourceUsage = Station->GetResourceUseType(Request.Resource);

		if(NeedOutput && StationResourceUsage != EFlareResourcePriceContext::FactoryOutput)
		{
			continue;
		}

		if(NeedInput && (StationResourceUsage != EFlareResourcePriceContext::FactoryInput &&
						 StationResourceUsage != EFlareResourcePriceContext::ConsumerConsumption &&
						 StationResourceUsage != EFlareResourcePriceContext::MaintenanceConsumption))
		{
			continue;
		}

		uint32 StationFreeSpace = Station->GetCargoBay()->GetFreeSpaceForResource(Request.Resource, Request.Client->GetCompany());
		uint32 StationResourceQuantity = Station->GetCargoBay()->GetResourceQuantity(Request.Resource, Request.Client->GetCompany());

		if (StationFreeSpace == 0 && StationResourceQuantity == 0)
		{
			continue;
		}

		float Score = 0;
		float FullRatio =  (float) StationResourceQuantity / (float) (StationResourceQuantity + StationFreeSpace);
		float EmptyRatio = 1 - FullRatio;
		uint32 UnloadMaxQuantity  = 0;
		uint32 LoadMaxQuantity  = 0;


		// Check cargo limit
		if(NeedOutput && Request.CargoLimit != -1 && FullRatio < Request.CargoLimit)
		{
			continue;
		}

		if(NeedInput && Request.CargoLimit != -1 && FullRatio > Request.CargoLimit)
		{
			continue;
		}

		if(Station->GetCargoBay()->WantBuy(Request.Resource, Request.Client->GetCompany()))
		{
			UnloadMaxQuantity = StationFreeSpace;
			UnloadMaxQuantity  = FMath::Min(UnloadMaxQuantity , AvailableQuantity);
		}

		if(Station->GetCargoBay()->WantSell(Request.Resource, Request.Client->GetCompany()))
		{
			LoadMaxQuantity = StationResourceQuantity;
			LoadMaxQuantity = FMath::Min(LoadMaxQuantity , FreeSpace);
		}

		if(Station->GetCompany() == Request.Client->GetCompany())
		{
			Score += UnloadMaxQuantity * UnloadQuantityScoreMultiplier;
			Score += LoadMaxQuantity * LoadQuantityScoreMultiplier;
		}
		else
		{
			EFlareResourcePriceContext::Type ResourceUsage = Station->GetResourceUseType(Request.Resource);

			uint32 MaxBuyableQuantity = Request.Client->GetCompany()->GetMoney() / Sector->GetResourcePrice(Request.Resource, ResourceUsage);
			LoadMaxQuantity = FMath::Min(LoadMaxQuantity , MaxBuyableQuantity);

			uint32 MaxSellableQuantity = Station->GetCompany()->GetMoney() / Sector->GetResourcePrice(Request.Resource, ResourceUsage);
			UnloadMaxQuantity = FMath::Min(UnloadMaxQuantity , MaxSellableQuantity);

			Score += UnloadMaxQuantity * SellQuantityScoreMultiplier;
			Score += LoadMaxQuantity * BuyQuantityScoreMultiplier;
		}

		Score *= 1 + (FullRatio * FullRatioBonus) + (EmptyRatio * EmptyRatioBonus);

		if(Score > 0 && Score > BestScore)
		{
			BestScore = Score;
			BestStation = Station;
		}
	}

	return BestStation;
}

int32 SectorHelper::Trade(UFlareSimulatedSpacecraft*  SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource, int32 MaxQuantity)
{
	if(!SourceSpacecraft->CanTradeWith(DestinationSpacecraft))
	{
		FLOG("Both spacecraft cannot trade");
		return 0;
	}

	int32 ResourcePrice = SourceSpacecraft->GetCurrentSector()->GetTransfertResourcePrice(SourceSpacecraft, DestinationSpacecraft, Resource);
	int32 QuantityToTake = MaxQuantity;

	if (SourceSpacecraft->GetCompany() != DestinationSpacecraft->GetCompany())
	{
		// Limit transaction bay available money
		int32 MaxAffordableQuantity = DestinationSpacecraft->GetCompany()->GetMoney() / ResourcePrice;
		QuantityToTake = FMath::Min(QuantityToTake, MaxAffordableQuantity);
	}
	int32 ResourceCapacity = DestinationSpacecraft->GetCargoBay()->GetFreeSpaceForResource(Resource, SourceSpacecraft->GetCompany());

	QuantityToTake = FMath::Min(QuantityToTake, ResourceCapacity);
	
	int32 TakenResources = SourceSpacecraft->GetCargoBay()->TakeResources(Resource, QuantityToTake, DestinationSpacecraft->GetCompany());
	int32 GivenResources = DestinationSpacecraft->GetCargoBay()->GiveResources(Resource, TakenResources, SourceSpacecraft->GetCompany());

	// Pay
	if (GivenResources > 0 && SourceSpacecraft->GetCompany() != DestinationSpacecraft->GetCompany())
	{
		int64 Price = ResourcePrice * GivenResources;
		DestinationSpacecraft->GetCompany()->TakeMoney(Price);
		SourceSpacecraft->GetCompany()->GiveMoney(Price);

		SourceSpacecraft->GetCompany()->GiveReputation(DestinationSpacecraft->GetCompany(), 0.5f, true);
		DestinationSpacecraft->GetCompany()->GiveReputation(SourceSpacecraft->GetCompany(), 0.5f, true);
	}
	
	// Set the trading state if not player fleet
	if (GivenResources > 0)
	{
		AFlarePlayerController* PC = SourceSpacecraft->GetGame()->GetPC();
		UFlareFleet* PlayerFleet = PC->GetPlayerFleet();
		FCHECK(PC);

		if (SourceSpacecraft->GetCurrentFleet() != PlayerFleet && !SourceSpacecraft->IsStation())
		{
			SourceSpacecraft->SetTrading(true);
		}

		if (DestinationSpacecraft->GetCurrentFleet() != PlayerFleet && !DestinationSpacecraft->IsStation())
		{
			DestinationSpacecraft->SetTrading(true);
		}
	}

	return GivenResources;

}
