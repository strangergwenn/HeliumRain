
#include "FlareCompanyAI.h"
#include "../../Flare.h"
#include "FlareAIBehavior.h"

#include "../FlareGame.h"
#include "../FlareGameTools.h"
#include "../FlareCompany.h"
#include "../FlareSectorHelper.h"
#include "../FlareScenarioTools.h"

#include "../../Data/FlareResourceCatalog.h"
#include "../../Data/FlareFactoryCatalogEntry.h"
#include "../../Data/FlareTechnologyCatalog.h"
#include "../../Data/FlareSpacecraftCatalog.h"
#include "../../Data/FlareSpacecraftComponentsCatalog.h"

#include "../../Economy/FlareFactory.h"
#include "../../Economy/FlareCargoBay.h"

#include "../../Player/FlarePlayerController.h"

#include "../../Quests/FlareQuestManager.h"
#include "../../Quests/FlareQuestGenerator.h"

#include "../../Spacecrafts/FlareSimulatedSpacecraft.h"


#define STATION_CONSTRUCTION_PRICE_BONUS 1.2

// TODO, make it depend on company's nature
#define AI_CARGO_DIVERSITY_THRESHOLD 1
#define AI_CARGO_SIZE_DIVERSITY_THRESHOLD 5
#define AI_CARGO_SIZE_DIVERSITY_THRESHOLD_BASE 15

#define AI_MILITARY_DIVERSITY_THRESHOLD 1
#define AI_MILITARY_SIZE_DIVERSITY_THRESHOLD 5
#define AI_MILITARY_SIZE_DIVERSITY_THRESHOLD_BASE 5

// TODO, make it depend on company's nature
#define AI_CARGO_PEACE_MILILTARY_THRESHOLD 10

// If one cargo out of X ships is wrecked, the fleet is unhealthy
#define AI_CARGO_HEALTHY_THRESHOLD 5

//#define DEBUG_AI_WAR_MILITARY_MOVEMENT
//#define DEBUG_AI_BATTLE_STATES
//#define DEBUG_AI_BUDGET
#define LOCTEXT_NAMESPACE "FlareCompanyAI"


DECLARE_CYCLE_STAT(TEXT("FlareCompanyAI UpdateDiplomacy"), STAT_FlareCompanyAI_UpdateDiplomacy, STATGROUP_Flare);

DECLARE_CYCLE_STAT(TEXT("FlareCompanyAI UpdateTrading"), STAT_FlareCompanyAI_UpdateTrading, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareCompanyAI UpdateTrading Ships"), STAT_FlareCompanyAI_UpdateTrading_Ships, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareCompanyAI UpdateTrading BestDealLoop"), STAT_FlareCompanyAI_UpdateTrading_BestDealLoop, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareCompanyAI UpdateTrading BestDealFound"), STAT_FlareCompanyAI_UpdateTrading_BestDealFound, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareCompanyAI FindBestDealForShipFromSector"), STAT_FlareCompanyAI_FindBestDealForShipFromSector, STATGROUP_Flare);



DECLARE_CYCLE_STAT(TEXT("FlareCompanyAI UpdateTrading Sectors"), STAT_FlareCompanyAI_UpdateTrading_Sectors, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareCompanyAI UpdateTrading Best Deal"), STAT_FlareCompanyAI_UpdateTrading_BestDeal, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareCompanyAI UpdateTrading Deal"), STAT_FlareCompanyAI_UpdateTrading_Deal, STATGROUP_Flare);

DECLARE_CYCLE_STAT(TEXT("FlareCompanyAI CargosEvasion"), STAT_FlareCompanyAI_CargosEvasion, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareCompanyAI RepairAndRefill"), STAT_FlareCompanyAI_RepairAndRefill, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareCompanyAI ProcessBudget"), STAT_FlareCompanyAI_ProcessBudget, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareCompanyAI UpdateMilitaryMovement"), STAT_FlareCompanyAI_UpdateMilitaryMovement, STATGROUP_Flare);


/*----------------------------------------------------
	Public API
----------------------------------------------------*/

UFlareCompanyAI::UFlareCompanyAI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AllBudgets.Add(EFlareBudget::Military);
	AllBudgets.Add(EFlareBudget::Station);
	AllBudgets.Add(EFlareBudget::Technology);
	AllBudgets.Add(EFlareBudget::Trade);
}

void UFlareCompanyAI::Load(UFlareCompany* ParentCompany, const FFlareCompanyAISave& Data)
{
	Company = ParentCompany;
	Game = Company->GetGame();
	AIData = Data;

	// Setup Behavior
	Behavior = NewObject<UFlareAIBehavior>(this, UFlareAIBehavior::StaticClass());
}

FFlareCompanyAISave* UFlareCompanyAI::Save()
{
	return &AIData;
}

void UFlareCompanyAI::Tick()
{
	if (Game && Company != Game->GetPC()->GetCompany())
	{
	}
}

void UFlareCompanyAI::Simulate()
{
	if (Game && Company != Game->GetPC()->GetCompany())
	{
		Behavior->Load(Company);

		CheckBattleResolution();
		UpdateDiplomacy();

		WorldStats = WorldHelper::ComputeWorldResourceStats(Game);
		Shipyards = FindShipyards();

		// Compute input and output ressource equation (ex: 100 + 10/ day)
		WorldResourceVariation.Empty();
		for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
		{
			UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];
			SectorVariation Variation = ComputeSectorResourceVariation(Sector);

			WorldResourceVariation.Add(Sector, Variation);
			//DumpSectorResourceVariation(Sector, &Variation);
		}

		Behavior->Simulate();

		PurchaseResearch();

		// Check if at war
		if(Company->AtWar())
		{
			AIData.Pacifism += Behavior->PacifismIncrementRate;
		}
		else
		{
			AIData.Pacifism -= Behavior->PacifismDecrementRate;
		}

		if(IdleCargoCapacity == 0)
		{
			AIData.Pacifism += Behavior->PacifismIncrementRate/3;
		}

		for(UFlareSimulatedSpacecraft* Spacecraft : Company->GetCompanySpacecrafts())
		{
			if(Spacecraft->GetDamageSystem()->GetGlobalDamageRatio() < 0.99)
			{
				AIData.Pacifism+= Behavior->PacifismIncrementRate * 0.5;
			}
		}

		AIData.Pacifism = FMath::Clamp(AIData.Pacifism, 0.f,100.f);

		FLOGV("Pacifism for %s : %f (IdleCargoCapacity=%d)", *Company->GetCompanyName().ToString(), AIData.Pacifism, IdleCargoCapacity);

	}
}

void UFlareCompanyAI::PurchaseResearch()
{
	FText Reason;
	if (AIData.ResearchProject == NAME_None || !Company->IsTechnologyAvailable(AIData.ResearchProject, Reason, true))
	{
		// Find a new research
		TArray<FFlareTechnologyDescription*> ResearchCandidates;

		for(UFlareTechnologyCatalogEntry* Technology : GetGame()->GetTechnologyCatalog()->TechnologyCatalog)
		{
			if(Company->IsTechnologyAvailable(Technology->Data.Identifier, Reason, true))
			{
				ResearchCandidates.Add(&Technology->Data);
			}
		}

		if(ResearchCandidates.Num() == 0)
		{
			// No research to research
			return;
		}

		int32 PickIndex = FMath::RandRange(0, ResearchCandidates.Num() - 1);
		AIData.ResearchProject = ResearchCandidates[PickIndex]->Identifier;
	}

	// Try to buy
	if(Company->IsTechnologyAvailable(AIData.ResearchProject, Reason))
	{
		Company->UnlockTechnology(AIData.ResearchProject);
	}
}

void UFlareCompanyAI::DestroySpacecraft(UFlareSimulatedSpacecraft* Spacecraft)
{
}


/*----------------------------------------------------
	Internal subsystems
----------------------------------------------------*/

void UFlareCompanyAI::UpdateDiplomacy()
{
	SCOPE_CYCLE_COUNTER(STAT_FlareCompanyAI_UpdateDiplomacy);

	Behavior->Load(Company);
	Behavior->UpdateDiplomacy();
}

//#define DEBUG_AI_TRADING
#define DEBUG_AI_TRADING_COMPANY "MSY"
#define DEBUG_AI_TRADING_SECTOR_B "the-forge"
#define DEBUG_AI_TRADING_RESOURCES "sio2"

void UFlareCompanyAI::UpdateTrading()
{
	SCOPE_CYCLE_COUNTER(STAT_FlareCompanyAI_UpdateTrading);

	IdleCargoCapacity = 0;
	TArray<UFlareSimulatedSpacecraft*> IdleCargos = FindIdleCargos();
#ifdef DEBUG_AI_TRADING
	if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
	{
		FLOGV("UFlareCompanyAI::UpdateTrading : %s has %d idle ships", *Company->GetCompanyName().ToString(), IdleCargos.Num());
	}
#endif

	// TODO Check the option of waiting for some resource to fill the cargo in local sector
	// TODO reduce attrativeness of already ship on the same spot
	// TODO hub by stock, % of world production max
	// TODO always keep money for production
	
	// For best option, if local, buy and travel, if not local, travel
	
	// For all current trade route in a sector (if not in a sector, it's not possible to modify then)
	//      -> Compute the resource balance in the dest sector and the resource balance in the source sector
	//			-> If the balance is negative in the dest sector, and positive un the source add a cargo
	//      -> Compute the current transport rate for the resource (resource/day)(mean on multiple travel) and the max transport rate
	//			-> If current is a lot below the max, remove a cargo

	// If inactive cargo
	//  compute max negative balance. Find nearest sector with a positive balance.
	//  create a route.
	//  assign enough capacity to match the min(negative balance, positive balance)
		

	IdleCargos.Sort([](const UFlareSimulatedSpacecraft& Left, const UFlareSimulatedSpacecraft& Right)
	{
		if(Left.GetActiveCargoBay()->GetFreeSlotCount() != Right.GetActiveCargoBay()->GetFreeSlotCount())
		{
			return Left.GetActiveCargoBay()->GetFreeSlotCount() < Right.GetActiveCargoBay()->GetFreeSlotCount();
		}
		else if(Left.GetActiveCargoBay()->GetUsedCargoSpace() > 0 && Right.GetActiveCargoBay()->GetUsedCargoSpace() > 0)
		{
			return Left.GetActiveCargoBay()->GetFreeCargoSpace() > Left.GetActiveCargoBay()->GetFreeCargoSpace();
		}
		else if(Left.GetActiveCargoBay()->GetUsedCargoSpace() == 0 && Right.GetActiveCargoBay()->GetUsedCargoSpace() == 0)
		{
			if(Left.GetActiveCargoBay()->GetFreeCargoSpace() == Right.GetActiveCargoBay()->GetFreeCargoSpace())
			{
				return Left.GetImmatriculation() < Right.GetImmatriculation();
			}
			else
			{
				return Left.GetActiveCargoBay()->GetFreeCargoSpace() > Right.GetActiveCargoBay()->GetFreeCargoSpace();
			}
		}
		else
		{
			return Left.GetActiveCargoBay()->GetUsedCargoSpace() > Right.GetActiveCargoBay()->GetUsedCargoSpace();
		}
	});


	for (int32 ShipIndex = 0; ShipIndex < IdleCargos.Num(); ShipIndex++)
	{
		SCOPE_CYCLE_COUNTER(STAT_FlareCompanyAI_UpdateTrading_Ships);

		UFlareSimulatedSpacecraft* Ship = IdleCargos[ShipIndex];

#ifdef DEBUG_AI_TRADING
		if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
		{
			FLOGV("UFlareCompanyAI::UpdateTrading : Search something to do for %s", *Ship->GetImmatriculation().ToString());
		}
#endif

		/*FLOGV("UFlareCompanyAI::UpdateTrading : Search something to do for %s", *Ship->GetImmatriculation().ToString());
		FLOGV(" - GetFreeSlotCount: %d", Ship->GetActiveCargoBay()->GetFreeSlotCount());
		FLOGV(" - GetUsedCargoSpace: %d", Ship->GetActiveCargoBay()->GetUsedCargoSpace());
		FLOGV(" - GetFreeCargoSpace: %d", Ship->GetActiveCargoBay()->GetFreeCargoSpace());
*/
		
		SectorDeal BestDeal;
		BestDeal.BuyQuantity = 0;
		BestDeal.Score = 0;
		BestDeal.Resource = NULL;
		BestDeal.SectorA = NULL;
		BestDeal.SectorB = NULL;
		
		// Stay here option
		
		for (int32 SectorAIndex = 0; SectorAIndex < Company->GetKnownSectors().Num(); SectorAIndex++)
		{
			SCOPE_CYCLE_COUNTER(STAT_FlareCompanyAI_UpdateTrading_Sectors);
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
				SCOPE_CYCLE_COUNTER(STAT_FlareCompanyAI_UpdateTrading_BestDealLoop);

				SectorBestDeal = FindBestDealForShipFromSector(Ship, SectorA, &BestDeal);
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

		if (BestDeal.Resource)
		{
			SCOPE_CYCLE_COUNTER(STAT_FlareCompanyAI_UpdateTrading_BestDealFound);

#ifdef DEBUG_AI_TRADING
			if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
			{
				FLOGV("UFlareCompanyAI::UpdateTrading : Best balance for %s (%s) : %f score",
					*Ship->GetImmatriculation().ToString(), *Ship->GetCurrentSector()->GetSectorName().ToString(), BestDeal.Score / 100);
				FLOGV("UFlareCompanyAI::UpdateTrading -> Transfer %s from %s to %s",
					*BestDeal.Resource->Name.ToString(), *BestDeal.SectorA->GetSectorName().ToString(), *BestDeal.SectorB->GetSectorName().ToString());
			}
#endif
			if (Ship->GetCurrentSector() == BestDeal.SectorA)
			{
				SCOPE_CYCLE_COUNTER(STAT_FlareCompanyAI_UpdateTrading_BestDeal);

				// Already in A, buy resources and go to B
				if (BestDeal.BuyQuantity == 0)
				{
					if (Ship->GetCurrentSector() != BestDeal.SectorB)
					{
						// Already buy resources,go to B
						Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), BestDeal.SectorB);
#ifdef DEBUG_AI_TRADING
						if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
						{
							FLOGV("UFlareCompanyAI::UpdateTrading -> Travel to %s to sell", *BestDeal.SectorB->GetSectorName().ToString());
						}
#endif
					}
				}
				else
				{

					SectorHelper::FlareTradeRequest Request;
					Request.Resource = BestDeal.Resource;
					Request.Operation = EFlareTradeRouteOperation::LoadOrBuy;
					Request.Client = Ship;
					Request.CargoLimit = 0.f;
					if(BestDeal.Resource == GetGame()->GetScenarioTools()->FleetSupply)
					{
						Request.MaxQuantity = FMath::Min(BestDeal.BuyQuantity, Ship->GetActiveCargoBay()->GetFreeSpaceForResource(BestDeal.Resource, Ship->GetCompany()));
					}
					else
					{
						Request.MaxQuantity = Ship->GetActiveCargoBay()->GetFreeSpaceForResource(BestDeal.Resource, Ship->GetCompany());
					}

					UFlareSimulatedSpacecraft* StationCandidate = SectorHelper::FindTradeStation(Request);

					int32 BroughtResource = 0;
					if (StationCandidate)
					{
						BroughtResource = SectorHelper::Trade(StationCandidate, Ship, BestDeal.Resource, Request.MaxQuantity);
#ifdef DEBUG_AI_TRADING
						if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
						{
							FLOGV("UFlareCompanyAI::UpdateTrading -> Buy %d / %d to %s", BroughtResource, BestDeal.BuyQuantity, *StationCandidate->GetImmatriculation().ToString());
						}
#endif
					}

					// TODO reduce computed sector stock


					if (BroughtResource > 0)
					{
						// Virtualy decrease the stock for other ships in sector A
						SectorVariation* SectorVariationA = &WorldResourceVariation[BestDeal.SectorA];
						struct ResourceVariation* VariationA = &SectorVariationA->ResourceVariations[BestDeal.Resource];
						VariationA->OwnedStock -= BroughtResource;


						// Virtualy say some capacity arrive in sector B
						SectorVariation* SectorVariationB = &WorldResourceVariation[BestDeal.SectorB];
						SectorVariationB->IncomingCapacity += BroughtResource;

						// Virtualy decrease the capacity for other ships in sector B
						struct ResourceVariation* VariationB = &SectorVariationB->ResourceVariations[BestDeal.Resource];
						VariationB->OwnedCapacity -= BroughtResource;
					}
					else if (BroughtResource == 0)
					{
						// Failed to buy the promised resources, remove the deal from the list
						SectorVariation* SectorVariationA = &WorldResourceVariation[BestDeal.SectorA];
						struct ResourceVariation* VariationA = &SectorVariationA->ResourceVariations[BestDeal.Resource];
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
			else
			{
				SCOPE_CYCLE_COUNTER(STAT_FlareCompanyAI_UpdateTrading_Deal);

				if (BestDeal.SectorA != Ship->GetCurrentSector())
				{
					Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), BestDeal.SectorA);
#ifdef DEBUG_AI_TRADING
					if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
					{
						FLOGV("UFlareCompanyAI::UpdateTrading -> Travel to %s to buy", *BestDeal.SectorA->GetSectorName().ToString());
					}
#endif
				}
				else
				{
#ifdef DEBUG_AI_TRADING
					if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
					{
						FLOGV("UFlareCompanyAI::UpdateTrading -> Wait to %s", *BestDeal.SectorA->GetSectorName().ToString());
					}
#endif
				}

				// Reserve the deal by virtualy decrease the stock for other ships
				SectorVariation* SectorVariationA = &WorldResourceVariation[BestDeal.SectorA];
				struct ResourceVariation* VariationA = &SectorVariationA->ResourceVariations[BestDeal.Resource];
				VariationA->OwnedStock -= BestDeal.BuyQuantity;
				// Virtualy say some capacity arrive in sector B
				SectorVariation* SectorVariationB = &WorldResourceVariation[BestDeal.SectorB];
				SectorVariationB->IncomingCapacity += BestDeal.BuyQuantity;

				// Virtualy decrease the capacity for other ships in sector B
				struct ResourceVariation* VariationB = &SectorVariationB->ResourceVariations[BestDeal.Resource];
				VariationB->OwnedCapacity -= BestDeal.BuyQuantity;
			}

			if (Ship->GetCurrentSector() == BestDeal.SectorB && !Ship->IsTrading())
			{
				// Try to sell
				// Try to unload or sell
				SectorHelper::FlareTradeRequest Request;
				Request.Resource = BestDeal.Resource;
				Request.Operation = EFlareTradeRouteOperation::UnloadOrSell;
				Request.Client = Ship;
				Request.CargoLimit = 1.f;
				Request.MaxQuantity = Ship->GetActiveCargoBay()->GetResourceQuantity(BestDeal.Resource, Ship->GetCompany());
#ifdef DEBUG_AI_TRADING
				if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
				{
					FLOGV("UFlareCompanyAI::UpdateTrading -> FindTradeStation for max %d",  Request.MaxQuantity);
				}
#endif

				UFlareSimulatedSpacecraft* StationCandidate = SectorHelper::FindTradeStation(Request);

				if (StationCandidate)
				{
					int32 SellQuantity = SectorHelper::Trade(Ship, StationCandidate, BestDeal.Resource, Request.MaxQuantity);
#ifdef DEBUG_AI_TRADING
					if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
					{
						FLOGV("UFlareCompanyAI::UpdateTrading -> Sell %d / %d to %s", SellQuantity, Request.MaxQuantity, *StationCandidate->GetImmatriculation().ToString());
					}
#endif
				}
			}
		}
		else
		{
#ifdef DEBUG_AI_TRADING
			if (Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
			{
				FLOGV("UFlareCompanyAI::UpdateTrading : %s found nothing to do", *Ship->GetImmatriculation().ToString());
			}
#endif

			if (Ship->GetActiveCargoBay()->GetFreeSlotCount() > 0)
			{
				IdleCargoCapacity += Ship->GetActiveCargoBay()->GetCapacity() * Ship->GetActiveCargoBay()->GetFreeSlotCount();
			}

			// TODO recruit to build station
		}
	}
}

void UFlareCompanyAI::RepairAndRefill()
{
	SCOPE_CYCLE_COUNTER(STAT_FlareCompanyAI_RepairAndRefill);

	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];
		SectorHelper::RepairFleets(Sector, Company);
	}

	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];
		SectorHelper::RefillFleets(Sector, Company);
	}
}

#define DEBUG_AI_CONSTRUCTION 0

void UFlareCompanyAI::UpdateBestScore(float Score,
									  UFlareSimulatedSector* Sector,
									  FFlareSpacecraftDescription* StationDescription,
									  UFlareSimulatedSpacecraft *Station,
									  float* BestScore,
									  FFlareSpacecraftDescription** BestStationDescription,
									  UFlareSimulatedSpacecraft** BestStation,
									  UFlareSimulatedSector** BestSector)
{
	//FLOGV("UpdateBestScore Score=%f BestScore=%f", Score, *BestScore);

	// Change best if we found better
	if (Score > 0.f && (!BestStationDescription || Score > *BestScore))
	{
		//FLOGV("New Best : Score=%f", Score);

		*BestScore = Score;
		*BestStationDescription = (Station ? Station->GetDescription() : StationDescription);
		*BestStation = Station;
		*BestSector = Sector;
	}
}

/*----------------------------------------------------
	Budget
----------------------------------------------------*/

void UFlareCompanyAI::SpendBudget(EFlareBudget::Type Type, int64 Amount)
{
	// A project spend money, dispatch available money for others projects


#ifdef DEBUG_AI_BUDGET
	FLOGV("%s spend %lld on %d", *Company->GetCompanyName().ToString(), Amount, (Type+0));
#endif
	ModifyBudget(Type, -Amount);

	float TotalWeight = 0;

	for (EFlareBudget::Type Budget : AllBudgets)
	{
		TotalWeight += Behavior->GetBudgetWeight(Budget);
	}

	for (EFlareBudget::Type Budget : AllBudgets)
	{
		ModifyBudget(Budget, Amount * Behavior->GetBudgetWeight(Budget) / TotalWeight);
	}
}

int64 UFlareCompanyAI::GetBudget(EFlareBudget::Type Type)
{
	switch(Type)
	{
		case EFlareBudget::Military:
			return AIData.BudgetMilitary;
		break;
		case EFlareBudget::Station:
			return AIData.BudgetStation;
		break;
		case EFlareBudget::Technology:
			return AIData.BudgetTechnology;
		break;
		case EFlareBudget::Trade:
			return AIData.BudgetTrade;
		break;
	}
#ifdef DEBUG_AI_BUDGET
	FLOGV("GetBudget: unknown budget type %d", Type);
#endif
	return 0;
}

void UFlareCompanyAI::ModifyBudget(EFlareBudget::Type Type, int64 Amount)
{
	switch(Type)
	{
		case EFlareBudget::Military:
			AIData.BudgetMilitary += Amount;
#ifdef DEBUG_AI_BUDGET
			FLOGV("New military budget %lld (%lld)", AIData.BudgetMilitary, Amount);
#endif
		break;
		case EFlareBudget::Station:
			AIData.BudgetStation += Amount;
#ifdef DEBUG_AI_BUDGET
			FLOGV("New station budget %lld (%lld)", AIData.BudgetStation, Amount);
#endif
		break;
		case EFlareBudget::Technology:
			AIData.BudgetTechnology += Amount;
#ifdef DEBUG_AI_BUDGET
			FLOGV("New technology budget %lld (%lld)", AIData.BudgetTechnology, Amount);
#endif
		break;
		case EFlareBudget::Trade:
			AIData.BudgetTrade += Amount;
#ifdef DEBUG_AI_BUDGET
			FLOGV("New trade budget %lld (%lld)", AIData.BudgetTrade, Amount);
#endif
		break;
#ifdef DEBUG_AI_BUDGET
	default:
			FLOGV("ModifyBudget: unknown budget type %d", Type);
#endif
	}
}

void UFlareCompanyAI::ProcessBudget(TArray<EFlareBudget::Type> BudgetToProcess)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareCompanyAI_ProcessBudget);

	// Find
#ifdef DEBUG_AI_BUDGET
	FLOGV("Process budget for %s (%d projects)", *Company->GetCompanyName().ToString(), BudgetToProcess.Num());
#endif

	EFlareBudget::Type MaxBudgetType = EFlareBudget::None;
	int64 MaxBudgetAmount = 0;

	for (EFlareBudget::Type Type : BudgetToProcess)
	{
		int64 Budget = GetBudget(Type);
		if (MaxBudgetType == EFlareBudget::None || MaxBudgetAmount < Budget)
		{
			MaxBudgetType = Type;
			MaxBudgetAmount = Budget;
		}
	}

	if (MaxBudgetType == EFlareBudget::None)
	{
		// Nothing to do
		return;
	}
#ifdef DEBUG_AI_BUDGET
	FLOGV("max budget for %d with %lld", MaxBudgetType + 0, MaxBudgetAmount);
#endif

	bool Lock = false;
	bool Idle = true;

	if (Behavior->GetBudgetWeight(MaxBudgetType) > 0)
	{
		switch (MaxBudgetType)
		{
			case EFlareBudget::Military:
				ProcessBudgetMilitary(MaxBudgetAmount, Lock, Idle);
			break;
			case EFlareBudget::Trade:
				ProcessBudgetTrade(MaxBudgetAmount, Lock, Idle);
			break;
			case EFlareBudget::Station:
				ProcessBudgetStation(MaxBudgetAmount, false, Lock, Idle);
			break;
			case EFlareBudget::Technology:
				ProcessBudgetStation(MaxBudgetAmount, true, Lock, Idle);
			break;
		}
	}

	if (Lock)
	{
#ifdef DEBUG_AI_BUDGET
		FLOG("Lock");
#endif
		// Process no other projets
		return;
	}

	if (Idle)
	{
#ifdef DEBUG_AI_BUDGET
		FLOG("Idle");
#endif
		// Nothing to buy consume a part of its budget
		SpendBudget(MaxBudgetType, MaxBudgetAmount / 100);
	}

	BudgetToProcess.Remove(MaxBudgetType);
	ProcessBudget(BudgetToProcess);
}

void UFlareCompanyAI::ProcessBudgetMilitary(int64 BudgetAmount, bool& Lock, bool& Idle)
{
	// Min confidence level
	float MinConfidenceLevel = 1;

	for (UFlareCompany* OtherCompany : Game->GetGameWorld()->GetCompanies())
	{
		if (OtherCompany == Company)
		{
			continue;
		}

		TArray<UFlareCompany*> Allies;
		float ConfidenceLevel = Company->GetConfidenceLevel(OtherCompany, Allies);
		if (MinConfidenceLevel > ConfidenceLevel)
		{
			MinConfidenceLevel = ConfidenceLevel;
		}
	}

	if (!Company->AtWar() && (MinConfidenceLevel > Behavior->ConfidenceTarget || AIData.Pacifism > 90))
	{
		// Army size is ok
		Idle = true;
		Lock = false;
		return;
	}

	Idle = false;

	int64 ProjectCost = UpdateWarShipAcquisition(false);

	if (ProjectCost > 0 && ProjectCost < BudgetAmount / 2)
	{
		Lock = true;
	}
}

void UFlareCompanyAI::ProcessBudgetTrade(int64 BudgetAmount, bool& Lock, bool& Idle)
{
	int32 DamagedCargosCapacity = GetDamagedCargosCapacity();

	//FLOGV("%s DamagedCargosCapacity=%d", *Company->GetCompanyName().ToString(), DamagedCargosCapacity);
	//FLOGV("%s IdleCargoCapacity=%d", *Company->GetCompanyName().ToString(),IdleCargoCapacity);
	//FLOGV("%s CargosCapacity=%d", *Company->GetCompanyName().ToString(),GetCargosCapacity());

	float IdleRatio = float(IdleCargoCapacity + DamagedCargosCapacity) / GetCargosCapacity();

	if (IdleRatio > 0.1f)
	{
		//FLOG("fleet ok");
		// Trade fllet size is ok
		Idle = true;
		Lock = false;
		return;
	}

	Idle = false;

	int64 ProjectCost = UpdateCargoShipAcquisition();

	//FLOGV("ProjectCost %lld", ProjectCost);

	if (ProjectCost > 0 && ProjectCost < BudgetAmount / 2)
	{
		//FLOG("lock : BudgetAmount %lld");
		Lock = true;
	}
}

void UFlareCompanyAI::ProcessBudgetStation(int64 BudgetAmount, bool Technology, bool& Lock, bool& Idle)
{
	Idle = false;
	// Prepare resources for station-building analysis
	float BestScore = 0;
	UFlareSimulatedSector* BestSector = NULL;
	FFlareSpacecraftDescription* BestStationDescription = NULL;
	UFlareSimulatedSpacecraft* BestStation = NULL;
	TArray<UFlareSpacecraftCatalogEntry*>& StationCatalog = Game->GetSpacecraftCatalog()->StationCatalog;

	//Check if a construction is in progress
	for(UFlareSimulatedSpacecraft* Station : Company->GetCompanyStations())
	{
		if(Station->IsUnderConstruction())
		{
			return;
		}
	}

	// Loop on sector list
	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];

		// Loop on catalog
		for (int32 StationIndex = 0; StationIndex < StationCatalog.Num(); StationIndex++)
		{
			FFlareSpacecraftDescription* StationDescription = &StationCatalog[StationIndex]->Data;

			if (StationDescription->IsSubstation)
			{
				// Never try to build substations
				continue;
			}

			if (!Company->IsTechnologyUnlockedStation(StationDescription))
			{
				continue;
			}

			// Check sector limitations
			TArray<FText> Reasons;
			if (!Sector->CanBuildStation(StationDescription, Company, Reasons, true))
			{
				continue;
			}


			int32 UpdatableStationCountForThisKind = 0;
			for(UFlareSimulatedSpacecraft* StationCandidate : Company->GetCompanyStations())
			{
				if(StationDescription == StationCandidate->GetDescription() && StationCandidate->GetLevel() < StationDescription->MaxLevel)
				{
					UpdatableStationCountForThisKind++;
				}
			}

			if(UpdatableStationCountForThisKind >= 2)
			{
				// Prefer update if possible
				continue;
			}


			//FLOGV("> Analyse build %s in %s", *StationDescription->Name.ToString(), *Sector->GetSectorName().ToString());

			// Count factories for the company, compute rentability in each sector for each station
			for (int32 FactoryIndex = 0; FactoryIndex < StationDescription->Factories.Num(); FactoryIndex++)
			{
				FFlareFactoryDescription* FactoryDescription = &StationDescription->Factories[FactoryIndex]->Data;

				// Add weight if the company already have another station in this type
				float Score = ComputeConstructionScoreForStation(Sector, StationDescription, FactoryDescription, NULL, Technology);

				UpdateBestScore(Score, Sector, StationDescription, NULL, &BestScore, &BestStationDescription, &BestStation, &BestSector);
			}

			if (StationDescription->Factories.Num() == 0)
			{
				float Score = ComputeConstructionScoreForStation(Sector, StationDescription, NULL, NULL, Technology);
				UpdateBestScore(Score, Sector, StationDescription, NULL, &BestScore, &BestStationDescription, &BestStation, &BestSector);
			}
		}

		for (int32 StationIndex = 0; StationIndex < Sector->GetSectorStations().Num(); StationIndex++)
		{
			UFlareSimulatedSpacecraft* Station = Sector->GetSectorStations()[StationIndex];
			if (Station->GetCompany() != Company)
			{
				// Only AI company station
				continue;
			}

			if (GetGame()->GetQuestManager()->IsTradeQuestUseStation(Station))
			{
				// Do not update stations used by quests
				continue;
			}

			if (Station->GetLevel() >= Station->GetDescription()->MaxLevel)
			{
				continue;
			}

			int32 StationCountForThisKind = 0;
			int32 StationWithLowerLevelInSectorForThisKind = 0;
			for(UFlareSimulatedSpacecraft* StationCandidate : Company->GetCompanyStations())
			{
				if(Station->GetDescription() == StationCandidate->GetDescription())
				{
					StationCountForThisKind++;

					if(StationCandidate->GetLevel() < Station->GetLevel())
					{
						StationWithLowerLevelInSectorForThisKind++;
					}
				}
			}

			if(StationWithLowerLevelInSectorForThisKind > 0)
			{
				continue;
			}

			if (StationCountForThisKind < 2)
			{
				// Don't upgrade the only station the company have to avoid deadlock
				continue;
			}

			//FLOGV("> Analyse upgrade %s in %s", *Station->GetImmatriculation().ToString(), *Sector->GetSectorName().ToString());

			// Count factories for the company, compute rentability in each sector for each station
			for (int32 FactoryIndex = 0; FactoryIndex < Station->GetDescription()->Factories.Num(); FactoryIndex++)
			{
				FFlareFactoryDescription* FactoryDescription = &Station->GetDescription()->Factories[FactoryIndex]->Data;

				// Add weight if the company already have another station in this type
				float Score = ComputeConstructionScoreForStation(Sector, Station->GetDescription(), FactoryDescription, Station, Technology);

				UpdateBestScore(Score, Sector, Station->GetDescription(), Station, &BestScore, &BestStationDescription, &BestStation, &BestSector);
			}

			if (Station->GetDescription()->Factories.Num() == 0)
			{
				float Score = ComputeConstructionScoreForStation(Sector, Station->GetDescription(), NULL, Station, Technology);
				UpdateBestScore(Score, Sector, Station->GetDescription(), Station, &BestScore, &BestStationDescription, &BestStation, &BestSector);
			}

		}
	}

	if (BestSector && BestStationDescription)
	{
#ifdef DEBUG_AI_BUDGET
		FLOGV("UFlareCompanyAI::UpdateStationConstruction : %s >>> %s in %s (upgrade: %d) Score=%f",
			*Company->GetCompanyName().ToString(),
			*BestStationDescription->Name.ToString(),
			*BestSector->GetSectorName().ToString(),
			(BestStation != NULL),BestScore);
#endif
		float StationPrice = ComputeStationPrice(BestSector, BestStationDescription, BestStation);
		UFlareSimulatedSpacecraft* BuiltStation = NULL;
		TArray<FText> Reasons;
		if(BestStation)
		{
			if(BestSector->UpgradeStation(BestStation))
			{
				BuiltStation = BestStation;
			}
		}
		else if (BestSector->CanBuildStation(BestStationDescription, Company, Reasons))
		{
			BuiltStation = BestSector->BuildStation(BestStationDescription, Company);
		}

		if (BuiltStation)
		{

#ifdef DEBUG_AI_BUDGET
			FLOG("Start construction");
#endif

			SpendBudget(Technology ? EFlareBudget::Technology : EFlareBudget::Station, StationPrice);

			GameLog::AIConstructionStart(Company, BestSector, BestStationDescription, BestStation);
		}

		if (StationPrice < BudgetAmount)
		{
			Lock = true;
		}
	}
	else
	{
		Idle = true;
	}
}

int64 UFlareCompanyAI::UpdateCargoShipAcquisition()
{
	// For the transport pass, the best ship is choose. The best ship is the one with the small capacity, but
	// only if the is no more then the AI_CARGO_DIVERSITY_THERESOLD


	// Check if a ship is building
	if (IsBuildingShip(false))
	{
		//FLOG("IsBuildingShip");
		return 0;
	}

	const FFlareSpacecraftDescription* ShipDescription = FindBestShipToBuild(false);
	if (ShipDescription == NULL)
	{
		//FLOG("Find no ship to build");
		return 0;
	}

	return OrderOneShip(ShipDescription);
}

int64 UFlareCompanyAI::UpdateWarShipAcquisition(bool limitToOne)
{
	// For the war pass there is 2 states : slow preventive ship buy. And war state.
	//
	// - In the first state, the company will limit his army to a percentage of his value.
	//   It will create only one ship at once
	// - In the second state, it is war, the company will limit itself to de double of the
	//   army value of all enemies and buy as many ship it can.


	// Check if a ship is building
	if (limitToOne && IsBuildingShip(true))
	{
		return 0;
	}

	const FFlareSpacecraftDescription* ShipDescription = FindBestShipToBuild(true);

	return OrderOneShip(ShipDescription);
}


/*----------------------------------------------------
	Military AI
----------------------------------------------------*/


void UFlareCompanyAI::UpdateMilitaryMovement()
{
	SCOPE_CYCLE_COUNTER(STAT_FlareCompanyAI_UpdateMilitaryMovement);

	if (Company->AtWar())
	{
		UpdateWarMilitaryMovement();
	}
	else
	{
		UpdatePeaceMilitaryMovement();
	}
}

void UFlareCompanyAI::CheckBattleResolution()
{
#ifdef DEBUG_AI_BATTLE_STATES
			FLOGV("CheckBattleResolution for %s : %d sector with battle",
				*Company->GetCompanyName().ToString(),
				SectorWithBattle.Num());
#endif

	for (UFlareSimulatedSector* Sector : SectorWithBattle)
	{
		bool BattleLost = false;
		bool BattleWin = false;
		FFlareSectorBattleState BattleState =  Sector->GetSectorBattleState(Company);
		if (BattleState.InBattle)
		{
			if (BattleState.InFight)
			{
				// Still no winner
			}
			else if (BattleState.BattleWon)
			{
				BattleWin = true;
			}
			else
			{
				BattleLost = true;
			}
		}
		else
		{
			if (BattleState.HasDanger)
			{
				BattleLost = true;
			}
			else
			{
				BattleWin = true;
			}
		}

		if (BattleLost)
		{
			AIData.Caution += Behavior->DefeatAdaptation;
#ifdef DEBUG_AI_BATTLE_STATES
			FLOGV("%s lost battle in %s, new caution is %f",
				*Company->GetCompanyName().ToString(), *Sector->GetSectorName().ToString(),
				AIData.Caution);
#endif

		}

		if (BattleWin)
		{
			AIData.Caution -= Behavior->DefeatAdaptation;
#ifdef DEBUG_AI_BATTLE_STATES
			FLOGV("%s win battle in %s, new caution is %f",
				*Company->GetCompanyName().ToString(), *Sector->GetSectorName().ToString(),
				AIData.Caution);
#endif
		}
	}
}


void UFlareCompanyAI::CheckBattleState()
{
	SectorWithBattle.Empty();

	for (UFlareSimulatedSector* Sector : Company->GetKnownSectors())
	{
		FFlareSectorBattleState BattleState =  Sector->GetSectorBattleState(Company);
		if (BattleState.InFight)
		{
			SectorWithBattle.Add(Sector);
		}
	}
}

bool UFlareCompanyAI::HasHealthyTradeFleet() const
{
	int32 IncapacitatedCargosCount = FindIncapacitatedCargos().Num();

	if (AI_CARGO_HEALTHY_THRESHOLD * IncapacitatedCargosCount > Company->GetCompanyShips().Num())
	{
		return false;
	}
	else
	{
		return true;
	}
}

TArray<WarTargetIncomingFleet> UFlareCompanyAI::GenerateWarTargetIncomingFleets(AIWarContext& WarContext, UFlareSimulatedSector* DestinationSector)
{
	TArray<WarTargetIncomingFleet> IncomingFleetList;

	for (UFlareTravel* Travel : Game->GetGameWorld()->GetTravels())
	{
		if (Travel->GetDestinationSector() != DestinationSector)
		{
			continue;
		}

		if (!WarContext.Allies.Contains(Travel->GetFleet()->GetFleetCompany()))
		{
			continue;
		}

		int64 TravelDuration = Travel->GetRemainingTravelDuration();
		int32 ArmyCombatPoints = 0;


		for (UFlareSimulatedSpacecraft* Ship : Travel->GetFleet()->GetShips())
		{
			ArmyCombatPoints += Ship->GetCombatPoints(true);
		}

		// Add an entry or modify one
		bool ExistingTravelFound = false;
		for (WarTargetIncomingFleet& Fleet : IncomingFleetList)
		{
			if (Fleet.TravelDuration  == TravelDuration)
			{
				Fleet.ArmyCombatPoints += ArmyCombatPoints;
				ExistingTravelFound = true;
				break;
			}
		}

		if (!ExistingTravelFound)
		{
			WarTargetIncomingFleet Fleet;
			Fleet.TravelDuration = TravelDuration;
			Fleet.ArmyCombatPoints = ArmyCombatPoints;
			IncomingFleetList.Add(Fleet);
		}
	}
	return IncomingFleetList;
}

inline static bool WarTargetComparator(const WarTarget& ip1, const WarTarget& ip2)
{
	bool SELECT_TARGET1 = true;
	bool SELECT_TARGET2 = false;

	if (ip1.EnemyArmyCombatPoints > 0 && ip2.EnemyArmyCombatPoints == 0)
	{
		return SELECT_TARGET1;
	}

	if (ip2.EnemyArmyCombatPoints > 0 && ip1.EnemyArmyCombatPoints == 0)
	{
		return SELECT_TARGET2;
	}

	if (ip1.EnemyArmyCombatPoints > 0 && ip2.EnemyArmyCombatPoints > 0)
	{
		// Defend military
		if (ip1.OwnedMilitaryCount > ip2.OwnedMilitaryCount)
		{
			return SELECT_TARGET1;
		}

		if (ip2.OwnedMilitaryCount > ip1.OwnedMilitaryCount)
		{
			return SELECT_TARGET2;
		}

		// Defend station
		if (ip1.OwnedStationCount > ip2.OwnedStationCount)
		{
			return SELECT_TARGET1;
		}

		if (ip2.OwnedStationCount > ip1.OwnedStationCount)
		{
			return SELECT_TARGET2;
		}

		// Defend cargo
		if (ip1.OwnedCargoCount > ip2.OwnedCargoCount)
		{
			return SELECT_TARGET1;
		}

		if (ip2.OwnedCargoCount > ip1.OwnedCargoCount)
		{
			return SELECT_TARGET2;
		}

		return ip1.EnemyArmyCombatPoints > ip2.EnemyArmyCombatPoints;
	}


	// Cargo or station
	return FMath::RandBool();
}


TArray<WarTarget> UFlareCompanyAI::GenerateWarTargetList(AIWarContext& WarContext)
{
	TArray<WarTarget> WarTargetList;

	for (UFlareSimulatedSector* Sector : WarContext.KnownSectors)
	{
		bool IsTarget = false;

		if (Sector->GetSectorBattleState(Company).HasDanger)
		{
			IsTarget = true;
		}

		for (UFlareSimulatedSpacecraft* Spacecraft : Sector->GetSectorSpacecrafts())
		{
			if (!Spacecraft->IsStation() && !Spacecraft->IsMilitary() && Spacecraft->GetDamageSystem()->IsUncontrollable())
			{
				// Don't target uncontrollable ships
				continue;
			}

			if (WarContext.Enemies.Contains(Spacecraft->GetCompany()))
			{
				IsTarget = true;
				break;
			}
		}

		if (!IsTarget)
		{
			continue;
		}

		WarTarget Target;
		Target.Sector = Sector;
		Target.EnemyArmyCombatPoints = 0;
		Target.EnemyArmyLCombatPoints = 0;
		Target.EnemyArmySCombatPoints = 0;
		Target.EnemyCargoCount = 0;
		Target.EnemyStationCount = 0;
		Target.OwnedArmyCombatPoints = 0;
		Target.OwnedArmyAntiSCombatPoints = 0;
		Target.OwnedArmyAntiLCombatPoints = 0;
		Target.OwnedCargoCount = 0;
		Target.OwnedStationCount = 0;
		Target.OwnedMilitaryCount = 0;
		Target.WarTargetIncomingFleets = GenerateWarTargetIncomingFleets(WarContext, Sector);


		for (UFlareSimulatedSpacecraft* Spacecraft : Sector->GetSectorSpacecrafts())
		{
			if (WarContext.Enemies.Contains(Spacecraft->GetCompany()))
			{
				if (Spacecraft->IsStation())
				{
					Target.EnemyStationCount++;
				}
				else
				{
					if (Spacecraft->IsMilitary())
					{
						int32 ShipCombatPoints = Spacecraft->GetCombatPoints(true);
						Target.EnemyArmyCombatPoints += ShipCombatPoints;

						if (Spacecraft->GetSize() == EFlarePartSize::L)
						{
							Target.EnemyArmyLCombatPoints += ShipCombatPoints;
						}
						else
						{
							Target.EnemyArmySCombatPoints += ShipCombatPoints;
						}

						if(ShipCombatPoints > 0)
						{
							Target.ArmedDefenseCompanies.AddUnique(Spacecraft->GetCompany());
						}
					}
					else
					{
						Target.EnemyCargoCount++;
					}
				}
			}
			else if (WarContext.Allies.Contains(Spacecraft->GetCompany()))
			{
				if (Spacecraft->IsStation())
				{
					Target.OwnedStationCount++;
				}
				else
				{
					if (Spacecraft->IsMilitary())
					{
						int32 ShipCombatPoints= Spacecraft->GetCombatPoints(true);

						Target.OwnedArmyCombatPoints += ShipCombatPoints;
						Target.OwnedMilitaryCount++;

						if (Spacecraft->GetWeaponsSystem()->HasAntiLargeShipWeapon())
						{
							Target.OwnedArmyAntiLCombatPoints += ShipCombatPoints;
						}

						if (Spacecraft->GetWeaponsSystem()->HasAntiSmallShipWeapon())
						{
							Target.OwnedArmyAntiSCombatPoints += ShipCombatPoints;
						}
					}
					else
					{
						Target.OwnedCargoCount++;
					}
				}
			}
		}


		if (Target.OwnedArmyAntiLCombatPoints <= Target.EnemyArmyLCombatPoints * Behavior->RetreatThreshold ||
				Target.OwnedArmyAntiSCombatPoints <= Target.EnemyArmySCombatPoints * Behavior->RetreatThreshold)
		{
			WarTargetList.Add(Target);

			// Retreat
			TArray<UFlareSimulatedSpacecraft*> MovableShips = GenerateWarShipList(WarContext, Target.Sector);
			if (MovableShips.Num() > 0 && Target.EnemyArmyCombatPoints > 0)
			{
				// Find nearest sector without danger with available FS and travel there
				UFlareSimulatedSector* RetreatSector = FindNearestSectorWithFS(WarContext, Target.Sector);
				if (!RetreatSector)
				{
					RetreatSector = FindNearestSectorWithPeace(WarContext, Target.Sector);
				}

				if (RetreatSector)
				{
					for (UFlareSimulatedSpacecraft* Ship : MovableShips)
					{
						FLOGV("UpdateWarMilitaryMovement %s : move %s from %s to %s for retreat",
							*Company->GetCompanyName().ToString(),
							*Ship->GetImmatriculation().ToString(),
							*Ship->GetCurrentSector()->GetSectorName().ToString(),
							*RetreatSector->GetSectorName().ToString());

						Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), RetreatSector);
					}
				}
			}
		}
	}

	WarTargetList.Sort(&WarTargetComparator);

	return WarTargetList;
}

TArray<DefenseSector> UFlareCompanyAI::GenerateDefenseSectorList(AIWarContext& WarContext)
{
	TArray<DefenseSector> DefenseSectorList;

	for (UFlareSimulatedSector* Sector : WarContext.KnownSectors)
	{
		FFlareSectorBattleState BattleState = Sector->GetSectorBattleState(Company);

		if (BattleState.HasDanger)
		{
			continue;
		}

		DefenseSector Target;
		Target.Sector = Sector;
		Target.CombatPoints = 0;
		Target.ArmyAntiSCombatPoints = 0;
		Target.ArmyAntiLCombatPoints = 0;
		Target.ArmyLargeShipCombatPoints = 0;
		Target.ArmySmallShipCombatPoints = 0;
		Target.LargeShipArmyCount = 0;
		Target.SmallShipArmyCount = 0;
		Target.PrisonersKeeper = NULL;

		if(BattleState.BattleWon)
		{
			// Keep prisoners
			int32 MinCombatPoints = MAX_int32;

			for (UFlareSimulatedSpacecraft* Ship : Sector->GetSectorShips())
			{
				int32 ShipCombatPoints= Ship->GetCombatPoints(true);

				if (!WarContext.Allies.Contains(Ship->GetCompany())
				 || Ship->CanTravel() == false
				 || ShipCombatPoints == 0)
				{
					continue;
				}

				if (ShipCombatPoints < MinCombatPoints)
				{
					MinCombatPoints = ShipCombatPoints;
					Target.PrisonersKeeper = Ship;
				}
			}
		}

		for (UFlareSimulatedSpacecraft* Ship : Sector->GetSectorShips())
		{
			int32 ShipCombatPoints= Ship->GetCombatPoints(true);

			if (!WarContext.Allies.Contains(Ship->GetCompany())
			 || Ship->CanTravel() == false
			 || ShipCombatPoints == 0
			 || Ship == Target.PrisonersKeeper)
			{
				continue;
			}
			
			Target.CombatPoints += ShipCombatPoints;
			if (Ship->GetSize() == EFlarePartSize::L)
			{
				Target.ArmyLargeShipCombatPoints += ShipCombatPoints;
				Target.LargeShipArmyCount++;
			}
			else
			{
				Target.ArmySmallShipCombatPoints += ShipCombatPoints;
				Target.SmallShipArmyCount++;
			}

			if (Ship->GetWeaponsSystem()->HasAntiLargeShipWeapon())
			{
				Target.ArmyAntiLCombatPoints += ShipCombatPoints;
			}

			if (Ship->GetWeaponsSystem()->HasAntiSmallShipWeapon())
			{
				Target.ArmyAntiSCombatPoints += ShipCombatPoints;
			}
		}

		Target.CapturingStation = false;
		TArray<UFlareSimulatedSpacecraft*>& Stations =  Sector->GetSectorStations();


		if(Company->GetCaptureOrderCountInSector(Sector) > 0)
		{
			Target.CapturingStation = true;
		}
		else
		{
			for (UFlareSimulatedSpacecraft* Station : Stations)
			{
				// Capturing station
				if (WarContext.Enemies.Contains(Station->GetCompany()) && Company->CanStartCapture(Station))
				{
					Target.CapturingStation = true;
					break;
				}
			}

		}



		if (Target.CombatPoints > 0 || Target.PrisonersKeeper != nullptr)
		{
			DefenseSectorList.Add(Target);
		}
	}

	return DefenseSectorList;
}

TArray<UFlareSimulatedSpacecraft*> UFlareCompanyAI::GenerateWarShipList(AIWarContext& WarContext, UFlareSimulatedSector* Sector, UFlareSimulatedSpacecraft* ExcludeShip)
{
	TArray<UFlareSimulatedSpacecraft*> WarShips;

	for (UFlareSimulatedSpacecraft* Ship : Sector->GetSectorShips())
	{
		if (WarContext.Allies.Contains(Ship->GetCompany())
		 &&  Ship->CanTravel()
		 && !Ship->GetDamageSystem()->IsDisarmed()
		 && !Ship->GetDamageSystem()->IsStranded()
		 && Ship != ExcludeShip)
		{
			WarShips.Add(Ship);
		}
	}

	return WarShips;
}

int64 UFlareCompanyAI::GetDefenseSectorTravelDuration(TArray<DefenseSector>& DefenseSectorList, const DefenseSector& OriginSector)
{
	int64 MaxTravelDuration = 0;

	for (DefenseSector& Sector : DefenseSectorList)
	{
		int64 TravelDuration = UFlareTravel::ComputeTravelDuration(GetGame()->GetGameWorld(), OriginSector.Sector, Sector.Sector, Company);
		if (TravelDuration > MaxTravelDuration)
		{
			MaxTravelDuration = TravelDuration;
		}
	}

	return MaxTravelDuration;
}

TArray<DefenseSector> UFlareCompanyAI::GetDefenseSectorListInRange(TArray<DefenseSector>& DefenseSectorList, const DefenseSector& OriginSector, int64 MaxTravelDuration)
{
	TArray<DefenseSector> Sectors;

	for (DefenseSector& Sector : DefenseSectorList)
	{
		if (Sector.Sector == OriginSector.Sector)
		{
			continue;
		}

		int64 TravelDuration = UFlareTravel::ComputeTravelDuration(GetGame()->GetGameWorld(), OriginSector.Sector, Sector.Sector, Company);
		if (TravelDuration <= MaxTravelDuration)
		{
			Sectors.Add(Sector);
		}
	}

	return Sectors;
}

inline static bool SectorDefenseDistanceComparator(const DefenseSector& ip1, const DefenseSector& ip2)
{
	int64 ip1TravelDuration = UFlareTravel::ComputeTravelDuration(ip1.Sector->GetGame()->GetGameWorld(), ip1.TempBaseSector, ip1.Sector, NULL);
	int64 ip2TravelDuration = UFlareTravel::ComputeTravelDuration(ip1.Sector->GetGame()->GetGameWorld(), ip2.TempBaseSector, ip2.Sector, NULL);

	return (ip1TravelDuration < ip2TravelDuration);
}

TArray<DefenseSector> UFlareCompanyAI::SortSectorsByDistance(UFlareSimulatedSector* BaseSector, TArray<DefenseSector> SectorsToSort)
{
	for (DefenseSector& Sector : SectorsToSort)
	{
		Sector.TempBaseSector = BaseSector;
	}

	SectorsToSort.Sort(&SectorDefenseDistanceComparator);
	return SectorsToSort;
}

void AIWarContext::Generate()
{
	float AttackThresholdSum = 0;
	float AttackThresholdCount = 0;

	for (UFlareCompany* Ally :  Allies)
	{
		for(UFlareSimulatedSector* Sector: Ally->GetKnownSectors())
		{
			KnownSectors.AddUnique(Sector);
		}

		Ally->GetAI()->GetBehavior()->Load(Ally);

		AttackThresholdSum += Ally->GetAI()->GetBehavior()->GetAttackThreshold();
		AttackThresholdCount++;
	}

	AttackThreshold = AttackThresholdSum/AttackThresholdCount;
}

void UFlareCompanyAI::UpdateWarMilitaryMovement()
{

	auto GenerateAlliesCode = [&](UFlareCompany* iCompany)
	{
		int32 AlliesCode = 0x0;
		int32 CompanyMask = 0x1;

		for(UFlareCompany* OtherCompany : Game->GetGameWorld()->GetCompanies())
		{
			if(iCompany->IsAtWar(OtherCompany))
			{
				AlliesCode |= CompanyMask;
			}

			CompanyMask = CompanyMask<<1;
		}

		return AlliesCode;
	};

	auto GenerateAlliesList = [&]() {
		TArray<UFlareCompany*> Allies;
		Allies.Add(Company);
		int32 AlliesCode = GenerateAlliesCode(Company);

		for (UFlareCompany* OtherCompany: Company->GetOtherCompanies())
		{
			if(OtherCompany->IsPlayerCompany())
			{
				continue;
			}

			if(GenerateAlliesCode(OtherCompany) == AlliesCode)
			{
				Allies.Add(OtherCompany);
			}
		}

		return Allies;
	};

	auto GenerateEnemiesList = [&]() {
		TArray<UFlareCompany*> Enemies;

		for (UFlareCompany* OtherCompany: Company->GetOtherCompanies())
		{
			if(Company->IsAtWar(OtherCompany))
			{
				Enemies.Add(OtherCompany);
			}
		}

		return Enemies;
	};

	AIWarContext WarContext;

	WarContext.Allies = GenerateAlliesList();
	WarContext.Enemies = GenerateEnemiesList();
	WarContext.Generate();

	TArray<WarTarget> TargetList = GenerateWarTargetList(WarContext);
	TArray<DefenseSector> DefenseSectorList = GenerateDefenseSectorList(WarContext);

#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
	FLOGV("UpdateWarMilitaryMovement for %s : Target count %d, Defense sector count %d",
		*Company->GetCompanyName().ToString(),
		TargetList.Num(),
		DefenseSectorList.Num());
#endif

	// Manage attacking fleets
	for (WarTarget& Target : TargetList)
	{
		TArray<DefenseSector> SortedDefenseSectorList = SortSectorsByDistance(Target.Sector, DefenseSectorList);
		for (DefenseSector& Sector : SortedDefenseSectorList)
		{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("check attack from %s to %s : CombatPoints=%d, EnemyArmyCombatPoints=%d",
					*Sector.Sector->GetSectorName().ToString(),
					*Target.Sector->GetSectorName().ToString(),
					 Sector.CombatPoints, Target.EnemyArmyCombatPoints);
#endif

			// Check if the army is strong enough
			if (Sector.CombatPoints < Target.EnemyArmyCombatPoints * WarContext.AttackThreshold)
			{
				// Army too weak
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("army at %s too weak to attack %s : CombatPoints = %lld, EnemyArmyCombatPoints = %lld ",
					*Sector.Sector->GetSectorName().ToString(),
					*Target.Sector->GetSectorName().ToString(),
					Sector.CombatPoints, Target.EnemyArmyCombatPoints);
#endif

				continue;
			}

			if (Target.EnemyArmyCombatPoints == 0 && Sector.CapturingStation)
			{
				// Capturing station, don't move
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("army at %s won't move to attack %s : capturing station",
					*Sector.Sector->GetSectorName().ToString(),
					*Target.Sector->GetSectorName().ToString());
#endif

				continue;
			}

			// Check if there is an incomming fleet bigger than local
			bool DefenseFleetFound = false;
			int64 TravelDuration = UFlareTravel::ComputeTravelDuration(GetGame()->GetGameWorld(), Sector.Sector, Target.Sector, Company);
			for (WarTargetIncomingFleet& Fleet : Target.WarTargetIncomingFleets)
			{
				// Incoming fleet will be late, ignore it
				if (Fleet.TravelDuration > TravelDuration)
				{
					continue;
				}

				// Incoming fleet is too weak, ignore it
				else if (Fleet.ArmyCombatPoints <  Target.EnemyArmyCombatPoints * WarContext.AttackThreshold)
				{
					continue;
				}

				DefenseFleetFound = true;
				break;
			}

			// Defense already incomming
			if (DefenseFleetFound)
			{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("army at %s won't move to attack %s : another fleet is coming",
					*Sector.Sector->GetSectorName().ToString(),
					*Target.Sector->GetSectorName().ToString());
#endif
				continue;
			}

			// Should go defend ! Assemble a fleet
			int32 AntiLFleetCombatPoints = 0;
			int32 AntiSFleetCombatPoints = 0;
			int32 AntiLFleetCombatPointsLimit = Target.EnemyArmyLCombatPoints * WarContext.AttackThreshold * 1.5;
			int32 AntiSFleetCombatPointsLimit = Target.EnemyArmySCombatPoints * WarContext.AttackThreshold * 1.5;
			TArray<UFlareSimulatedSpacecraft*> MovableShips = GenerateWarShipList(WarContext, Sector.Sector, Sector.PrisonersKeeper);


			// Check if weapon are optimal
			// Check if the army is strong enough
			if (Sector.ArmyAntiLCombatPoints < Target.EnemyArmyLCombatPoints * WarContext.AttackThreshold ||
					Sector.ArmyAntiSCombatPoints < Target.EnemyArmySCombatPoints * WarContext.AttackThreshold)
			{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("army at %s want to attack %s after upgrade : ArmyAntiLCombatPoints=%d, EnemyArmyLCombatPoints=%d, ArmyAntiSCombatPoints=%d, EnemyArmySCombatPoints=%d",
					*Sector.Sector->GetSectorName().ToString(),
					*Target.Sector->GetSectorName().ToString(),
					 Sector.ArmyAntiLCombatPoints, Target.EnemyArmyLCombatPoints,Sector.ArmyAntiSCombatPoints, Target.EnemyArmySCombatPoints);
#endif
				if (!UpgradeMilitaryFleet(WarContext, Target, Sector, MovableShips))
				{
					// cannot upgrade, don't attack
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("army at %s won't attack %s : upgrade failed",
					*Sector.Sector->GetSectorName().ToString(),
					*Target.Sector->GetSectorName().ToString());
#endif
					continue;
				}
			}

#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
			FLOGV("army at %s attack %s !",
				*Sector.Sector->GetSectorName().ToString(),
				*Target.Sector->GetSectorName().ToString());
#endif

			// Send random ships
			int32 MinShipToSend = FMath::Max(Target.EnemyCargoCount, Target.EnemyStationCount);
			int32 SentShips = 0;

#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
			FLOGV("- MovableShips= %d",MovableShips.Num());
			FLOGV("- MinShipToSend= %d",MinShipToSend);
			FLOGV("- AntiLFleetCombatPointsLimit= %d",AntiLFleetCombatPointsLimit);
			FLOGV("- AntiSFleetCombatPointsLimit= %d",AntiSFleetCombatPointsLimit);
#endif

			while (MovableShips.Num() > 0 &&
				   ((SentShips < MinShipToSend) || (AntiLFleetCombatPoints < AntiLFleetCombatPointsLimit || AntiSFleetCombatPoints < AntiSFleetCombatPointsLimit)))
			{
				int32 ShipIndex = FMath::RandRange(0, MovableShips.Num()-1);

				UFlareSimulatedSpacecraft* SelectedShip = MovableShips[ShipIndex];
				MovableShips.RemoveAt(ShipIndex);

				int32 ShipCombatPoints = SelectedShip->GetCombatPoints(true);
				if (SelectedShip->GetWeaponsSystem()->HasAntiLargeShipWeapon())
				{
					AntiLFleetCombatPoints += ShipCombatPoints;
					Sector.CombatPoints -= ShipCombatPoints;
					Sector.ArmyAntiLCombatPoints -= ShipCombatPoints;
				}
				else if (SelectedShip->GetWeaponsSystem()->HasAntiSmallShipWeapon())
				{
					AntiSFleetCombatPoints += ShipCombatPoints;
					Sector.CombatPoints -= ShipCombatPoints;
					Sector.ArmyAntiSCombatPoints -= ShipCombatPoints;
				}
				else
				{
					// Not useful don't travel
					continue;
				}

				FLOGV("UpdateWarMilitaryMovement %s : move %s from %s to %s for attack",
					*Company->GetCompanyName().ToString(),
					*SelectedShip->GetImmatriculation().ToString(),
					*SelectedShip->GetCurrentSector()->GetSectorName().ToString(),
					*Target.Sector->GetSectorName().ToString());
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT

				FLOGV("- AntiLFleetCombatPoints= %d",AntiLFleetCombatPoints);
				FLOGV("- AntiSFleetCombatPoints= %d",AntiSFleetCombatPoints);
#endif

				Game->GetGameWorld()->StartTravel(SelectedShip->GetCurrentFleet(), Target.Sector);
				SentShips++;
			}

			if (Sector.CombatPoints == 0)
			{
				DefenseSectorList.Remove(Sector);
			}
		}
	}

	// Manage remaining ships for defense
	for (DefenseSector& Sector : DefenseSectorList)
	{
		// Capturing station, don't move
		if (Sector.CapturingStation)
		{
			// Start capture
			for(UFlareSimulatedSpacecraft* Station: Sector.Sector->GetSectorStations())
			{
				Company->StartCapture(Station);
			}

#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
			FLOGV("army at %s won't move to defend: capturing station",
				*Sector.Sector->GetSectorName().ToString());
#endif
			continue;
		}

		// Check if need fleet supply
		int32 NeededFS;
		int32 TotalNeededFS;

		int32 CumulatedTotalNeededFS = 0;
		int64 MaxDuration;

		TArray<UFlareSimulatedSpacecraft*> MovableShips = GenerateWarShipList(WarContext, Sector.Sector, Sector.PrisonersKeeper);

		SectorHelper::GetRefillFleetSupplyNeeds(Sector.Sector, MovableShips, NeededFS, TotalNeededFS, MaxDuration);
		CumulatedTotalNeededFS += TotalNeededFS;

		SectorHelper::GetRepairFleetSupplyNeeds(Sector.Sector, MovableShips, NeededFS, TotalNeededFS, MaxDuration);
		CumulatedTotalNeededFS += TotalNeededFS;

		if (CumulatedTotalNeededFS > 0)
		{
			// FS need

			int32 AvailableFS;
			int32 OwnedFS;
			int32 AffordableFS;
			SectorHelper::GetAvailableFleetSupplyCount(Sector.Sector, Company, OwnedFS, AvailableFS, AffordableFS);

			if (AvailableFS == 0)
			{
				// Find nearest sector without danger with available FS and travel there
				UFlareSimulatedSector* RepairSector = FindNearestSectorWithFS(WarContext, Sector.Sector);
				if (RepairSector)
				{
					for (UFlareSimulatedSpacecraft* Ship : MovableShips)
					{
						FLOGV("UpdateWarMilitaryMovement %s : move %s from %s to %s for repair/refill",
							*Company->GetCompanyName().ToString(),
							*Ship->GetImmatriculation().ToString(),
							*Ship->GetCurrentSector()->GetSectorName().ToString(),
							*RepairSector->GetSectorName().ToString());

						Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), RepairSector);
					}

					continue;
				}
			}
		}

		int64 MaxTravelDuration = GetDefenseSectorTravelDuration(DefenseSectorList, Sector);
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("army at %s MaxTravelDuration=%lld",
				*Sector.Sector->GetSectorName().ToString(), MaxTravelDuration);
#endif

		for (int32 TravelDuration = 1; TravelDuration <= MaxTravelDuration; TravelDuration++)
		{
			TArray<DefenseSector> DefenseSectorListInRange = GetDefenseSectorListInRange(DefenseSectorList, Sector, TravelDuration);

#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("army at %s DefenseSectorListInRange(%d)=%d",
				*Sector.Sector->GetSectorName().ToString(), TravelDuration, DefenseSectorListInRange.Num());
#endif

			if (DefenseSectorListInRange.Num() == 0)
			{
				continue;
			}

			// Find bigger
			DefenseSector StrongestSector;
			StrongestSector.Sector = NULL;
			StrongestSector.CombatPoints = 0;
			for (DefenseSector& DistantSector : DefenseSectorListInRange)
			{
				if (!StrongestSector.Sector || StrongestSector.CombatPoints < DistantSector.CombatPoints)
				{
					StrongestSector = DistantSector;
				}

			}

			if (StrongestSector.CombatPoints > Sector.CombatPoints)
			{
				// There is a stronger sector, travel here if no incoming army before
				bool IncomingFleet = false;
				TArray<WarTargetIncomingFleet> WarTargetIncomingFleets = GenerateWarTargetIncomingFleets(WarContext, StrongestSector.Sector);
				for (WarTargetIncomingFleet& Fleet : WarTargetIncomingFleets)
				{
					if (Fleet.TravelDuration <= TravelDuration)
					{
						IncomingFleet = true;
						break;
					}
				}

				// Wait incoming fleets
				if (IncomingFleet)
				{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("army at %s won't move to defend: incoming fleet",
				*Sector.Sector->GetSectorName().ToString());
#endif
					break;
				}
				
				// Send ships
				TArray<UFlareSimulatedSpacecraft*> StillMovableShips = GenerateWarShipList(WarContext, Sector.Sector, Sector.PrisonersKeeper);
				for (UFlareSimulatedSpacecraft* Ship : StillMovableShips)
				{
					FLOGV("UpdateWarMilitaryMovement%s : move %s from %s to %s for defense",
						*Company->GetCompanyName().ToString(),
						*Ship->GetImmatriculation().ToString(),
						*Ship->GetCurrentSector()->GetSectorName().ToString(),
						*StrongestSector.Sector->GetSectorName().ToString());

					Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), StrongestSector.Sector);
				}

				Sector.CombatPoints = 0;
			}
			else
			{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("army at %s won't move to defend at %s: army is weaker (StrongestSector.CombatPoints=%d Sector.CombatPoints=%d)",
				*Sector.Sector->GetSectorName().ToString(),
				*StrongestSector.Sector->GetSectorName().ToString(),
				StrongestSector.CombatPoints, Sector.CombatPoints);
#endif
			}


			break;
		}
	}
}

UFlareSimulatedSector* UFlareCompanyAI::FindNearestSectorWithPeace(AIWarContext& WarContext, UFlareSimulatedSector* OriginSector)
{
	UFlareSimulatedSector* NearestSector = NULL;
	int64 NearestDuration = 0;

	for (UFlareSimulatedSector* Sector : WarContext.KnownSectors)
	{
		FFlareSectorBattleState BattleState = Sector->GetSectorBattleState(Company);
		if (BattleState.HasDanger)
		{
			continue;
		}

		int64 Duration = UFlareTravel::ComputeTravelDuration(Sector->GetGame()->GetGameWorld(), OriginSector, Sector, Company);

		if (!NearestSector || Duration < NearestDuration)
		{
			NearestSector = Sector;
			NearestDuration = Duration;
		}

	}
	return NearestSector;
}

UFlareSimulatedSector* UFlareCompanyAI::FindNearestSectorWithFS(AIWarContext& WarContext, UFlareSimulatedSector* OriginSector)
{
	UFlareSimulatedSector* NearestSector = NULL;
	int64 NearestDuration = 0;

	for (UFlareSimulatedSector* Sector : WarContext.KnownSectors)
	{
		FFlareSectorBattleState BattleState = Sector->GetSectorBattleState(Company);
		if (BattleState.HasDanger)
		{
			continue;
		}

		int32 AvailableFS;
		int32 OwnedFS;
		int32 AffordableFS;

		SectorHelper::GetAvailableFleetSupplyCount(Sector, Company, OwnedFS, AvailableFS, AffordableFS);

		if (AvailableFS == 0)
		{
			continue;
		}

		int64 Duration = UFlareTravel::ComputeTravelDuration(Sector->GetGame()->GetGameWorld(), OriginSector, Sector, Company);

		if (!NearestSector || Duration < NearestDuration)
		{
			NearestSector = Sector;
			NearestDuration = Duration;
		}

	}
	return NearestSector;
}

UFlareSimulatedSector* UFlareCompanyAI::FindNearestSectorWithUpgradePossible(AIWarContext& WarContext, UFlareSimulatedSector* OriginSector)
{
	UFlareSimulatedSector* NearestSector = NULL;
	int64 NearestDuration = 0;

	for (UFlareSimulatedSector* Sector : WarContext.KnownSectors)
	{
		if (!Sector->CanUpgrade(Company))
		{
			continue;
		}

		int64 Duration = UFlareTravel::ComputeTravelDuration(Sector->GetGame()->GetGameWorld(), OriginSector, Sector, Company);

		if (!NearestSector || Duration < NearestDuration)
		{
			NearestSector = Sector;
			NearestDuration = Duration;
		}

	}
	return NearestSector;
}

bool UFlareCompanyAI::UpgradeShip(UFlareSimulatedSpacecraft* Ship, EFlarePartSize::Type WeaponTargetSize)
{
	UFlareSpacecraftComponentsCatalog* Catalog = Game->GetPC()->GetGame()->GetShipPartsCatalog();

	// Upgrade weapon
	if (!Ship->CanUpgrade(EFlarePartType::Weapon))
	{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("failed to upgrade %s : cannot upgrade weapons",
					*Ship->GetImmatriculation().ToString());
#endif
		return false;
	}

	// iterate to find best weapon
	{
		FFlareSpacecraftComponentDescription* BestWeapon = NULL;
		TArray< FFlareSpacecraftComponentDescription* > PartListData;
		Catalog->GetWeaponList(PartListData, Ship->GetSize());

		for (FFlareSpacecraftComponentDescription* Part : PartListData)
		{
			if (!Ship->GetCompany()->IsTechnologyUnlockedPart(Part))
			{
				continue;
			}

			if (!Part->WeaponCharacteristics.IsWeapon)
			{
				continue;
			}

			if (WeaponTargetSize == EFlarePartSize::L && Part->WeaponCharacteristics.AntiLargeShipValue < 0.5)
			{
				continue;
			}

			if (WeaponTargetSize == EFlarePartSize::S && Part->WeaponCharacteristics.AntiSmallShipValue < 0.5)
			{
				continue;
			}


			// Compatible target
			bool HasChance = FMath::FRand() < 0.7;
			if (!BestWeapon || (BestWeapon->Cost < Part->Cost && HasChance))
			{
				BestWeapon = Part;
			}

		}

		if (BestWeapon)
		{
			int32 WeaponGroupCount = Ship->GetDescription()->WeaponGroups.Num();
			int64 TransactionCost = 0;
			for (int32 WeaponGroupIndex = 0; WeaponGroupIndex < WeaponGroupCount; WeaponGroupIndex++)
			{
				FFlareSpacecraftComponentDescription* OldWeapon = Ship->GetCurrentPart(EFlarePartType::Weapon, WeaponGroupIndex);
				TransactionCost += Ship->GetUpgradeCost(BestWeapon, OldWeapon);
			}

			if (TransactionCost > Ship->GetCompany()->GetMoney())
			{
				// Cannot afford
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("failed to upgrade %s : cannot afford upgrade",
					*Ship->GetImmatriculation().ToString());
#endif
				return false;
			}
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
			FLOGV("%s upgrade its weapons to %s", *Ship->GetImmatriculation().ToString(), *BestWeapon->Identifier.ToString());
#endif
			for (int32 WeaponGroupIndex = 0; WeaponGroupIndex < WeaponGroupCount; WeaponGroupIndex++)
			{
				if (Ship->UpgradePart(BestWeapon, WeaponGroupIndex))
				{
					Ship->GetCompany()->GetAI()->SpendBudget(EFlareBudget::Military, TransactionCost);
				}
				else
				{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
					FLOGV("failed to upgrade %s : upgrade failed for unknown reseon",
						*Ship->GetImmatriculation().ToString());
#endif

					// Something fail
					return false;
				}
			}
		}
	}

	// Chance to upgrade rcs (optional)
	if (FMath::RandBool() && Ship->CanUpgrade(EFlarePartType::RCS)) // 50 % chance
	{
		// iterate to find best par
		FFlareSpacecraftComponentDescription* OldPart = Ship->GetCurrentPart(EFlarePartType::RCS, 0);
		FFlareSpacecraftComponentDescription* BestPart = OldPart;
		TArray< FFlareSpacecraftComponentDescription* > PartListData;
		Catalog->GetRCSList(PartListData, Ship->GetSize());

		for (FFlareSpacecraftComponentDescription* Part : PartListData)
		{
			bool HasChance = FMath::RandBool();
			if (!BestPart || (BestPart->Cost < Part->Cost && HasChance))
			{
				BestPart = Part;
			}
		}

		if (BestPart != OldPart)
		{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
			FLOGV("%s upgrade its rcs to %s", *Ship->GetImmatriculation().ToString(), *BestPart->Identifier.ToString());
#endif
			if (Ship->UpgradePart(BestPart, 0))
			{
				int64 TransactionCost = Ship->GetUpgradeCost(BestPart, OldPart);
				SpendBudget(EFlareBudget::Military, TransactionCost);
			}
		}
	}

	// Chance to upgrade pod (optional)
	if (FMath::RandBool() && Ship->CanUpgrade(EFlarePartType::OrbitalEngine)) // 50 % chance
	{
		// iterate to find best par
		FFlareSpacecraftComponentDescription* OldPart = Ship->GetCurrentPart(EFlarePartType::OrbitalEngine, 0);
		FFlareSpacecraftComponentDescription* BestPart = OldPart;
		TArray< FFlareSpacecraftComponentDescription* > PartListData;
		Catalog->GetEngineList(PartListData, Ship->GetSize());

		for (FFlareSpacecraftComponentDescription* Part : PartListData)
		{
			bool HasChance = FMath::RandBool();
			if (!BestPart || (BestPart->Cost < Part->Cost && HasChance))
			{
				BestPart = Part;
			}
		}

		if (BestPart != OldPart)
		{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
			FLOGV("%s upgrade its orbital engines to %s", *Ship->GetImmatriculation().ToString(), *BestPart->Identifier.ToString());
#endif
			if (Ship->UpgradePart(BestPart, 0))
			{
				int64 TransactionCost = Ship->GetUpgradeCost(BestPart, OldPart);
				SpendBudget(EFlareBudget::Military, TransactionCost);
			}
		}
	}

	return true;
}

bool UFlareCompanyAI::UpgradeMilitaryFleet(AIWarContext& WarContext,  WarTarget Target, DefenseSector& Sector, TArray<UFlareSimulatedSpacecraft*> &MovableShips)
{
	// First check if upgrade is possible in sector
	// If not, find the closest sector where upgrade is possible and travel here
	if (!Sector.Sector->CanUpgrade(Company))
	{
		UFlareSimulatedSector* UpgradeSector = FindNearestSectorWithUpgradePossible(WarContext, Sector.Sector);
		if (UpgradeSector)
		{
			for (UFlareSimulatedSpacecraft* Ship : MovableShips)
			{
				FLOGV("UpdateWarMilitaryMovement %s : move %s from %s to %s for upgrade",
					*Company->GetCompanyName().ToString(),
					*Ship->GetImmatriculation().ToString(),
					*Ship->GetCurrentSector()->GetSectorName().ToString(),
					*UpgradeSector->GetSectorName().ToString());

				Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), UpgradeSector);
			}
		}
		else
		{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("failed to find upgrade sector from %s",
					*Sector.Sector->GetSectorName().ToString());
#endif
		}
		return false;
	}

	// If upgrade possible
	bool UpgradeFailed = false;

	//for each L ship
	for (UFlareSimulatedSpacecraft* Ship : MovableShips)
	{
		if (Ship->GetSize() != EFlarePartSize::L)
		{
			// L only
			continue;
		}

		bool EnoughAntiL = Sector.ArmyAntiLCombatPoints >= Target.EnemyArmyLCombatPoints * WarContext.AttackThreshold;
		bool EnoughAntiS = Sector.ArmyAntiSCombatPoints >= Target.EnemyArmySCombatPoints * WarContext.AttackThreshold;
		int32 CombatPoints = Ship->GetCombatPoints(true);
		bool HasAntiLargeShipWeapon = Ship->GetWeaponsSystem()->HasAntiLargeShipWeapon();
		bool HasAntiSmallShipWeapon = Ship->GetWeaponsSystem()->HasAntiSmallShipWeapon();

		//if no enought anti L, upgrade to anti L
		if (!EnoughAntiL && !HasAntiLargeShipWeapon)
		{
			bool Upgraded = UpgradeShip(Ship, EFlarePartSize::L);
			if (Upgraded)
			{
				Sector.ArmyAntiLCombatPoints += CombatPoints;
				if (HasAntiSmallShipWeapon)
				{
					Sector.ArmyAntiSCombatPoints -= CombatPoints;
				}
			}
			else
			{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("failed to upgrade %s to anti L",
					*Ship->GetImmatriculation().ToString());
#endif
				UpgradeFailed = true;
			}
		}
		// if enought anti L and L ship value is not nesessary, upgrade to Anti S
		else if (!EnoughAntiS && HasAntiLargeShipWeapon)
		{
			bool EnoughAntiLWithoutShip = (Sector.ArmyAntiLCombatPoints - CombatPoints) >= Target.EnemyArmyLCombatPoints * WarContext.AttackThreshold;
			if (EnoughAntiLWithoutShip)
			{
				bool Upgraded = UpgradeShip(Ship, EFlarePartSize::S);
				if (Upgraded)
				{
					Sector.ArmyAntiSCombatPoints += CombatPoints;
					Sector.ArmyAntiLCombatPoints -= CombatPoints;
				}
				else
				{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("failed to upgrade %s to anti S",
					*Ship->GetImmatriculation().ToString());
#endif
					UpgradeFailed = true;
				}
			}
		}
	}

	//for each S ship
	for (UFlareSimulatedSpacecraft* Ship : MovableShips)
	{

		if (Ship->GetSize() != EFlarePartSize::S)
		{
			// S only
			continue;
		}

		bool EnoughAntiL = Sector.ArmyAntiLCombatPoints >= Target.EnemyArmyLCombatPoints * WarContext.AttackThreshold;
		bool EnoughAntiS = Sector.ArmyAntiSCombatPoints >= Target.EnemyArmySCombatPoints * WarContext.AttackThreshold;
		int32 CombatPoints = Ship->GetCombatPoints(true);
		bool HasAntiLargeShipWeapon = Ship->GetWeaponsSystem()->HasAntiLargeShipWeapon();
		bool HasAntiSmallShipWeapon = Ship->GetWeaponsSystem()->HasAntiSmallShipWeapon();

		//if no enought anti S, upgrade to anti S
		if (!EnoughAntiS && !HasAntiSmallShipWeapon)
		{
			bool Upgraded = UpgradeShip(Ship, EFlarePartSize::S);
			if (Upgraded)
			{
				Sector.ArmyAntiSCombatPoints += CombatPoints;
				if (HasAntiLargeShipWeapon)
				{
					Sector.ArmyAntiLCombatPoints -= CombatPoints;
				}
			}
			else
			{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("failed to upgrade %s to anti S",
					*Ship->GetImmatriculation().ToString());
#endif
				UpgradeFailed = true;
			}
		}
		// if enought anti S and S ship value is not nesessary, upgrade to Anti L
		else if (!EnoughAntiL && HasAntiSmallShipWeapon)
		{
			bool EnoughAntiSWithoutShip = (Sector.ArmyAntiSCombatPoints - CombatPoints) >= Target.EnemyArmySCombatPoints * WarContext.AttackThreshold;
			if (EnoughAntiSWithoutShip)
			{
				bool Upgraded = UpgradeShip(Ship, EFlarePartSize::L);
				if (Upgraded)
				{
					Sector.ArmyAntiLCombatPoints += CombatPoints;
					Sector.ArmyAntiSCombatPoints -= CombatPoints;
				}
				else
				{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("failed to upgrade %s to anti L",
					*Ship->GetImmatriculation().ToString());
#endif
					UpgradeFailed = true;
				}
			}
		}
	}

	// if enought anti S and L, upgrade to min ratio
	if ((Sector.ArmyAntiLCombatPoints >= Target.EnemyArmyLCombatPoints * WarContext.AttackThreshold) &&
			(Sector.ArmyAntiSCombatPoints >= Target.EnemyArmySCombatPoints * WarContext.AttackThreshold))
	{
		for (UFlareSimulatedSpacecraft* Ship : MovableShips)
		{
			float AntiLRatio = (Target.EnemyArmyLCombatPoints > 0 ?
									Sector.ArmyAntiLCombatPoints / Target.EnemyArmyLCombatPoints * WarContext.AttackThreshold :
									1000);
			float AntiSRatio = (Target.EnemyArmySCombatPoints > 0 ?
									Sector.ArmyAntiSCombatPoints / Target.EnemyArmySCombatPoints * WarContext.AttackThreshold :
									1000);

			if (AntiLRatio == AntiSRatio)
			{
				// Any change will change the balance
				break;
			}

			if (AntiLRatio > AntiSRatio)
			{
				if (Ship->GetWeaponsSystem()->HasAntiLargeShipWeapon())
				{
					int32 CombatPoints = Ship->GetCombatPoints(true);
					// See if upgrade to S improve this
					float FutureAntiLRatio = (Target.EnemyArmyLCombatPoints > 0 ?
											(Sector.ArmyAntiLCombatPoints - CombatPoints) / Target.EnemyArmyLCombatPoints * WarContext.AttackThreshold :
											1000);
					float FutureAntiSRatio = (Target.EnemyArmySCombatPoints > 0 ?
											(Sector.ArmyAntiSCombatPoints + CombatPoints) / Target.EnemyArmySCombatPoints * WarContext.AttackThreshold :
											1000);
					if (FutureAntiLRatio > FutureAntiSRatio)
					{
						// Ratio of ratio still unchanged, upgrade to S
						bool Upgraded = UpgradeShip(Ship, EFlarePartSize::S);
						if (Upgraded)
						{
							Sector.ArmyAntiSCombatPoints += CombatPoints;
							Sector.ArmyAntiLCombatPoints -= CombatPoints;
						}
						else
						{
							UpgradeFailed = true;
						}
					}
				}
			}
			else
			{
				if (Ship->GetWeaponsSystem()->HasAntiSmallShipWeapon())
				{
					int32 CombatPoints = Ship->GetCombatPoints(true);
					// See if upgrade to S improve this
					float FutureAntiLRatio = (Target.EnemyArmyLCombatPoints > 0 ?
											(Sector.ArmyAntiLCombatPoints + CombatPoints) / Target.EnemyArmyLCombatPoints * WarContext.AttackThreshold :
											1000);
					float FutureAntiSRatio = (Target.EnemyArmySCombatPoints > 0 ?
											(Sector.ArmyAntiSCombatPoints - CombatPoints) / Target.EnemyArmySCombatPoints * WarContext.AttackThreshold :
											1000);
					if (FutureAntiSRatio > FutureAntiLRatio)
					{
						// Ratio of ratio still unchanged, upgrade to L
						bool Upgraded = UpgradeShip(Ship, EFlarePartSize::L);
						if (Upgraded)
						{
							Sector.ArmyAntiLCombatPoints += CombatPoints;
							Sector.ArmyAntiSCombatPoints -= CombatPoints;
						}
						else
						{
							UpgradeFailed = true;
						}
					}
				}
			}

		}
	}

	bool FinalEnoughAntiL = Sector.ArmyAntiLCombatPoints >= Target.EnemyArmyLCombatPoints * WarContext.AttackThreshold;
	bool FinalEnoughAntiS = Sector.ArmyAntiSCombatPoints >= Target.EnemyArmySCombatPoints * WarContext.AttackThreshold;

	#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
					FLOGV("upgrade at %s failed FinalEnoughAntiL=%d, FinalEnoughAntiS=%d",
						*Sector.Sector->GetSectorName().ToString(),
						  FinalEnoughAntiL, FinalEnoughAntiS);
					FLOGV("ArmyAntiLCombatPoints=%d EnemyArmyLCombatPoints=%d ArmyAntiSCombatPoints=%d EnemyArmySCombatPoints=%d",
						Sector.ArmyAntiLCombatPoints, Target.EnemyArmyLCombatPoints,
						Sector.ArmyAntiSCombatPoints, Target.EnemyArmySCombatPoints);
	#endif

	return !UpgradeFailed || (FinalEnoughAntiL && FinalEnoughAntiS);
}

//#define DEBUG_AI_PEACE_MILITARY_MOVEMENT

void UFlareCompanyAI::UpdatePeaceMilitaryMovement()
{
	CompanyValue TotalValue = Company->GetCompanyValue();

	int64 TotalDefendableValue = TotalValue.StationsValue + TotalValue.StockValue + TotalValue.ShipsValue - TotalValue.ArmyValue;
	float TotalDefenseRatio = (float) TotalValue.ArmyValue / (float) TotalDefendableValue;

#ifdef DEBUG_AI_PEACE_MILITARY_MOVEMENT
	FLOGV("UpdatePeaceMilitaryMovement for %s : TotalDefendableValue %lld, TotalDefenseRatio %f",
		*Company->GetCompanyName().ToString(),
		TotalDefendableValue,
		TotalDefenseRatio);
#endif
	
	TArray<UFlareSimulatedSpacecraft*> ShipsToMove;
	TArray<UFlareSimulatedSector*> LowDefenseSectors;

	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];
		CompanyValue SectorValue = Company->GetCompanyValue(Sector, true);

		int64 SectorDefendableValue = SectorValue.StationsValue + SectorValue.StockValue + SectorValue.ShipsValue - SectorValue.ArmyValue;
		int64 SectorArmyValue = SectorValue.ArmyValue;
		float SectorDefenseRatio = (float) SectorArmyValue / (float) SectorDefendableValue;

		if ((SectorDefendableValue == 0 && SectorArmyValue > 0) || SectorDefenseRatio > TotalDefenseRatio * 1.5)
		{

#ifdef DEBUG_AI_PEACE_MILITARY_MOVEMENT
			FLOGV("UpdatePeaceMilitaryMovement %s SectorDefendableValue %lld, SectorDefenseRatio %f",
				*Sector->GetSectorName().ToString(),
				SectorDefendableValue,
				SectorDefenseRatio);
#endif

			// Too much defense here, move a ship, pick a random ship and add to the ship to move list
			TArray<UFlareSimulatedSpacecraft*> ShipCandidates;
			TArray<UFlareSimulatedSpacecraft*>&SectorShips = Sector->GetSectorShips();
			for (UFlareSimulatedSpacecraft* ShipCandidate : SectorShips)
			{
				if (ShipCandidate->GetCompany() != Company)
				{
					continue;
				}

				if (!ShipCandidate->IsMilitary()  || !ShipCandidate->CanTravel() || ShipCandidate->GetDamageSystem()->IsDisarmed())
				{
					continue;
				}

				ShipCandidates.Add(ShipCandidate);
			}

			if (ShipCandidates.Num() > 1 || (SectorDefendableValue == 0 && ShipCandidates.Num() > 0))
			{
				UFlareSimulatedSpacecraft* SelectedShip = ShipCandidates[FMath::RandRange(0, ShipCandidates.Num()-1)];
				ShipsToMove.Add(SelectedShip);

				#ifdef DEBUG_AI_PEACE_MILITARY_MOVEMENT
							FLOGV("UpdatePeaceMilitaryMovement - %s has high defense: pick %s", *Sector->GetSectorName().ToString(), *SelectedShip->GetImmatriculation().ToString());
				#endif
			}
			else
			{
#ifdef DEBUG_AI_PEACE_MILITARY_MOVEMENT
				FLOGV("UpdatePeaceMilitaryMovement - %s has high defense: no available ships", *Sector->GetSectorName().ToString());
#endif
			}

		}

		// Too few defense, add to the target sector list
		else if (SectorDefendableValue > 0 && SectorDefenseRatio < TotalDefenseRatio )
		{
#ifdef DEBUG_AI_PEACE_MILITARY_MOVEMENT
			FLOGV("UpdatePeaceMilitaryMovement - %s has low defense", *Sector->GetSectorName().ToString());
#endif
			LowDefenseSectors.Add(Sector);
		}
	}

	// Find destination sector
	for (UFlareSimulatedSpacecraft* Ship: ShipsToMove)
	{
		int64 MinDurationTravel = 0;
		UFlareSimulatedSector* BestSectorCandidate = NULL;

		for (UFlareSimulatedSector* SectorCandidate : LowDefenseSectors)
		{
			int64 TravelDuration = UFlareTravel::ComputeTravelDuration(Game->GetGameWorld(), Ship->GetCurrentSector(), SectorCandidate, Company);
			if (BestSectorCandidate == NULL || MinDurationTravel > TravelDuration)
			{
				MinDurationTravel = TravelDuration;
				BestSectorCandidate = SectorCandidate;
			}
		}

		// Low defense sector in reach
		if (BestSectorCandidate)
		{
			FLOGV("UpdatePeaceMilitaryMovement > move %s from %s to %s",
				*Ship->GetImmatriculation().ToString(),
				*Ship->GetCurrentSector()->GetSectorName().ToString(),
				*BestSectorCandidate->GetSectorName().ToString());

			Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), BestSectorCandidate);
		}
	}
}


/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

int64 UFlareCompanyAI::OrderOneShip(const FFlareSpacecraftDescription* ShipDescription)
{
	if (ShipDescription == NULL)
	{
		return 0;
	}

	int64 CompanyMoney = Company->GetMoney();
	float CostSafetyMargin = 1.1f;

	//FLOGV("OrderOneShip %s", *ShipDescription->Name.ToString());

	for (int32 ShipyardIndex = 0; ShipyardIndex < Shipyards.Num(); ShipyardIndex++)
	{
		UFlareSimulatedSpacecraft* Shipyard =Shipyards[ShipyardIndex];

		if (Shipyard->CanOrder(ShipDescription, Company))
		{
			int64 ShipPrice = UFlareGameTools::ComputeSpacecraftPrice(ShipDescription->Identifier, Shipyard->GetCurrentSector(), true);





			if (ShipPrice * CostSafetyMargin < CompanyMoney)
			{
				FName ShipClassToOrder = ShipDescription->Identifier;
				FLOGV("UFlareCompanyAI::UpdateShipAcquisition : Ordering spacecraft : '%s'", *ShipClassToOrder.ToString());
				Shipyard->ShipyardOrderShip(Company, ShipClassToOrder);

				SpendBudget((ShipDescription->IsMilitary() ? EFlareBudget::Military : EFlareBudget::Trade), ShipPrice);

				return 0;
			}
			else
			{
				return ShipPrice;
			}
		}
	}

	return 0;
}

//#define DEBUG_AI_SHIP_ORDER
const FFlareSpacecraftDescription* UFlareCompanyAI::FindBestShipToBuild(bool Military)
{
	int32 ShipSCount = 0;
	int32 ShipLCount = 0;

	// Count owned ships
	TMap<const FFlareSpacecraftDescription*, int32> OwnedShipSCount;
	TMap<const FFlareSpacecraftDescription*, int32> OwnedShipLCount;
	for (int32 ShipIndex = 0; ShipIndex < Company->GetCompanyShips().Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = Company->GetCompanyShips()[ShipIndex];

		if (Military != Ship->IsMilitary())
		{
			continue;
		}

		if (Ship->GetSize() ==EFlarePartSize::S)
		{
			ShipSCount++;
			if (OwnedShipSCount.Contains(Ship->GetDescription()))
			{
				OwnedShipSCount[Ship->GetDescription()]++;
			}
			else
			{
				OwnedShipSCount.Add(Ship->GetDescription(), 1);
			}
		}
		else if (Ship->GetSize() ==EFlarePartSize::L)
		{
			ShipLCount++;
			if (OwnedShipLCount.Contains(Ship->GetDescription()))
			{
				OwnedShipLCount[Ship->GetDescription()]++;
			}
			else
			{
				OwnedShipLCount.Add(Ship->GetDescription(), 1);
			}
		}
	}

	// Count ships in production
	for (int32 ShipyardIndex = 0; ShipyardIndex < Shipyards.Num(); ShipyardIndex++)
	{
		UFlareSimulatedSpacecraft* Shipyard =Shipyards[ShipyardIndex];

		TArray<UFlareFactory*>& Factories = Shipyard->GetFactories();

		for (int32 Index = 0; Index < Factories.Num(); Index++)
		{
			UFlareFactory* Factory = Factories[Index];
			if (Factory->GetTargetShipCompany() == Company->GetIdentifier())
			{
				const FFlareSpacecraftDescription* BuildingShip = Game->GetSpacecraftCatalog()->Get(Factory->GetTargetShipClass());
				if (Military != BuildingShip->IsMilitary())
				{
					continue;
				}

				if (BuildingShip->Size ==EFlarePartSize::S)
				{
					ShipSCount++;
					if (OwnedShipSCount.Contains(BuildingShip))
					{
						OwnedShipSCount[BuildingShip]++;
					}
					else
					{
						OwnedShipSCount.Add(BuildingShip, 1);
					}
				}
				else if (BuildingShip->Size ==EFlarePartSize::L)
				{
					ShipLCount++;
					if (OwnedShipLCount.Contains(BuildingShip))
					{
						OwnedShipLCount[BuildingShip]++;
					}
					else
					{
						OwnedShipLCount.Add(BuildingShip, 1);
					}
				}

			}
		}
	}

	// Choose which size to pick
	int32 Diversity = (Military ? AI_MILITARY_DIVERSITY_THRESHOLD : AI_CARGO_DIVERSITY_THRESHOLD);
	int32 SizeDiversity = (Military ? AI_MILITARY_SIZE_DIVERSITY_THRESHOLD : AI_CARGO_SIZE_DIVERSITY_THRESHOLD);
	int32 SizeDiversityBase = (Military ? AI_MILITARY_SIZE_DIVERSITY_THRESHOLD_BASE : AI_CARGO_SIZE_DIVERSITY_THRESHOLD_BASE);
	bool PickLShip = true;
	if (SizeDiversityBase + (ShipLCount) * SizeDiversity > ShipSCount)
	{
		PickLShip = false;
	}

#ifdef DEBUG_AI_SHIP_ORDER
			FLOGV("FindBestShipToBuild for %s (military=%d) pick L=%d",
				  *Company->GetCompanyName().ToString(),
				  Military,
				  PickLShip);
			FLOGV("- ShipSCount %d", ShipSCount);
			FLOGV("- ShipLCount %d", ShipLCount);
			FLOGV("- SizeDiversity %d", SizeDiversity);
			FLOGV("- Diversity %d", Diversity);
#endif

	// List possible ship candidates
	TArray<const FFlareSpacecraftDescription*> CandidateShips;
	for (auto& CatalogEntry : Game->GetSpacecraftCatalog()->ShipCatalog)
	{
		const FFlareSpacecraftDescription* Description = &CatalogEntry->Data;
		if (Military == Description->IsMilitary())
		{
			if (PickLShip == (Description->Size == EFlarePartSize::L))
			{
				CandidateShips.Add(Description);
			}
		}
	}
	
	// Sort by size
	struct FSortBySmallerShip
	{
		FORCEINLINE bool operator()(const FFlareSpacecraftDescription& A, const FFlareSpacecraftDescription& B) const
		{			
			if (A.Mass > B.Mass)
			{
				return false;
			}
			else if (A.Mass < B.Mass)
			{
				return true;
			}
			else if (A.IsMilitary())
			{
				if (!B.IsMilitary())
				{
					return false;
				}
				else
				{
					return A.WeaponGroups.Num() < B.WeaponGroups.Num();
				}
			}
			else
			{
				return true;
			}
		}
	};
	CandidateShips.Sort(FSortBySmallerShip());

	// Find the first ship that is diverse enough, from small to large
	const FFlareSpacecraftDescription* BestShipDescription = NULL;
	TMap<const FFlareSpacecraftDescription*, int32>* OwnedShipCount = (PickLShip ? &OwnedShipLCount : &OwnedShipSCount);
	for (const FFlareSpacecraftDescription* Description : CandidateShips)
	{
		if (BestShipDescription == NULL)
		{
			BestShipDescription = Description;
			continue;
		}
		
		int32 CandidateCount = 0;
		if (OwnedShipCount->Contains(Description))
		{
			CandidateCount = (*OwnedShipCount)[Description];
		}

		int32 BestCount = 0;
		if (OwnedShipCount->Contains(BestShipDescription))
		{
			BestCount = (*OwnedShipCount)[BestShipDescription];
		}
		
		bool BestBigger = (Military ? BestShipDescription->Mass > Description-> Mass : BestShipDescription->GetCapacity() > Description->GetCapacity());
		
		bool Select = false;
		if (BestBigger && BestCount + Diversity > CandidateCount)
		{
			Select = true;
		}
		else if (!BestBigger && CandidateCount + Diversity < BestCount)
		{
			Select = true;
		}

		if (Select)
		{
			BestShipDescription = Description;
		}
	}

	if (BestShipDescription == NULL)
	{
		FLOG("ERROR: no ship to build");
		return NULL;
	}

	return BestShipDescription;
}

bool UFlareCompanyAI::IsBuildingShip(bool Military)
{
	for (int32 ShipyardIndex = 0; ShipyardIndex < Shipyards.Num(); ShipyardIndex++)
	{
		UFlareSimulatedSpacecraft* Shipyard =Shipyards[ShipyardIndex];

		TArray<UFlareFactory*>& Factories = Shipyard->GetFactories();

		for (int32 Index = 0; Index < Factories.Num(); Index++)
		{
			UFlareFactory* Factory = Factories[Index];
			if (Factory->GetTargetShipCompany() == Company->GetIdentifier())
			{
				FFlareSpacecraftDescription* BuildingShip = Game->GetSpacecraftCatalog()->Get(Factory->GetTargetShipClass());
				if (Military == BuildingShip->IsMilitary())
				{
					return true;
				}
			}
		}
	}
	return false;
}


TArray<UFlareSimulatedSpacecraft*> UFlareCompanyAI::FindShipyards()
{
	TArray<UFlareSimulatedSpacecraft*> ShipyardList;

	// Find shipyard
	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];

		for (int32 StationIndex = 0; StationIndex < Sector->GetSectorStations().Num(); StationIndex++)
		{
			UFlareSimulatedSpacecraft* Station = Sector->GetSectorStations()[StationIndex];

			if (Company->GetWarState(Station->GetCompany()) == EFlareHostility::Hostile)
			{
				continue;
			}

			TArray<UFlareFactory*>& Factories = Station->GetFactories();

			for (int32 Index = 0; Index < Factories.Num(); Index++)
			{
				UFlareFactory* Factory = Factories[Index];
				if (Factory->IsShipyard())
				{
					ShipyardList.Add(Station);
					break;
				}
			}
		}
	}

	return ShipyardList;
}

TArray<UFlareSimulatedSpacecraft*> UFlareCompanyAI::FindIncapacitatedCargos() const
{
	TArray<UFlareSimulatedSpacecraft*> IncapacitatedCargos;

	for (UFlareSimulatedSpacecraft* Ship : Company->GetCompanyShips())
	{
		if (Ship->GetActiveCargoBay()->GetCapacity() > 0 && (Ship->GetDamageSystem()->IsStranded() || Ship->GetDamageSystem()->IsUncontrollable()))
		{
			IncapacitatedCargos.Add(Ship);
		}
	}

	return IncapacitatedCargos;
}

TArray<UFlareSimulatedSpacecraft*> UFlareCompanyAI::FindIdleCargos() const
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

void UFlareCompanyAI::CargosEvasion()
{
	SCOPE_CYCLE_COUNTER(STAT_FlareCompanyAI_CargosEvasion);

	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];

		if (!Sector->GetSectorBattleState(Company).HasDanger)
		{
			continue;
		}

		// Use intermediate list as travel modify the sector list
		TArray<UFlareSimulatedSpacecraft*> CargoToTravel;

		for (int32 ShipIndex = 0 ; ShipIndex < Sector->GetSectorShips().Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = Sector->GetSectorShips()[ShipIndex];
			if (Ship->GetCompany() != Company || !Ship->CanTravel() || Ship->IsMilitary())
			{
				continue;
			}

			CargoToTravel.Add(Ship);
		}

		if (CargoToTravel.Num() > 0)
		{
			// Find nearest safe sector
			// Or if no safe sector, go to the farest sector to maximise travel time
			UFlareSimulatedSector* SafeSector = NULL;
			UFlareSimulatedSector* DistantUnsafeSector = NULL;
			int64 MinDurationTravel = 0;
			int64 MaxDurationTravel = 0;

			for (int32 SectorIndex2 = 0; SectorIndex2 < Company->GetKnownSectors().Num(); SectorIndex2++)
			{
				UFlareSimulatedSector* SectorCandidate = Company->GetKnownSectors()[SectorIndex2];
				int64 TravelDuration = UFlareTravel::ComputeTravelDuration(Game->GetGameWorld(), Sector, SectorCandidate, Company);

				if (DistantUnsafeSector == NULL || MaxDurationTravel < TravelDuration)
				{
					MaxDurationTravel = TravelDuration;
					DistantUnsafeSector = SectorCandidate;
				}

				if (SectorCandidate->GetSectorBattleState(Company).HasDanger)
				{
					// Dont go in a dangerous sector
					continue;
				}


				if (SafeSector == NULL || MinDurationTravel > TravelDuration)
				{
					MinDurationTravel = TravelDuration;
					SafeSector = SectorCandidate;
				}
			}

			for (UFlareSimulatedSpacecraft* Ship: CargoToTravel)
			{
				if (SafeSector)
				{
					Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), SafeSector);
				}
				else if (DistantUnsafeSector)
				{
					Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), DistantUnsafeSector);
				}
			}
		}
	}
}

int32 UFlareCompanyAI::GetDamagedCargosCapacity()
{
	int32 DamagedCapacity = 0;
	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];
		
		for (int32 ShipIndex = 0 ; ShipIndex < Sector->GetSectorShips().Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = Sector->GetSectorShips()[ShipIndex];
			if (Ship->GetCompany() != Company || Ship->IsTrading() || (Ship->GetCurrentFleet() && Ship->GetCurrentFleet()->IsTraveling()) || Ship->GetCurrentTradeRoute() != NULL || Ship->GetActiveCargoBay()->GetCapacity() == 0)
			{
				continue;
			}

			if (Ship->GetDamageSystem()->IsStranded())
			{
				DamagedCapacity += Ship->GetActiveCargoBay()->GetCapacity();
			}

		}
	}
	return DamagedCapacity;
}

int32 UFlareCompanyAI::GetCargosCapacity()
{
	int32 Capacity = 0;
	for (UFlareSimulatedSpacecraft* Ship : Company->GetCompanyShips())
	{
		if (Ship->GetCompany() != Company || Ship->GetActiveCargoBay()->GetCapacity() == 0)
		{
			continue;
		}

		Capacity += Ship->GetActiveCargoBay()->GetCapacity();
	}
	return Capacity;
}

TArray<UFlareSimulatedSpacecraft*> UFlareCompanyAI::FindIdleMilitaryShips() const
{
	TArray<UFlareSimulatedSpacecraft*> IdleMilitaryShips;

	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];


		for (int32 ShipIndex = 0 ; ShipIndex < Sector->GetSectorShips().Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = Sector->GetSectorShips()[ShipIndex];
			if (Ship->GetCompany() != Company || !Ship->IsMilitary() || (Ship->GetCurrentFleet() && Ship->GetCurrentFleet()->IsTraveling()))
			{
				continue;
			}

			IdleMilitaryShips.Add(Ship);
		}
	}

	return IdleMilitaryShips;
}

float UFlareCompanyAI::GetShipyardUsageRatio() const
{
	float LargeSum = 0;
	float LargeCount = 0;
	float SmallSum = 0;
	float SmallCount = 0;

	for (UFlareSimulatedSpacecraft* Shipyard: Shipyards)
	{
		for (UFlareFactory* Factory : Shipyard->GetFactories())
		{
			if (Factory->IsLargeShipyard())
			{
				LargeCount++;
			}
			else
			{
				SmallCount++;
			}

			if (Factory->GetTargetShipCompany() != NAME_None)
			{
				if (Factory->IsLargeShipyard())
				{
					LargeSum++;
				}
				else
				{
					SmallSum++;
				}
			}
		}
	}

	float LargeRatio = (LargeCount > 0 ? LargeSum / LargeCount : 1.f);
	float SmallRatio = (SmallCount > 0 ? SmallSum / SmallCount : 1.f);

	return FMath::Max(LargeRatio, SmallRatio);
}

float UFlareCompanyAI::ComputeConstructionScoreForStation(UFlareSimulatedSector* Sector, FFlareSpacecraftDescription* StationDescription, FFlareFactoryDescription* FactoryDescription, UFlareSimulatedSpacecraft* Station, bool Technology) const
{
	// The score is a number between 0 and infinity. A classical score is 1. If 0, the company don't want to build this station

	// Multiple parameter impact the score
	// Base score is 1
	// x sector affility
	// x resource affility if produce a resource
	// x customer, maintenance, shipyard affility if as the capability
	//
	// Then some world state multiplier occurs
	// - for factories : if the resource world flow of a input resourse is negative, multiply by 1 to 0 for 0 to x% (x is resource afficility) of negative ratio
	// - for factories : if the resource world flow of a output resourse is positive, multiply by 1 to 0 for 0 to x% (x is resource afficility) of positive ratio
	// - for customer, if customer affility > customer consumption in sector reduce the score
	// - maintenance, same with world FS consumption
	// - for shipyard, if a own shipyard is not used, don't do one
	//
	// - x 2 to 0, for the current price of input and output resource. If output resource price is min : 0, if max : 2. Inverse for input
	//
	// - Time to pay the construction price multiply from 1 for 1 day to 0 for infinity. 0.5 at 200 days

	float Score = 1.0f;

	/*if (StationDescription->Capabilities.Contains(EFlareSpacecraftCapability::Maintenance))
	{
		FLOGV(">>>>>Score for %s in %s", *StationDescription->Identifier.ToString(), *Sector->GetIdentifier().ToString());
	}*/

	if(Technology != StationDescription->IsResearch())
	{
		return 0;
	}

	//TODO customer, maintenance and shipyard limit

	Score *= Behavior->GetSectorAffility(Sector);
	//FLOGV(" after sector Affility: %f", Score);




	if (StationDescription->Capabilities.Contains(EFlareSpacecraftCapability::Consumer))
	{
		Score *= Behavior->ConsumerAffility;

		const SectorVariation* ThisSectorVariation = &WorldResourceVariation[Sector];

		float MaxScoreModifier = 0;

		for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->ConsumerResources.Num(); ResourceIndex++)
		{
			FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->ConsumerResources[ResourceIndex]->Data;
			const struct ResourceVariation* Variation = &ThisSectorVariation->ResourceVariations[Resource];


			float Consumption = Sector->GetPeople()->GetRessourceConsumption(Resource, false);
			//FLOGV("%s comsumption = %f", *Resource->Name.ToString(), Consumption);

			float ReserveStock =  Variation->ConsumerMaxStock / 10.f;
			//FLOGV("ReserveStock = %f", ReserveStock);
			if (Consumption < ReserveStock)
			{
				float ScoreModifier = 2.f * ((Consumption / ReserveStock) - 0.5);
				if (ScoreModifier > MaxScoreModifier)
				{
					MaxScoreModifier = ScoreModifier;
				}
			}
			else if (Consumption > 0)
			{
				MaxScoreModifier = 1;
				break;
			}
			// If superior, keep 1
		}
		Score *= MaxScoreModifier;
		float StationPrice = ComputeStationPrice(Sector, StationDescription, Station);
		Score *= 1.f + 1/StationPrice;
		//FLOGV("MaxScoreModifier = %f", MaxScoreModifier);
	}
	else if (StationDescription->Capabilities.Contains(EFlareSpacecraftCapability::Maintenance))
	{
		Score *= Behavior->MaintenanceAffility;

		const SectorVariation* ThisSectorVariation = &WorldResourceVariation[Sector];

		float MaxScoreModifier = 0;

		for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->MaintenanceResources.Num(); ResourceIndex++)
		{
			FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->MaintenanceResources[ResourceIndex]->Data;
			const struct ResourceVariation* Variation = &ThisSectorVariation->ResourceVariations[Resource];


			int32 Consumption = WorldStats[Resource].Consumption / Company->GetKnownSectors().Num();
			//FLOGV("%s comsumption = %d", *Resource->Name.ToString(), Consumption);

			float ReserveStock =  Variation->MaintenanceMaxStock;
			//FLOGV("ReserveStock = %f", ReserveStock);
			if (Consumption < ReserveStock)
			{
				float ScoreModifier = 2.f * ((Consumption / ReserveStock) - 0.5);

				if (ScoreModifier > MaxScoreModifier)
				{
					MaxScoreModifier = ScoreModifier;
				}
			}
			else if (Consumption > 0)
			{
				MaxScoreModifier = 1;
				break;
			}
			// If superior, keep 1
		}
		Score *= MaxScoreModifier;
		//FLOGV("MaxScoreModifier = %f", MaxScoreModifier);

		float StationPrice = ComputeStationPrice(Sector, StationDescription, Station);
		Score *= 1.f + 1/StationPrice;

	}
	else if (FactoryDescription && FactoryDescription->IsResearch())
	{

		// Underflow malus
		for (int32 ResourceIndex = 0; ResourceIndex < FactoryDescription->CycleCost.InputResources.Num(); ResourceIndex++)
		{
			const FFlareFactoryResource* Resource = &FactoryDescription->CycleCost.InputResources[ResourceIndex];

			float MaxVolume = FMath::Max(WorldStats[&Resource->Resource->Data].Production, WorldStats[&Resource->Resource->Data].Consumption);
			if (MaxVolume > 0)
			{
				float UnderflowRatio = WorldStats[&Resource->Resource->Data].Balance / MaxVolume;
				if (UnderflowRatio < 0)
				{
					float UnderflowMalus = FMath::Clamp((UnderflowRatio * 100)  / 20.f + 1.f, 0.f, 1.f);
					Score *= UnderflowMalus;
					//FLOGV("    MaxVolume %f", MaxVolume);
					//FLOGV("    UnderflowRatio %f", UnderflowRatio);
					//FLOGV("    UnderflowMalus %f", UnderflowMalus);
				}
			}
			else
			{
				if(Technology)
				{
					FLOG("No input production");
				}
				// No input production, ignore this station
				return 0;
			}
		}

		float StationPrice = ComputeStationPrice(Sector, StationDescription, Station);
		Score *= 1.f + 1/StationPrice;
	}
	else if (FactoryDescription && FactoryDescription->IsShipyard())
	{
		Score *= Behavior->ShipyardAffility;

		Score *= GetShipyardUsageRatio() * 0.5;

		// Underflow malus
		for (int32 ResourceIndex = 0; ResourceIndex < FactoryDescription->CycleCost.InputResources.Num(); ResourceIndex++)
		{
			const FFlareFactoryResource* Resource = &FactoryDescription->CycleCost.InputResources[ResourceIndex];

			float MaxVolume = FMath::Max(WorldStats[&Resource->Resource->Data].Production, WorldStats[&Resource->Resource->Data].Consumption);
			if (MaxVolume > 0)
			{
				float UnderflowRatio = WorldStats[&Resource->Resource->Data].Balance / MaxVolume;
				if (UnderflowRatio < 0)
				{
					float UnderflowMalus = FMath::Clamp((UnderflowRatio * 100)  / 20.f + 1.f, 0.f, 1.f);
					Score *= UnderflowMalus;
					//FLOGV("    MaxVolume %f", MaxVolume);
					//FLOGV("    UnderflowRatio %f", UnderflowRatio);
					//FLOGV("    UnderflowMalus %f", UnderflowMalus);
				}
			}
			else
			{
				// No input production, ignore this station
				return 0;
			}
		}

		float StationPrice = ComputeStationPrice(Sector, StationDescription, Station);
		Score *= 1.f + 1/StationPrice;

		//FLOGV("Score=%f for %s in %s", Score, *StationDescription->Identifier.ToString(), *Sector->GetIdentifier().ToString());

	}
	else if (FactoryDescription)
	{
		float GainPerCycle = 0;

		GainPerCycle -= FactoryDescription->CycleCost.ProductionCost;



		// Factory
		for (int32 ResourceIndex = 0; ResourceIndex < FactoryDescription->CycleCost.InputResources.Num(); ResourceIndex++)
		{
			const FFlareFactoryResource* Resource = &FactoryDescription->CycleCost.InputResources[ResourceIndex];
			GainPerCycle -= Sector->GetResourcePrice(&Resource->Resource->Data, EFlareResourcePriceContext::FactoryInput) * Resource->Quantity;

			float MaxVolume = FMath::Max(WorldStats[&Resource->Resource->Data].Production, WorldStats[&Resource->Resource->Data].Consumption);
			if (MaxVolume > 0)
			{
				float UnderflowRatio = WorldStats[&Resource->Resource->Data].Balance / MaxVolume;
				if (UnderflowRatio < 0)
				{
					float UnderflowMalus = FMath::Clamp((UnderflowRatio * 100)  / 20.f + 1.f, 0.f, 1.f);
					Score *= UnderflowMalus;
					//FLOGV("    MaxVolume %f", MaxVolume);
					//FLOGV("    UnderflowRatio %f", UnderflowRatio);
					//FLOGV("    UnderflowMalus %f", UnderflowMalus);
				}
			}
			else
			{
				// No input production, ignore this station
				return 0;
			}

			float ResourcePrice = Sector->GetPreciseResourcePrice(&Resource->Resource->Data);
			float PriceRatio = (ResourcePrice - (float) Resource->Resource->Data.MinPrice) / (float) (Resource->Resource->Data.MaxPrice - Resource->Resource->Data.MinPrice);

			Score *= (1 - PriceRatio) * 2;
		}

		//FLOGV(" after input: %f", Score);

		if (Score == 0)
		{
			return 0;
		}

		for (int32 ResourceIndex = 0; ResourceIndex < FactoryDescription->CycleCost.OutputResources.Num(); ResourceIndex++)
		{
			const FFlareFactoryResource* Resource = &FactoryDescription->CycleCost.OutputResources[ResourceIndex];
			GainPerCycle += Sector->GetResourcePrice(&Resource->Resource->Data, EFlareResourcePriceContext::FactoryOutput) * Resource->Quantity;

			float ResourceAffility = Behavior->GetResourceAffility(&Resource->Resource->Data);
			Score *= ResourceAffility;


			//FLOGV(" ResourceAffility for %s: %f", *Resource->Resource->Data.Identifier.ToString(), ResourceAffility);

			float MaxVolume = FMath::Max(WorldStats[&Resource->Resource->Data].Production, WorldStats[&Resource->Resource->Data].Consumption);
			if (MaxVolume > 0)
			{
				float OverflowRatio = WorldStats[&Resource->Resource->Data].Balance / MaxVolume;
				if (OverflowRatio > 0)
				{
					float OverflowMalus = FMath::Clamp(1.f - ((OverflowRatio - 0.1f) * 100)  / ResourceAffility, 0.f, 1.f);
					Score *= OverflowMalus;
					//FLOGV("    MaxVolume %f", MaxVolume);
					//FLOGV("    OverflowRatio %f", OverflowRatio);
					//FLOGV("    OverflowMalus %f", OverflowMalus);
				}
			}
			else
			{
				Score *= 1000;
			}

			float ResourcePrice = Sector->GetPreciseResourcePrice(&Resource->Resource->Data);
			float PriceRatio = (ResourcePrice - (float) Resource->Resource->Data.MinPrice) / (float) (Resource->Resource->Data.MaxPrice - Resource->Resource->Data.MinPrice);


			//FLOGV("    PriceRatio %f", PriceRatio);


			Score *= PriceRatio * 2;
		}

		//FLOGV(" after output: %f", Score);

		if(FactoryDescription->CycleCost.ProductionTime <=0)
		{
			// Contruction cycle
			return 0;
		}

		float GainPerDay = GainPerCycle / FactoryDescription->CycleCost.ProductionTime;
		if (GainPerDay < 0)
		{
			return 0;
		}

		float StationPrice = ComputeStationPrice(Sector, StationDescription, Station);
		float DayToPayPrice = StationPrice / GainPerDay;

		float HalfRatioDelay = 1500;

		float PaybackMalus = (HalfRatioDelay -1.f)/(DayToPayPrice+(HalfRatioDelay -2.f)); // 1for 1 day, 0.5 for 1500 days
		Score *= PaybackMalus;
	}
	else
	{
		return 0;
	}

	//FLOGV(" GainPerCycle: %f", GainPerCycle);
	//FLOGV(" GainPerDay: %f", GainPerDay);
	//FLOGV(" StationPrice: %f", StationPrice);
	//FLOGV(" DayToPayPrice: %f", DayToPayPrice);
	//FLOGV(" PaybackMalus: %f", PaybackMalus);

	/*if (StationDescription->Capabilities.Contains(EFlareSpacecraftCapability::Consumer) ||
			StationDescription->Capabilities.Contains(EFlareSpacecraftCapability::Maintenance) ||
			StationDescription->Capabilities.Contains(EFlareSpacecraftCapability::Storage)
			)
	{
	FLOGV("Score=%f for %s in %s", Score, *StationDescription->Identifier.ToString(), *Sector->GetIdentifier().ToString());
	}*/

	return Score;
}

float UFlareCompanyAI::ComputeStationPrice(UFlareSimulatedSector* Sector, FFlareSpacecraftDescription* StationDescription, UFlareSimulatedSpacecraft* Station) const
{
	float StationPrice;

	if (Station)
	{
		// Upgrade
		StationPrice = STATION_CONSTRUCTION_PRICE_BONUS * (Station->GetStationUpgradeFee() +  UFlareGameTools::ComputeSpacecraftPrice(StationDescription->Identifier, Sector, true, false, false)) - 1;
	}
	else
	{
		// Construction
		StationPrice = STATION_CONSTRUCTION_PRICE_BONUS * UFlareGameTools::ComputeSpacecraftPrice(StationDescription->Identifier, Sector, true, true, false, Company);
	}
	return StationPrice;

}


SectorVariation UFlareCompanyAI::ComputeSectorResourceVariation(UFlareSimulatedSector* Sector) const
{
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

void UFlareCompanyAI::DumpSectorResourceVariation(UFlareSimulatedSector* Sector, TMap<FFlareResourceDescription*, struct ResourceVariation>* SectorVariation) const
{
	FLOGV("DumpSectorResourceVariation : sector %s resource variation: ", *Sector->GetSectorName().ToString());
	for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
		struct ResourceVariation* Variation = &(*SectorVariation)[Resource];
		if (Variation->OwnedFlow ||
				Variation->FactoryFlow ||
				Variation->OwnedStock ||
				Variation->FactoryStock ||
				Variation->StorageStock ||
				Variation->OwnedCapacity ||
				Variation->FactoryCapacity ||
				Variation->StorageCapacity ||
				Variation->MaintenanceCapacity
				)
		{
			FLOGV(" - Resource %s", *Resource->Name.ToString());
			if (Variation->OwnedFlow)
				FLOGV("   owned flow %d / day", Variation->OwnedFlow);
			if (Variation->FactoryFlow)
				FLOGV("   factory flow %d / day", Variation->FactoryFlow);
			if (Variation->OwnedStock)
				FLOGV("   owned stock %d", Variation->OwnedStock);
			if (Variation->FactoryStock)
				FLOGV("   factory stock %d", Variation->FactoryStock);
			if (Variation->StorageStock)
				FLOGV("   storage stock %d", Variation->StorageStock);
			if (Variation->OwnedCapacity)
				FLOGV("   owned capacity %d", Variation->OwnedCapacity);
			if (Variation->FactoryCapacity)
				FLOGV("   factory capacity %d", Variation->FactoryCapacity);
			if (Variation->StorageCapacity)
				FLOGV("   storage capacity %d", Variation->StorageCapacity);
			if (Variation->MaintenanceCapacity)
				FLOGV("   maintenance capacity %d", Variation->MaintenanceCapacity);
		}

	}
}

SectorDeal UFlareCompanyAI::FindBestDealForShipFromSector(UFlareSimulatedSpacecraft* Ship, UFlareSimulatedSector* SectorA, SectorDeal* DealToBeat)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareCompanyAI_FindBestDealForShipFromSector);


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

		SectorVariation* SectorVariationA = &(WorldResourceVariation[SectorA]);
		SectorVariation* SectorVariationB = &(WorldResourceVariation[SectorB]);

		for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
		{
			FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
			struct ResourceVariation* VariationA = &SectorVariationA->ResourceVariations[Resource];
			struct ResourceVariation* VariationB = &SectorVariationB->ResourceVariations[Resource];

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

#undef LOCTEXT_NAMESPACE
