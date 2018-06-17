#include "FlareAITradeHelper.h"

#include "../../Spacecrafts/FlareSimulatedSpacecraft.h"
#include "../FlareCompany.h"
#include "../FlareSectorHelper.h"
#include "../../Economy/FlareCargoBay.h"
#include "FlareAIBehavior.h"


DECLARE_CYCLE_STAT(TEXT("AITradeHelper FindBestDealForShipFromSector"), STAT_AITradeHelper_FindBestDealForShipFromSector, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("AITradeHelper FindBestDealForShip"), STAT_AITradeHelper_FindBestDealForShip, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("AITradeHelper FindBestDealForShip Sectors"), STAT_AITradeHelper_FindBestDealForShip_Sectors, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("AITradeHelper FindBestDealForShip Loop"), STAT_AITradeHelper_FindBestDealForShip_Loop, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("AITradeHelper ApplyDeal"), STAT_AITradeHelper_ApplyDeal, STATGROUP_Flare);


TArray<UFlareSimulatedSpacecraft*> AITradeHelper::FindIdleCargos(UFlareCompany* Company)
{
	TArray<UFlareSimulatedSpacecraft*> IdleCargos;

	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];


		for (int32 ShipIndex = 0 ; ShipIndex < Sector->GetSectorShips().Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = Sector->GetSectorShips()[ShipIndex];
			if (Ship->GetCompany() != Company || Ship->GetDamageSystem()->IsStranded() || Ship->IsTrading() || (Ship->GetCurrentFleet() && Ship->GetCurrentFleet()->IsTraveling()) || Ship->GetCurrentTradeRoute() != NULL || Ship->GetActiveCargoBay()->GetCapacity() == 0)
			{
				continue;
			}

			IdleCargos.Add(Ship);
		}
	}

	return IdleCargos;
}

SectorDeal AITradeHelper::FindBestDealForShipFromSector(UFlareSimulatedSpacecraft* Ship, UFlareSimulatedSector* SectorA, SectorDeal* DealToBeat, TMap<UFlareSimulatedSector*, SectorVariation> const& WorldResourceVariation)
{
	SCOPE_CYCLE_COUNTER(STAT_AITradeHelper_FindBestDealForShipFromSector);

	UFlareCompany* Company = Ship->GetCompany();
	AFlareGame* Game = Ship->GetGame();
	UFlareAIBehavior* Behavior = Company->GetAI()->GetBehavior();


	SectorDeal BestDeal;
	BestDeal.Resource = NULL;
	BestDeal.BuyQuantity = 0;
	BestDeal.Score = DealToBeat->Score;
	BestDeal.Resource = NULL;
	BestDeal.SectorA = NULL;
	BestDeal.SectorB = NULL;

	if (SectorA->GetSectorBattleState(Company).HasDanger)
	{
		return BestDeal;
	}

	for (int32 SectorBIndex = 0; SectorBIndex < Company->GetKnownSectors().Num(); SectorBIndex++)
	{
		UFlareSimulatedSector* SectorB = Company->GetKnownSectors()[SectorBIndex];

		int64 TravelTimeToA;
		int64 TravelTimeToB;

		if (SectorB->GetSectorBattleState(Company).HasDanger)
		{
			return BestDeal;
		}

		if (Ship->GetCurrentSector() == SectorA)
		{
			TravelTimeToA = 0;
		}
		else
		{
			TravelTimeToA = UFlareTravel::ComputeTravelDuration(Game->GetGameWorld(), Ship->GetCurrentSector(), SectorA, Company);
		}

		if (SectorA == SectorB)
		{
			// Stay in sector option
			TravelTimeToB = 0;
		}
		else
		{
			// Travel time

			TravelTimeToB = UFlareTravel::ComputeTravelDuration(Game->GetGameWorld(), SectorA, SectorB, Company);

		}
		int64 TravelTime = TravelTimeToA + TravelTimeToB;


#ifdef DEBUG_AI_TRADING
		/*if (SectorA->GetIdentifier() != "lighthouse" || SectorB->GetIdentifier() != "boneyard")
		{
			continue;
		}*/

		if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY
				&& SectorB->GetIdentifier() == DEBUG_AI_TRADING_SECTOR_B)
		{
			FLOGV("Travel %s -> %s -> %s : %lld days", *Ship->GetCurrentSector()->GetSectorName().ToString(),
			*SectorA->GetSectorName().ToString(), *SectorB->GetSectorName().ToString(), TravelTime);
		}
#endif

		SectorVariation const* SectorVariationA = &(WorldResourceVariation[SectorA]);
		SectorVariation const* SectorVariationB = &(WorldResourceVariation[SectorB]);

		for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
		{
			FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
			struct ResourceVariation const* VariationA = &SectorVariationA->ResourceVariations[Resource];
			struct ResourceVariation const* VariationB = &SectorVariationB->ResourceVariations[Resource];

#ifdef DEBUG_AI_TRADING
		if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY
				&& SectorB->GetIdentifier() == DEBUG_AI_TRADING_SECTOR_B
				&& Resource->Identifier == DEBUG_AI_TRADING_RESOURCES)
		{
			FLOGV("- Check for %s", *Resource->Name.ToString());
			FLOGV("!VariationA->OwnedFlow %d",VariationA->OwnedFlow);
			FLOGV("!VariationA->FactoryFlow %d",VariationA->FactoryFlow);
			FLOGV("!VariationA->OwnedStock %d",VariationA->OwnedStock);
			FLOGV("!VariationA->FactoryStock %d",VariationA->FactoryStock);
			FLOGV("!VariationA->StorageStock %d",VariationA->StorageStock);
			FLOGV("!VariationA->OwnedCapacity %d",VariationA->OwnedCapacity);
			FLOGV("!VariationA->FactoryCapacity %d",VariationA->FactoryCapacity);
			FLOGV("!VariationA->StorageCapacity %d",VariationA->StorageCapacity);
			FLOGV("!VariationA->MaintenanceCapacity %d",VariationA->MaintenanceCapacity);
			FLOGV("!VariationA->MinCapacity %d",VariationA->MinCapacity);
			FLOGV("!VariationA->IncomingResources %d",VariationA->IncomingResources);
			FLOGV("!VariationB->OwnedFlow %d",VariationB->OwnedFlow);
			FLOGV("!VariationB->FactoryFlow %d",VariationB->FactoryFlow);
			FLOGV("!VariationB->OwnedStock %d",VariationB->OwnedStock);
			FLOGV("!VariationB->FactoryStock %d",VariationB->FactoryStock);
			FLOGV("!VariationB->StorageStock %d",VariationB->StorageStock);
			FLOGV("!VariationB->OwnedCapacity %d",VariationB->OwnedCapacity);
			FLOGV("!VariationB->FactoryCapacity %d",VariationB->FactoryCapacity);
			FLOGV("!VariationB->StorageCapacity %d", VariationB->StorageCapacity);
			FLOGV("!VariationB->MaintenanceCapacity %d", VariationB->MaintenanceCapacity);
			FLOGV("!VariationB->MinCapacity %d", VariationB->MinCapacity);
			FLOGV("!VariationB->IncomingResources %d", VariationB->IncomingResources);
			FLOGV("  - VariationA %p", VariationA);
			FLOGV("  - VariationB %p", VariationB);
		}

		/*if (Resource->Identifier != "fuel")
		{
			continue;
		}*/
#endif


			if (!VariationA->OwnedFlow &&
				!VariationA->FactoryFlow &&
				!VariationA->OwnedStock &&
				!VariationA->FactoryStock &&
				!VariationA->StorageStock &&
				!VariationA->OwnedCapacity &&
				!VariationA->FactoryCapacity &&
				!VariationA->StorageCapacity &&
				!VariationA->MaintenanceCapacity &&
				!VariationA->MinCapacity &&
				!VariationB->OwnedFlow &&
				!VariationB->FactoryFlow &&
				!VariationB->OwnedStock &&
				!VariationB->FactoryStock &&
				!VariationB->StorageStock &&
				!VariationB->OwnedCapacity &&
				!VariationB->FactoryCapacity &&
				!VariationB->StorageCapacity &&
				!VariationB->MaintenanceCapacity &&
				!VariationB->MinCapacity)
			{
				continue;
			}


			int32 InitialQuantity = Ship->GetActiveCargoBay()->GetResourceQuantity(Resource, Ship->GetCompany());
			int32 FreeSpace = Ship->GetActiveCargoBay()->GetFreeSpaceForResource(Resource, Ship->GetCompany());

			int32 StockInAAfterTravel =
				VariationA->OwnedStock
				+ VariationA->FactoryStock
				+ VariationA->StorageStock
				- (VariationA->OwnedFlow * TravelTimeToA)
				- (VariationA->FactoryFlow * TravelTimeToA);
#ifdef DEBUG_AI_TRADING
			if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY
					&& SectorB->GetIdentifier() == DEBUG_AI_TRADING_SECTOR_B
					&& Resource->Identifier == DEBUG_AI_TRADING_RESOURCES)
			{
				FLOGV("InitialQuantity %d", InitialQuantity);
				FLOGV("StockInAAfterTravel %d", StockInAAfterTravel);
			}
#endif

			if (StockInAAfterTravel <= 0 && InitialQuantity == 0)
			{
				continue;
			}

			int32 CanBuyQuantity = FMath::Min(FreeSpace, StockInAAfterTravel);
			CanBuyQuantity = FMath::Max(0, CanBuyQuantity);

			// Affordable quantity
			CanBuyQuantity = FMath::Min(CanBuyQuantity, (int32)(Company->GetMoney() / SectorA->GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryInput)));

			int32 TimeToGetB = TravelTime + (CanBuyQuantity > 0 ? 1 : 0); // If full, will not buy so no trade time in A

			int32 LocalCapacity = VariationB->OwnedCapacity
					+ VariationB->FactoryCapacity
					+ VariationB->StorageCapacity
					+ VariationB->MaintenanceCapacity;

			if (VariationB->MinCapacity > 0)
			{
				// The nerf system make big capacity malus in whole sector if a big station is near full
				// If there is an empty small station in the sector, this station will not get any resource
				// as the sector will be avoided by trade ships
				LocalCapacity = FMath::Max(LocalCapacity, VariationB->MinCapacity);
			}

			int32 CapacityInBAfterTravel =
				LocalCapacity
				+ VariationB->OwnedFlow * TimeToGetB
				+ VariationB->FactoryFlow * TimeToGetB;
			if (TimeToGetB > 0)
			{
				CapacityInBAfterTravel -= VariationB->IncomingResources;
			}

			int32 SellQuantity = FMath::Min(CapacityInBAfterTravel, CanBuyQuantity + InitialQuantity);
			int32  BuyQuantity = FMath::Max(0, SellQuantity - InitialQuantity);

			// Use price details

			int32 MoneyGain = 0;
			int32 QuantityToSell = SellQuantity;

			int32 OwnedCapacity = FMath::Max(0, (int32)(VariationB->OwnedCapacity + VariationB->OwnedFlow * TravelTime));
			int32 MaintenanceCapacity = VariationB->MaintenanceCapacity;
			int32 FactoryCapacity = FMath::Max(0, (int32)(VariationB->FactoryCapacity + VariationB->FactoryFlow * TravelTime));
			int32 StorageCapacity = VariationB->StorageCapacity;

			if(OwnedCapacity + MaintenanceCapacity + FactoryCapacity + StorageCapacity < VariationB->MinCapacity)
			{
				FactoryCapacity += VariationB->MinCapacity;
			}

			int32 OwnedSellQuantity = FMath::Min(OwnedCapacity, QuantityToSell);
			MoneyGain += OwnedSellQuantity * SectorB->GetResourcePrice(Resource, EFlareResourcePriceContext::Default) * 2;
			QuantityToSell -= OwnedSellQuantity;

			int32 MaintenanceSellQuantity = FMath::Min(MaintenanceCapacity, QuantityToSell);
			MoneyGain += MaintenanceSellQuantity * SectorB->GetResourcePrice(Resource, EFlareResourcePriceContext::MaintenanceConsumption);
			QuantityToSell -= MaintenanceSellQuantity;

			int32 FactorySellQuantity = FMath::Min(FactoryCapacity, QuantityToSell);
			MoneyGain += FactorySellQuantity * SectorB->GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryInput);
			QuantityToSell -= FactorySellQuantity;

			int32 StorageSellQuantity = FMath::Min(StorageCapacity, QuantityToSell);
			MoneyGain += StorageSellQuantity * SectorB->GetResourcePrice(Resource, EFlareResourcePriceContext::Default);
			QuantityToSell -= StorageSellQuantity;

			int32 MoneySpend = 0;
			int32 QuantityToBuy = BuyQuantity;

			int32 OwnedStock = FMath::Max(0, (int32)(VariationA->OwnedStock - VariationA->OwnedFlow * TravelTimeToA));
			int32 FactoryStock = FMath::Max(0, (int32)(VariationA->FactoryStock - VariationA->FactoryFlow * TravelTimeToA));
			int32 StorageStock = VariationA->StorageStock;


			int32 OwnedBuyQuantity = FMath::Min(OwnedStock, QuantityToBuy);
			MoneySpend += OwnedBuyQuantity * SectorA->GetResourcePrice(Resource, EFlareResourcePriceContext::Default) * 0.5;
			QuantityToBuy -= OwnedBuyQuantity;

			int32 FactoryBuyQuantity = FMath::Min(FactoryStock, QuantityToBuy);
			MoneySpend += FactoryBuyQuantity * SectorA->GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryOutput);
			QuantityToBuy -= FactoryBuyQuantity;

			int32 StorageBuyQuantity = FMath::Min(StorageStock, QuantityToBuy);
			MoneySpend += StorageBuyQuantity * SectorA->GetResourcePrice(Resource, EFlareResourcePriceContext::Default);
			QuantityToBuy -= StorageBuyQuantity;


			// TODO per station computation
			// TODO prefer own transport

			int32 MoneyBalance = MoneyGain - MoneySpend;

			float MoneyBalanceParDay = (float)MoneyBalance / (float)(TimeToGetB + 1); // 1 day to sell

			bool Temporisation = false;
			if (BuyQuantity == 0 && Ship->GetCurrentSector() != SectorA)
			{
				// If can't buy in A and A is not local, it's just a temporisation route. Better to do nothing.
				// Accepting to be idle help to avoid building ships
				Temporisation = true;
			}

			MoneyBalanceParDay *= Behavior->GetResourceAffility(Resource);

			float Score = MoneyBalanceParDay
					* Behavior->GetResourceAffility(Resource)
					* (Behavior->GetSectorAffility(SectorA) + Behavior->GetSectorAffility(SectorB));

			if (VariationB->HighPriority > 0)
			{
				Score *= VariationB->HighPriority;
			}
#ifdef DEBUG_AI_TRADING
			if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY
					&& SectorB->GetIdentifier() == DEBUG_AI_TRADING_SECTOR_B
					&& Resource->Identifier == DEBUG_AI_TRADING_RESOURCES)
			{
				FLOGV(" -> IncomingCapacity=%d", SectorVariationA->IncomingCapacity);
				FLOGV(" -> IncomingResources=%d", VariationA->IncomingResources);
				FLOGV(" -> InitialQuantity=%d", InitialQuantity);
				FLOGV(" -> FreeSpace=%d", FreeSpace);
				FLOGV(" -> StockInAAfterTravel=%d", StockInAAfterTravel);
				FLOGV(" -> BuyQuantity=%d", BuyQuantity);
				FLOGV(" -> CapacityInBAfterTravel=%d", CapacityInBAfterTravel);
				FLOGV(" -> SellQuantity=%d", SellQuantity);
				FLOGV(" -> MoneyGain=%f", MoneyGain/100.f);
				FLOGV(" -> MoneySpend=%f", MoneySpend/100.f);
				FLOGV("   -> OwnedBuyQuantity=%d", OwnedBuyQuantity);
				FLOGV("   -> FactoryBuyQuantity=%d", FactoryBuyQuantity);
				FLOGV("   -> StorageBuyQuantity=%d", StorageBuyQuantity);
				FLOGV(" -> MoneyBalance=%f", MoneyBalance/100.f);
				FLOGV(" -> MoneyBalanceParDay=%f", MoneyBalanceParDay/100.f);
				FLOGV(" -> Resource affility=%f", Behavior->GetResourceAffility(Resource));
				FLOGV(" -> SectorA affility=%f", Behavior->GetSectorAffility(SectorA));
				FLOGV(" -> SectorB affility=%f", Behavior->GetSectorAffility(SectorB));
				FLOGV(" -> Temporisation=%d", Temporisation);
				FLOGV(" -> HighPriority=%d", VariationB->HighPriority);
				FLOGV(" -> Score=%f", Score);
			}
#endif

			if (Score > BestDeal.Score && !Temporisation)
			{
				BestDeal.Score = Score;
				BestDeal.SectorA = SectorA;
				BestDeal.SectorB = SectorB;
				BestDeal.Resource = Resource;
				BestDeal.BuyQuantity = BuyQuantity;

#ifdef DEBUG_AI_TRADING
				if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
				{
					//FLOGV("Travel %s -> %s -> %s : %lld days", *Ship->GetCurrentSector()->GetSectorName().ToString(),
					//*SectorA->GetSectorName().ToString(), *SectorB->GetSectorName().ToString(), TravelTime);

					FLOGV("New Best Resource %s", *Resource->Name.ToString())


				/*	FLOGV(" -> IncomingCapacity=%d", SectorVariationA->IncomingCapacity);
					FLOGV(" -> IncomingResources=%d", VariationA->IncomingResources);
					FLOGV(" -> InitialQuantity=%d", InitialQuantity);
					FLOGV(" -> FreeSpace=%d", FreeSpace);
					FLOGV(" -> StockInAAfterTravel=%d", StockInAAfterTravel);
					FLOGV(" -> BuyQuantity=%d", BuyQuantity);
					FLOGV(" -> CapacityInBAfterTravel=%d", CapacityInBAfterTravel);
					FLOGV(" -> SellQuantity=%u", SellQuantity);
					FLOGV(" -> MoneyGain=%f", MoneyGain/100.f);
					FLOGV(" -> MoneySpend=%f", MoneySpend/100.f);
					FLOGV("   -> OwnedBuyQuantity=%d", OwnedBuyQuantity);
					FLOGV("   -> FactoryBuyQuantity=%d", FactoryBuyQuantity);
					FLOGV("   -> StorageBuyQuantity=%d", StorageBuyQuantity);
					FLOGV(" -> MoneyBalance=%f", MoneyBalance/100.f);
					FLOGV(" -> MoneyBalanceParDay=%f", MoneyBalanceParDay/100.f);
					FLOGV(" -> Resource affility=%f", Behavior->GetResourceAffility(Resource));
					FLOGV(" -> SectorA affility=%f", Behavior->GetSectorAffility(SectorA));
					FLOGV(" -> SectorB affility=%f", Behavior->GetSectorAffility(SectorB));*/
					//FLOGV(" -> Score=%f", Score);
				}
#endif
			}
		}
	}

	return BestDeal;
}

SectorDeal AITradeHelper::FindBestDealForShip(UFlareSimulatedSpacecraft* Ship, TMap<UFlareSimulatedSector*, SectorVariation>& WorldResourceVariation)
{
	SCOPE_CYCLE_COUNTER(STAT_AITradeHelper_FindBestDealForShip);

	UFlareCompany* Company = Ship->GetCompany();

	SectorDeal BestDeal;
	BestDeal.BuyQuantity = 0;
	BestDeal.Score = 0;
	BestDeal.Resource = NULL;
	BestDeal.SectorA = NULL;
	BestDeal.SectorB = NULL;

	// Stay here option
	for (int32 SectorAIndex = 0; SectorAIndex < Company->GetKnownSectors().Num(); SectorAIndex++)
	{
		SCOPE_CYCLE_COUNTER(STAT_AITradeHelper_FindBestDealForShip_Sectors);
		UFlareSimulatedSector* SectorA = Company->GetKnownSectors()[SectorAIndex];

		SectorDeal SectorBestDeal;
		SectorBestDeal.Resource = NULL;
		SectorBestDeal.BuyQuantity = 0;
		SectorBestDeal.Score = 0;
		SectorBestDeal.Resource = NULL;
		SectorBestDeal.SectorA = NULL;
		SectorBestDeal.SectorB = NULL;

		while (true)
		{
			SCOPE_CYCLE_COUNTER(STAT_AITradeHelper_FindBestDealForShip_Loop);

			SectorBestDeal = AITradeHelper::FindBestDealForShipFromSector(Ship, SectorA, &BestDeal, WorldResourceVariation);
			if (!SectorBestDeal.Resource)
			{
				// No best deal found
				break;
			}

			SectorVariation* SectorVariationA = &WorldResourceVariation[SectorA];
			if (Ship->GetCurrentSector() != SectorA && SectorVariationA->IncomingCapacity > 0 && SectorBestDeal.BuyQuantity > 0)
			{
				//FLOGV("UFlareCompanyAI::UpdateTrading : IncomingCapacity to %s = %d", *SectorA->GetSectorName().ToString(), SectorVariationA->IncomingCapacity);
				int32 UsedIncomingCapacity = FMath::Min(SectorBestDeal.BuyQuantity, SectorVariationA->IncomingCapacity);

				SectorVariationA->IncomingCapacity -= UsedIncomingCapacity;
				struct ResourceVariation* VariationA = &SectorVariationA->ResourceVariations[SectorBestDeal.Resource];
				VariationA->OwnedStock -= UsedIncomingCapacity;
			}
			else
			{
				break;
			}
		}

		if (SectorBestDeal.Resource)
		{
			BestDeal = SectorBestDeal;
		}
	}

	return BestDeal;
}

void AITradeHelper::ApplyDeal(UFlareSimulatedSpacecraft* Ship, SectorDeal const&Deal, TMap<UFlareSimulatedSector*, SectorVariation>* WorldResourceVariation)
{
	SCOPE_CYCLE_COUNTER(STAT_AITradeHelper_ApplyDeal);

	AFlareGame* Game = Ship->GetGame();


#ifdef DEBUG_AI_TRADING
	if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
	{
		FLOGV("UFlareCompanyAI::UpdateTrading : Best balance for %s (%s) : %f score",
			*Ship->GetImmatriculation().ToString(), *Ship->GetCurrentSector()->GetSectorName().ToString(), Deal.Score / 100);
		FLOGV("UFlareCompanyAI::UpdateTrading -> Transfer %s from %s to %s",
			*Deal.Resource->Name.ToString(), *Deal.SectorA->GetSectorName().ToString(), *Deal.SectorB->GetSectorName().ToString());
	}
#endif
	if (Ship->GetCurrentSector() == Deal.SectorA)
	{
		// Already in A, buy resources and go to B
		if (Deal.BuyQuantity == 0)
		{
			if (Ship->GetCurrentSector() != Deal.SectorB)
			{
				// Already buy resources,go to B
				Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), Deal.SectorB);
#ifdef DEBUG_AI_TRADING
				if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
				{
					FLOGV("UFlareCompanyAI::UpdateTrading -> Travel to %s to sell", *Deal.SectorB->GetSectorName().ToString());
				}
#endif
			}
		}
		else
		{

			SectorHelper::FlareTradeRequest Request;
			Request.Resource = Deal.Resource;
			Request.Operation = EFlareTradeRouteOperation::LoadOrBuy;
			Request.Client = Ship;
			Request.CargoLimit = 0.f;
			if(Deal.Resource == Game->GetScenarioTools()->FleetSupply)
			{
				Request.MaxQuantity = FMath::Min(Deal.BuyQuantity, Ship->GetActiveCargoBay()->GetFreeSpaceForResource(Deal.Resource, Ship->GetCompany()));
			}
			else
			{
				Request.MaxQuantity = Ship->GetActiveCargoBay()->GetFreeSpaceForResource(Deal.Resource, Ship->GetCompany());
			}

			UFlareSimulatedSpacecraft* StationCandidate = Deal.BuyStation != nullptr ? Deal.BuyStation : SectorHelper::FindTradeStation(Request);

			int32 BroughtResource = 0;
			if (StationCandidate)
			{
				BroughtResource = SectorHelper::Trade(StationCandidate, Ship, Deal.Resource, Request.MaxQuantity);
#ifdef DEBUG_AI_TRADING
				if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
				{
					FLOGV("UFlareCompanyAI::UpdateTrading -> Buy %d / %d to %s", BroughtResource, Deal.BuyQuantity, *StationCandidate->GetImmatriculation().ToString());
				}
#endif
			}

			// TODO reduce computed sector stock

			if(WorldResourceVariation != nullptr)
			{
				if (BroughtResource > 0)
				{
					// Virtualy decrease the stock for other ships in sector A
					SectorVariation* SectorVariationA = &(*WorldResourceVariation)[Deal.SectorA];
					struct ResourceVariation* VariationA = &SectorVariationA->ResourceVariations[Deal.Resource];
					VariationA->OwnedStock -= BroughtResource;


					// Virtualy say some capacity arrive in sector B
					SectorVariation* SectorVariationB = &(*WorldResourceVariation)[Deal.SectorB];
					SectorVariationB->IncomingCapacity += BroughtResource;

					// Virtualy decrease the capacity for other ships in sector B
					struct ResourceVariation* VariationB = &SectorVariationB->ResourceVariations[Deal.Resource];
					VariationB->OwnedCapacity -= BroughtResource;
				}
				else if (BroughtResource == 0)
				{
					// Failed to buy the promised resources, remove the deal from the list
					SectorVariation* SectorVariationA = &(*WorldResourceVariation)[Deal.SectorA];
					struct ResourceVariation* VariationA = &SectorVariationA->ResourceVariations[Deal.Resource];
					VariationA->FactoryStock = 0;
					VariationA->OwnedStock = 0;
					VariationA->StorageStock = 0;
					if (VariationA->OwnedFlow > 0)
						VariationA->OwnedFlow = 0;
					if (VariationA->FactoryFlow > 0)
						VariationA->FactoryFlow = 0;
	#ifdef DEBUG_AI_TRADING
					if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
					{
						FLOG("UFlareCompanyAI::UpdateTrading -> Buy failed, remove the deal from the list");
					}
	#endif
				}
			}
		}
	}
	else
	{
		if (Deal.SectorA != Ship->GetCurrentSector())
		{
			Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), Deal.SectorA);
#ifdef DEBUG_AI_TRADING
			if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
			{
				FLOGV("UFlareCompanyAI::UpdateTrading -> Travel to %s to buy", *Deal.SectorA->GetSectorName().ToString());
			}
#endif
		}
		else
		{
#ifdef DEBUG_AI_TRADING
			if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
			{
				FLOGV("UFlareCompanyAI::UpdateTrading -> Wait to %s", *Deal.SectorA->GetSectorName().ToString());
			}
#endif
		}

		if(WorldResourceVariation != nullptr)
		{
			// Reserve the deal by virtualy decrease the stock for other ships
			SectorVariation* SectorVariationA = &(*WorldResourceVariation)[Deal.SectorA];
			struct ResourceVariation* VariationA = &SectorVariationA->ResourceVariations[Deal.Resource];
			VariationA->OwnedStock -= Deal.BuyQuantity;
			// Virtualy say some capacity arrive in sector B
			SectorVariation* SectorVariationB = &(*WorldResourceVariation)[Deal.SectorB];
			SectorVariationB->IncomingCapacity += Deal.BuyQuantity;

			// Virtualy decrease the capacity for other ships in sector B
			struct ResourceVariation* VariationB = &SectorVariationB->ResourceVariations[Deal.Resource];
			VariationB->OwnedCapacity -= Deal.BuyQuantity;
		}
	}

	if (Ship->GetCurrentSector() == Deal.SectorB && !Ship->IsTrading())
	{
		// Try to sell
		// Try to unload or sell
		SectorHelper::FlareTradeRequest Request;
		Request.Resource = Deal.Resource;
		Request.Operation = EFlareTradeRouteOperation::UnloadOrSell;
		Request.Client = Ship;
		Request.CargoLimit = 1.f;
		Request.MaxQuantity = Ship->GetActiveCargoBay()->GetResourceQuantity(Deal.Resource, Ship->GetCompany());
#ifdef DEBUG_AI_TRADING
		if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
		{
			FLOGV("UFlareCompanyAI::UpdateTrading -> FindTradeStation for max %d",  Request.MaxQuantity);
		}
#endif

		UFlareSimulatedSpacecraft* StationCandidate = Deal.SellStation != nullptr ? Deal.SellStation : SectorHelper::FindTradeStation(Request);

		if (StationCandidate)
		{
			int32 SellQuantity = SectorHelper::Trade(Ship, StationCandidate, Deal.Resource, Request.MaxQuantity);
#ifdef DEBUG_AI_TRADING
			if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
			{
				FLOGV("UFlareCompanyAI::UpdateTrading -> Sell %d / %d to %s", SellQuantity, Request.MaxQuantity, *StationCandidate->GetImmatriculation().ToString());
			}
#endif
		}
	}
}

SectorVariation AITradeHelper::ComputeSectorResourceVariation(UFlareCompany* Company, UFlareSimulatedSector* Sector)
{
	AFlareGame* Game = Company->GetGame();
	UFlareAIBehavior* Behavior = Company->GetAI()->GetBehavior();

	SectorVariation SectorVariation;
	for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
		struct ResourceVariation ResourceVariation;
		ResourceVariation.OwnedFlow = 0;
		ResourceVariation.FactoryFlow = 0;
		ResourceVariation.OwnedStock = 0;
		ResourceVariation.FactoryStock = 0;
		ResourceVariation.StorageStock = 0;
		ResourceVariation.OwnedCapacity = 0;
		ResourceVariation.FactoryCapacity = 0;
		ResourceVariation.StorageCapacity = 0;
		ResourceVariation.MaintenanceCapacity = 0;
		ResourceVariation.IncomingResources = 0;
		ResourceVariation.MinCapacity = 0;
		ResourceVariation.ConsumerMaxStock = 0;
		ResourceVariation.MaintenanceMaxStock = 0;
		ResourceVariation.HighPriority = 0;

		SectorVariation.ResourceVariations.Add(Resource, ResourceVariation);
	}

	int32 OwnedCustomerStation = 0;
	int32 NotOwnedCustomerStation = 0;

	for (int32 StationIndex = 0 ; StationIndex < Sector->GetSectorStations().Num(); StationIndex++)
	{
		UFlareSimulatedSpacecraft* Station = Sector->GetSectorStations()[StationIndex];


		if (Station->GetCompany()->GetWarState(Company) == EFlareHostility::Hostile)
		{
			continue;
		}

		int32 InitialSlotCapacity = Station->GetActiveCargoBay()->GetSlotCapacity();

		for (int32 FactoryIndex = 0; FactoryIndex < Station->GetFactories().Num(); FactoryIndex++)
		{
			UFlareFactory* Factory = Station->GetFactories()[FactoryIndex];
			if ((!Factory->IsActive() || !Factory->IsNeedProduction()))
			{
				// No resources needed
				continue;
			}

			// Input flow
			for (int32 ResourceIndex = 0; ResourceIndex < Factory->GetInputResourcesCount(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = Factory->GetInputResource(ResourceIndex);
				struct ResourceVariation* Variation = &SectorVariation.ResourceVariations[Resource];

				int64 ProductionDuration = Factory->GetProductionDuration();
				if (ProductionDuration == 0)
				{
					ProductionDuration = 10;
				}

				if(Station->IsUnderConstruction())
				{
					Variation->HighPriority += 1000000;
				}


				int32 Flow = FMath::CeilToInt(float(Factory->GetInputResourceQuantity(ResourceIndex)) / float(ProductionDuration));

				int32 CanBuyQuantity =  FMath::Max(0, (int32) (Station->GetCompany()->GetMoney() / Sector->GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryInput)));

				if (Flow == 0)
				{
					continue;
				}

				if (Factory->IsProducing())
				{
					if (Company == Station->GetCompany())
					{
						Variation->OwnedFlow += Flow;
					}
					else
					{
						Flow = FMath::Min(Flow, CanBuyQuantity);
						Variation->FactoryFlow += Flow;
					}
				}

				int32 ResourceQuantity = Station->GetActiveCargoBay()->GetResourceQuantity(Resource, Company);
				int32 MaxCapacity = Station->GetActiveCargoBay()->GetFreeSpaceForResource(Resource, Company);

				float BaseSlotCapacity = InitialSlotCapacity / Station->GetLevel();

				int32 SlotCapacity = InitialSlotCapacity;

				int32 Capacity = FMath::Min(MaxCapacity, (SlotCapacity - ResourceQuantity));

				if (ResourceQuantity < SlotCapacity)
				{
					if (Company == Station->GetCompany())
					{

						Variation->OwnedCapacity += Capacity;
					}
					else
					{
						Capacity = FMath::Min(Capacity, CanBuyQuantity);
						Variation->FactoryCapacity += Capacity * Behavior->TradingSell;
					}



				}
			}

			// Ouput flow
			for (int32 ResourceIndex = 0; ResourceIndex < Factory->GetOutputResourcesCount(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = Factory->GetOutputResource(ResourceIndex);
				struct ResourceVariation* Variation = &SectorVariation.ResourceVariations[Resource];

				int64 ProductionDuration = Factory->GetProductionDuration();
				if (ProductionDuration == 0)
				{
					ProductionDuration = 10;
				}



				int32 Flow = FMath::CeilToInt(float(Factory->GetOutputResourceQuantity(ResourceIndex)) / float(ProductionDuration));


				if (Flow == 0)
				{
					continue;
				}

				if (Factory->IsProducing())
				{
					if (Company == Station->GetCompany())
					{
						Variation->OwnedFlow -= Flow;
					}
					else
					{
						Variation->FactoryFlow -= Flow;
					}
				}

				int32 Stock = Station->GetActiveCargoBay()->GetResourceQuantity(Resource, Company);

				if (Company == Station->GetCompany())
				{
					Variation->OwnedStock += Stock;
				}
				else
				{
					Variation->FactoryStock += Stock * Behavior->TradingBuy;
				}

			}


			// TODO storage

		}

		// Customer flow
		if (!Station->IsUnderConstruction() && Station->HasCapability(EFlareSpacecraftCapability::Consumer))
		{
			if (Company == Station->GetCompany())
			{
				OwnedCustomerStation++;
			}
			else
			{
				NotOwnedCustomerStation++;
			}

			for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->ConsumerResources.Num(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->ConsumerResources[ResourceIndex]->Data;
				struct ResourceVariation* Variation = &SectorVariation.ResourceVariations[Resource];

				int32 ResourceQuantity = Station->GetActiveCargoBay()->GetResourceQuantity(Resource, Company);
				int32 MaxCapacity = Station->GetActiveCargoBay()->GetFreeSpaceForResource(Resource, Company);

				int32 CanBuyQuantity =  (int32) (Station->GetCompany()->GetMoney() / Sector->GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryInput));

				float BaseSlotCapacity = InitialSlotCapacity / Station->GetLevel();
				int32 SlotCapacity = InitialSlotCapacity;

				int32 Capacity = FMath::Min(MaxCapacity, (SlotCapacity - ResourceQuantity));

				// Dept are allowed for sell to customers
				if (ResourceQuantity < SlotCapacity)
				{
					if (Company == Station->GetCompany())
					{
						Variation->OwnedCapacity += Capacity;
					}
					else
					{
						Capacity = FMath::Min(Capacity, CanBuyQuantity);
						Variation->FactoryCapacity += Capacity * Behavior->TradingSell;
					}
				}

				Variation->ConsumerMaxStock += Station->GetActiveCargoBay()->GetSlotCapacity();

			}
		}

		// Maintenance
		if (!Station->IsUnderConstruction() && Station->HasCapability(EFlareSpacecraftCapability::Maintenance))
		{
			for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->MaintenanceResources.Num(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->MaintenanceResources[ResourceIndex]->Data;
				struct ResourceVariation* Variation = &SectorVariation.ResourceVariations[Resource];

				int32 ResourceQuantity = Station->GetActiveCargoBay()->GetResourceQuantity(Resource, Company);
				int32 MaxCapacity = Station->GetActiveCargoBay()->GetFreeSpaceForResource(Resource, Company);

				int32 CanBuyQuantity =  (int32) (Station->GetCompany()->GetMoney() / Sector->GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryInput));
				int32 Capacity = FMath::Min(MaxCapacity, (InitialSlotCapacity - ResourceQuantity));
				int32 SlotCapacity = InitialSlotCapacity;


				float BaseSlotCapacity = SlotCapacity / Station->GetLevel();

				if (ResourceQuantity < SlotCapacity)
				{

					if (Company == Station->GetCompany())
					{
						Variation->OwnedCapacity += Capacity;
					}
					else
					{
						Capacity = FMath::Min(Capacity, CanBuyQuantity);
						Variation->FactoryCapacity += Capacity * Behavior->TradingSell;
					}
				}
				Variation->MaintenanceMaxStock += Station->GetActiveCargoBay()->GetSlotCapacity();

				// The owned resell its own FS

				int32 Stock = Station->GetActiveCargoBay()->GetResourceQuantity(Resource, Company);


				if (Company == Station->GetCompany())
				{
					Variation->OwnedStock += Stock;
				}

			}
		}
	}

	if (OwnedCustomerStation || NotOwnedCustomerStation)
	{
		float OwnedCustomerRatio = (float) OwnedCustomerStation / (float) (OwnedCustomerStation + NotOwnedCustomerStation);
		float NotOwnedCustomerRatio = (float) NotOwnedCustomerStation / (float) (OwnedCustomerStation + NotOwnedCustomerStation);

		for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->ConsumerResources.Num(); ResourceIndex++)
		{
			FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->ConsumerResources[ResourceIndex]->Data;
			struct ResourceVariation* Variation = &SectorVariation.ResourceVariations[Resource];


			int32 Consumption = Sector->GetPeople()->GetRessourceConsumption(Resource, false);

			Variation->OwnedFlow = OwnedCustomerRatio * Consumption;
			Variation->FactoryFlow = NotOwnedCustomerRatio * Consumption * Behavior->TradingSell;
		}
	}

	// Compute incoming capacity and resources
	SectorVariation.IncomingCapacity = 0;
	for (int32 TravelIndex = 0; TravelIndex < Game->GetGameWorld()->GetTravels().Num(); TravelIndex++)
	{
		UFlareTravel* Travel = Game->GetGameWorld()->GetTravels()[TravelIndex];
		if (Travel->GetDestinationSector() != Sector)
		{
			continue;
		}

		int64 RemainingTravelDuration = FMath::Max((int64) 1, Travel->GetRemainingTravelDuration());

		UFlareFleet* IncomingFleet = Travel->GetFleet();


		for (int32 ShipIndex = 0; ShipIndex < IncomingFleet->GetShips().Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = IncomingFleet->GetShips()[ShipIndex];

			if (Ship->GetActiveCargoBay()->GetSlotCapacity() == 0 && Ship->GetDamageSystem()->IsStranded())
			{
				continue;
			}

			if( Ship->GetCompany()->GetMoney() > 0)
			{
				SectorVariation.IncomingCapacity += Ship->GetActiveCargoBay()->GetCapacity() / RemainingTravelDuration;
			}


			TArray<FFlareCargo>& CargoBaySlots = Ship->GetActiveCargoBay()->GetSlots();
			for (int32 CargoIndex = 0; CargoIndex < CargoBaySlots.Num(); CargoIndex++)
			{
				FFlareCargo& Cargo = CargoBaySlots[CargoIndex];

				if (!Cargo.Resource)
				{
					continue;
				}
				struct ResourceVariation* Variation = &SectorVariation.ResourceVariations[Cargo.Resource];

				Variation->IncomingResources += Cargo.Quantity / (RemainingTravelDuration * 0.5);
			}
		}
	}

	// Add damage fleet and repair to maintenance capacity
	for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->MaintenanceResources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->MaintenanceResources[ResourceIndex]->Data;
		struct ResourceVariation* Variation = &SectorVariation.ResourceVariations[Resource];

		for (int CompanyIndex = 0; CompanyIndex < Game->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
		{
			UFlareCompany* OtherCompany = Game->GetGameWorld()->GetCompanies()[CompanyIndex];

			if (OtherCompany->GetWarState(Company) == EFlareHostility::Hostile)
			{
				continue;
			}

			int32 NeededFS;
			int32 TotalNeededFS;
			int64 MaxDuration;
			int32 NeededFSSum = 0;

			SectorHelper::GetRefillFleetSupplyNeeds(Sector, OtherCompany, NeededFS, TotalNeededFS, MaxDuration);
			NeededFSSum += TotalNeededFS;

			SectorHelper::GetRepairFleetSupplyNeeds(Sector, OtherCompany, NeededFS, TotalNeededFS, MaxDuration);
			NeededFSSum += TotalNeededFS;

			if(OtherCompany == Company)
			{
				Variation->MaintenanceCapacity += NeededFSSum;
			}
			else
			{
				FFlareResourceDescription* FleetSupply = Sector->GetGame()->GetScenarioTools()->FleetSupply;
				int32 CanBuyQuantity =  (int32) (OtherCompany->GetMoney() / Sector->GetResourcePrice(FleetSupply, EFlareResourcePriceContext::FactoryInput));

				Variation->MaintenanceCapacity += FMath::Min(CanBuyQuantity, NeededFSSum);
			}
		}
	}

	return SectorVariation;
}


AITradeNeeds AITradeHelper::GenerateTradingNeeds(UFlareWorld* World)
{
	AITradeNeeds Needs;

	return Needs;
}

AITradeSources AITradeHelper::GenerateTradingSources(UFlareWorld* World)
{
	AITradeSources Sources;


	return Sources;
}

AITradeIdleShips AITradeHelper::GenerateIdleShips(UFlareWorld* World)
{
	AITradeIdleShips Ships;

	return Ships;
}


inline static bool NeedComparatorComparator(const AITradeNeed& n1, const AITradeNeed& n2)
{
	return n1.Ratio > n2.Ratio;
}


void AITradeHelper::ComputeGlobalTrading(UFlareWorld* World, AITradeNeeds& Needs, AITradeSources& Sources, AITradeIdleShips& IdleShips)
{
	while(Needs.List.Num() > 0)
	{
		Needs.List.Sort(&NeedComparatorComparator);

		TArray<AITradeNeed> KeepList;

		for(AITradeNeed& Need : Needs.List)
		{
			bool Keep = ProcessNeed(Need, Sources, IdleShips);

			if(Keep)
			{
				KeepList.Add(Need);
			}
		}

		Needs.List = KeepList;
	}
}

bool AITradeHelper::ProcessNeed(AITradeNeed& Need, AITradeSources& Sources, AITradeIdleShips& IdleShips)
{
	AITradeSource* Source = FindBestSource(Need.Resource, Need.Sector, Need.Company);

	if(Source == nullptr)
	{
		// No possible source, don't keep
		return false;
	}

	int32 UsedQuantity = 0;

	if(Source->Ship != nullptr)
	{
		// Source in ship, consume source and process transfer
		Sources.ConsumeSource(Source);
		UsedQuantity = FMath::Min(Need.Quantity, Source->Quantity);

		SectorDeal Deal;
		Deal.SectorA = Need.Sector;
		Deal.SectorB = Need.Sector;
		Deal.BuyStation = nullptr;
		Deal.SellStation = Need.Station;

		ApplyDeal(Source->Ship, Deal, nullptr);
	}
	else
	{
		UFlareSimulatedSpacecraft* Ship = FindBestShip(IdleShips, Source->Sector, Source->Company, Need.Company);

		if(Ship == nullptr)
		{
			// No availble ship, don't keep
			return false;
		}

		UsedQuantity = FMath::Min(Need.Quantity, Source->Quantity);
		UsedQuantity = FMath::Min(UsedQuantity, Ship->GetActiveCargoBay()->GetFreeSpaceForResource(Need.Resource, Ship->GetCompany()));


		SectorDeal Deal;
		Deal.SectorA = Source->Sector;
		Deal.SectorB = Need.Sector;
		Deal.BuyStation = Source->Station;
		Deal.SellStation = Need.Station;
		Deal.BuyQuantity = UsedQuantity;

		ApplyDeal(Ship, Deal, nullptr);

		Sources.ConsumeSource(Source, UsedQuantity);
		IdleShips.ConsumeShip(Ship);
	}

	Need.Quantity -= UsedQuantity;

	if(Need.Quantity <= 0)
	{
		// Need fulfill
		return false;
	}
	else
	{
		return true;
	}
}

AITradeSource* AITradeHelper::FindBestSource(FFlareResourceDescription* Resource, UFlareSimulatedSector* Sector, UFlareCompany* Company)
{
#error TODO
}

UFlareSimulatedSpacecraft* AITradeHelper::FindBestShip(AITradeIdleShips& IdleShips, UFlareSimulatedSector* Sector, UFlareCompany* SourceCompany, UFlareCompany* NeedCompany)
{
	#error TODO
}

void AITradeSources::ConsumeSource(AITradeSource*)
{
	#error TODO
}

void AITradeSources::ConsumeSource(AITradeSource*, int32 Quantity)
{
	#error TODO
}

void AITradeIdleShips::ConsumeShip(UFlareSimulatedSpacecraft* Ship)
{
	#error TODO
}
