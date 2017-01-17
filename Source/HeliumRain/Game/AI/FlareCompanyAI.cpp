
#include "../../Flare.h"
#include "FlareCompanyAI.h"
#include "FlareAIBehavior.h"

#include "../FlareGame.h"
#include "../FlareCompany.h"
#include "../FlareSectorHelper.h"

#include "../../Economy/FlareCargoBay.h"
#include "../../Spacecrafts/FlareSimulatedSpacecraft.h"


#define STATION_CONSTRUCTION_PRICE_BONUS 1.2
// TODO, make it depend on player CA
#define AI_NERF_RATIO 0.5
// TODO, make it depend on company's nature
#define AI_CARGO_DIVERSITY_THRESOLD 10
#define AI_CARGO_SIZE_DIVERSITY_THRESOLD 15
#define AI_MILITARY_DIVERSITY_THRESOLD 4
#define AI_MILITARY_SIZE_DIVERSITY_THRESOLD 15

// TODO, make it depend on company's nature
#define AI_CARGO_PEACE_MILILTARY_THRESOLD 10


//#define DEBUG_AI_WAR_MILITARY_MOVEMENT
//#define DEBUG_AI_BATTLE_STATES
//#define DEBUG_AI_BUDGET

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

	ConstructionProjectStationDescription = NULL;
	ConstructionProjectSector = NULL;
	ConstructionProjectStation = NULL;
	ConstructionProjectNeedCapacity = AIData.ConstructionProjectNeedCapacity;
	ConstructionShips.Empty();
	ConstructionStaticShips.Empty();

	if(AIData.ConstructionProjectSectorIdentifier != NAME_None)
	{
		ConstructionProjectSector = Game->GetGameWorld()->FindSector(AIData.ConstructionProjectSectorIdentifier);
	}

	if(AIData.ConstructionProjectStationIdentifier != NAME_None)
	{
		ConstructionProjectStation = Game->GetGameWorld()->FindSpacecraft(AIData.ConstructionProjectStationIdentifier);
	}

	if(AIData.ConstructionProjectStationDescriptionIdentifier != NAME_None)
	{
		ConstructionProjectStationDescription = Game->GetSpacecraftCatalog()->Get(AIData.ConstructionProjectStationDescriptionIdentifier);
	}

	for (int32 ShipIndex = 0; ShipIndex < AIData.ConstructionShipsIdentifiers.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = Game->GetGameWorld()->FindSpacecraft(AIData.ConstructionShipsIdentifiers[ShipIndex]);
		if (Ship)
		{
			ConstructionShips.Add(Ship);
		}
	}

	for (int32 ShipIndex = 0; ShipIndex < AIData.ConstructionStaticShipsIdentifiers.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = Game->GetGameWorld()->FindSpacecraft(AIData.ConstructionStaticShipsIdentifiers[ShipIndex]);
		if (Ship)
		{
			ConstructionStaticShips.Add(Ship);
		}
	}

	// Setup Behavior
	Behavior = NewObject<UFlareAIBehavior>(this, UFlareAIBehavior::StaticClass());
}

FFlareCompanyAISave* UFlareCompanyAI::Save()
{

	AIData.ConstructionShipsIdentifiers.Empty();
	AIData.ConstructionStaticShipsIdentifiers.Empty();
	AIData.ConstructionProjectStationDescriptionIdentifier = NAME_None;
	AIData.ConstructionProjectSectorIdentifier = NAME_None;
	AIData.ConstructionProjectStationIdentifier = NAME_None;
	AIData.ConstructionProjectNeedCapacity = ConstructionProjectNeedCapacity;

	if(ConstructionProjectStationDescription)
	{
		AIData.ConstructionProjectStationDescriptionIdentifier = ConstructionProjectStationDescription->Identifier;
	}

	if(ConstructionProjectSector)
	{
		AIData.ConstructionProjectSectorIdentifier = ConstructionProjectSector->GetIdentifier();
	}

	if(ConstructionProjectStation)
	{
		AIData.ConstructionProjectStationIdentifier = ConstructionProjectStation->GetImmatriculation();
	}

	for (int32 ShipIndex = 0; ShipIndex < ConstructionShips.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = ConstructionShips[ShipIndex];
		AIData.ConstructionShipsIdentifiers.Add(Ship->GetImmatriculation());
	}

	for (int32 ShipIndex = 0; ShipIndex < ConstructionStaticShips.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = ConstructionStaticShips[ShipIndex];
		AIData.ConstructionStaticShipsIdentifiers.Add(Ship->GetImmatriculation());
	}

	return &AIData;
}

void UFlareCompanyAI::Tick()
{
	if (Game && Company != Game->GetPC()->GetCompany())
	{
		UpdateDiplomacy();
	}
}

void UFlareCompanyAI::Simulate()
{
	if (Game && Company != Game->GetPC()->GetCompany())
	{
		Behavior->Load(Company);

		CheckBattleResolution();
		UpdateDiplomacy();
	
		ResourceFlow = ComputeWorldResourceFlow();
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
	}
}

void UFlareCompanyAI::DestroySpacecraft(UFlareSimulatedSpacecraft* Spacecraft)
{
	// Don't keep reference on destroyed stations
	if (ConstructionProjectStation == Spacecraft)
	{
		ClearConstructionProject();
	}

	// Don't keep reference on destroyed ship
	ConstructionShips.Remove(Spacecraft);
	ConstructionStaticShips.Remove(Spacecraft);
}


/*----------------------------------------------------
	Internal subsystems
----------------------------------------------------*/

void UFlareCompanyAI::UpdateDiplomacy()
{
	Behavior->Load(Company);
	Behavior->UpdateDiplomacy();
}

//#define DEBUG_AI_TRADING
#define DEBUG_AI_TRADING_COMPANY "PIR"

void UFlareCompanyAI::UpdateTrading()
{
	IdleCargoCapacity = 0;
	TArray<UFlareSimulatedSpacecraft*> IdleCargos = FindIdleCargos();
#ifdef DEBUG_AI_TRADING
	if(Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
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
		
	for (int32 ShipIndex = 0; ShipIndex < IdleCargos.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = IdleCargos[ShipIndex];

		//	FLOGV("UFlareCompanyAI::UpdateTrading : Search something to do for %s", *Ship->GetImmatriculation().ToString());
		
		SectorDeal BestDeal;
		BestDeal.BuyQuantity = 0;
		BestDeal.Score = 0;
		BestDeal.Resource = NULL;
		BestDeal.SectorA = NULL;
		BestDeal.SectorB = NULL;
		
		// Stay here option
		
		for (int32 SectorAIndex = 0; SectorAIndex < Company->GetKnownSectors().Num(); SectorAIndex++)
		{
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
#ifdef DEBUG_AI_TRADING
			if(Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
			{
				FLOGV("UFlareCompanyAI::UpdateTrading : Best balance for %s (%s) : %f score",
					*Ship->GetImmatriculation().ToString(), *Ship->GetCurrentSector()->GetSectorName().ToString(), BestDeal.Score / 100);
				FLOGV("UFlareCompanyAI::UpdateTrading -> Transfer %s from %s to %s",
					*BestDeal.Resource->Name.ToString(), *BestDeal.SectorA->GetSectorName().ToString(), *BestDeal.SectorB->GetSectorName().ToString());
			}
#endif
			if (Ship->GetCurrentSector() == BestDeal.SectorA)
			{
				// Already in A, buy resources and go to B
				if (BestDeal.BuyQuantity == 0)
				{
					if (Ship->GetCurrentSector() != BestDeal.SectorB)
					{
						// Already buy resources,go to B
						Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), BestDeal.SectorB);
#ifdef DEBUG_AI_TRADING
						if(Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
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
					Request.CargoLimit = AI_NERF_RATIO;
					Request.MaxQuantity = Ship->GetCargoBay()->GetFreeSpaceForResource(BestDeal.Resource, Ship->GetCompany());

					UFlareSimulatedSpacecraft* StationCandidate = SectorHelper::FindTradeStation(Request);

					int32 BroughtResource = 0;
					if (StationCandidate)
					{
						BroughtResource = SectorHelper::Trade(StationCandidate, Ship, BestDeal.Resource, Request.MaxQuantity);
#ifdef DEBUG_AI_TRADING
						if(Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
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
						if(Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
						{
							FLOG("UFlareCompanyAI::UpdateTrading -> Buy failed, remove the deal from the list");
						}
#endif
					}
				}
			}
			else
			{
				if (BestDeal.SectorA != Ship->GetCurrentSector())
				{
					Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), BestDeal.SectorA);
#ifdef DEBUG_AI_TRADING
					if(Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
					{
						FLOGV("UFlareCompanyAI::UpdateTrading -> Travel to %s to buy", *BestDeal.SectorA->GetSectorName().ToString());
					}
#endif
				}
				else
				{
#ifdef DEBUG_AI_TRADING
					if(Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
					{
						FLOGV("UFlareCompanyAI::UpdateTrading -> Wait to %s", *BestDeal.SectorA->GetSectorName().ToString());
					}
#endif
				}

				// Reserve the deal by virtualy decrease the stock for other ships
				SectorVariation* SectorVariationA = &WorldResourceVariation[BestDeal.SectorA];
				struct ResourceVariation* VariationA = &SectorVariationA->ResourceVariations[BestDeal.Resource];
				VariationA->OwnedStock -= BestDeal.BuyQuantity;
			}

			if (Ship->GetCurrentSector() == BestDeal.SectorB && !Ship->IsTrading())
			{
				// Try to sell
				// Try to unload or sell
				SectorHelper::FlareTradeRequest Request;
				Request.Resource = BestDeal.Resource;
				Request.Operation = EFlareTradeRouteOperation::UnloadOrSell;
				Request.Client = Ship;
				Request.CargoLimit = AI_NERF_RATIO;
				Request.MaxQuantity = Ship->GetCargoBay()->GetResourceQuantity(BestDeal.Resource, Ship->GetCompany());

				UFlareSimulatedSpacecraft* StationCandidate = SectorHelper::FindTradeStation(Request);

				if (StationCandidate)
				{
					int32 SellQuantity = SectorHelper::Trade(Ship, StationCandidate, BestDeal.Resource, Request.MaxQuantity);
#ifdef DEBUG_AI_TRADING
					if(Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
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
			if(Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
			{
				FLOGV("UFlareCompanyAI::UpdateTrading : %s found nothing to do", *Ship->GetImmatriculation().ToString());
			}
#endif
			//FLOGV("UFlareCompanyAI::UpdateTrading : HasProject ? %d ConstructionProjectNeedCapacity %d", (ConstructionProjectStationDescription != NULL), ConstructionProjectNeedCapacity);

			bool Usefull = false;

			if (ConstructionProjectStationDescription && ConstructionProjectSector && ConstructionProjectNeedCapacity > 0)
			{
				if (Ship->GetCargoBay()->GetFreeSlotCount() > 0)
				{
					ConstructionShips.Add(Ship);
					ConstructionProjectNeedCapacity -= Ship->GetCargoBay()->GetFreeSlotCount() * Ship->GetCargoBay()->GetSlotCapacity();
					Usefull = true;
				}
				else
				{
					TArray<FFlareResourceDescription*> MissingResources;
					MissingResourcesQuantity.GetKeys(MissingResources);
					for (int32 ResourceIndex = 0; ResourceIndex < MissingResources.Num(); ResourceIndex++)
					{
						FFlareResourceDescription* MissingResource = MissingResources[ResourceIndex];
						int32 Quantity = Ship->GetCargoBay()->GetResourceQuantity(MissingResource, Ship->GetCompany());
						int32 MissingQuantity =  MissingResourcesQuantity[MissingResource];

						int32 UsefullQuantity = FMath::Min(Quantity, MissingQuantity);

						if (UsefullQuantity > 0)
						{
							// Usefull ship
							ConstructionShips.Add(Ship);
							ConstructionProjectNeedCapacity -= UsefullQuantity;
							MissingResourcesQuantity[MissingResource] = MissingQuantity - UsefullQuantity;
							Usefull = true;
						}
					}
				}
				//FLOGV("UFlareCompanyAI::UpdateTrading : %s add to construction", *Ship->GetImmatriculation().ToString());
			}

			if (!Usefull && Ship->GetCargoBay()->GetFreeSlotCount() > 0)
			{
				IdleCargoCapacity += Ship->GetCargoBay()->GetCapacity() * Ship->GetCargoBay()->GetFreeSlotCount();
			}

			// TODO recruit to build station
		}
	}
}

void UFlareCompanyAI::RepairAndRefill()
{
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
									  float* CurrentConstructionScore,
									  float* BestScore,
									  FFlareSpacecraftDescription** BestStationDescription,
									  UFlareSimulatedSpacecraft** BestStation,
									  UFlareSimulatedSector** BestSector)
{
	//FLOGV("UpdateBestScore Score=%f BestScore=%f", Score, *BestScore);

	// Update current construction score
	if (ConstructionProjectSector == Sector &&
			(Station ? ConstructionProjectStation == Station : ConstructionProjectStationDescription == StationDescription))
	{
		*CurrentConstructionScore = Score;
		//FLOGV("Current : Score=%f", Score);
	}

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


void UFlareCompanyAI::UpdateStationConstruction()
{


	// Compute shipyard need shipyard
	// Count turn before a ship is buildable to add weigth to this option

	// Compute the place the farest from all shipyard

	// Compute the time to pay the price with the station

	// If best option weight > 1, build it.

	// TODO Save ConstructionProjectStation

	if (ConstructionProjectStationDescription && ConstructionProjectSector)
	{
		TArray<FText> Reasons;
		bool ShouldBeAbleToBuild = true;
		if (!ConstructionProjectStation && !ConstructionProjectSector->CanBuildStation(ConstructionProjectStationDescription, Company, Reasons, true))
		{
			ShouldBeAbleToBuild = false;
		}


		if(ShouldBeAbleToBuild)
		{
			// TODO Need at least one cargo
			// TODO Buy cost keeping marging

			bool BuildSuccess = false;
			if(ConstructionProjectStation)
			{
				BuildSuccess = ConstructionProjectSector->CanUpgradeStation(ConstructionProjectStation, Reasons) &&
						ConstructionProjectSector->UpgradeStation(ConstructionProjectStation);
			}
			else
			{
				BuildSuccess = ConstructionProjectSector->CanBuildStation(ConstructionProjectStationDescription, Company, Reasons, false) &&
						(ConstructionProjectSector->BuildStation(ConstructionProjectStationDescription, Company) != NULL);
			}

			// Build success clean contruction project
			if (BuildSuccess)
			{
#if DEBUG_AI_CONSTRUCTION
				FLOGV("UFlareCompanyAI::UpdateStationConstruction %s build %s in %s",
					*Company->GetCompanyName().ToString(),
					*ConstructionProjectStationDescription->Name.ToString(),
					*ConstructionProjectSector->GetSectorName().ToString());
#endif
				ClearConstructionProject();
			}

			// Cannot build
			else
			{
#if DEBUG_AI_CONSTRUCTION
				FLOGV("UFlareCompanyAI::UpdateStationConstruction %s failed to build %s in %s",
					*Company->GetCompanyName().ToString(),
					*ConstructionProjectStationDescription->Name.ToString(),
					*ConstructionProjectSector->GetSectorName().ToString());
#endif
				
				int32 NeedCapacity = UFlareGameTools::ComputeConstructionCapacity(ConstructionProjectStationDescription->Identifier, Game);
				if (NeedCapacity > IdleCargoCapacity)
				{
					IdleCargoCapacity -= NeedCapacity;
				}

				FindResourcesForStationConstruction();
			}
		}

		// Abandon build project
		else
		{
#if DEBUG_AI_CONSTRUCTION
			FLOGV("UFlareCompanyAI::UpdateStationConstruction %s abandon building of %s in %s (upgrade: %d) : cannot build for strange reason",
				*Company->GetCompanyName().ToString(), *ConstructionProjectStationDescription->Name.ToString(), *ConstructionProjectSector->GetSectorName().ToString(),
				(ConstructionProjectStation != NULL));
#endif

			if (ConstructionProjectStationDescription && ConstructionProjectSector)
			{
				float StationPrice = ComputeStationPrice(ConstructionProjectSector, ConstructionProjectStationDescription, ConstructionProjectStation);
				SpendBudget(EFlareBudget::Station, -StationPrice);
			}
			ClearConstructionProject();

		}
	}
}
void UFlareCompanyAI::ClearConstructionProject()
{
	ConstructionProjectStationDescription = NULL;
	ConstructionProjectStation = NULL;
	ConstructionProjectSector = NULL;
	ConstructionProjectNeedCapacity = 0;
	ConstructionShips.Empty();
	ConstructionStaticShips.Empty();
}

void UFlareCompanyAI::FindResourcesForStationConstruction()
{

	// First, place construction ships in construction sector.

	// TODO simplify
	// TODO make price very attractive
	// TODO make capacity very high

	TArray<UFlareSimulatedSpacecraft *> ShipsInConstructionSector;
	TArray<UFlareSimulatedSpacecraft *> ShipsInOtherSector;
	TArray<UFlareSimulatedSpacecraft *> ShipsToTravel;


	// Generate ships lists
	for (int32 ShipIndex = 0; ShipIndex < ConstructionShips.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = ConstructionShips[ShipIndex];
		if (Ship->GetCurrentSector() == ConstructionProjectSector)
		{
			ShipsInConstructionSector.Add(Ship);
		}
		else if (Ship->GetCurrentSector() != NULL)
		{
			ShipsInOtherSector.Add(Ship);
			ShipsToTravel.Add(Ship);
		}
	}

	MissingResourcesQuantity.Empty();
	MissingStaticResourcesQuantity.Empty();
	TArray<int32> TotalResourcesQuantity;

	// List missing ressources
	for (int32 ResourceIndex = 0; ResourceIndex < ConstructionProjectStationDescription->CycleCost.InputResources.Num(); ResourceIndex++)
	{
		FFlareFactoryResource* Resource = &ConstructionProjectStationDescription->CycleCost.InputResources[ResourceIndex];

		int32 NeededQuantity = Resource->Quantity;

		TotalResourcesQuantity.Add(NeededQuantity);

		int32 OwnedQuantity = 0;
		int32 OwnedStaticQuantity = 0;

		// Add not in sector ships resources
		for (int32 ShipIndex = 0; ShipIndex < ConstructionShips.Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = ConstructionShips[ShipIndex];
			OwnedQuantity += Ship->GetCargoBay()->GetResourceQuantity(&Resource->Resource->Data, Ship->GetCompany());
		}

		for (int32 ShipIndex = 0; ShipIndex < ConstructionStaticShips.Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = ConstructionStaticShips[ShipIndex];
			OwnedStaticQuantity += Ship->GetCargoBay()->GetResourceQuantity(&Resource->Resource->Data, Ship->GetCompany());
		}

		if (NeededQuantity > OwnedQuantity)
		{
			MissingResourcesQuantity.Add(&Resource->Resource->Data, NeededQuantity - OwnedQuantity);
		}

		if (NeededQuantity > OwnedQuantity)
		{
			MissingStaticResourcesQuantity.Add(&Resource->Resource->Data, NeededQuantity - OwnedStaticQuantity);
		}
	}

	// Check static ships
	// 2 pass, first remove not empty slot, then empty slot
	for (int32 ShipIndex = 0; ShipIndex < ConstructionStaticShips.Num(); ShipIndex++)
	{
		if(MissingStaticResourcesQuantity.Num() ==  0)
		{
			break;
		}

		UFlareSimulatedSpacecraft* Ship = ConstructionStaticShips[ShipIndex];


		for(uint32 SlotIndex = 0; SlotIndex < Ship->GetCargoBay()->GetSlotCount(); SlotIndex++)
		{
			FFlareCargo* Cargo = Ship->GetCargoBay()->GetSlot(SlotIndex);
			if(Cargo->Quantity > 0)
			{

				int32 SlotFreeSpace = Ship->GetCargoBay()->GetSlotCapacity() - Cargo->Quantity;

				if(!MissingStaticResourcesQuantity.Contains(Cargo->Resource))
				{
					continue;
				}

				int32 MissingResourceQuantity = MissingStaticResourcesQuantity[Cargo->Resource];

				MissingResourceQuantity -= SlotFreeSpace;
				if(MissingResourceQuantity <= 0)
				{
					MissingStaticResourcesQuantity.Remove(Cargo->Resource);
				}
				else
				{
					MissingStaticResourcesQuantity[Cargo->Resource] = MissingResourceQuantity;
				}
			}
		}
	}

	for (int32 ShipIndex = 0; ShipIndex < ConstructionStaticShips.Num(); ShipIndex++)
	{
		if(MissingStaticResourcesQuantity.Num() ==  0)
		{
			break;
		}

		TArray<FFlareResourceDescription*> MissingResources;
		MissingStaticResourcesQuantity.GetKeys(MissingResources);

		int32 FirstResourceQuantity = MissingStaticResourcesQuantity[MissingResources[0]];

		UFlareSimulatedSpacecraft* Ship = ConstructionStaticShips[ShipIndex];

		for(uint32 SlotIndex = 0; SlotIndex < Ship->GetCargoBay()->GetSlotCount(); SlotIndex++)
		{
			FirstResourceQuantity-= Ship->GetCargoBay()->GetSlotCapacity();
			if(FirstResourceQuantity <=0)
			{
				break;
			}
		}


		if(FirstResourceQuantity <=0)
		{
			MissingStaticResourcesQuantity.Remove(MissingResources[0]);
		}
		else
		{
			MissingStaticResourcesQuantity[MissingResources[0]] = FirstResourceQuantity;
		}
	}

	// Add static construction ship
	for (int32 ShipIndex = 0; ShipIndex < ConstructionShips.Num(); ShipIndex++)
	{

		if(MissingStaticResourcesQuantity.Num() == 0)
		{
			break;
		}

		UFlareSimulatedSpacecraft* Ship = ConstructionShips[ShipIndex];
		if(ConstructionStaticShips.Contains(Ship))
		{
			continue;
		}

		ConstructionStaticShips.Add(Ship);
		ConstructionProjectNeedCapacity += Ship->GetCargoBay()->GetCapacity();

		for(uint32 SlotIndex = 0; SlotIndex < Ship->GetCargoBay()->GetSlotCount(); SlotIndex++)
		{
			if(MissingStaticResourcesQuantity.Num() == 0)
			{
				break;
			}
			TArray<FFlareResourceDescription*> MissingResources;
			MissingStaticResourcesQuantity.GetKeys(MissingResources);

			int32 FirstResourceQuantity = MissingStaticResourcesQuantity[MissingResources[0]];

			FirstResourceQuantity-= Ship->GetCargoBay()->GetSlotCapacity();
			if(FirstResourceQuantity <=0)
			{
				MissingStaticResourcesQuantity.Remove(MissingResources[0]);
			}
			else
			{
				MissingStaticResourcesQuantity[MissingResources[0]] = FirstResourceQuantity;
			}
		}

	}

	// Send static ship to construction sector
	for (int32 ShipIndex = 0; ShipIndex < ConstructionStaticShips.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = ConstructionStaticShips[ShipIndex];
		if (!Ship->GetCurrentFleet()->IsTraveling()  &&
				Ship->GetCurrentSector() != ConstructionProjectSector &&
				!ConstructionProjectSector->GetSectorBattleState(Company).HasDanger)
		{
			Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), ConstructionProjectSector);
		}

		ShipsInConstructionSector.Remove(Ship);
		ShipsInOtherSector.Remove(Ship);
		ShipsToTravel.Remove(Ship);

#if DEBUG_AI_CONSTRUCTION
		FLOGV("Static Construction ship %s", *Ship->GetImmatriculation().ToString());
#endif
	}

	// First strep, agregate ressources in construction sector
	for (int32 ShipIndex = 0; ShipIndex < ShipsInConstructionSector.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = ShipsInConstructionSector[ShipIndex];

#if DEBUG_AI_CONSTRUCTION
		FLOGV("Construction ship %s", *Ship->GetImmatriculation().ToString());
#endif

		// Give to others ships
		for (uint32 CargoIndex = 0; CargoIndex < Ship->GetCargoBay()->GetSlotCount(); CargoIndex++)
		{
			FFlareCargo* Cargo = Ship->GetCargoBay()->GetSlot(CargoIndex);

			if (Cargo->Resource == NULL)
			{
				continue;
			}

			FFlareResourceDescription* ResourceToGive = Cargo->Resource;
			int32 QuantityToGive = Ship->GetCargoBay()->GetResourceQuantity(ResourceToGive, Ship->GetCompany());

#if DEBUG_AI_CONSTRUCTION
			FLOGV("  %d %s to give", QuantityToGive, *ResourceToGive->Name.ToString());
#endif

			for (int32 StaticShipIndex = 0; QuantityToGive > 0 && StaticShipIndex < ConstructionStaticShips.Num(); StaticShipIndex++)
			{
				UFlareSimulatedSpacecraft* StaticShip = ConstructionStaticShips[StaticShipIndex];
				if(StaticShip->GetCurrentSector() != ConstructionProjectSector)
				{
					continue;
				}

				int32 GivenQuantity = StaticShip->GetCargoBay()->GiveResources(ResourceToGive, QuantityToGive, Ship->GetCompany());

				Ship->GetCargoBay()->TakeResources(ResourceToGive, GivenQuantity, StaticShip->GetCompany());

				QuantityToGive -= GivenQuantity;

#if DEBUG_AI_CONSTRUCTION
				FLOGV("  %d given to %s", QuantityToGive, *StaticShip->GetImmatriculation().ToString());
#endif

				if (QuantityToGive == 0)
				{
					break;
				}
			}
		}

		// Then add to "to travel" ship list if can contain some missing resources
		TArray<FFlareResourceDescription*> MissingResources;
		MissingResourcesQuantity.GetKeys(MissingResources);
		for (int32 ResourceIndex = 0; ResourceIndex < MissingResources.Num(); ResourceIndex++)
		{
			FFlareResourceDescription* MissingResource = MissingResources[ResourceIndex];

			if (Ship->GetCargoBay()->GetFreeSpaceForResource(MissingResource, Ship->GetCompany()))
			{
				// Can do more work
				ShipsToTravel.Add(Ship);
				break;
			}
		}
	}

	// if no missing ressources
	if (MissingResourcesQuantity.Num() == 0)
	{
		for (int32 ShipIndex = 0; ShipIndex < ShipsToTravel.Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = ShipsToTravel[ShipIndex];

#if DEBUG_AI_CONSTRUCTION
			FLOGV("Construction ship %s to flush", *Ship->GetImmatriculation().ToString());
#endif

			if (Ship->GetCargoBay()->GetUsedCargoSpace() > 0)
			{
				// If at least 1 resource, go to construction sector
				if(!ConstructionProjectSector->GetSectorBattleState(Company).HasDanger)
				{
					Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), ConstructionProjectSector);
				}
			}
			else
			{
				// This ship is no more needed, release it
				ConstructionShips.Remove(Ship);
			}
		}
	}
	else
	{
		// Still some resource to get
		for (int32 ShipIndex = 0; ShipIndex < ShipsToTravel.Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = ShipsToTravel[ShipIndex];

#if DEBUG_AI_CONSTRUCTION
			FLOGV("Construction ship %s to travel", *Ship->GetImmatriculation().ToString());
#endif

			TArray<FFlareResourceDescription*> MissingResources;
			MissingResourcesQuantity.GetKeys(MissingResources);
			for (int32 ResourceIndex = 0; ResourceIndex < MissingResources.Num(); ResourceIndex++)
			{
				if (Ship->IsTrading())
				{
					break;
				}

				FFlareResourceDescription* MissingResource = MissingResources[ResourceIndex];

#if DEBUG_AI_CONSTRUCTION
				if (!MissingResourcesQuantity.Contains(MissingResource))
				{
					FLOGV("UFlareCompanyAI::FindResourcesForStationConstruction : !!! MissingResourcesQuantity doesn't contain %s 0", *MissingResource->Name.ToString());
				}
#endif

				int32 MissingResourceQuantity = MissingResourcesQuantity[MissingResource];
				int32 Capacity = Ship->GetCargoBay()->GetFreeSpaceForResource(MissingResource, Ship->GetCompany());
				int32 QuantityToBuy = FMath::Min(Capacity, MissingResourceQuantity);
				int32 TakenQuantity = 0;


				// Try to load or buy
				SectorHelper::FlareTradeRequest Request;
				Request.Resource = MissingResource;
				Request.Operation = EFlareTradeRouteOperation::LoadOrBuy;
				Request.Client = Ship;
				Request.CargoLimit = AI_NERF_RATIO;
				Request.MaxQuantity = QuantityToBuy;

				UFlareSimulatedSpacecraft* StationCandidate = SectorHelper::FindTradeStation(Request);

				if (StationCandidate)
				{
					TakenQuantity = SectorHelper::Trade(StationCandidate, Ship, MissingResource, Request.MaxQuantity);

#if DEBUG_AI_CONSTRUCTION
					FLOGV("  %d %s taken to %s", TakenQuantity, *MissingResource->Name.ToString(), *StationCandidate->GetImmatriculation().ToString());
#endif
				}

				MissingResourceQuantity -= TakenQuantity;
				if (MissingResourceQuantity <= 0)
				{
					MissingResourcesQuantity.Remove(MissingResource);
				}
				else
				{
#if DEBUG_AI_CONSTRUCTION
					if (!MissingResourcesQuantity.Contains(MissingResource))
					{
						FLOGV("UFlareCompanyAI::FindResourcesForStationConstruction : !!! MissingResourcesQuantity doesn't contain %s 1", *MissingResource->Name.ToString());
					}
#endif
					MissingResourcesQuantity[MissingResource] = MissingResourceQuantity;
				}
			}

			bool IsFull = true;
			for (int32 ResourceIndex = 0; ResourceIndex < MissingResources.Num(); ResourceIndex++)
			{
				FFlareResourceDescription* MissingResource = MissingResources[ResourceIndex];

				if (Ship->GetCargoBay()->GetFreeSpaceForResource(MissingResource, Ship->GetCompany()))
				{
					// Can do more work
					IsFull = false;
					break;
				}
			}

			if (IsFull)
			{
				// Go to construction sector
				if(!ConstructionProjectSector->GetSectorBattleState(Company).HasDanger)
				{
					Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), ConstructionProjectSector);
				}
				//FLOGV("  full, travel to %s", *ConstructionProjectSector->GetSectorName().ToString());
			}
			else
			{
				// Refresh missing resources
				MissingResources.Empty();
				MissingResourcesQuantity.GetKeys(MissingResources);

				UFlareSimulatedSector* BestSector = NULL;
				FFlareResourceDescription* BestResource = NULL;
				float BestScore = 0;
				int32 BestEstimateTake = 0;

				// Look for station with stock
				for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
				{
					UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];

#if DEBUG_AI_CONSTRUCTION
					if (!WorldResourceVariation.Contains(Sector))
					{
						FLOGV("UFlareCompanyAI::FindResourcesForStationConstruction : !!! WorldResourceVariation doesn't contain %s", *Sector->GetSectorName().ToString());
					}
#endif

					SectorVariation* SectorVariation = &WorldResourceVariation[Sector];
					
					for (int32 ResourceIndex = 0; ResourceIndex < MissingResources.Num(); ResourceIndex++)
					{
						FFlareResourceDescription* MissingResource = MissingResources[ResourceIndex];
						
						struct ResourceVariation* Variation = &SectorVariation->ResourceVariations[MissingResource];

						int32 Stock = Variation->FactoryStock + Variation->OwnedStock + Variation->StorageStock;

						//FLOGV("Stock in %s for %s : %d", *Sector->GetSectorName().ToString(), *MissingResource->Name.ToString(), Stock);


						if (Stock <= 0)
						{
							continue;
						}

						// Sector with missing ressource stock
#if DEBUG_AI_CONSTRUCTION
						if (!MissingResourcesQuantity.Contains(MissingResource))
						{
							FLOGV("UFlareCompanyAI::FindResourcesForStationConstruction : !!! MissingResourcesQuantity doesn't contain %s 2", *MissingResource->Name.ToString());
						}
#endif

						int32 MissingResourceQuantity = MissingResourcesQuantity[MissingResource];
						int32 Capacity = Ship->GetCargoBay()->GetFreeSpaceForResource(MissingResource, Ship->GetCompany());
						
						float Score = FMath::Min(Stock, MissingResourceQuantity);
						Score = FMath::Min(Score, (float)Capacity);

						/*FLOGV("MissingResourceQuantity %d", MissingResourceQuantity);
						FLOGV("Capacity %d", Capacity);
						FLOGV("Score %d", Score);*/

						if (Score > 0 && (BestSector == NULL || BestScore < Score))
						{
							/*FLOGV("Best sector with stock %s for %s. Score = %f", *Sector->GetSectorName().ToString(), *MissingResource->Name.ToString(), Score);
							FLOGV("Stock = %d",Stock);
							FLOGV("Variation->FactoryStock = %d",Variation->FactoryStock);
							FLOGV("Variation->OwnedStock = %d",Variation->OwnedStock);
							FLOGV("Variation->StorageStock = %d",Variation->StorageStock);*/

							BestSector = Sector;
							BestScore = Score;
							BestResource = MissingResource;
							BestEstimateTake = FMath::Min(Capacity, Stock);
						}
					}
				}

				if (!BestSector)
				{
					// Try a sector with a flow
					for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
					{
						UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];

						if(Sector->GetSectorBattleState(Company).HasDanger)
						{
							continue;
						}

						SectorVariation* SectorVariation = &WorldResourceVariation[Sector];


						for (int32 ResourceIndex = 0; ResourceIndex < MissingResources.Num(); ResourceIndex++)
						{
							FFlareResourceDescription* MissingResource = MissingResources[ResourceIndex];


							struct ResourceVariation* Variation = &SectorVariation->ResourceVariations[MissingResource];

							int32 Flow = Variation->FactoryFlow + Variation->OwnedFlow;


							//FLOGV("Flow in %s for %s : %d", *Sector->GetSectorName().ToString(), *MissingResource->Name.ToString(), Flow);


							if (Flow >= 0)
							{
								continue;
							}

							// Sector with missing ressource stock
#if DEBUG_AI_CONSTRUCTION
							if (!MissingResourcesQuantity.Contains(MissingResource))
							{
								FLOGV("UFlareCompanyAI::FindResourcesForStationConstruction : !!! MissingResourcesQuantity doesn't contain %s 3", *MissingResource->Name.ToString());
							}
#endif
							int32 MissingResourceQuantity = MissingResourcesQuantity[MissingResource];
							int32 Capacity = Ship->GetCargoBay()->GetFreeSpaceForResource(MissingResource, Ship->GetCompany());
							
							/* Owned stock will be set negative if multiple cargo go here. This will impact the score */
							float Score = FMath::Min((float)Flow / (Variation->OwnedStock-1),(float) MissingResourceQuantity);
							Score = FMath::Min(Score, (float)Capacity);
							
							/*FLOGV("MissingResourceQuantity %d", MissingResourceQuantity);
							FLOGV("Capacity %d", Capacity);
							FLOGV("Score %f", Score);*/
							
							if (Score > 0 && (BestSector == NULL || BestScore < Score))
							{
								/*FLOGV("Best sector with flow %s for %s. Score = %f", *Sector->GetSectorName().ToString(), *MissingResource->Name.ToString(), Score);
								FLOGV("Flow = %d",Flow);
								FLOGV("Variation->FactoryFlow = %d",Variation->FactoryFlow);
								FLOGV("Variation->OwnedFlow = %d",Variation->OwnedFlow);*/
								
								BestSector = Sector;
								BestScore = Score;
								BestResource = MissingResource;
								BestEstimateTake = FMath::Min(Capacity, Flow);
							}
						}
					}
				}

				if (BestSector)
				{
					// Travel to sector
					Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), BestSector);
#if DEBUG_AI_CONSTRUCTION
					FLOGV("  travel to %s", *BestSector->GetSectorName().ToString());
#endif
					// Decrease missing quantity
#if DEBUG_AI_CONSTRUCTION
					if (!MissingResourcesQuantity.Contains(BestResource))
					{
						FLOGV("UFlareCompanyAI::FindResourcesForStationConstruction : !!! MissingResourcesQuantity doesn't contain %s 4", *BestResource->Name.ToString());
					}
#endif
					MissingResourcesQuantity[BestResource] -= FMath::Max(0, BestEstimateTake);
					SectorVariation* SectorVariation = &WorldResourceVariation[BestSector];
					struct ResourceVariation* Variation = &SectorVariation->ResourceVariations[BestResource];

					Variation->OwnedStock -= FMath::Max(0, BestEstimateTake);
				}

			}
		}
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
	// Find
#ifdef DEBUG_AI_BUDGET
	FLOGV("Process budget for %s (%d projects)", *Company->GetCompanyName().ToString(), BudgetToProcess.Num());
#endif

	EFlareBudget::Type MaxBudgetType = EFlareBudget::None;
	int64 MaxBudgetAmount = 0;

	for (EFlareBudget::Type Type : BudgetToProcess)
	{
		int64 Budget = GetBudget(Type);
		if(MaxBudgetType == EFlareBudget::None || MaxBudgetAmount < Budget)
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

	if(Behavior->GetBudgetWeight(MaxBudgetType) > 0)
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
				ProcessBudgetStation(MaxBudgetAmount, Lock, Idle);
			break;
			case EFlareBudget::Technology:
				// TODO
			break;
		}
	}

	if(Lock)
	{
#ifdef DEBUG_AI_BUDGET
		FLOG("Lock");
#endif
		// Process no other projets
		return;
	}

	if(Idle)
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
		if(OtherCompany == Company)
		{
			continue;
		}

		if(OtherCompany->GetReputation(Company) > 0)
		{
			// Friendly
			continue;
		}

		float ConfidenceLevel = Company->GetConfidenceLevel(OtherCompany);
		if(MinConfidenceLevel > ConfidenceLevel)
		{
			MinConfidenceLevel = ConfidenceLevel;
		}
	}

	if (!Company->AtWar() && MinConfidenceLevel > Behavior->ConfidenceTarget)
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
	if (IdleCargoCapacity + DamagedCargosCapacity > 0)
	{
		// Trade fllet size is ok
		Idle = true;
		Lock = false;
		return;
	}

	Idle = false;

	int64 ProjectCost = UpdateCargoShipAcquisition();

	if (ProjectCost > 0 && ProjectCost < BudgetAmount / 2)
	{
		Lock = true;
	}
}

void UFlareCompanyAI::ProcessBudgetStation(int64 BudgetAmount, bool& Lock, bool& Idle)
{
	Idle = false;
	// Prepare resources for station-building analysis
	float BestScore = 0;
	float CurrentConstructionScore = 0;
	UFlareSimulatedSector* BestSector = NULL;
	FFlareSpacecraftDescription* BestStationDescription = NULL;
	UFlareSimulatedSpacecraft* BestStation = NULL;
	TArray<UFlareSpacecraftCatalogEntry*>& StationCatalog = Game->GetSpacecraftCatalog()->StationCatalog;
#ifdef DEBUG_AI_BUDGET
	FLOGV("UFlareCompanyAI::UpdateStationConstruction statics ships : %d construction ships : %d",
		  ConstructionStaticShips.Num(), ConstructionShips.Num());
#endif

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

			// Check sector limitations
			TArray<FText> Reasons;
			if (!Sector->CanBuildStation(StationDescription, Company, Reasons, true))
			{
				continue;
			}

			//FLOGV("> Analyse build %s in %s", *StationDescription->Name.ToString(), *Sector->GetSectorName().ToString());

			// Count factories for the company, compute rentability in each sector for each station
			for (int32 FactoryIndex = 0; FactoryIndex < StationDescription->Factories.Num(); FactoryIndex++)
			{
				FFlareFactoryDescription* FactoryDescription = &StationDescription->Factories[FactoryIndex]->Data;


				// Add weight if the company already have another station in this type
				float Score = ComputeConstructionScoreForStation(Sector, StationDescription, FactoryDescription, NULL);

				UpdateBestScore(Score, Sector, StationDescription, NULL, &CurrentConstructionScore, &BestScore, &BestStationDescription, &BestStation, &BestSector);
			}

			if (StationDescription->Factories.Num() == 0)
			{
				float Score = ComputeConstructionScoreForStation(Sector, StationDescription, NULL, NULL);
				UpdateBestScore(Score, Sector, StationDescription, NULL, &CurrentConstructionScore, &BestScore, &BestStationDescription, &BestStation, &BestSector);
			}
		}

		for (int32 StationIndex = 0; StationIndex < Sector->GetSectorStations().Num(); StationIndex++)
		{
			UFlareSimulatedSpacecraft* Station = Sector->GetSectorStations()[StationIndex];
			if(Station->GetCompany() != Company)
			{
				// Only AI company station
				continue;
			}

			//FLOGV("> Analyse upgrade %s in %s", *Station->GetImmatriculation().ToString(), *Sector->GetSectorName().ToString());

			// Count factories for the company, compute rentability in each sector for each station
			for (int32 FactoryIndex = 0; FactoryIndex < Station->GetDescription()->Factories.Num(); FactoryIndex++)
			{
				FFlareFactoryDescription* FactoryDescription = &Station->GetDescription()->Factories[FactoryIndex]->Data;

				// Add weight if the company already have another station in this type
				float Score = ComputeConstructionScoreForStation(Sector, Station->GetDescription(), FactoryDescription, Station);

				UpdateBestScore(Score, Sector, Station->GetDescription(), Station, &CurrentConstructionScore, &BestScore, &BestStationDescription, &BestStation, &BestSector);
			}

			if (Station->GetDescription()->Factories.Num() == 0)
			{
				float Score = ComputeConstructionScoreForStation(Sector, Station->GetDescription(), NULL, Station);
				UpdateBestScore(Score, Sector, Station->GetDescription(), Station, &CurrentConstructionScore, &BestScore, &BestStationDescription, &BestStation, &BestSector);
			}

		}
	}

	if (CurrentConstructionScore == 0)
	{
		if (ConstructionProjectStationDescription && ConstructionProjectSector)
		{
			float StationPrice = ComputeStationPrice(ConstructionProjectSector, ConstructionProjectStationDescription, ConstructionProjectStation);
			SpendBudget(EFlareBudget::Station, -StationPrice);
		}

		ClearConstructionProject();
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
		// Start construction only if can afford to buy the station

		float StationPrice = ComputeStationPrice(BestSector, BestStationDescription, BestStation);

		bool StartConstruction = true;

		if (CurrentConstructionScore * 1.5 > BestScore)
		{
#ifdef DEBUG_AI_BUDGET
			FLOGV("    dont change construction yet : current score is %f but best score is %f", CurrentConstructionScore, BestScore);
#endif
		}
		else
		{
			// TODO count owned resources
			if (StationPrice > Company->GetMoney())
			{
				StartConstruction = false;
#ifdef DEBUG_AI_BUDGET
				FLOGV("    dont build yet :station cost %f but company has only %lld", StationPrice, Company->GetMoney());
#endif
			}

			int32 NeedCapacity = UFlareGameTools::ComputeConstructionCapacity(BestStationDescription->Identifier, Game);



			if (NeedCapacity > IdleCargoCapacity * 1.5)
			{
				IdleCargoCapacity -= NeedCapacity * 1.5; // Keep margin
				StartConstruction = false;
#ifdef DEBUG_AI_BUDGET
				FLOGV("    dont build yet :station need %d idle capacity but company has only %d", NeedCapacity, IdleCargoCapacity);
#endif
			}

			if (StartConstruction)
			{
				// Cancel previous price
				if (ConstructionProjectStationDescription && ConstructionProjectSector)
				{
					float PreviousStationPrice = ComputeStationPrice(ConstructionProjectSector, ConstructionProjectStationDescription, ConstructionProjectStation);
					SpendBudget(EFlareBudget::Station, -PreviousStationPrice);
				}

#ifdef DEBUG_AI_BUDGET
				FLOG("Start construction");
#endif
				ConstructionProjectStationDescription = BestStationDescription;
				ConstructionProjectSector = BestSector;
				ConstructionProjectStation = BestStation;
				ConstructionProjectNeedCapacity = NeedCapacity * 1.5;
				ConstructionShips.Empty();
				ConstructionStaticShips.Empty();

				SpendBudget(EFlareBudget::Station, StationPrice);
#ifdef DEBUG_AI_BUDGET
				FLOGV("  ConstructionProjectNeedCapacity = %d", ConstructionProjectNeedCapacity);
#endif
				GameLog::AIConstructionStart(Company, ConstructionProjectSector, ConstructionProjectStationDescription, ConstructionProjectStation);
			}
			else if (ConstructionProjectStationDescription && ConstructionProjectSector)
			{
#ifdef DEBUG_AI_BUDGET
				FLOGV("UFlareCompanyAI::UpdateStationConstruction %s abandon building of %s in %s (upgrade: %d) : want to change construction", *Company->GetCompanyName().ToString(), *ConstructionProjectStationDescription->Name.ToString(), *ConstructionProjectSector->GetSectorName().ToString(), (ConstructionProjectStation != NULL));
#endif
				ClearConstructionProject();
				SpendBudget(EFlareBudget::Station, -StationPrice);
			}

			if(StationPrice < BudgetAmount / 2)
			{
				Lock = true;
			}
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
	if(IsBuildingShip(false))
	{
		return 0;
	}

	FFlareSpacecraftDescription* ShipDescription = FindBestShipToBuild(false);
	if(ShipDescription == NULL)
	{
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
	if(limitToOne && IsBuildingShip(true))
	{
		return 0;
	}

	FFlareSpacecraftDescription* ShipDescription = FindBestShipToBuild(true);

	return OrderOneShip(ShipDescription);
}


/*----------------------------------------------------
	Military AI
----------------------------------------------------*/


void UFlareCompanyAI::UpdateMilitaryMovement()
{
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
		if(BattleState.InBattle)
		{
			if(BattleState.InFight)
			{
				// Still no winner
			}
			else if(BattleState.BattleWon)
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

		if(BattleLost)
		{
			AIData.Caution += Behavior->DefeatAdaptation;
#ifdef DEBUG_AI_BATTLE_STATES
			FLOGV("%s lost battle in %s, new caution is %f",
				*Company->GetCompanyName().ToString(), *Sector->GetSectorName().ToString(),
				AIData.Caution);
#endif

		}

		if(BattleWin)
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

TArray<WarTargetIncomingFleet> UFlareCompanyAI::GenerateWarTargetIncomingFleets(UFlareSimulatedSector* DestinationSector)
{
	TArray<WarTargetIncomingFleet> IncomingFleetList;

	for (UFlareTravel* Travel : Game->GetGameWorld()->GetTravels())
	{
		if (Travel->GetDestinationSector() != DestinationSector)
		{
			continue;
		}

		if (Travel->GetFleet()->GetFleetCompany() != Company)
		{
			continue;
		}

		int64 TravelDuration = Travel->GetRemainingTravelDuration();
		int64 ArmyValue = 0;


		for (UFlareSimulatedSpacecraft* Ship : Travel->GetFleet()->GetShips())
		{
			ArmyValue += Ship->ComputeCombatValue();
		}

		// Add an entry or modify one
		bool ExistingTravelFound = false;
		for(WarTargetIncomingFleet& Fleet : IncomingFleetList)
		{
			if(Fleet.TravelDuration  == TravelDuration)
			{
				Fleet.ArmyValue += ArmyValue;
				ExistingTravelFound = true;
				break;
			}
		}

		if (!ExistingTravelFound)
		{
			WarTargetIncomingFleet Fleet;
			Fleet.TravelDuration = TravelDuration;
			Fleet.ArmyValue = ArmyValue;
			IncomingFleetList.Add(Fleet);
		}
	}
	return IncomingFleetList;
}

inline static bool WarTargetComparator(const WarTarget& ip1, const WarTarget& ip2)
{
	bool SELECT_TARGET1 = true;
	bool SELECT_TARGET2 = false;

	if(ip1.EnemyArmyValue > 0 && ip2.EnemyArmyValue == 0)
	{
		return SELECT_TARGET1;
	}

	if(ip2.EnemyArmyValue > 0 && ip1.EnemyArmyValue == 0)
	{
		return SELECT_TARGET2;
	}

	if(ip1.EnemyArmyValue > 0 && ip2.EnemyArmyValue > 0)
	{
		// Defend military
		if(ip1.OwnedMilitaryCount > ip2.OwnedMilitaryCount)
		{
			return SELECT_TARGET1;
		}

		if(ip2.OwnedMilitaryCount > ip1.OwnedMilitaryCount)
		{
			return SELECT_TARGET2;
		}

		// Defend station
		if(ip1.OwnedStationCount > ip2.OwnedStationCount)
		{
			return SELECT_TARGET1;
		}

		if(ip2.OwnedStationCount > ip1.OwnedStationCount)
		{
			return SELECT_TARGET2;
		}

		// Defend cargo
		if(ip1.OwnedCargoCount > ip2.OwnedCargoCount)
		{
			return SELECT_TARGET1;
		}

		if(ip2.OwnedCargoCount > ip1.OwnedCargoCount)
		{
			return SELECT_TARGET2;
		}

		return ip1.EnemyArmyValue > ip2.EnemyArmyValue;
	}


	// Cargo or station
	return FMath::RandBool();
}


TArray<WarTarget> UFlareCompanyAI::GenerateWarTargetList()
{
	TArray<WarTarget> WarTargetList;

	for (UFlareSimulatedSector* Sector : Company->GetKnownSectors())
	{
		bool IsTarget = false;

		if (Sector->GetSectorBattleState(Company).HasDanger)
		{
			IsTarget = true;
		}

		for(UFlareSimulatedSpacecraft* Spacecraft : Sector->GetSectorSpacecrafts())
		{
			if(!Spacecraft->IsStation() && !Spacecraft->IsMilitary() && Spacecraft->GetDamageSystem()->IsUncontrollable())
			{
				// Don't target uncontrollable ships
				continue;
			}

			if(Spacecraft->GetCompany()->GetWarState(Company) == EFlareHostility::Hostile)
			{
				IsTarget = true;
				break;
			}
		}

		if(!IsTarget)
		{
			continue;
		}

		WarTarget Target;
		Target.Sector = Sector;
		Target.EnemyArmyValue = 0;
		Target.EnemyArmyLValue = 0;
		Target.EnemyArmySValue = 0;
		Target.EnemyCargoCount = 0;
		Target.EnemyStationCount = 0;
		Target.OwnedArmyValue = 0;
		Target.OwnedArmyAntiSValue = 0;
		Target.OwnedArmyAntiLValue = 0;
		Target.OwnedCargoCount = 0;
		Target.OwnedStationCount = 0;
		Target.OwnedMilitaryCount = 0;
		Target.WarTargetIncomingFleets = GenerateWarTargetIncomingFleets(Sector);

		for(UFlareSimulatedSpacecraft* Spacecraft : Sector->GetSectorSpacecrafts())
		{
			EFlareHostility::Type Hostility = Spacecraft->GetCompany()->GetWarState(Company);

			if (Hostility == EFlareHostility::Hostile)
			{
				if(Spacecraft->IsStation())
				{
					Target.EnemyStationCount++;
				}
				else
				{
					if(Spacecraft->IsMilitary())
					{
						int64 ShipValue = Spacecraft->ComputeCombatValue();
						Target.EnemyArmyValue += ShipValue;

						if (Spacecraft->GetSize() == EFlarePartSize::L)
						{
							Target.EnemyArmyLValue += ShipValue;
						}
						else
						{
							Target.EnemyArmySValue += ShipValue;
						}
					}
					else
					{
						Target.EnemyCargoCount++;
					}
				}
			}
			else if (Hostility == EFlareHostility::Owned)
			{
				if(Spacecraft->IsStation())
				{
					Target.OwnedStationCount++;
				}
				else
				{
					if(Spacecraft->IsMilitary())
					{
						int64 ShipValue = Spacecraft->ComputeCombatValue();

						Target.OwnedArmyValue += ShipValue;
						Target.OwnedMilitaryCount++;

						if (Spacecraft->GetWeaponsSystem()->HasAntiLargeShipWeapon())
						{
							Target.OwnedArmyAntiLValue += ShipValue;
						}

						if (Spacecraft->GetWeaponsSystem()->HasAntiSmallShipWeapon())
						{
							Target.OwnedArmyAntiSValue += ShipValue;
						}
					}
					else
					{
						Target.OwnedCargoCount++;
					}
				}
			}
		}


		if (Target.OwnedArmyAntiLValue <= Target.EnemyArmyLValue * Behavior->RetreatThreshold ||
				Target.OwnedArmyAntiSValue <= Target.EnemyArmySValue * Behavior->RetreatThreshold)
		{
			WarTargetList.Add(Target);

			// Retreat
			TArray<UFlareSimulatedSpacecraft*> MovableShips = GenerateWarShipList(Target.Sector);
			if (MovableShips.Num() > 0)
			{
				// Find nearest sector without danger with available FS and travel there
				UFlareSimulatedSector* RetreatSector = FindNearestSectorWithFS(Target.Sector);
				if (!RetreatSector)
				{
					RetreatSector = FindNearestSectorWithPeace(Target.Sector);
				}

				if(RetreatSector)
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

TArray<DefenseSector> UFlareCompanyAI::GenerateDefenseSectorList()
{
	TArray<DefenseSector> DefenseSectorList;

	for (UFlareSimulatedSector* Sector : Company->GetKnownSectors())
	{
		if(Sector->GetSectorBattleState(Company).HasDanger)
		{
			continue;
		}

		DefenseSector Target;
		Target.Sector = Sector;
		Target.ArmyValue = 0;
		Target.ArmyAntiSValue = 0;
		Target.ArmyAntiLValue = 0;
		Target.LargeShipArmyValue = 0;
		Target.SmallShipArmyValue = 0;
		Target.LargeShipArmyCount = 0;
		Target.SmallShipArmyCount = 0;

		for (UFlareSimulatedSpacecraft* Ship : Sector->GetSectorShips())
		{
			int64 ShipValue = Ship->ComputeCombatValue();

			if (Ship->GetCompany() != Company
			 || Ship->CanTravel() == false
			 || ShipValue == 0)
			{
				continue;
			}
			
			Target.ArmyValue += ShipValue;
			if (Ship->GetSize() == EFlarePartSize::L)
			{
				Target.LargeShipArmyValue += ShipValue;
				Target.LargeShipArmyCount++;
			}
			else
			{
				Target.SmallShipArmyValue += ShipValue;
				Target.SmallShipArmyCount++;
			}

			if (Ship->GetWeaponsSystem()->HasAntiLargeShipWeapon())
			{
				Target.ArmyAntiLValue += ShipValue;
			}

			if (Ship->GetWeaponsSystem()->HasAntiSmallShipWeapon())
			{
				Target.ArmyAntiSValue += ShipValue;
			}
		}

		Target.CapturingStation = false;
		TArray<UFlareSimulatedSpacecraft*>& Stations =  Sector->GetSectorStations();
		for (UFlareSimulatedSpacecraft* Station : Stations)
		{
			// Capturing station
			if (Station->GetCompany()->GetWarState(Company) == EFlareHostility::Hostile)
			{
				Target.CapturingStation = true;
				break;
			}
		}

		if (Target.ArmyValue > 0)
		{
			DefenseSectorList.Add(Target);
		}
	}

	return DefenseSectorList;
}

TArray<UFlareSimulatedSpacecraft*> UFlareCompanyAI::GenerateWarShipList(UFlareSimulatedSector* Sector)
{
	TArray<UFlareSimulatedSpacecraft*> WarShips;

	for (UFlareSimulatedSpacecraft* Ship : Sector->GetSectorShips())
	{
		if ( Ship->GetCompany() == Company
		 &&  Ship->CanTravel()
		 && !Ship->GetDamageSystem()->IsDisarmed()
		 && !Ship->GetDamageSystem()->IsStranded())
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
		int64 TravelDuration = UFlareTravel::ComputeTravelDuration(GetGame()->GetGameWorld(), OriginSector.Sector, Sector.Sector);
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
		if(Sector.Sector == OriginSector.Sector)
		{
			continue;
		}

		int64 TravelDuration = UFlareTravel::ComputeTravelDuration(GetGame()->GetGameWorld(), OriginSector.Sector, Sector.Sector);
		if (TravelDuration <= MaxTravelDuration)
		{
			Sectors.Add(Sector);
		}
	}

	return Sectors;
}

inline static bool SectorDefenseDistanceComparator(const DefenseSector& ip1, const DefenseSector& ip2)
{
	int64 ip1TravelDuration = UFlareTravel::ComputeTravelDuration(ip1.Sector->GetGame()->GetGameWorld(), ip1.TempBaseSector, ip1.Sector);
	int64 ip2TravelDuration = UFlareTravel::ComputeTravelDuration(ip1.Sector->GetGame()->GetGameWorld(), ip2.TempBaseSector, ip2.Sector);

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

void UFlareCompanyAI::UpdateWarMilitaryMovement()
{
	TArray<WarTarget> TargetList = GenerateWarTargetList();
	TArray<DefenseSector> DefenseSectorList = GenerateDefenseSectorList();

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
				FLOGV("check attack from %s to %s : ArmyValue=%lld, EnemyArmyValue=%lld",
					*Sector.Sector->GetSectorName().ToString(),
					*Target.Sector->GetSectorName().ToString(),
					 Sector.ArmyValue, Target.EnemyArmyValue);
#endif

			// Check if the army is strong enough
			if (Sector.ArmyValue < Target.EnemyArmyValue * Behavior->GetAttackThreshold())
			{
				// Army too weak
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("army at %s too weak to attack %s : ArmyValue = %lld, EnemyArmyValue = %lld ",
					*Sector.Sector->GetSectorName().ToString(),
					*Target.Sector->GetSectorName().ToString(),
					Sector.ArmyValue, Target.EnemyArmyValue);
#endif

				continue;
			}

			if(Target.EnemyArmyValue == 0 && Sector.CapturingStation)
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
			int64 TravelDuration = UFlareTravel::ComputeTravelDuration(GetGame()->GetGameWorld(), Sector.Sector, Target.Sector);
			for (WarTargetIncomingFleet& Fleet : Target.WarTargetIncomingFleets)
			{
				// Incoming fleet will be late, ignore it
				if (Fleet.TravelDuration > TravelDuration)
				{
					continue;
				}

				// Incoming fleet is too weak, ignore it
				else if (Fleet.ArmyValue <  Target.EnemyArmyValue * Behavior->GetAttackThreshold())
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
			int64 AntiLFleetValue = 0;
			int64 AntiSFleetValue = 0;
			int64 AntiLFleetValueLimit = Target.EnemyArmyLValue * Behavior->GetAttackThreshold() * 1.5;
			int64 AntiSFleetValueLimit = Target.EnemyArmySValue * Behavior->GetAttackThreshold() * 1.5;
			TArray<UFlareSimulatedSpacecraft*> MovableShips = GenerateWarShipList(Sector.Sector);


			// Check if weapon are optimal
			// Check if the army is strong enough
			if (Sector.ArmyAntiLValue < Target.EnemyArmyLValue * Behavior->GetAttackThreshold() ||
					Sector.ArmyAntiSValue < Target.EnemyArmySValue * Behavior->GetAttackThreshold())
			{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("army at %s want to attack %s after upgrade : ArmyAntiLValue=%lld, EnemyArmyLValue=%lld, ArmyAntiSValue=%lld, EnemyArmySValue=%lld",
					*Sector.Sector->GetSectorName().ToString(),
					*Target.Sector->GetSectorName().ToString(),
					 Sector.ArmyAntiLValue, Target.EnemyArmyLValue,Sector.ArmyAntiSValue, Target.EnemyArmySValue);
#endif
				if(!UpgradeMilitaryFleet(Target, Sector, MovableShips))
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
			while (MovableShips.Num() > 0 &&
				   ((SentShips < MinShipToSend) || (AntiLFleetValue < AntiLFleetValueLimit || AntiSFleetValue < AntiSFleetValueLimit)))
			{
				int32 ShipIndex = FMath::RandRange(0, MovableShips.Num()-1);

				UFlareSimulatedSpacecraft* SelectedShip = MovableShips[ShipIndex];
				MovableShips.RemoveAt(ShipIndex);

				int64 ShipValue = SelectedShip->ComputeCombatValue();
				if (SelectedShip->GetWeaponsSystem()->HasAntiLargeShipWeapon())
				{
					AntiLFleetValue += ShipValue;
					Sector.ArmyValue -= ShipValue;
					Sector.ArmyAntiLValue -= ShipValue;
				}
				else if (SelectedShip->GetWeaponsSystem()->HasAntiSmallShipWeapon())
				{
					AntiSFleetValue += ShipValue;
					Sector.ArmyValue -= ShipValue;
					Sector.ArmyAntiSValue -= ShipValue;
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

				Game->GetGameWorld()->StartTravel(SelectedShip->GetCurrentFleet(), Target.Sector);
				SentShips++;
			}

			if (Sector.ArmyValue == 0)
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

		SectorHelper::GetRefillFleetSupplyNeeds(Sector.Sector, Company, NeededFS, TotalNeededFS);
		CumulatedTotalNeededFS += TotalNeededFS;

		SectorHelper::GetRepairFleetSupplyNeeds(Sector.Sector, Company, NeededFS, TotalNeededFS);
		CumulatedTotalNeededFS += TotalNeededFS;

		if (CumulatedTotalNeededFS > 0)
		{
			// FS need

			int32 AvailableFS;
			int32 OwnedFS;
			int32 AffordableFS;
			SectorHelper::GetAvailableFleetSupplyCount(Sector.Sector, Company, OwnedFS, AvailableFS, AffordableFS);

			if(AvailableFS == 0)
			{
				// Find nearest sector without danger with available FS and travel there
				UFlareSimulatedSector* RepairSector = FindNearestSectorWithFS(Sector.Sector);
				if (RepairSector)
				{
					TArray<UFlareSimulatedSpacecraft*> MovableShips = GenerateWarShipList(Sector.Sector);
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
				FLOGV("army at %s DefenseSectorListInRange(%d)=%lld",
				*Sector.Sector->GetSectorName().ToString(), TravelDuration, DefenseSectorListInRange.Num());
#endif

			if (DefenseSectorListInRange.Num() == 0)
			{
				continue;
			}

			// Find bigger
			DefenseSector StrongestSector;
			StrongestSector.Sector = NULL;
			StrongestSector.ArmyValue = 0;
			for (DefenseSector& DistantSector : DefenseSectorListInRange)
			{
				if(!StrongestSector.Sector || StrongestSector.ArmyValue < DistantSector.ArmyValue)
				{
					StrongestSector = DistantSector;
				}

			}

			if (StrongestSector.ArmyValue > Sector.ArmyValue)
			{
				// There is a stronger sector, travel here if no incoming army before
				bool IncomingFleet = false;
				TArray<WarTargetIncomingFleet> WarTargetIncomingFleets = GenerateWarTargetIncomingFleets(StrongestSector.Sector);
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
				TArray<UFlareSimulatedSpacecraft*> MovableShips = GenerateWarShipList(Sector.Sector);
				for (UFlareSimulatedSpacecraft* Ship : MovableShips)
				{
					FLOGV("UpdateWarMilitaryMovement%s : move %s from %s to %s for defense",
						*Company->GetCompanyName().ToString(),
						*Ship->GetImmatriculation().ToString(),
						*Ship->GetCurrentSector()->GetSectorName().ToString(),
						*StrongestSector.Sector->GetSectorName().ToString());

					Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), StrongestSector.Sector);
				}

				Sector.ArmyValue = 0;
			}
			else
			{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("army at %s won't move to defend at %s: army is weaker (StrongestSector.ArmyValue=%lld Sector.ArmyValue=%lld)",
				*Sector.Sector->GetSectorName().ToString(),
				*StrongestSector.Sector->GetSectorName().ToString(),
				StrongestSector.ArmyValue, Sector.ArmyValue);
#endif
			}


			break;
		}
	}
}

UFlareSimulatedSector* UFlareCompanyAI::FindNearestSectorWithPeace(UFlareSimulatedSector* OriginSector)
{
	UFlareSimulatedSector* NearestSector = NULL;
	int64 NearestDuration = 0;

	for (UFlareSimulatedSector* Sector : Company->GetKnownSectors())
	{
		FFlareSectorBattleState BattleState = Sector->GetSectorBattleState(Company);
		if (BattleState.HasDanger)
		{
			continue;
		}

		int64 Duration = UFlareTravel::ComputeTravelDuration(Sector->GetGame()->GetGameWorld(), OriginSector, Sector);

		if (!NearestSector || Duration < NearestDuration)
		{
			NearestSector = Sector;
			NearestDuration = Duration;
		}

	}
	return NearestSector;
}

UFlareSimulatedSector* UFlareCompanyAI::FindNearestSectorWithFS(UFlareSimulatedSector* OriginSector)
{
	UFlareSimulatedSector* NearestSector = NULL;
	int64 NearestDuration = 0;

	for (UFlareSimulatedSector* Sector : Company->GetKnownSectors())
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

		if(AvailableFS == 0)
		{
			continue;
		}

		int64 Duration = UFlareTravel::ComputeTravelDuration(Sector->GetGame()->GetGameWorld(), OriginSector, Sector);

		if (!NearestSector || Duration < NearestDuration)
		{
			NearestSector = Sector;
			NearestDuration = Duration;
		}

	}
	return NearestSector;
}

UFlareSimulatedSector* UFlareCompanyAI::FindNearestSectorWithUpgradePossible(UFlareSimulatedSector* OriginSector)
{
	UFlareSimulatedSector* NearestSector = NULL;
	int64 NearestDuration = 0;

	for (UFlareSimulatedSector* Sector : Company->GetKnownSectors())
	{
		if (!Sector->CanUpgrade(Company))
		{
			continue;
		}

		int64 Duration = UFlareTravel::ComputeTravelDuration(Sector->GetGame()->GetGameWorld(), OriginSector, Sector);

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
			if(!Part->WeaponCharacteristics.IsWeapon)
			{
				continue;
			}

			if(WeaponTargetSize == EFlarePartSize::L)
			{
				if(Part->WeaponCharacteristics.BombCharacteristics.IsBomb && FMath::FRand() < 0.8)
				{
					continue;
				}

				if (Part->WeaponCharacteristics.DamageType == EFlareShellDamageType::HEAT)
				{
					// Compatible target
					bool HasChance = FMath::FRand() < 0.7;
					if(!BestWeapon || (BestWeapon->Cost < Part->Cost && HasChance))
					{
						BestWeapon = Part;
					}
				}
			}
			else
			{
				if(Part->WeaponCharacteristics.BombCharacteristics.IsBomb)
				{
					continue;
				}

				if (Part->WeaponCharacteristics.DamageType != EFlareShellDamageType::HEAT)
				{
					// Compatible target
					bool HasChance = FMath::RandBool();
					if(!BestWeapon || (BestWeapon->Cost < Part->Cost && HasChance))
					{
						BestWeapon = Part;
					}
				}

			}
		}

		if(BestWeapon)
		{
			int32 WeaponGroupCount = Ship->GetDescription()->WeaponGroups.Num();
			int64 TransactionCost = 0;
			for(int32 WeaponGroupIndex = 0; WeaponGroupIndex < WeaponGroupCount; WeaponGroupIndex++)
			{
				FFlareSpacecraftComponentDescription* OldWeapon = Ship->GetCurrentPart(EFlarePartType::Weapon, WeaponGroupIndex);
				TransactionCost += Ship->GetUpgradeCost(BestWeapon, OldWeapon);
			}

			if(TransactionCost > Company->GetMoney())
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
			for(int32 WeaponGroupIndex = 0; WeaponGroupIndex < WeaponGroupCount; WeaponGroupIndex++)
			{
				if(Ship->UpgradePart(BestWeapon, WeaponGroupIndex))
				{
					SpendBudget(EFlareBudget::Military, TransactionCost);
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
	if(FMath::RandBool() && Ship->CanUpgrade(EFlarePartType::RCS)) // 50 % chance
	{
		// iterate to find best par
		FFlareSpacecraftComponentDescription* OldPart = Ship->GetCurrentPart(EFlarePartType::RCS, 0);
		FFlareSpacecraftComponentDescription* BestPart = OldPart;
		TArray< FFlareSpacecraftComponentDescription* > PartListData;
		Catalog->GetRCSList(PartListData, Ship->GetSize());

		for (FFlareSpacecraftComponentDescription* Part : PartListData)
		{
			bool HasChance = FMath::RandBool();
			if(!BestPart || (BestPart->Cost < Part->Cost && HasChance))
			{
				BestPart = Part;
			}
		}

		if(BestPart != OldPart)
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
	if(FMath::RandBool() && Ship->CanUpgrade(EFlarePartType::OrbitalEngine)) // 50 % chance
	{
		// iterate to find best par
		FFlareSpacecraftComponentDescription* OldPart = Ship->GetCurrentPart(EFlarePartType::OrbitalEngine, 0);
		FFlareSpacecraftComponentDescription* BestPart = OldPart;
		TArray< FFlareSpacecraftComponentDescription* > PartListData;
		Catalog->GetEngineList(PartListData, Ship->GetSize());

		for (FFlareSpacecraftComponentDescription* Part : PartListData)
		{
			bool HasChance = FMath::RandBool();
			if(!BestPart || (BestPart->Cost < Part->Cost && HasChance))
			{
				BestPart = Part;
			}
		}

		if(BestPart != OldPart)
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

bool UFlareCompanyAI::UpgradeMilitaryFleet(WarTarget Target, DefenseSector& Sector, TArray<UFlareSimulatedSpacecraft*> &MovableShips)
{
	// First check if upgrade is possible in sector
	// If not, find the closest sector where upgrade is possible and travel here
	if(!Sector.Sector->CanUpgrade(Company))
	{
		UFlareSimulatedSector* UpgradeSector = FindNearestSectorWithUpgradePossible(Sector.Sector);
		if(UpgradeSector)
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

		bool EnoughAntiL = Sector.ArmyAntiLValue >= Target.EnemyArmyLValue * Behavior->GetAttackThreshold();
		bool EnoughAntiS = Sector.ArmyAntiSValue >= Target.EnemyArmySValue * Behavior->GetAttackThreshold();
		int64 CombatValue = Ship->ComputeCombatValue();
		bool HasAntiLargeShipWeapon = Ship->GetWeaponsSystem()->HasAntiLargeShipWeapon();
		bool HasAntiSmallShipWeapon = Ship->GetWeaponsSystem()->HasAntiSmallShipWeapon();

		//if no enought anti L, upgrade to anti L
		if (!EnoughAntiL && !HasAntiLargeShipWeapon)
		{
			bool Upgraded = UpgradeShip(Ship, EFlarePartSize::L);
			if (Upgraded)
			{
				Sector.ArmyAntiLValue += CombatValue;
				if (HasAntiSmallShipWeapon)
				{
					Sector.ArmyAntiSValue -= CombatValue;
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
		else if(!EnoughAntiS && HasAntiLargeShipWeapon)
		{
			bool EnoughAntiLWithoutShip = (Sector.ArmyAntiLValue - CombatValue) >= Target.EnemyArmyLValue * Behavior->GetAttackThreshold();
			if(EnoughAntiLWithoutShip)
			{
				bool Upgraded = UpgradeShip(Ship, EFlarePartSize::S);
				if (Upgraded)
				{
					Sector.ArmyAntiSValue += CombatValue;
					Sector.ArmyAntiLValue -= CombatValue;
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

		bool EnoughAntiL = Sector.ArmyAntiLValue >= Target.EnemyArmyLValue * Behavior->GetAttackThreshold();
		bool EnoughAntiS = Sector.ArmyAntiSValue >= Target.EnemyArmySValue * Behavior->GetAttackThreshold();
		int64 CombatValue = Ship->ComputeCombatValue();
		bool HasAntiLargeShipWeapon = Ship->GetWeaponsSystem()->HasAntiLargeShipWeapon();
		bool HasAntiSmallShipWeapon = Ship->GetWeaponsSystem()->HasAntiSmallShipWeapon();

		//if no enought anti S, upgrade to anti S
		if (!EnoughAntiS && !HasAntiSmallShipWeapon)
		{
			bool Upgraded = UpgradeShip(Ship, EFlarePartSize::S);
			if (Upgraded)
			{
				Sector.ArmyAntiSValue += CombatValue;
				if (HasAntiLargeShipWeapon)
				{
					Sector.ArmyAntiLValue -= CombatValue;
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
		else if(!EnoughAntiL && HasAntiSmallShipWeapon)
		{
			bool EnoughAntiSWithoutShip = (Sector.ArmyAntiSValue - CombatValue) >= Target.EnemyArmySValue * Behavior->GetAttackThreshold();
			if(EnoughAntiSWithoutShip)
			{
				bool Upgraded = UpgradeShip(Ship, EFlarePartSize::L);
				if (Upgraded)
				{
					Sector.ArmyAntiLValue += CombatValue;
					Sector.ArmyAntiSValue -= CombatValue;
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
	if ((Sector.ArmyAntiLValue >= Target.EnemyArmyLValue * Behavior->GetAttackThreshold()) &&
			(Sector.ArmyAntiSValue >= Target.EnemyArmySValue * Behavior->GetAttackThreshold()))
	{
		for (UFlareSimulatedSpacecraft* Ship : MovableShips)
		{
			float AntiLRatio = (Target.EnemyArmyLValue > 0 ?
									Sector.ArmyAntiLValue / Target.EnemyArmyLValue * Behavior->GetAttackThreshold() :
									1000);
			float AntiSRatio = (Target.EnemyArmySValue > 0 ?
									Sector.ArmyAntiSValue / Target.EnemyArmySValue * Behavior->GetAttackThreshold() :
									1000);

			if(AntiLRatio == AntiSRatio)
			{
				// Any change will change the balance
				break;
			}

			if(AntiLRatio > AntiSRatio)
			{
				if (Ship->GetWeaponsSystem()->HasAntiLargeShipWeapon())
				{
					int64 CombatValue = Ship->ComputeCombatValue();
					// See if upgrade to S improve this
					float FutureAntiLRatio = (Target.EnemyArmyLValue > 0 ?
											(Sector.ArmyAntiLValue - CombatValue) / Target.EnemyArmyLValue * Behavior->GetAttackThreshold() :
											1000);
					float FutureAntiSRatio = (Target.EnemyArmySValue > 0 ?
											(Sector.ArmyAntiSValue + CombatValue) / Target.EnemyArmySValue * Behavior->GetAttackThreshold() :
											1000);
					if(FutureAntiLRatio > FutureAntiSRatio)
					{
						// Ratio of ratio still unchanged, upgrade to S
						bool Upgraded = UpgradeShip(Ship, EFlarePartSize::S);
						if (Upgraded)
						{
							Sector.ArmyAntiSValue += CombatValue;
							Sector.ArmyAntiLValue -= CombatValue;
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
					int64 CombatValue = Ship->ComputeCombatValue();
					// See if upgrade to S improve this
					float FutureAntiLRatio = (Target.EnemyArmyLValue > 0 ?
											(Sector.ArmyAntiLValue + CombatValue) / Target.EnemyArmyLValue * Behavior->GetAttackThreshold() :
											1000);
					float FutureAntiSRatio = (Target.EnemyArmySValue > 0 ?
											(Sector.ArmyAntiSValue - CombatValue) / Target.EnemyArmySValue * Behavior->GetAttackThreshold() :
											1000);
					if(FutureAntiSRatio > FutureAntiLRatio)
					{
						// Ratio of ratio still unchanged, upgrade to L
						bool Upgraded = UpgradeShip(Ship, EFlarePartSize::L);
						if (Upgraded)
						{
							Sector.ArmyAntiLValue += CombatValue;
							Sector.ArmyAntiSValue -= CombatValue;
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

	bool FinalEnoughAntiL = Sector.ArmyAntiLValue >= Target.EnemyArmyLValue * Behavior->GetAttackThreshold();
	bool FinalEnoughAntiS = Sector.ArmyAntiSValue >= Target.EnemyArmySValue * Behavior->GetAttackThreshold();

	#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
					FLOGV("upgrade at %s failed FinalEnoughAntiL=%d, FinalEnoughAntiS=%d",
						*Sector.Sector->GetSectorName().ToString(),
						  FinalEnoughAntiL, FinalEnoughAntiS);
					FLOGV("ArmyAntiLValue=%lld EnemyArmyLValue=%lld ArmyAntiSValue=%lld EnemyArmySValue=%lld",
						Sector.ArmyAntiLValue, Target.EnemyArmyLValue,
						Sector.ArmyAntiSValue, Target.EnemyArmySValue);
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
		else if(SectorDefendableValue > 0 && SectorDefenseRatio < TotalDefenseRatio )
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
			int64 TravelDuration = UFlareTravel::ComputeTravelDuration(Game->GetGameWorld(), Ship->GetCurrentSector(), SectorCandidate);
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

int64 UFlareCompanyAI::OrderOneShip(FFlareSpacecraftDescription* ShipDescription)
{
	if(ShipDescription == NULL)
	{
		return 0;
	}

	for(int32 ShipyardIndex = 0; ShipyardIndex < Shipyards.Num(); ShipyardIndex++)
	{
		UFlareSimulatedSpacecraft* Shipyard =Shipyards[ShipyardIndex];

		TArray<UFlareFactory*>& Factories = Shipyard->GetFactories();

		for (int32 Index = 0; Index < Factories.Num(); Index++)
		{
			UFlareFactory* Factory = Factories[Index];

			// Can produce only if nobody as order a ship and nobody is buidling a ship
			if (Factory->GetOrderShipCompany() == NAME_None && Factory->GetTargetShipCompany() == NAME_None)
			{
				int64 CompanyMoney = Company->GetMoney();

				float CostSafetyMargin = 1.1f;

				// Large factory
				if (Factory->IsLargeShipyard()&& ShipDescription->Size != EFlarePartSize::L)
				{
					// Not compatible factory
					continue;
				}

				// Large factory
				if (Factory->IsSmallShipyard()&& ShipDescription->Size != EFlarePartSize::S)
				{
					// Not compatible factory
					continue;
				}

				int64 ShipPrice = UFlareGameTools::ComputeSpacecraftPrice(ShipDescription->Identifier, Shipyard->GetCurrentSector(), true);

				if (ShipPrice * CostSafetyMargin < CompanyMoney)
				{
					FName ShipClassToOrder = ShipDescription->Identifier;
					FLOGV("UFlareCompanyAI::UpdateShipAcquisition : Ordering spacecraft : '%s'", *ShipClassToOrder.ToString());
					Factory->OrderShip(Company, ShipClassToOrder);
					Factory->Start();

					SpendBudget((ShipDescription->IsMilitary() ? EFlareBudget::Military : EFlareBudget::Trade), ShipPrice);

					return 0;
				}
				else
				{
					return ShipPrice;
				}
			}
		}
	}

	return 0;
}

//#define DEBUG_AI_SHIP_ORDER
FFlareSpacecraftDescription* UFlareCompanyAI::FindBestShipToBuild(bool Military)
{

	int32 ShipSCount = 0;
	int32 ShipLCount = 0;

	// Count owned ships
	TMap<FFlareSpacecraftDescription*, int32> OwnedShipSCount;
	TMap<FFlareSpacecraftDescription*, int32> OwnedShipLCount;
	for(int32 ShipIndex = 0; ShipIndex < Company->GetCompanyShips().Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = Company->GetCompanyShips()[ShipIndex];

		if(Military != Ship->IsMilitary())
		{
			continue;
		}

		if(Ship->GetSize() ==EFlarePartSize::S)
		{
			ShipSCount++;
			if(OwnedShipSCount.Contains(Ship->GetDescription()))
			{
				OwnedShipSCount[Ship->GetDescription()]++;
			}
			else
			{
				OwnedShipSCount.Add(Ship->GetDescription(), 1);
			}
		}
		else if(Ship->GetSize() ==EFlarePartSize::L)
		{
			ShipLCount++;
			if(OwnedShipLCount.Contains(Ship->GetDescription()))
			{
				OwnedShipLCount[Ship->GetDescription()]++;
			}
			else
			{
				OwnedShipLCount.Add(Ship->GetDescription(), 1);
			}
		}
	}

	// Count ship in production
	for(int32 ShipyardIndex = 0; ShipyardIndex < Shipyards.Num(); ShipyardIndex++)
	{
		UFlareSimulatedSpacecraft* Shipyard =Shipyards[ShipyardIndex];

		TArray<UFlareFactory*>& Factories = Shipyard->GetFactories();

		for (int32 Index = 0; Index < Factories.Num(); Index++)
		{
			UFlareFactory* Factory = Factories[Index];
			if(Factory->GetTargetShipCompany() == Company->GetIdentifier())
			{
				FFlareSpacecraftDescription* BuildingShip = Game->GetSpacecraftCatalog()->Get(Factory->GetTargetShipClass());
				if(Military != BuildingShip->IsMilitary())
				{
					continue;
				}

				if(BuildingShip->Size ==EFlarePartSize::S)
				{
					ShipSCount++;
					if(OwnedShipSCount.Contains(BuildingShip))
					{
						OwnedShipSCount[BuildingShip]++;
					}
					else
					{
						OwnedShipSCount.Add(BuildingShip, 1);
					}
				}
				else if(BuildingShip->Size ==EFlarePartSize::L)
				{
					ShipLCount++;
					if(OwnedShipLCount.Contains(BuildingShip))
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




	FFlareSpacecraftDescription* BestShipDescription = NULL;

	int32 SizeDiversity = (Military ? AI_MILITARY_SIZE_DIVERSITY_THRESOLD : AI_CARGO_SIZE_DIVERSITY_THRESOLD);
	int32 Diversity = (Military ? AI_MILITARY_DIVERSITY_THRESOLD : AI_CARGO_DIVERSITY_THRESOLD);
	bool PickLShip = true;
	if((ShipLCount+1) * SizeDiversity > ShipSCount)
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

	for (int SpacecraftIndex = 0; SpacecraftIndex < Game->GetSpacecraftCatalog()->ShipCatalog.Num(); SpacecraftIndex++)
	{
		UFlareSpacecraftCatalogEntry* Entry = Game->GetSpacecraftCatalog()->ShipCatalog[SpacecraftIndex];
		FFlareSpacecraftDescription* Description = &Entry->Data;

		if(Military != Description->IsMilitary())
		{
			continue;
		}

		if (PickLShip && Description->Size != EFlarePartSize::L)
		{
			continue;
		}

		if (!PickLShip && Description->Size != EFlarePartSize::S)
		{
			continue;
		}


		TMap<FFlareSpacecraftDescription*, int32>* OwnedShipCount = (PickLShip ? &OwnedShipLCount : &OwnedShipSCount);



		if(BestShipDescription == NULL)
		{
			BestShipDescription = Description;
			continue;
		}

		int32 CandidateCount = 0;
		int32 BestCount = 0;


		if (OwnedShipCount->Contains(Description))
		{
			CandidateCount = (*OwnedShipCount)[Description];
		}

		if (OwnedShipCount->Contains(BestShipDescription))
		{
			BestCount = (*OwnedShipCount)[BestShipDescription];
		}


		bool BestBigger = (Military?  BestShipDescription->Mass > Description-> Mass
									: BestShipDescription->GetCapacity() > Description->GetCapacity());


		bool Select = false;
		if(BestBigger && BestCount * Diversity > CandidateCount)
		{
			Select = true;
		}
		else if (!BestBigger && CandidateCount * Diversity < BestCount)
		{
			Select = true;
		}

		if(Select)
		{
			BestShipDescription = Description;
		}
	}

	if(BestShipDescription == NULL)
	{
		FLOG("ERROR: no ship to build");
		return NULL;
	}

	return BestShipDescription;
}

bool UFlareCompanyAI::IsBuildingShip(bool Military)
{
	for(int32 ShipyardIndex = 0; ShipyardIndex < Shipyards.Num(); ShipyardIndex++)
	{
		UFlareSimulatedSpacecraft* Shipyard =Shipyards[ShipyardIndex];

		TArray<UFlareFactory*>& Factories = Shipyard->GetFactories();

		for (int32 Index = 0; Index < Factories.Num(); Index++)
		{
			UFlareFactory* Factory = Factories[Index];
			if(Factory->GetTargetShipCompany() == Company->GetIdentifier())
			{
				FFlareSpacecraftDescription* BuildingShip = Game->GetSpacecraftCatalog()->Get(Factory->GetTargetShipClass());
				if(Military == BuildingShip->IsMilitary())
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

			if(Company->GetWarState(Station->GetCompany()) == EFlareHostility::Hostile)
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



TArray<UFlareSimulatedSpacecraft*> UFlareCompanyAI::FindIdleCargos() const
{
	TArray<UFlareSimulatedSpacecraft*> IdleCargos;

	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];


		for (int32 ShipIndex = 0 ; ShipIndex < Sector->GetSectorShips().Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = Sector->GetSectorShips()[ShipIndex];
			if (Ship->GetCompany() != Company || Ship->GetDamageSystem()->IsStranded() || Ship->IsTrading() || (Ship->GetCurrentFleet() && Ship->GetCurrentFleet()->IsTraveling()) || Ship->GetCurrentTradeRoute() != NULL || Ship->GetCargoBay()->GetCapacity() == 0 || ConstructionShips.Contains(Ship))
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
	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];

		if(!Sector->GetSectorBattleState(Company).HasDanger)
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

		if(CargoToTravel.Num() > 0)
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
				int64 TravelDuration = UFlareTravel::ComputeTravelDuration(Game->GetGameWorld(), Sector, SectorCandidate);

				if(DistantUnsafeSector == NULL || MaxDurationTravel < TravelDuration)
				{
					MaxDurationTravel = TravelDuration;
					DistantUnsafeSector = SectorCandidate;
				}

				if(SectorCandidate->GetSectorBattleState(Company).HasDanger)
				{
					// Dont go in a dangerous sector
					continue;
				}


				if(SafeSector == NULL || MinDurationTravel > TravelDuration)
				{
					MinDurationTravel = TravelDuration;
					SafeSector = SectorCandidate;
				}
			}

			for(UFlareSimulatedSpacecraft* Ship: CargoToTravel)
			{
				if(SafeSector)
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
			if (Ship->GetCompany() != Company || Ship->IsTrading() || (Ship->GetCurrentFleet() && Ship->GetCurrentFleet()->IsTraveling()) || Ship->GetCurrentTradeRoute() != NULL || Ship->GetCargoBay()->GetCapacity() == 0 || ConstructionShips.Contains(Ship))
			{
				continue;
			}

			if(Ship->GetDamageSystem()->IsStranded())
			{
				DamagedCapacity += Ship->GetCargoBay()->GetCapacity();
			}

		}
	}
	return DamagedCapacity;
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
		for(UFlareFactory* Factory : Shipyard->GetFactories())
		{
			if(Factory->IsLargeShipyard())
			{
				LargeCount++;
			}
			else
			{
				SmallCount++;
			}

			if(Factory->GetTargetShipCompany() != NAME_None)
			{
				if(Factory->IsLargeShipyard())
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

float UFlareCompanyAI::ComputeConstructionScoreForStation(UFlareSimulatedSector* Sector, FFlareSpacecraftDescription* StationDescription, FFlareFactoryDescription* FactoryDescription, UFlareSimulatedSpacecraft* Station) const
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

	/*if(StationDescription->Capabilities.Contains(EFlareSpacecraftCapability::Maintenance))
	{
		FLOGV(">>>>>Score for %s in %s", *StationDescription->Identifier.ToString(), *Sector->GetIdentifier().ToString());
	}*/


	//TODO customer, maintenance and shipyard limit

	Score *= Behavior->GetSectorAffility(Sector);
	//FLOGV(" after sector Affility: %f", Score);


	if(StationDescription->Capabilities.Contains(EFlareSpacecraftCapability::Consumer))
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
			else if(Consumption > 0)
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
	else if(StationDescription->Capabilities.Contains(EFlareSpacecraftCapability::Maintenance))
	{
		Score *= Behavior->MaintenanceAffility;

		const SectorVariation* ThisSectorVariation = &WorldResourceVariation[Sector];

		float MaxScoreModifier = 0;

		for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->MaintenanceResources.Num(); ResourceIndex++)
		{
			FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->MaintenanceResources[ResourceIndex]->Data;
			const struct ResourceVariation* Variation = &ThisSectorVariation->ResourceVariations[Resource];


			int32 Consumption = Sector->GetPeople()->GetBasePopulation() / 10;
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
			else if(Consumption > 0)
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
	else if (FactoryDescription && FactoryDescription->IsShipyard())
	{
		Score *= Behavior->ShipyardAffility;

		Score *= GetShipyardUsageRatio() * 0.5;

		// Underflow malus
		for (int32 ResourceIndex = 0; ResourceIndex < FactoryDescription->CycleCost.InputResources.Num(); ResourceIndex++)
		{
			const FFlareFactoryResource* Resource = &FactoryDescription->CycleCost.InputResources[ResourceIndex];

			float MaxVolume = FMath::Max(WorldStats[&Resource->Resource->Data].Production, WorldStats[&Resource->Resource->Data].Consumption);
			if(MaxVolume > 0)
			{
				float UnderflowRatio = WorldStats[&Resource->Resource->Data].Balance / MaxVolume;
				if(UnderflowRatio < 0)
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
			if(MaxVolume > 0)
			{
				float UnderflowRatio = WorldStats[&Resource->Resource->Data].Balance / MaxVolume;
				if(UnderflowRatio < 0)
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

		if(Score == 0)
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
			if(MaxVolume > 0)
			{
				float OverflowRatio = WorldStats[&Resource->Resource->Data].Balance / MaxVolume;
				if(OverflowRatio > 0)
				{
					float OverflowMalus = FMath::Clamp(1.f - (OverflowRatio * 100)  / ResourceAffility, 0.f, 1.f);
					Score *= OverflowMalus;
					//FLOGV("    MaxVolume %f", MaxVolume);
					//FLOGV("    OverflowRatio %f", OverflowRatio);
					//FLOGV("    OverflowMalus %f", OverflowMalus);
				}
			}

			float ResourcePrice = Sector->GetPreciseResourcePrice(&Resource->Resource->Data);
			float PriceRatio = (ResourcePrice - (float) Resource->Resource->Data.MinPrice) / (float) (Resource->Resource->Data.MaxPrice - Resource->Resource->Data.MinPrice);


			//FLOGV("    PriceRatio %f", PriceRatio);


			Score *= PriceRatio * 2;
		}

		//FLOGV(" after output: %f", Score);

		float GainPerDay = GainPerCycle / FactoryDescription->CycleCost.ProductionTime;
		if(GainPerDay < 0)
		{
			// TODO Shipyard
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

	/*if(StationDescription->Capabilities.Contains(EFlareSpacecraftCapability::Consumer) ||
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

	if(Station)
	{
		// Upgrade
		StationPrice = STATION_CONSTRUCTION_PRICE_BONUS * (Station->GetStationUpgradeFee() +  UFlareGameTools::ComputeSpacecraftPrice(StationDescription->Identifier, Sector, true, false));
	}
	else
	{
		// Construction
		StationPrice = STATION_CONSTRUCTION_PRICE_BONUS * UFlareGameTools::ComputeSpacecraftPrice(StationDescription->Identifier, Sector, true, true);
	}
	return StationPrice;

}


SectorVariation UFlareCompanyAI::ComputeSectorResourceVariation(UFlareSimulatedSector* Sector) const
{
	SectorVariation SectorVariation;
	for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
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

		int32 SlotCapacity = Station->GetCargoBay()->GetSlotCapacity();

		for (int32 FactoryIndex = 0; FactoryIndex < Station->GetFactories().Num(); FactoryIndex++)
		{
			UFlareFactory* Factory = Station->GetFactories()[FactoryIndex];
			if ((!Factory->IsActive() || !Factory->IsNeedProduction()))
			{
				// No resources needed
				break;
			}

			// Input flow
			for (int32 ResourceIndex = 0; ResourceIndex < Factory->GetInputResourcesCount(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = Factory->GetInputResource(ResourceIndex);
				struct ResourceVariation* Variation = &SectorVariation.ResourceVariations[Resource];


				int32 Flow = Factory->GetInputResourceQuantity(ResourceIndex) / Factory->GetProductionDuration();

				int32 CanBuyQuantity =  (int32) (Station->GetCompany()->GetMoney() / Sector->GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryInput));


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

				int32 ResourceQuantity = Station->GetCargoBay()->GetResourceQuantity(Resource, Company);
				int32 Capacity = SlotCapacity - ResourceQuantity;
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

				// The AI don't let anything for the player : it's too hard
				// Make the AI ignore the sector with not enought stock or to little capacity
				Variation->OwnedCapacity -= SlotCapacity * AI_NERF_RATIO;

				float EmptyRatio = (float) Capacity / (float) SlotCapacity;
				if (EmptyRatio > AI_NERF_RATIO/2)
				{
					Variation->MinCapacity = FMath::Max(Variation->MinCapacity, (int32) (Capacity - SlotCapacity * AI_NERF_RATIO));
				}
			}

			// Ouput flow
			for (int32 ResourceIndex = 0; ResourceIndex < Factory->GetOutputResourcesCount(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = Factory->GetOutputResource(ResourceIndex);
				struct ResourceVariation* Variation = &SectorVariation.ResourceVariations[Resource];

				int32 Flow = Factory->GetOutputResourceQuantity(ResourceIndex) / Factory->GetProductionDuration();

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

				int32 Stock = Station->GetCargoBay()->GetResourceQuantity(Resource, Company);
				if (Company == Station->GetCompany())
				{
					Variation->OwnedStock += Stock;
				}
				else
				{
					Variation->FactoryStock += Stock * Behavior->TradingBuy;
				}

				// The AI don't let anything for the player : it's too hard
				// Make the AI ignore the sector with not enought stock or to little capacity
				Variation->OwnedStock -= SlotCapacity * AI_NERF_RATIO;
			}


			// TODO storage

		}

		// Customer flow
		if (Station->HasCapability(EFlareSpacecraftCapability::Consumer))
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

				int32 ResourceQuantity = Station->GetCargoBay()->GetResourceQuantity(Resource, Company);
				int32 Capacity = SlotCapacity - ResourceQuantity;
				// Dept are allowed for sell to customers
				if (ResourceQuantity < SlotCapacity)
				{
					if (Company == Station->GetCompany())
					{
						Variation->OwnedCapacity += Capacity;
					}
					else
					{
						Variation->FactoryCapacity += Capacity * Behavior->TradingSell;
					}
				}

				// The AI don't let anything for the player : it's too hard
				// Make the AI ignore the sector with not enought stock or to little capacity
				Variation->OwnedCapacity -= SlotCapacity * AI_NERF_RATIO;
				Variation->ConsumerMaxStock += Station->GetCargoBay()->GetSlotCapacity();

				float EmptyRatio = (float) Capacity / (float) SlotCapacity;
				if (EmptyRatio > AI_NERF_RATIO/2)
				{
					Variation->MinCapacity = FMath::Max(Variation->MinCapacity, (int32) (Capacity - SlotCapacity * AI_NERF_RATIO));
				}

			}
		}

		// Maintenance
		if (Station->HasCapability(EFlareSpacecraftCapability::Maintenance))
		{
			for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->MaintenanceResources.Num(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->MaintenanceResources[ResourceIndex]->Data;
				struct ResourceVariation* Variation = &SectorVariation.ResourceVariations[Resource];

				int32 ResourceQuantity = Station->GetCargoBay()->GetResourceQuantity(Resource, Company);

				int32 CanBuyQuantity =  (int32) (Station->GetCompany()->GetMoney() / Sector->GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryInput));
				int32 Capacity = SlotCapacity - ResourceQuantity;

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
				Variation->MaintenanceMaxStock += Station->GetCargoBay()->GetSlotCapacity();

				// The AI don't let anything for the player : it's too hard
				// Make the AI ignore the sector with not enought stock or to little capacity
				Variation->OwnedCapacity -= SlotCapacity * AI_NERF_RATIO;

				float EmptyRatio = (float) Capacity / (float) SlotCapacity;
				if (EmptyRatio > AI_NERF_RATIO/2)
				{
					Variation->MinCapacity = FMath::Max(Variation->MinCapacity, (int32) (Capacity - SlotCapacity * AI_NERF_RATIO));
				}

				// The owned resell its own FS

				int32 Stock = Station->GetCargoBay()->GetResourceQuantity(Resource, Company);
				if (Company == Station->GetCompany())
				{
					Variation->OwnedStock += Stock;
				}

				// The AI don't let anything for the player : it's too hard
				// Make the AI ignore the sector with not enought stock or to little capacity
				Variation->OwnedStock -= SlotCapacity * AI_NERF_RATIO;

			}
		}

		// Station construction incitation
		/*if (ConstructionProjectSector == Sector)
		{
			for (int32 ResourceIndex = 0; ResourceIndex < ConstructionProjectStation->CycleCost.InputResources.Num() ; ResourceIndex++)
			{
				FFlareFactoryResource* Resource = &ConstructionProjectStation->CycleCost.InputResources[ResourceIndex];
				struct ResourceVariation* Variation = &SectorVariation.ResourceVariations[&Resource->Resource->Data];
				Variation->OwnedCapacity += Resource->Quantity;
			}
		}*/
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

			if (Ship->GetCargoBay()->GetSlotCapacity() == 0 && Ship->GetDamageSystem()->IsStranded())
			{
				continue;
			}
			SectorVariation.IncomingCapacity += Ship->GetCargoBay()->GetCapacity() / RemainingTravelDuration;

			TArray<FFlareCargo>& CargoBaySlots = Ship->GetCargoBay()->GetSlots();
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

			SectorHelper::GetRefillFleetSupplyNeeds(Sector, OtherCompany, NeededFS, TotalNeededFS);
			Variation->MaintenanceCapacity += TotalNeededFS;

			SectorHelper::GetRepairFleetSupplyNeeds(Sector, OtherCompany, NeededFS, TotalNeededFS);
			Variation->MaintenanceCapacity += TotalNeededFS;
		}
	}

	return SectorVariation;
}

void UFlareCompanyAI::DumpSectorResourceVariation(UFlareSimulatedSector* Sector, TMap<FFlareResourceDescription*, struct ResourceVariation>* SectorVariation) const
{
	FLOGV("DumpSectorResourceVariation : sector %s resource variation: ", *Sector->GetSectorName().ToString());
	for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
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
			TravelTimeToA = UFlareTravel::ComputeTravelDuration(Game->GetGameWorld(), Ship->GetCurrentSector(), SectorA);
		}

		if (SectorA == SectorB)
		{
			// Stay in sector option
			TravelTimeToB = 0;
		}
		else
		{
			// Travel time

			TravelTimeToB = UFlareTravel::ComputeTravelDuration(Game->GetGameWorld(), SectorA, SectorB);

		}
		int64 TravelTime = TravelTimeToA + TravelTimeToB;


#ifdef DEBUG_AI_TRADING
		/*if(SectorA->GetIdentifier() != "lighthouse" || SectorB->GetIdentifier() != "boneyard")
		{
			continue;
		}*/

		if(Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
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
		if(Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
		{
			FLOGV("- Check for %s", *Resource->Name.ToString());
		}

		/*if(Resource->Identifier != "fuel")
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
				!VariationB->OwnedFlow &&
				!VariationB->FactoryFlow &&
				!VariationB->OwnedStock &&
				!VariationB->FactoryStock &&
				!VariationB->StorageStock &&
				!VariationB->OwnedCapacity &&
				!VariationB->FactoryCapacity &&
				!VariationB->StorageCapacity &&
				!VariationB->MaintenanceCapacity)
			{
				continue;
			}


			int32 InitialQuantity = Ship->GetCargoBay()->GetResourceQuantity(Resource, Ship->GetCompany());
			int32 FreeSpace = Ship->GetCargoBay()->GetFreeSpaceForResource(Resource, Ship->GetCompany());

			int32 StockInAAfterTravel =
				VariationA->OwnedStock
				+ VariationA->FactoryStock
				+ VariationA->StorageStock
				- (VariationA->OwnedFlow * TravelTimeToA)
				- (VariationA->FactoryFlow * TravelTimeToA);

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

			// Station construction incitation
			/*if (SectorB == ConstructionProjectSector)
			{
			for (int32 ConstructionResourceIndex = 0; ConstructionResourceIndex < ConstructionProjectStation->CycleCost.InputResources.Num() ; ConstructionResourceIndex++)
			{
			FFlareFactoryResource* ConstructionResource = &ConstructionProjectStation->CycleCost.InputResources[ConstructionResourceIndex];

			if (Resource == &ConstructionResource->Resource->Data)
			{
			MoneyGain *= STATION_CONSTRUCTION_PRICE_BONUS;
			break;
			}
			}
			}*/

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

#ifdef DEBUG_AI_TRADING
			if(Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
			{
				FLOGV(" -> IncomingCapacity=%d", SectorVariationA->IncomingCapacity);
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
				FLOGV(" -> SectorB affility=%f", Behavior->GetSectorAffility(SectorB));
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
				if(Company->GetShortName() == DEBUG_AI_TRADING_COMPANY)
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

TMap<FFlareResourceDescription*, int32> UFlareCompanyAI::ComputeWorldResourceFlow() const
{
	TMap<FFlareResourceDescription*, int32> WorldResourceFlow;
	for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;

		WorldResourceFlow.Add(Resource, 0);
	}

	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];
		int32 CustomerStation = 0;


		for (int32 StationIndex = 0; StationIndex < Sector->GetSectorStations().Num(); StationIndex++)
		{
			UFlareSimulatedSpacecraft* Station = Sector->GetSectorStations()[StationIndex];


			if (Station->GetCompany()->GetWarState(Company) == EFlareHostility::Hostile)
			{
				continue;
			}

			if (Station->HasCapability(EFlareSpacecraftCapability::Consumer))
			{
				CustomerStation++;
			}

			for (int32 FactoryIndex = 0; FactoryIndex < Station->GetFactories().Num(); FactoryIndex++)
			{
				UFlareFactory* Factory = Station->GetFactories()[FactoryIndex];
				if ((!Factory->IsActive() || !Factory->IsNeedProduction()))
				{
					// No resources needed
					break;
				}

				// Input flow
				for (int32 ResourceIndex = 0; ResourceIndex < Factory->GetInputResourcesCount(); ResourceIndex++)
				{
					FFlareResourceDescription* Resource = Factory->GetInputResource(ResourceIndex);


					int32 Flow = Factory->GetInputResourceQuantity(ResourceIndex) / Factory->GetProductionDuration();
					WorldResourceFlow[Resource] -= Flow;
				}

				// Ouput flow
				for (int32 ResourceIndex = 0; ResourceIndex < Factory->GetOutputResourcesCount(); ResourceIndex++)
				{
					FFlareResourceDescription* Resource = Factory->GetOutputResource(ResourceIndex);

					int32 Flow = Factory->GetOutputResourceQuantity(ResourceIndex) / Factory->GetProductionDuration();
					WorldResourceFlow[Resource] += Flow;
				}
			}
		}

		if (CustomerStation)
		{
			for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->ConsumerResources.Num(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->ConsumerResources[ResourceIndex]->Data;

				int32 Consumption = Sector->GetPeople()->GetRessourceConsumption(Resource, false);
				WorldResourceFlow[Resource] -= Consumption;
			}
		}

	}

	return WorldResourceFlow;
}
