
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

// TODO, make it depend on company's nature
#define AI_CARGO_PEACE_MILILTARY_THRESOLD 10


/*----------------------------------------------------
	Public API
----------------------------------------------------*/

UFlareCompanyAI::UFlareCompanyAI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
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
	Behavior->Load(Company);
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
		// Simulate company attitude towards others
		UpdateDiplomacy();
	
		ResourceFlow = ComputeWorldResourceFlow();
		WorldStats = WorldHelper::ComputeWorldResourceStats(Game);
		Shipyards = FindShipyards();

		// Compute input and output ressource equation (ex: 100 + 10/ day)
		TMap<UFlareSimulatedSector*, SectorVariation> WorldResourceVariation;
		for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
		{
			UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];
			SectorVariation Variation = ComputeSectorResourceVariation(Sector);

			WorldResourceVariation.Add(Sector, Variation);
			//DumpSectorResourceVariation(Sector, &Variation);
		}

		// Update trade routes
		int32 IdleCargoCapacity = UpdateTrading(WorldResourceVariation);
	
		// Create or upgrade stations
		UpdateStationConstruction(WorldResourceVariation, IdleCargoCapacity);

		// Buy ships
		UpdateShipAcquisition(IdleCargoCapacity);
	}
}

void UFlareCompanyAI::DestroySpacecraft(UFlareSimulatedSpacecraft* Spacecraft)
{
	// Don't keep reference on destroyed ship
	ConstructionShips.Remove(Spacecraft);
}


/*----------------------------------------------------
	Internal subsystems
----------------------------------------------------*/

void UFlareCompanyAI::UpdateDiplomacy()
{
	for (int32 CompanyIndex = 0; CompanyIndex < Game->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
	{
		UFlareCompany* OtherCompany = Game->GetGameWorld()->GetCompanies()[CompanyIndex];

		if (OtherCompany == Company)
		{
			continue;
		}

		if (Company->GetHostility(OtherCompany) == EFlareHostility::Hostile && Company->GetReputation(OtherCompany) > -100)
		{
			Company->SetHostilityTo(OtherCompany, false);
		}
		else if (Company->GetHostility(OtherCompany) != EFlareHostility::Hostile && Company->GetReputation(OtherCompany) <= -100)
		{
			Company->SetHostilityTo(OtherCompany, true);
			if (OtherCompany == Game->GetPC()->GetCompany())
			{
				OtherCompany->SetHostilityTo(Company, true);
			}
		}
	}
}

int32 UFlareCompanyAI::UpdateTrading(TMap<UFlareSimulatedSector*, SectorVariation>& WorldResourceVariation)
{
	int32 IdleCargoCapacity = 0;
	TArray<UFlareSimulatedSpacecraft*> IdleCargos = FindIdleCargos();
	FLOGV("UFlareCompanyAI::UpdateTrading : %s has %d idle ships", *Company->GetCompanyName().ToString(), IdleCargos.Num());

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

		if (Ship->IsTrading())
		{
			continue;
		}
		//	FLOGV("UFlareCompanyAI::UpdateTrading : Search something to do for %s", *Ship->GetImmatriculation().ToString());
		
		SectorDeal BestDeal;
		BestDeal.BuyQuantity = 0;
		BestDeal.MoneyBalanceParDay = 0;
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
			SectorBestDeal.MoneyBalanceParDay = 0;
			SectorBestDeal.Resource = NULL;
			SectorBestDeal.SectorA = NULL;
			SectorBestDeal.SectorB = NULL;
			
			while (true)
			{
				SectorBestDeal = FindBestDealForShipFromSector(Ship, SectorA, &BestDeal, &WorldResourceVariation);
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
			FLOGV("UFlareCompanyAI::UpdateTrading : Best balance for %s (%s) : %f credit per day",
				*Ship->GetImmatriculation().ToString(), *Ship->GetCurrentSector()->GetSectorName().ToString(), BestDeal.MoneyBalanceParDay / 100);
			FLOGV("UFlareCompanyAI::UpdateTrading -> Transfer %s from %s to %s",
				*BestDeal.Resource->Name.ToString(), *BestDeal.SectorA->GetSectorName().ToString(), *BestDeal.SectorB->GetSectorName().ToString());
			if (Ship->GetCurrentSector() == BestDeal.SectorA)
			{
				// Already in A, buy resources and go to B
				if (BestDeal.BuyQuantity == 0)
				{
					if (Ship->GetCurrentSector() != BestDeal.SectorB)
					{
						// Already buy resources,go to B
						Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), BestDeal.SectorB);
						FLOGV("UFlareCompanyAI::UpdateTrading -> Travel to %s to sell", *BestDeal.SectorB->GetSectorName().ToString());
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

					uint32 BroughtResource = 0;
					if (StationCandidate)
					{
						BroughtResource = SectorHelper::Trade(StationCandidate, Ship, BestDeal.Resource, Request.MaxQuantity);
						FLOGV("UFlareCompanyAI::UpdateTrading -> Buy %d / %d to %s", BroughtResource, BestDeal.BuyQuantity, *StationCandidate->GetImmatriculation().ToString());
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

						FLOG("UFlareCompanyAI::UpdateTrading -> Buy failed, remove the deal from the list");
					}
				}
			}
			else
			{
				if (BestDeal.SectorA != Ship->GetCurrentSector())
				{
					Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), BestDeal.SectorA);
					FLOGV("UFlareCompanyAI::UpdateTrading -> Travel to %s to buy", *BestDeal.SectorA->GetSectorName().ToString());
				}
				else
				{
					FLOGV("UFlareCompanyAI::UpdateTrading -> Wait to %s", *BestDeal.SectorA->GetSectorName().ToString());
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
					FLOGV("UFlareCompanyAI::UpdateTrading -> Sell %d / %d to %s", SellQuantity, Request.MaxQuantity, *StationCandidate->GetImmatriculation().ToString());
				}
			}
		}
		else
		{
			//FLOGV("UFlareCompanyAI::UpdateTrading : %s found nothing to do", *Ship->GetImmatriculation().ToString());

			//FLOGV("UFlareCompanyAI::UpdateTrading : HasProject ? %d ConstructionProjectNeedCapacity %d", (ConstructionProjectStationDescription != NULL), ConstructionProjectNeedCapacity);
			if (ConstructionProjectStationDescription && ConstructionProjectSector && ConstructionProjectNeedCapacity > 0 && Ship->GetCargoBay()->GetUsedCargoSpace() == 0)
			{
				//FLOGV("UFlareCompanyAI::UpdateTrading : %s add to construction", *Ship->GetImmatriculation().ToString());

				ConstructionShips.Add(Ship);
				ConstructionProjectNeedCapacity -= Ship->GetCargoBay()->GetCapacity();
			}
			else
			{
				IdleCargoCapacity += Ship->GetCargoBay()->GetCapacity();
			}

			// TODO recruit to build station
		}
	}

	return IdleCargoCapacity;
}

void UFlareCompanyAI::UpdateStationConstruction(TMap<UFlareSimulatedSector*, SectorVariation>& WorldResourceVariation, int32& IdleCargoCapacity)
{
	// Prepare resources for station-building analysis
	float BestScore = 0;
	float CurrentConstructionScore = 0;
	UFlareSimulatedSector* BestSector = NULL;
	FFlareSpacecraftDescription* BestStationDescription = NULL;
	UFlareSimulatedSpacecraft* BestStation = NULL;
	TArray<UFlareSpacecraftCatalogEntry*>& StationCatalog = Game->GetSpacecraftCatalog()->StationCatalog;

	FLOGV("UFlareCompanyAI::UpdateStationConstruction statics ships : %d construction ships : %d",
		  ConstructionStaticShips.Num(), ConstructionShips.Num());

	// Loop on sector list
	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];

		// Loop on catalog
		for (int32 StationIndex = 0; StationIndex < StationCatalog.Num(); StationIndex++)
		{
			FFlareSpacecraftDescription* StationDescription = &StationCatalog[StationIndex]->Data;

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
				TPair<float, float> ScoreResults = ComputeConstructionScoreForStation(Sector, StationDescription, FactoryDescription, NULL);
				float Score = ScoreResults.Key;
				float GainPerDay = ScoreResults.Value;


				// Update current construction score
				if (ConstructionProjectSector == Sector && ConstructionProjectStationDescription == StationDescription)
				{
					CurrentConstructionScore = Score;
				}

				// Change best if we found better
				if (GainPerDay > 1.f && (!BestStationDescription || Score > BestScore))
				{
					//FLOGV("New Best : GainPerDay=%f Score=%f", GainPerDay, Score);

					BestScore = Score;
					BestStationDescription = StationDescription;
					BestStation = NULL;
					BestSector = Sector;
				}
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
				TPair<float, float> ScoreResults = ComputeConstructionScoreForStation(Sector, Station->GetDescription(), FactoryDescription, Station);
				float Score = ScoreResults.Key;
				float GainPerDay = ScoreResults.Value;

				// Update current construction score
				if (ConstructionProjectSector == Sector && ConstructionProjectStation == Station)
				{
					CurrentConstructionScore = Score;
				}

				// Change best if we found better
				if (GainPerDay > 0 && (!BestStationDescription || Score > BestScore))
				{
					//FLOGV("New Best : GainPerDay=%f Score=%f", GainPerDay, Score);

					BestScore = Score;
					BestStationDescription = Station->GetDescription();
					BestStation = Station;
					BestSector = Sector;
				}
			}

		}



	}

	if (BestSector && BestStationDescription)
	{
		FLOGV("UFlareCompanyAI::UpdateStationConstruction : %s >>> %s in %s (upgrade: %d) Score=%f", *Company->GetCompanyName().ToString(), *BestStationDescription->Name.ToString(), *BestSector->GetSectorName().ToString(), (BestStation != NULL),BestScore);

		// Start construction only if can afford to buy the station

		float StationPrice = ComputeStationPrice(BestSector, BestStationDescription, BestStation);

		bool StartConstruction = true;

		if (CurrentConstructionScore * 1.5 > BestScore)
		{
			FLOGV("    dont change construction yet : current score is %f but best score is %f", CurrentConstructionScore, BestScore);
		}
		else
		{
			if (StationPrice > Company->GetMoney())
			{
				StartConstruction = false;
				FLOGV("    dont build yet :station cost %f but company has only %lld", StationPrice, Company->GetMoney());
			}

			int32 NeedCapacity = UFlareGameTools::ComputeConstructionCapacity(BestStationDescription->Identifier, Game);



			if (NeedCapacity > IdleCargoCapacity * 1.5)
			{
				IdleCargoCapacity -= NeedCapacity * 1.5; // Keep margin
				StartConstruction = false;
				FLOGV("    dont build yet :station need %d idle capacity but company has only %d", NeedCapacity, IdleCargoCapacity);
			}

			if (StartConstruction)
			{
				FLOG("Start construction");
				ConstructionProjectStationDescription = BestStationDescription;
				ConstructionProjectSector = BestSector;
				ConstructionProjectStation = BestStation;
				ConstructionProjectNeedCapacity = NeedCapacity * 1.5;
				ConstructionShips.Empty();
				ConstructionStaticShips.Empty();

				FLOGV("  ConstructionProjectNeedCapacity = %d", ConstructionProjectNeedCapacity);
			}
		}
	}

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

			if(BuildSuccess)
			{
				// Build success clean contruction project
				FLOGV("UFlareCompanyAI::UpdateStationConstruction %s build %s in %s", *Company->GetCompanyName().ToString(), *ConstructionProjectStationDescription->Name.ToString(), *ConstructionProjectSector->GetSectorName().ToString());

				ConstructionProjectStationDescription = NULL;
				ConstructionProjectStation = NULL;
				ConstructionProjectSector = NULL;
				ConstructionProjectNeedCapacity = 0;
				ConstructionShips.Empty();
				ConstructionStaticShips.Empty();
			}

			// Cannot build
			else
			{
				FLOGV("UFlareCompanyAI::UpdateStationConstruction %s failed to build %s in %s", *Company->GetCompanyName().ToString(), *ConstructionProjectStationDescription->Name.ToString(), *ConstructionProjectSector->GetSectorName().ToString());
				
				int32 NeedCapacity = UFlareGameTools::ComputeConstructionCapacity(ConstructionProjectStationDescription->Identifier, Game);
				if (NeedCapacity > IdleCargoCapacity)
				{
					IdleCargoCapacity -= NeedCapacity;
				}

				FindResourcesForStationConstruction(WorldResourceVariation);
			}
		}
		else
		{
			// Abandon build project
			FLOGV("UFlareCompanyAI::UpdateStationConstruction %s abandon building of %s in %s (upgrade: %d)", *Company->GetCompanyName().ToString(), *ConstructionProjectStationDescription->Name.ToString(), *ConstructionProjectSector->GetSectorName().ToString(), (ConstructionProjectStation != NULL));
			ConstructionProjectStationDescription = NULL;
			ConstructionProjectSector = NULL;
			ConstructionProjectStation = NULL;
			ConstructionProjectNeedCapacity = 0;
			ConstructionShips.Empty();
			ConstructionStaticShips.Empty();
		}
	}
}

void UFlareCompanyAI::FindResourcesForStationConstruction(TMap<UFlareSimulatedSector*, SectorVariation>& WorldResourceVariation)
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

	TMap<FFlareResourceDescription *, int32> MissingResourcesQuantity;
	TMap<FFlareResourceDescription *, int32> MissingStaticResourcesQuantity;
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
		if (!Ship->GetCurrentFleet()->IsTraveling()  && Ship->GetCurrentSector() != ConstructionProjectSector)
		{
			Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), ConstructionProjectSector);
		}

		ShipsInConstructionSector.Remove(Ship);
		ShipsInOtherSector.Remove(Ship);
		ShipsToTravel.Remove(Ship);

		FLOGV("Static Construction ship %s", *Ship->GetImmatriculation().ToString());
	}




	// First strep, agregate ressources in construction sector
	for (int32 ShipIndex = 0; ShipIndex < ShipsInConstructionSector.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = ShipsInConstructionSector[ShipIndex];

		FLOGV("Construction ship %s", *Ship->GetImmatriculation().ToString());


		// Give to others ships
		for (uint32 CargoIndex = 0; CargoIndex < Ship->GetCargoBay()->GetSlotCount(); CargoIndex++)
		{
			FFlareCargo* Cargo = Ship->GetCargoBay()->GetSlot(CargoIndex);

			if (Cargo->Resource == NULL)
			{
				continue;
			}

			FFlareResourceDescription* ResourceToGive = Cargo->Resource;
			uint32 QuantityToGive = Ship->GetCargoBay()->GetResourceQuantity(ResourceToGive, Ship->GetCompany());

			FLOGV("  %d %s to give", QuantityToGive, *ResourceToGive->Name.ToString());

			for (int32 StaticShipIndex = 0; QuantityToGive > 0 && StaticShipIndex < ConstructionStaticShips.Num(); StaticShipIndex++)
			{
				UFlareSimulatedSpacecraft* StaticShip = ConstructionStaticShips[StaticShipIndex];
				if(StaticShip->GetCurrentSector() != ConstructionProjectSector)
				{
					continue;
				}


				uint32 GivenQuantity = StaticShip->GetCargoBay()->GiveResources(ResourceToGive, QuantityToGive, Ship->GetCompany());

				Ship->GetCargoBay()->TakeResources(ResourceToGive, GivenQuantity, StaticShip->GetCompany());

				QuantityToGive -= GivenQuantity;


				FLOGV("  %d given to %s", QuantityToGive, *StaticShip->GetImmatriculation().ToString());

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
			FLOGV("Construction ship %s to flush", *Ship->GetImmatriculation().ToString());

			if (Ship->GetCargoBay()->GetUsedCargoSpace() > 0)
			{
				// If at least 1 resource, go to construction sector
				Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), ConstructionProjectSector);
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
			FLOGV("Construction ship %s to travel", *Ship->GetImmatriculation().ToString());


			TArray<FFlareResourceDescription*> MissingResources;
			MissingResourcesQuantity.GetKeys(MissingResources);
			for (int32 ResourceIndex = 0; ResourceIndex < MissingResources.Num(); ResourceIndex++)
			{
				if (Ship->IsTrading())
				{
					break;
				}

				FFlareResourceDescription* MissingResource = MissingResources[ResourceIndex];
				if (!MissingResourcesQuantity.Contains(MissingResource))
				{
					FLOGV("UFlareCompanyAI::FindResourcesForStationConstruction : !!! MissingResourcesQuantity doesn't contain %s 0", *MissingResource->Name.ToString());
				}
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
					FLOGV("  %d %s taken to %s", TakenQuantity, *MissingResource->Name.ToString(), *StationCandidate->GetImmatriculation().ToString());

				}

				MissingResourceQuantity -= TakenQuantity;
				if (MissingResourceQuantity <= 0)
				{
					MissingResourcesQuantity.Remove(MissingResource);
				}
				else
				{
					if (!MissingResourcesQuantity.Contains(MissingResource))
					{
						FLOGV("UFlareCompanyAI::FindResourcesForStationConstruction : !!! MissingResourcesQuantity doesn't contain %s 1", *MissingResource->Name.ToString());
					}
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
				Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), ConstructionProjectSector);
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

					if (!WorldResourceVariation.Contains(Sector))
					{
						FLOGV("UFlareCompanyAI::FindResourcesForStationConstruction : !!! WorldResourceVariation doesn't contain %s", *Sector->GetSectorName().ToString());
					}
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
						if (!MissingResourcesQuantity.Contains(MissingResource))
						{
							FLOGV("UFlareCompanyAI::FindResourcesForStationConstruction : !!! MissingResourcesQuantity doesn't contain %s 2", *MissingResource->Name.ToString());
						}
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
					FLOG("no best sector");
					// Try a sector with a flow
					for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
					{
						UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];

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
							if (!MissingResourcesQuantity.Contains(MissingResource))
							{
								FLOGV("UFlareCompanyAI::FindResourcesForStationConstruction : !!! MissingResourcesQuantity doesn't contain %s 3", *MissingResource->Name.ToString());
							}
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
					FLOGV("  travel to %s", *BestSector->GetSectorName().ToString());

					// Decrease missing quantity
					if (!MissingResourcesQuantity.Contains(BestResource))
					{
						FLOGV("UFlareCompanyAI::FindResourcesForStationConstruction : !!! MissingResourcesQuantity doesn't contain %s 4", *BestResource->Name.ToString());
					}
					MissingResourcesQuantity[BestResource] -= FMath::Max(0, BestEstimateTake);
					SectorVariation* SectorVariation = &WorldResourceVariation[BestSector];
					struct ResourceVariation* Variation = &SectorVariation->ResourceVariations[BestResource];

					Variation->OwnedStock -= FMath::Max(0, BestEstimateTake);
				}

			}
		}
	}
}

void UFlareCompanyAI::UpdateShipAcquisition(int32& IdleCargoCapacity)
{
	// 2 pass : tranport pass and military pass
	// For the transport pass only one ship is created at the same time
	// For the military pass, only one ship if not at war, as many ship as possible if at war

	FLOGV("UFlareCompanyAI::UpdateShipAcquisition : IdleCargoCapacity = %d", IdleCargoCapacity);


	if (IdleCargoCapacity <= 0)
	{
		UpdateCargoShipAcquisition();

	}

	UpdateWarShipAcquisition();
}

void UFlareCompanyAI::UpdateCargoShipAcquisition()
{
	// For the transport pass, the best ship is choose. The best ship is the one with the small capacity, but
	// only if the is no more then the AI_CARGO_DIVERSITY_THERESOLD


	// Check if a ship is building
	if(IsBuildingShip(false))
	{
		return;
	}

	FFlareSpacecraftDescription* ShipDescription = FindBestShipToBuild(false);
	if(ShipDescription == NULL)
	{
		return;
	}

	OrderOneShip(ShipDescription);
}

void UFlareCompanyAI::UpdateWarShipAcquisition()
{
	// For the war pass there is 2 states : slow preventive ship buy. And war state.
	//
	// - In the first state, the company will limit his army to a percentage of his value.
	//   It will create only one ship at once
	// - In the second state, it is war, the company will limit itself to de double of the
	//   army value of all enemies and buy as many ship it can.
	CompanyValue Value = Company->GetCompanyValue();

	// TODO, war behavior

	if(Value.ArmyValue > Value.TotalValue * AI_CARGO_PEACE_MILILTARY_THRESOLD)
	{
		// Enough army
		return;
	}

	// Check if a ship is building
	if(IsBuildingShip(true))
	{
		return;
	}

	FFlareSpacecraftDescription* ShipDescription = FindBestShipToBuild(true);

	OrderOneShip(ShipDescription);
}

/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

bool UFlareCompanyAI::OrderOneShip(FFlareSpacecraftDescription* ShipDescription)
{
	if(ShipDescription == NULL)
	{
		return false;
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

				float CostSafetyMargin = 2.0f;

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

				if (UFlareGameTools::ComputeSpacecraftPrice(ShipDescription->Identifier, Shipyard->GetCurrentSector(), true) * CostSafetyMargin < CompanyMoney)
				{
					FName ShipClassToOrder = ShipDescription->Identifier;
					FLOGV("UFlareCompanyAI::UpdateShipAcquisition : Ordering spacecraft : '%s'", *ShipClassToOrder.ToString());
					Factory->OrderShip(Company, ShipClassToOrder);
					Factory->Start();
					return true;
				}
			}
		}
	}

	return false;
}

FFlareSpacecraftDescription* UFlareCompanyAI::FindBestShipToBuild(bool Military)
{

	// Count owned ships
	TMap<FFlareSpacecraftDescription*, int32> OwnedShipCount;
	for(int32 ShipIndex = 0; ShipIndex < Company->GetCompanyShips().Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = Company->GetCompanyShips()[ShipIndex];

		if(OwnedShipCount.Contains(Ship->GetDescription()))
		{
			OwnedShipCount[Ship->GetDescription()]++;
		}
		else
		{
			OwnedShipCount.Add(Ship->GetDescription(), 1);
		}
	}

	FFlareSpacecraftDescription* BestShipDescription = NULL;
	FFlareSpacecraftDescription* BiggestShipDescription = NULL;

	for (int SpacecraftIndex = 0; SpacecraftIndex < Game->GetSpacecraftCatalog()->ShipCatalog.Num(); SpacecraftIndex++)
	{
		UFlareSpacecraftCatalogEntry* Entry = Game->GetSpacecraftCatalog()->ShipCatalog[SpacecraftIndex];
		FFlareSpacecraftDescription* Description = &Entry->Data;

		if(Military != Description->IsMilitary())
		{
			continue;
		}




		if (!OwnedShipCount.Contains(Description) || OwnedShipCount[Description] < AI_CARGO_DIVERSITY_THRESOLD)
		{
			if(BestShipDescription == NULL || (Military?
											   BestShipDescription->Mass > Description-> Mass
											   : BestShipDescription->GetCapacity() > Description->GetCapacity()))
			{
				BestShipDescription = Description;
			}
		}

		if(BiggestShipDescription == NULL || (Military ?
											  BestShipDescription->Mass < Description->Mass
											  : BiggestShipDescription->GetCapacity() < Description->GetCapacity()))
		{
			BiggestShipDescription = Description;
		}
	}


	if(BestShipDescription == NULL)
	{
		// If no best ship, the thresold is reach for each ship, so build the bigger ship
		BestShipDescription = BiggestShipDescription;
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
			if (Ship->GetCompany() != Company || Ship->IsTrading() || (Ship->GetCurrentFleet() && Ship->GetCurrentFleet()->IsTraveling()) || Ship->GetCurrentTradeRoute() != NULL || Ship->GetCargoBay()->GetCapacity() == 0 || ConstructionShips.Contains(Ship))
			{
				continue;
			}

			IdleCargos.Add(Ship);
		}
	}

	return IdleCargos;
}

TPair<float, float> UFlareCompanyAI::ComputeConstructionScoreForStation(UFlareSimulatedSector* Sector, FFlareSpacecraftDescription* StationDescription, FFlareFactoryDescription* FactoryDescription, UFlareSimulatedSpacecraft* Station) const
{

	// Detect shipyards
	if (FactoryDescription->IsShipyard())
	{
		// TODO Shipyard case
		return TPairInitializer<float, float>(0, 0);
	}


	float GainPerDay = 0;
	float GainPerCycle = 0;


	GainPerCycle -= FactoryDescription->CycleCost.ProductionCost;

	float Malus = 0;
	float Bonus = 0;

	for (int32 ResourceIndex = 0; ResourceIndex < FactoryDescription->CycleCost.InputResources.Num(); ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &FactoryDescription->CycleCost.InputResources[ResourceIndex];
		GainPerCycle -= Sector->GetResourcePrice(&Resource->Resource->Data, EFlareResourcePriceContext::FactoryInput) * Resource->Quantity;

		float NeededFlow = (float)Resource->Quantity / (float)FactoryDescription->CycleCost.ProductionTime;
		/*FLOGV("%s, %s: ResourceFlow = %d Flow needed = %f",
			  *FactoryDescription->Name.ToString(),
			  *Resource->Resource->Data.Name.ToString(),
			  ResourceFlow[&Resource->Resource->Data] ,NeededFlow);*/
		if (ResourceFlow[&Resource->Resource->Data] <= NeededFlow)
		{
			float DisponibilityMalus = (NeededFlow - (float)ResourceFlow[&Resource->Resource->Data]);
			Malus += DisponibilityMalus;
			//FLOGV("Factory %s as %f as malus for resource %s", *FactoryDescription->Name.ToString(), DisponibilityMalus, *Resource->Resource->Data.Name.ToString());

		}

		// TODO : moderate, like 10% margin with mals in 0 to -10%
		if(WorldStats[&Resource->Resource->Data].Balance < 0)
		{
			// Input resource in underflow, don't build a useless station
			return TPairInitializer<float, float>(0, 0);
		}

	}

	for (int32 ResourceIndex = 0; ResourceIndex < FactoryDescription->CycleCost.OutputResources.Num(); ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &FactoryDescription->CycleCost.OutputResources[ResourceIndex];
		GainPerCycle += Sector->GetResourcePrice(&Resource->Resource->Data, EFlareResourcePriceContext::FactoryOutput) * Resource->Quantity;

		float ProducedFlow = (float)Resource->Quantity / (float)FactoryDescription->CycleCost.ProductionTime;
		/*FLOGV("%s, %s: ResourceFlow = %d Flow produced = %f",
			  *FactoryDescription->Name.ToString(),
			  *Resource->Resource->Data.Name.ToString(),
			  ResourceFlow[&Resource->Resource->Data] ,ProducedFlow);*/
		if (ResourceFlow[&Resource->Resource->Data] <= 0)
		{
			float DisponibilityBonus = ProducedFlow - (float)ResourceFlow[&Resource->Resource->Data];
			//FLOGV("Factory %s as %f as bonus for resource %s", *FactoryDescription->Name.ToString(), DisponibilityBonus, *Resource->Resource->Data.Name.ToString());
			Bonus += DisponibilityBonus;
		}

		// TODO : moderate, like 10% margin with mals in 0 to 10%
		if(WorldStats[&Resource->Resource->Data].Balance > 0)
		{
			// Output resource in underflow, don't build a useless station
			return TPairInitializer<float, float>(0, 0);
		}

	}

	GainPerDay = GainPerCycle / FactoryDescription->CycleCost.ProductionTime;
	//FLOGV("%s in %s GainPerDay=%f", *StationDescription->Name.ToString(), *Sector->GetSectorName().ToString(), GainPerDay / 100);

	// Price with station resources prices bonus
	float StationPrice = ComputeStationPrice(Sector, StationDescription, Station);

	float DayToPayPrice = StationPrice / GainPerDay;
	float MissingMoneyRatio = FMath::Min(1.0f, Company->GetMoney() / StationPrice);
	//FLOGV("StationPrice=%f DayToPayPrice=%f MissingMoneyRatio=%f", StationPrice, DayToPayPrice, MissingMoneyRatio);

	float Score = (100.f / DayToPayPrice) * MissingMoneyRatio * MissingMoneyRatio;
	//FLOGV("%s in %s Score=%f", *StationDescription->Name.ToString(), *Sector->GetSectorName().ToString(), Score);
	//FLOGV("         Bonus=%f", Bonus);
	//FLOGV("         Malus=%f", Malus);

	if (Bonus > 0)
	{
		Score *= Bonus;
	}

	if (Malus > 0)
	{
		Score /= Malus;
	}

	//FLOGV("         Final score=%f", Score);


	return TPairInitializer<float, float>(Score, GainPerDay);
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
		ResourceVariation.IncomingResources = 0;
		ResourceVariation.MinCapacity = 0;

		SectorVariation.ResourceVariations.Add(Resource, ResourceVariation);
	}

	uint32 OwnedCustomerStation = 0;
	uint32 NotOwnedCustomerStation = 0;

	for (int32 StationIndex = 0 ; StationIndex < Sector->GetSectorStations().Num(); StationIndex++)
	{
		UFlareSimulatedSpacecraft* Station = Sector->GetSectorStations()[StationIndex];


		if (Station->GetCompany()->GetWarState(Company) == EFlareHostility::Hostile)
		{
			continue;
		}

		uint32 SlotCapacity = Station->GetCargoBay()->GetSlotCapacity();

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

				uint32 ResourceQuantity = Station->GetCargoBay()->GetResourceQuantity(Resource, Company);
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
						Variation->FactoryCapacity += Capacity;
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

				uint32 Flow = Factory->GetOutputResourceQuantity(ResourceIndex) / Factory->GetProductionDuration();

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

				uint32 Stock = Station->GetCargoBay()->GetResourceQuantity(Resource, Company);
				if (Company == Station->GetCompany())
				{
					Variation->OwnedStock += Stock;
				}
				else
				{
					Variation->FactoryStock += Stock;
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

				uint32 ResourceQuantity = Station->GetCargoBay()->GetResourceQuantity(Resource, Company);
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
						Variation->FactoryCapacity += Capacity;
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
		}

		// Maintenance
		if (Station->HasCapability(EFlareSpacecraftCapability::Maintenance))
		{
			for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->MaintenanceResources.Num(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->MaintenanceResources[ResourceIndex]->Data;
				struct ResourceVariation* Variation = &SectorVariation.ResourceVariations[Resource];

				uint32 ResourceQuantity = Station->GetCargoBay()->GetResourceQuantity(Resource, Company);

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
						Variation->FactoryCapacity += Capacity;
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


			uint32 Consumption = Sector->GetPeople()->GetRessourceConsumption(Resource);

			Variation->OwnedFlow = OwnedCustomerRatio * Consumption;
			Variation->FactoryFlow = NotOwnedCustomerRatio * Consumption;
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

			if (Ship->GetCargoBay()->GetSlotCapacity() == 0)
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
	// TODO Check if needed

	// Consider resource over 10 days of consumption as IncomingResources
	/*for (int32 StationIndex = 0 ; StationIndex < Sector->GetSectorStations().Num(); StationIndex++)
	{
		UFlareSimulatedSpacecraft* Station = Sector->GetSectorStations()[StationIndex];


		if (!Station->HasCapability(EFlareSpacecraftCapability::Storage) || Station->GetCompany()->GetWarState(Company) == EFlareHostility::Hostile)
		{
			continue;
		}


		for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
		{
			FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
			struct ResourceVariation* Variation = &SectorVariation.ResourceVariations[Resource];

			int32 TotalFlow =  Variation->FactoryFlow + Variation->OwnedFlow;

			if (TotalFlow >= 0)
			{
				int32 LongTermConsumption = TotalFlow * 10;
				int32 ResourceQuantity = Station->GetCargoBay()->GetResourceQuantity(Resource, Company);

				if (ResourceQuantity > LongTermConsumption)
				{
					Variation->IncomingResources += ResourceQuantity - LongTermConsumption;
				}
			}
		}


	}*/
	// TODO Check if needed

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
				Variation->StorageCapacity
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
		}

	}
}

SectorDeal UFlareCompanyAI::FindBestDealForShipFromSector(UFlareSimulatedSpacecraft* Ship, UFlareSimulatedSector* SectorA, SectorDeal* DealToBeat, TMap<UFlareSimulatedSector*, SectorVariation> *WorldResourceVariation) const
{
	SectorDeal BestDeal;
	BestDeal.Resource = NULL;
	BestDeal.BuyQuantity = 0;
	BestDeal.MoneyBalanceParDay = DealToBeat->MoneyBalanceParDay;
	BestDeal.Resource = NULL;
	BestDeal.SectorA = NULL;
	BestDeal.SectorB = NULL;

	for (int32 SectorBIndex = 0; SectorBIndex < Company->GetKnownSectors().Num(); SectorBIndex++)
	{
		UFlareSimulatedSector* SectorB = Company->GetKnownSectors()[SectorBIndex];

		int64 TravelTimeToA;
		int64 TravelTimeToB;


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


		SectorVariation* SectorVariationA = &(*WorldResourceVariation)[SectorA];
		SectorVariation* SectorVariationB = &(*WorldResourceVariation)[SectorB];

		for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
		{
			FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
			struct ResourceVariation* VariationA = &SectorVariationA->ResourceVariations[Resource];
			struct ResourceVariation* VariationB = &SectorVariationB->ResourceVariations[Resource];

			if (!VariationA->OwnedFlow &&
				!VariationA->FactoryFlow &&
				!VariationA->OwnedStock &&
				!VariationA->FactoryStock &&
				!VariationA->StorageStock &&
				!VariationA->OwnedCapacity &&
				!VariationA->FactoryCapacity &&
				!VariationA->StorageCapacity &&
				!VariationB->OwnedFlow &&
				!VariationB->FactoryFlow &&
				!VariationB->OwnedStock &&
				!VariationB->FactoryStock &&
				!VariationB->StorageStock &&
				!VariationB->OwnedCapacity &&
				!VariationB->FactoryCapacity &&
				!VariationB->StorageCapacity)
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
					+ VariationB->StorageCapacity;

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
			int32 FactoryCapacity = FMath::Max(0, (int32)(VariationB->FactoryCapacity + VariationB->FactoryFlow * TravelTime));
			int32 StorageCapacity = VariationB->StorageCapacity;

			int32 OwnedSellQuantity = FMath::Min(OwnedCapacity, QuantityToSell);
			MoneyGain += OwnedSellQuantity * SectorB->GetResourcePrice(Resource, EFlareResourcePriceContext::Default);
			QuantityToSell -= OwnedSellQuantity;

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
			MoneySpend += OwnedBuyQuantity * SectorA->GetResourcePrice(Resource, EFlareResourcePriceContext::Default);
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
			if (MoneyBalanceParDay > BestDeal.MoneyBalanceParDay && !Temporisation)
			{

				BestDeal.MoneyBalanceParDay = MoneyBalanceParDay;
				BestDeal.SectorA = SectorA;
				BestDeal.SectorB = SectorB;
				BestDeal.Resource = Resource;
				BestDeal.BuyQuantity = BuyQuantity;

				/*FLOGV("Travel %s -> %s -> %s : %lld days", *Ship->GetCurrentSector()->GetSectorName().ToString(),
				*SectorA->GetSectorName().ToString(), *SectorB->GetSectorName().ToString(), TravelTime);

				FLOGV("New Best Resource %s", *Resource->Name.ToString())

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
				FLOGV(" -> MoneyBalanceParDay=%f", MoneyBalanceParDay/100.f);*/
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
		uint32 CustomerStation = 0;


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


					uint32 Flow = Factory->GetInputResourceQuantity(ResourceIndex) / Factory->GetProductionDuration();
					WorldResourceFlow[Resource] -= Flow;
				}

				// Ouput flow
				for (int32 ResourceIndex = 0; ResourceIndex < Factory->GetOutputResourcesCount(); ResourceIndex++)
				{
					FFlareResourceDescription* Resource = Factory->GetOutputResource(ResourceIndex);

					uint32 Flow = Factory->GetOutputResourceQuantity(ResourceIndex) / Factory->GetProductionDuration();
					WorldResourceFlow[Resource] += Flow;
				}
			}
		}

		if (CustomerStation)
		{
			for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->ConsumerResources.Num(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->ConsumerResources[ResourceIndex]->Data;

				uint32 Consumption = Sector->GetPeople()->GetRessourceConsumption(Resource);
				WorldResourceFlow[Resource] -= Consumption;
			}
		}

	}

	return WorldResourceFlow;
}
