
#include "../../Flare.h"
#include "FlareCompanyAI.h"
#include "../FlareCompany.h"
#include "../FlareGame.h"
#include "../../Spacecrafts/FlareSimulatedSpacecraft.h"
#include "../../Economy/FlareCargoBay.h"
#include "../FlareSectorHelper.h"

#define STATION_CONSTRUCTION_PRICE_BONUS 1.2

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareCompanyAI::UFlareCompanyAI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareCompanyAI::Load(UFlareCompany* ParentCompany, const FFlareCompanyAISave& Data)
{
	Company = ParentCompany;
	Game = Company->GetGame();
	ResetShipGroup(EFlareCombatTactic::AttackMilitary);
}

FFlareCompanyAISave* UFlareCompanyAI::Save()
{
	return &AIData;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareCompanyAI::Simulate()
{
	if (Company == Game->GetPC()->GetCompany())
	{
		return;
	}

	//FLOGV("Simulate AI for %s", *Company->GetCompanyName().ToString());


	SimulateDiplomacy();



	TArray<UFlareSimulatedSpacecraft*> IdleCargos = FindIdleCargos();


	int IdleCargoCapacity = 0;

	int32 IdleShip = IdleCargos.Num();




	// Sell and immobilize construction ships
	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];



		for (int32 ShipIndex = 0 ; ShipIndex < Sector->GetSectorShips().Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = Sector->GetSectorShips()[ShipIndex];

			if(Ship->GetCompany() != Company || Ship->GetCargoBay()->GetCapacity() == 0 || ConstructionShips.Contains(Ship))
			{
				continue;
			}

			TArray<FFlareCargo>& CargoBaySlots = Ship->GetCargoBay()->GetSlots();
			for (int CargoIndex = 0; CargoIndex < CargoBaySlots.Num(); CargoIndex++)
			{
				FFlareCargo& Cargo = CargoBaySlots[CargoIndex];

				if (!Cargo.Resource)
				{
					continue;
				}


				// Try to unload or sell
				SectorHelper::FlareTradeRequest Request;
				Request.Resource = Cargo.Resource;
				Request.Operation = EFlareTradeRouteOperation::UnloadOrSell;
				Request.Client = Ship;
				Request.MaxQuantity = Ship->GetCargoBay()->GetResourceQuantity(Cargo.Resource);

				UFlareSimulatedSpacecraft* StationCandidate = SectorHelper::FindTradeStation(Request);

				if(StationCandidate)
				{
					SectorHelper::Trade(Ship, StationCandidate, Cargo.Resource, Request.MaxQuantity);
				}
			}

		}
	}

	FLOGV("%s has %d idle ships", *Company->GetCompanyName().ToString(), IdleShip);


	// Substract ship that are currently traveling to the sector and are not in a trade route


	// Trade route creation


	// TODO IF AT LEAST ONE IDLE SHIP

	TMap<UFlareSimulatedSector*, SectorVariation> WorldResourceVariation;

	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];


		// Compute input and output ressource equation (ex: 100 + 10/ day)

		// TODO, cache in sector for all company


		SectorVariation Variation = ComputeSectorResourceVariation(Sector);

		WorldResourceVariation.Add(Sector, Variation);
		//DumpSectorResourceVariation(Sector, &Variation);

	}


	for (int32 ShipIndex = 0 ; ShipIndex < IdleCargos.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = IdleCargos[ShipIndex];

		if(Ship->IsTrading())
		{
			continue;
		}
	//	FLOGV("Search something to do for %s", *Ship->GetImmatriculation().ToString());


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
				if(!SectorBestDeal.Resource)
				{
					// No best deal found
					break;
				}

				SectorVariation* SectorVariationA = &WorldResourceVariation[SectorA];
				if(Ship->GetCurrentSector() != SectorA && SectorVariationA->IncomingCapacity > 0 && SectorBestDeal.BuyQuantity > 0)
				{
					//FLOGV("IncomingCapacity to %s = %d", *SectorA->GetSectorName().ToString(), SectorVariationA->IncomingCapacity);
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

			if(SectorBestDeal.Resource)
			{
				BestDeal = SectorBestDeal;
			}
		}

		if(BestDeal.Resource)
		{
			FLOGV("Best balance for %s (%s) : %f credit per day", *Ship->GetImmatriculation().ToString(), *Ship->GetCurrentSector()->GetSectorName().ToString(), BestDeal.MoneyBalanceParDay/100);


			FLOGV(" -> Transfert %s from %s to %s", *BestDeal.Resource->Name.ToString(), *BestDeal.SectorA->GetSectorName().ToString(), *BestDeal.SectorB->GetSectorName().ToString());
			if (Ship->GetCurrentSector() == BestDeal.SectorA)
			{
				// Already in A, buy resources and go to B
				SectorHelper::FlareTradeRequest Request;
				Request.Resource = BestDeal.Resource;
				Request.Operation = EFlareTradeRouteOperation::LoadOrBuy;
				Request.Client = Ship;
				Request.MaxQuantity = Ship->GetCargoBay()->GetFreeSpaceForResource(BestDeal.Resource);

				UFlareSimulatedSpacecraft* StationCandidate = SectorHelper::FindTradeStation(Request);

				uint32 BroughtResource = 0;
				if(StationCandidate)
				{
					BroughtResource = SectorHelper::Trade(StationCandidate, Ship, BestDeal.Resource, Request.MaxQuantity);
				}

				// TODO reduce computed sector stock

				FLOGV(" -> Buy %d / %d", BroughtResource, BestDeal.BuyQuantity);
				if(BroughtResource == BestDeal.BuyQuantity)
				{
					// Virtualy decrease the stock for other ships in sector A
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
				else if(BroughtResource == 0)
				{
					// Fail to buy the promised resources, remove the deal from the list
					SectorVariation* SectorVariationA = &WorldResourceVariation[BestDeal.SectorA];
					struct ResourceVariation* VariationA = &SectorVariationA->ResourceVariations[BestDeal.Resource];
					VariationA->FactoryStock = 0;
					VariationA->OwnedStock = 0;
					VariationA->StorageStock = 0;
					if(VariationA->OwnedFlow > 0)
						VariationA->OwnedFlow = 0;
					if(VariationA->FactoryFlow > 0)
						VariationA->FactoryFlow = 0;

					FLOG(" -> Buy Fail remove the deal from the list");
				}
			}
			else
			{
				if (BestDeal.SectorA != Ship->GetCurrentSector())
				{
					Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), BestDeal.SectorA);
					FLOGV(" -> Travel to %s", *BestDeal.SectorA->GetSectorName().ToString());
				}
				else
				{
					FLOGV(" -> Wait to %s", *BestDeal.SectorA->GetSectorName().ToString());
				}

				// Reserve the deal by virtualy decrease the stock for other ships
				SectorVariation* SectorVariationA = &WorldResourceVariation[BestDeal.SectorA];
				struct ResourceVariation* VariationA = &SectorVariationA->ResourceVariations[BestDeal.Resource];
				VariationA->OwnedStock -= BestDeal.BuyQuantity;

				// Reserve the deal by virtualy decrease the capacity for other ships
				SectorVariation* SectorVariationB = &WorldResourceVariation[BestDeal.SectorB];
				struct ResourceVariation* VariationB = &SectorVariationB->ResourceVariations[BestDeal.Resource];
				VariationB->OwnedCapacity -= BestDeal.BuyQuantity;
			}
		}
		else
		{
			FLOGV("%s found nothing to do", *Ship->GetImmatriculation().ToString());

			if (ConstructionProjectStation && ConstructionProjectSector)
			{
				ConstructionShips.Add(Ship);
			}
			else
			{
				IdleCargoCapacity += Ship->GetCargoBay()->GetCapacity();
			}

			// TODO recrut to build station
		}
	}


	// TODO Check the option of waiting for some resource to fill the cargo in local sector

	// TODO reduce attrativeness of already ship on the same spot



	// For best option, if local, buy and travel
	// if not local, travel


	// For all current trade route in a sector (if not in a sector, it's not possible to modify then)
	//      -> Compute the resource balance in the dest sector and the resource balance in the source sector
	//			-> If the balance is negative in the dest sector, and positive un the source add a cargo
	//      -> Compute the current transport rate for the resource (resource/day)(mean on multiple travel) and the max transport rate
	//			-> If current is a lot below the max, remove a cargo

	// If inactive cargo
	// compute max negative balance. Find nearest sector with a positive balance.
	// create a route.
	// assign enought capacity to match the min(negative balance, positive balance)



	// TODO hub by stock, % of world production max




	//TODO always keep money for production
	// Acquire ship

	TArray<UFlareSpacecraftCatalogEntry*>& StationCatalog = Game->GetSpacecraftCatalog()->StationCatalog;

	TMap<FFlareResourceDescription*, int32> ResourceFlow = ComputeWorldResourceFlow();

	// Build station





	// Count factories for the company


	// Compute rentability in each sector for each station
	// Add weight if the company already have another station in this type

	float CurrentConstructionScore = 0;
	float BestScore = 0;
	FFlareSpacecraftDescription* BestStationDescription = NULL;
	UFlareSimulatedSector* BestSector = NULL;


	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];


		for (int32 StationIndex = 0; StationIndex < StationCatalog.Num(); StationIndex++)
		{
			FFlareSpacecraftDescription* StationDescription = &StationCatalog[StationIndex]->Data;

			// Check sector limitations
			TArray<FText> Reasons;
			if (!Sector->CanBuildStation(StationDescription, Company, Reasons, true))
			{
				continue;
			}



			for (int FactoryIndex = 0; FactoryIndex < StationDescription->Factories.Num(); FactoryIndex++)
			{
				FFlareFactoryDescription* FactoryDescription = &StationDescription->Factories[FactoryIndex]->Data;

				bool Shipyard = false;

				for (int32 Index = 0; Index < FactoryDescription->OutputActions.Num(); Index++)
				{
					if (FactoryDescription->OutputActions[Index].Action == EFlareFactoryAction::CreateShip)
					{
						Shipyard = true;
						break;
					}
				}

				if(Shipyard)
				{
					// TODO Shipyard case
					continue;
				}

				float GainPerDay = 0;
				float GainPerCycle = 0;

				GainPerCycle -= Sector->GetStationConstructionFee(FactoryDescription->CycleCost.ProductionCost);

				float Malus = 0;
				float Bonus = 0;

				for (int32 ResourceIndex = 0 ; ResourceIndex < FactoryDescription->CycleCost.InputResources.Num() ; ResourceIndex++)
				{
					const FFlareFactoryResource* Resource = &FactoryDescription->CycleCost.InputResources[ResourceIndex];
					GainPerCycle -= Sector->GetResourcePrice(&Resource->Resource->Data, EFlareResourcePriceContext::FactoryInput) * Resource->Quantity;

					float NeededFlow = (float) Resource->Quantity / (float) FactoryDescription->CycleCost.ProductionTime;
					//FLOGV("%s, %s: ResourceFlow = %d Flow needed = %f",
					//	  *FactoryDescription->Name.ToString(),
					//	  *Resource->Resource->Data.Name.ToString(),
					//	  ResourceFlow[&Resource->Resource->Data] ,NeededFlow);
					if(ResourceFlow[&Resource->Resource->Data] <= NeededFlow)
					{
						float DisponibilityMalus = (NeededFlow - (float) ResourceFlow[&Resource->Resource->Data]);
						Malus += DisponibilityMalus;
						//FLOGV("Factory %s as %f as malus for resource %s", *FactoryDescription->Name.ToString(), DisponibilityMalus, *Resource->Resource->Data.Name.ToString());

					}

				}

				for (int32 ResourceIndex = 0 ; ResourceIndex < FactoryDescription->CycleCost.OutputResources.Num() ; ResourceIndex++)
				{
					const FFlareFactoryResource* Resource = &FactoryDescription->CycleCost.OutputResources[ResourceIndex];
					GainPerCycle += Sector->GetResourcePrice(&Resource->Resource->Data, EFlareResourcePriceContext::FactoryOutput) * Resource->Quantity;


					float ProducedFlow = (float) Resource->Quantity / (float) FactoryDescription->CycleCost.ProductionTime;
					//FLOGV("%s, %s: ResourceFlow = %d Flow produced = %f",
					//	  *FactoryDescription->Name.ToString(),
					//	  *Resource->Resource->Data.Name.ToString(),
					//	  ResourceFlow[&Resource->Resource->Data] ,ProducedFlow);
					if(ResourceFlow[&Resource->Resource->Data] <=  0)
					{
						float DisponibilityBonus = ProducedFlow - (float) ResourceFlow[&Resource->Resource->Data];
						//FLOGV("Factory %s as %f as bonus for resource %s", *FactoryDescription->Name.ToString(), DisponibilityBonus, *Resource->Resource->Data.Name.ToString());
						Bonus += DisponibilityBonus;
					}

				}

				GainPerDay = GainPerCycle / FactoryDescription->CycleCost.ProductionTime;

				//FLOGV("%s in %s GainPerDay=%f", *StationDescription->Name.ToString(), *Sector->GetSectorName().ToString(), GainPerDay / 100);

				// Price with station resources prices bonus
				float StationPrice = STATION_CONSTRUCTION_PRICE_BONUS * UFlareGameTools::ComputeShipPrice(StationDescription->Identifier, Sector, true);
				float DayToPayPrice = StationPrice / GainPerDay;
				float MissingMoneyRatio = FMath::Min(1.0f, Company->GetMoney() / StationPrice);


				//FLOGV("StationPrice=%f DayToPayPrice=%f", StationPrice, DayToPayPrice);



				float Score =  (100.f / DayToPayPrice) * MissingMoneyRatio;

				//FLOGV("%s in %s Score=%f", *StationDescription->Name.ToString(), *Sector->GetSectorName().ToString(), Score);
				//FLOGV("         Bonus=%f", *StationDescription->Name.ToString(), *Sector->GetSectorName().ToString(), Bonus);
				//FLOGV("         Malus=%f", *StationDescription->Name.ToString(), *Sector->GetSectorName().ToString(), Malus);


				if(Bonus > 0)
				{
					Score *= Bonus;
				}

				if(Malus > 0)
				{
					Score /= Malus;
				}

				//FLOGV("         Final Score =%f", *StationDescription->Name.ToString(), *Sector->GetSectorName().ToString(), Score);


				if(ConstructionProjectSector == Sector && ConstructionProjectStation == StationDescription)
				{
					CurrentConstructionScore = Score;
				}

				if (GainPerDay > 0 && (!BestStationDescription || Score > BestScore))
				{
					//FLOGV("New Best : StationPrice=%f DayToPayPrice=%f", StationPrice, DayToPayPrice);
					//FLOGV("           MissingMoneyRatio=%f Score=%f", MissingMoneyRatio, Score);


					BestScore = Score;
					BestStationDescription = StationDescription;
					BestSector = Sector;
				}
			}
		}
	}


	if (BestSector && BestStationDescription)
	{
		FLOGV("%s >>> %s in %s Score=%f", *Company->GetCompanyName().ToString(),  *BestStationDescription->Name.ToString(), *BestSector->GetSectorName().ToString(), BestScore);

		// Start construction only if can afford to buy the station

		float StationPrice = STATION_CONSTRUCTION_PRICE_BONUS * UFlareGameTools::ComputeShipPrice(BestStationDescription->Identifier, BestSector, true);

		bool StartConstruction = true;

		if(CurrentConstructionScore * 1.5 > BestScore)
		{
			StartConstruction = false;
			FLOGV("    dont change construction yet : current score is %f but best score is %f", CurrentConstructionScore, BestScore);
		}

		if (StationPrice > Company->GetMoney())
		{
			StartConstruction = false;
			FLOGV("    dont build yet :station cost %f but company has only %lld", StationPrice, Company->GetMoney());
		}

		int32 NeedCapacity = UFlareGameTools::ComputeConstructionCapacity(BestStationDescription->Identifier, Game);
		if(NeedCapacity > IdleCargoCapacity)
		{
			StartConstruction = false;
			FLOGV("    dont build yet :station nedd %d idle capacity but company has only %d", NeedCapacity, IdleCargoCapacity);
			IdleCargoCapacity -= NeedCapacity * 1.5; // Keep margin
		}


		if (StartConstruction)
		{
			ConstructionProjectStation = BestStationDescription;
			ConstructionProjectSector = BestSector;
		}
	}


	// Compute shipyard need shipyard
	// Count turn before a ship is buildable to add weigth to this option



	// Compute the place the farest from all shipyard


	// Compute the time to pay the price with the station

	// If best option weight > 1, build it.


	// TODO Save ConstructionProjectStation



	if (ConstructionProjectStation && ConstructionProjectSector)
	{
		TArray<FText> Reasons;
		if (!ConstructionProjectSector->CanBuildStation(ConstructionProjectStation, Company, Reasons, true))
		{

			// Abandon build project
			FLOGV("%s abandon to build %s in %s", *Company->GetCompanyName().ToString(), *ConstructionProjectStation->Name.ToString(), *ConstructionProjectSector->GetSectorName().ToString());
			ConstructionProjectStation = NULL;
			ConstructionProjectSector = NULL;
			ConstructionShips.Empty();

		}
		else
		{
			// TODO Need at least one cargo


			// Don't start construction if not enought ship to get the resources

			// TODO Buy cost keeping marging

			// Try build station


			if (ConstructionProjectSector->BuildStation(ConstructionProjectStation, Company) != NULL)
			{
				FLOGV("%s build %s in %s", *Company->GetCompanyName().ToString(), *ConstructionProjectStation->Name.ToString(), *ConstructionProjectSector->GetSectorName().ToString());

				// Build success clean contruction project
				ConstructionProjectStation = NULL;
				ConstructionProjectSector = NULL;
				ConstructionShips.Empty();
			}
			else
			{


				// Cannot build
				FLOGV("%s fail to build %s in %s", *Company->GetCompanyName().ToString(), *ConstructionProjectStation->Name.ToString(), *ConstructionProjectSector->GetSectorName().ToString());

				// TODO make price very attractive
				// TODO make capacity very high

				int32 NeedCapacity = UFlareGameTools::ComputeConstructionCapacity(ConstructionProjectStation->Identifier, Game);
				if(NeedCapacity > IdleCargoCapacity)
				{
					IdleCargoCapacity -= NeedCapacity;
				}

				ManagerConstructionShips(WorldResourceVariation);
			}
		}
	}



	// Buy ships
	if(IdleCargoCapacity < 0)
	{
		FLOGV("Want buy cargo : IdleCargoCapacity = %d", IdleCargoCapacity);
		// Buy Omen
		// TODO buy all kind of ships

		// Find shipyard

		for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
		{
			UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];


			for (int32 StationIndex = 0 ; StationIndex < Sector->GetSectorStations().Num(); StationIndex++)
			{
				UFlareSimulatedSpacecraft* Station = Sector->GetSectorStations()[StationIndex];


				TArray<UFlareFactory*>& Factories = Station->GetFactories();
				for (int32 Index = 0; Index < Factories.Num(); Index++)
				{
					UFlareFactory* Factory = Factories[Index];

					if (!Factory->IsShipyard())
					{
						continue;

					}

					// Can produce only if nobody as order a ship and nobody is buidling a ship

					if(Factory->GetOrderShipCompany() == NAME_None && Factory->GetTargetShipCompany() == NAME_None)
					{
						FLOG("Shipyard is available");


						if (Factory->IsLargeShipyard())
						{
							FLOG("Order atlas");
							// TODO generic helper

							if(UFlareGameTools::ComputeShipPrice("ship-atlas", Sector, true) * 2 < Company->GetMoney())
							{

								Factory->OrderShip(Company, "ship-atlas");
								Factory->Start();
							}
							else
							{
								FLOG("Not enought money");
							}
						}
						else if (Factory->IsSmallShipyard())
						{
							FLOG("Order omen");
							// TODO generic helper

							if(UFlareGameTools::ComputeShipPrice("ship-omen", Sector, true) * 2 < Company->GetMoney())
							{

								Factory->OrderShip(Company, "ship-omen");
								Factory->Start();
							}
							else
							{
								FLOG("Not enought money");
							}
						}
					}
				}
			}

		}

	}


}

void UFlareCompanyAI::ManagerConstructionShips(TMap<UFlareSimulatedSector*, SectorVariation> & WorldResourceVariation)
{
	// TODO simplify

	TArray<UFlareSimulatedSpacecraft *> ShipsInConstructionSector;
	TArray<UFlareSimulatedSpacecraft *> ShipsInOtherSector;
	TArray<UFlareSimulatedSpacecraft *> ShipsToTravel;

	// Generate ships lists
	for (int32 ShipIndex = 0 ; ShipIndex < ConstructionShips.Num(); ShipIndex++)
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

	// List missing ressources
	for (int ResourceIndex = 0; ResourceIndex < ConstructionProjectStation->CycleCost.InputResources.Num() ; ResourceIndex++)
	{
		FFlareFactoryResource* Resource = &ConstructionProjectStation->CycleCost.InputResources[ResourceIndex];

		int32 NeededQuantity = Resource->Quantity;

		int32 OwnedQuantity = ConstructionProjectSector->GetResourceCount(Company, &Resource->Resource->Data, true);

		// Add not in sector ships resources
		for (int32 ShipIndex = 0 ; ShipIndex < ConstructionShips.Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = ConstructionShips[ShipIndex];
			if (Ship->GetCurrentSector() != ConstructionProjectSector)
			{
				OwnedQuantity = Ship->GetCargoBay()->GetResourceQuantity(&Resource->Resource->Data);
			}
		}

		if(NeededQuantity > OwnedQuantity)
		{
			MissingResourcesQuantity.Add(&Resource->Resource->Data, NeededQuantity - OwnedQuantity);
		}
	}

	// First strep, agregate ressources in construction sector
	for (int32 ShipIndex = 0 ; ShipIndex < ShipsInConstructionSector.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = ShipsInConstructionSector[ShipIndex];

		if (Ship->GetCargoBay()->GetFreeCargoSpace() == 0)
		{
			// Full, do nothing
			continue;
		}


		// If no full, give to others ships
		for (uint32 CargoIndex = 0 ; CargoIndex < Ship->GetCargoBay()->GetSlotCount() ; CargoIndex++)
		{
			FFlareCargo* Cargo = Ship->GetCargoBay()->GetSlot(CargoIndex);

			if (Cargo->Resource == NULL)
			{
				continue;
			}

			FFlareResourceDescription* ResourceToGive = Cargo->Resource;
			uint32 QuantityToGive = Ship->GetCargoBay()->GetResourceQuantity(ResourceToGive);


			for (int32 OtherShipIndex = ShipIndex+1 ; QuantityToGive > 0 && OtherShipIndex < ShipsInConstructionSector.Num(); OtherShipIndex++)
			{

				UFlareSimulatedSpacecraft* OtherShip = ShipsInConstructionSector[OtherShipIndex];
				uint32 GivenQuantity = OtherShip->GetCargoBay()->GiveResources(ResourceToGive, QuantityToGive);

				Ship->GetCargoBay()->TakeResources(ResourceToGive, GivenQuantity);

				QuantityToGive -= GivenQuantity;

				if (QuantityToGive == 0)
				{
					break;
				}
			}
		}

		// Then add to "to travel" ship list if can contain some missing resources
		TArray<FFlareResourceDescription*> MissingResources;
		MissingResourcesQuantity.GetKeys(MissingResources);
		for (int ResourceIndex = 0; ResourceIndex < MissingResources.Num() ; ResourceIndex++)
		{
			FFlareResourceDescription* MissingResource = MissingResources[ResourceIndex];

			if (Ship->GetCargoBay()->GetFreeSpaceForResource(MissingResource))
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
		for (int32 ShipIndex = 0 ; ShipIndex < ShipsToTravel.Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = ShipsToTravel[ShipIndex];
			if (Ship->GetCargoBay()->GetUsedCargoSpace() > 0)
			{
				// If at least 1 resource, go to construction sector
				Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), ConstructionProjectSector);
			}
			else
			{
				// This ship is no more needed, release it
				ConstructionShips.Remove(Ship);
				ShipIndex--;
			}
		}
	}
	else
	{
		// Still some resource to get
		for (int32 ShipIndex = 0 ; ShipIndex < ShipsToTravel.Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = ShipsToTravel[ShipIndex];

			TArray<FFlareResourceDescription*> MissingResources;
			MissingResourcesQuantity.GetKeys(MissingResources);
			for (int ResourceIndex = 0; ResourceIndex < MissingResources.Num() ; ResourceIndex++)
			{
				if(Ship->IsTrading())
				{
					break;
				}

				FFlareResourceDescription* MissingResource = MissingResources[ResourceIndex];
				if(!MissingResourcesQuantity.Contains(MissingResource))
				{
					FLOGV("!!!!!!!!! MissingResourcesQuantity don't contains %s 0", *MissingResource->Name.ToString());
				}
				int32 MissingResourceQuantity = MissingResourcesQuantity[MissingResource];

				int32 Capacity = Ship->GetCargoBay()->GetFreeSpaceForResource(MissingResource);

				int32 QuantityToBuy = FMath::Min(Capacity, MissingResourceQuantity);

				int32 TakenQuantity = 0;


				// Try to load or buy
				SectorHelper::FlareTradeRequest Request;
				Request.Resource = MissingResource;
				Request.Operation = EFlareTradeRouteOperation::LoadOrBuy;
				Request.Client = Ship;
				Request.MaxQuantity = QuantityToBuy;

				UFlareSimulatedSpacecraft* StationCandidate = SectorHelper::FindTradeStation(Request);

				if(StationCandidate)
				{
					TakenQuantity = SectorHelper::Trade(StationCandidate, Ship, MissingResource, Request.MaxQuantity);
				}


				MissingResourceQuantity -= TakenQuantity;
				if(MissingResourceQuantity == 0)
				{
					MissingResourcesQuantity.Remove(MissingResource);
				}
				else
				{
					if(!MissingResourcesQuantity.Contains(MissingResource))
					{
						FLOGV("!!!!!!!!! MissingResourcesQuantity don't contains %s 1", *MissingResource->Name.ToString());
					}
					MissingResourcesQuantity[MissingResource] = MissingResourceQuantity;
				}
			}

			bool IsFull = true;
			for (int ResourceIndex = 0; ResourceIndex < MissingResources.Num() ; ResourceIndex++)
			{
				FFlareResourceDescription* MissingResource = MissingResources[ResourceIndex];

				if (Ship->GetCargoBay()->GetFreeSpaceForResource(MissingResource))
				{
					// Can do more work
					IsFull = false;
					break;
				}
			}

			if(IsFull)
			{
				// Go to construction sector
				Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), ConstructionProjectSector);
			}
			else
			{
				// Refresh missing resources
				MissingResources.Empty();
				MissingResourcesQuantity.GetKeys(MissingResources);

				UFlareSimulatedSector* BestSector = NULL;
				FFlareResourceDescription* BestResource = NULL;
				int32 BestScore = 0;
				int32 BestEstimateTake = 0;

				// Look for station with stock
				for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
				{
					UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];

					if(!WorldResourceVariation.Contains(Sector))
					{
						FLOGV("!!!!!!!!! WorldResourceVariation don't contains %s", *Sector->GetSectorName().ToString());
					}
					SectorVariation* SectorVariation = &WorldResourceVariation[Sector];


					for (int ResourceIndex = 0; ResourceIndex < MissingResources.Num() ; ResourceIndex++)
					{
						FFlareResourceDescription* MissingResource = MissingResources[ResourceIndex];


						struct ResourceVariation* Variation = &SectorVariation->ResourceVariations[MissingResource];

						int32 Stock = Variation->FactoryStock + Variation->OwnedStock + Variation->StorageStock;

						if (Stock == 0)
						{
							continue;
						}

						// Sector with missing ressource stock
						if(!MissingResourcesQuantity.Contains(MissingResource))
						{
							FLOGV("!!!!!!!!! MissingResourcesQuantity don't contains %s 2", *MissingResource->Name.ToString());
						}
						int32 MissingResourceQuantity = MissingResourcesQuantity[MissingResource];
						int32 Capacity = Ship->GetCargoBay()->GetFreeSpaceForResource(MissingResource);



						int32 Score = FMath::Min(Stock, MissingResourceQuantity);
						Score = FMath::Min(Score, Capacity);

						if (Score > 0 && (BestSector == NULL || BestScore < Score))
						{
							BestSector = Sector;
							BestScore = Score;
							BestResource = MissingResource;
							BestEstimateTake = FMath::Min(Capacity, Stock);
						}
					}
				}

				if(!BestSector)
				{
					// Try a sector with a flow
					for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
					{
						UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];

						SectorVariation* SectorVariation = &WorldResourceVariation[Sector];


						for (int ResourceIndex = 0; ResourceIndex < MissingResources.Num() ; ResourceIndex++)
						{
							FFlareResourceDescription* MissingResource = MissingResources[ResourceIndex];


							struct ResourceVariation* Variation = &SectorVariation->ResourceVariations[MissingResource];

							int32 Flow = Variation->FactoryFlow + Variation->OwnedFlow;

							if (Flow == 0)
							{
								continue;
							}

							// Sector with missing ressource stock
							if(!MissingResourcesQuantity.Contains(MissingResource))
							{
								FLOGV("!!!!!!!!! MissingResourcesQuantity don't contains %s 3", *MissingResource->Name.ToString());
							}
							int32 MissingResourceQuantity = MissingResourcesQuantity[MissingResource];
							int32 Capacity = Ship->GetCargoBay()->GetFreeSpaceForResource(MissingResource);


							/* Owned stock will be set negative if multiple cargo go here. This will impact the score */
							int32 Score = FMath::Min(Flow + Variation->OwnedStock, MissingResourceQuantity);
							Score = FMath::Min(Score, Capacity);

							if (Score > 0 && (BestSector == NULL || BestScore < Score))
							{
								BestSector = Sector;
								BestScore = Score;
								BestResource = MissingResource;
								BestEstimateTake = FMath::Min(Capacity, Flow);
							}
						}
					}
				}

				if(BestSector)
				{
					// Travel to sector
					Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), BestSector);

					// Decrease missing quantity
					if(!MissingResourcesQuantity.Contains(BestResource))
					{
						FLOGV("!!!!!!!!! MissingResourcesQuantity don't contains %s 4", *BestResource->Name.ToString());
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

SectorDeal UFlareCompanyAI::FindBestDealForShipFromSector(UFlareSimulatedSpacecraft* Ship, UFlareSimulatedSector* SectorA, SectorDeal* DealToBeat, TMap<UFlareSimulatedSector*, SectorVariation> *WorldResourceVariation)
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

		if(SectorA == SectorB)
		{
			// Stay in sector option
			TravelTimeToA = 1;
			TravelTimeToB = 0;
		}
		else
		{
			// Travel time
			TravelTimeToA = UFlareTravel::ComputeTravelDuration(Game->GetGameWorld(), Ship->GetCurrentSector(), SectorA);
			TravelTimeToB = UFlareTravel::ComputeTravelDuration(Game->GetGameWorld(), SectorA, SectorB);

		}
		int64 TravelTime = TravelTimeToA + TravelTimeToB;


		SectorVariation* SectorVariationA = &(*WorldResourceVariation)[SectorA];
		SectorVariation* SectorVariationB = &(*WorldResourceVariation)[SectorB];

		for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
		{
			FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
			struct ResourceVariation* VariationA = &SectorVariationA->ResourceVariations[Resource];
			struct ResourceVariation* VariationB = &SectorVariationB->ResourceVariations[Resource];

			if(!VariationA->OwnedFlow &&
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


			int32 InitialQuantity = Ship->GetCargoBay()->GetResourceQuantity(Resource);
			int32 FreeSpace = Ship->GetCargoBay()->GetFreeSpaceForResource(Resource);

			int32 StockInAAfterTravel =
					VariationA->OwnedStock
					+ VariationA->FactoryStock
					+ VariationA->StorageStock
					- (VariationA->OwnedFlow * TravelTimeToA)
					- (VariationA->FactoryFlow * TravelTimeToA);

			if(StockInAAfterTravel <= 0 && InitialQuantity == 0)
			{
				continue;
			}

			int32 CanBuyQuantity = FMath::Min(FreeSpace, StockInAAfterTravel);

			// Affordable quantity
			CanBuyQuantity =  FMath::Min(CanBuyQuantity, (int32) (Company->GetMoney() / SectorA->GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryInput)));

			int32 CapacityInBAfterTravel =
					VariationB->OwnedCapacity
					+ VariationB->FactoryCapacity
					+ VariationB->StorageCapacity
					+ VariationB->OwnedFlow * TravelTime
					+ VariationB->FactoryFlow * TravelTime
					- VariationB->IncomingResources;

			int32 SellQuantity = FMath::Min(CapacityInBAfterTravel, CanBuyQuantity + InitialQuantity);
			int32  BuyQuantity = FMath::Max(0, SellQuantity - InitialQuantity);

			// Use price details

			int32 MoneyGain = 0;
			int32 QuantityToSell = SellQuantity;

			int32 OwnedCapacity = FMath::Max(0, (int32) (VariationB->OwnedCapacity + VariationB->OwnedFlow * TravelTime));
			int32 FactoryCapacity = FMath::Max(0, (int32) (VariationB->FactoryCapacity + VariationB->FactoryFlow * TravelTime));
			int32 StorageCapacity = VariationB->StorageCapacity;

			int32 OwnedSellQuantity = FMath::Min(OwnedCapacity, QuantityToSell);
			MoneyGain += OwnedSellQuantity * SectorB->GetResourcePrice(Resource, EFlareResourcePriceContext::Default) * 1.1; // Valorise transport to its own station
			QuantityToSell -= OwnedSellQuantity;

			int32 FactorySellQuantity = FMath::Min(FactoryCapacity, QuantityToSell);
			MoneyGain += FactorySellQuantity * SectorB->GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryInput);
			QuantityToSell -= FactorySellQuantity;

			int32 StorageSellQuantity = FMath::Min(StorageCapacity, QuantityToSell);
			MoneyGain += StorageSellQuantity * SectorB->GetResourcePrice(Resource, EFlareResourcePriceContext::Default);
			QuantityToSell -= StorageSellQuantity;

			int32 MoneySpend = 0;
			int32 QuantityToBuy = BuyQuantity;

			int32 OwnedStock = FMath::Max(0, (int32) (VariationA->OwnedStock - VariationA->OwnedFlow * TravelTimeToA));
			int32 FactoryStock = FMath::Max(0, (int32) (VariationA->FactoryStock - VariationA->FactoryFlow * TravelTimeToA));
			int32 StorageStock = VariationA->StorageStock;


			int32 OwnedBuyQuantity = FMath::Min(OwnedStock, QuantityToBuy);
			MoneySpend += OwnedBuyQuantity * SectorA->GetResourcePrice(Resource, EFlareResourcePriceContext::Default) * 0.9; // Valorise buy to self
			QuantityToBuy -= OwnedBuyQuantity;

			int32 FactoryBuyQuantity = FMath::Min(FactoryStock, QuantityToBuy);
			MoneySpend += FactoryBuyQuantity * SectorA->GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryOutput);
			QuantityToBuy -= FactoryBuyQuantity;

			int32 StorageBuyQuantity = FMath::Min(StorageStock, QuantityToBuy);
			MoneySpend += StorageBuyQuantity * SectorA->GetResourcePrice(Resource, EFlareResourcePriceContext::Default);
			QuantityToBuy -= StorageBuyQuantity;


			// Station construction incitation
			/*if (SectorB == ConstructionProjectSector)
			{
				for (int ConstructionResourceIndex = 0; ConstructionResourceIndex < ConstructionProjectStation->CycleCost.InputResources.Num() ; ConstructionResourceIndex++)
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

			float MoneyBalanceParDay = (float) MoneyBalance / (float) TravelTime;


			if(MoneyBalanceParDay > BestDeal.MoneyBalanceParDay)
			{

				BestDeal.MoneyBalanceParDay = MoneyBalanceParDay;
				BestDeal.SectorA = SectorA;
				BestDeal.SectorB = SectorB;
				BestDeal.Resource = Resource;
				BestDeal.BuyQuantity = BuyQuantity;

				/*FLOGV("Travel %s -> %s -> %s : %lld days", *Ship->GetCurrentSector()->GetSectorName().ToString(),
							*SectorA->GetSectorName().ToString(), *SectorB->GetSectorName().ToString(), TravelTime);

				FLOGV("New Best Resource %s", *Resource->Name.ToString())

				FLOGV(" -> IncomingCapacity=%u", SectorVariationA->IncomingCapacity);
				FLOGV(" -> IncomingResources=%u", VariationA->IncomingResources);
				FLOGV(" -> InitialQuantity=%u", InitialQuantity);
				FLOGV(" -> FreeSpace=%u", FreeSpace);
				FLOGV(" -> StockInAAfterTravel=%u", StockInAAfterTravel);
				FLOGV(" -> BuyQuantity=%u", BuyQuantity);
				FLOGV(" -> CapacityInBAfterTravel=%u", CapacityInBAfterTravel);
				FLOGV(" -> SellQuantity=%u", SellQuantity);
				FLOGV(" -> MoneyGain=%d", MoneyGain);
				FLOGV(" -> MoneySpend=%d", MoneySpend);
				FLOGV("   -> OwnedBuyQuantity=%d", OwnedBuyQuantity);
				FLOGV("   -> FactoryBuyQuantity=%d", FactoryBuyQuantity);
				FLOGV("   -> StorageBuyQuantity=%d", StorageBuyQuantity);
				FLOGV(" -> MoneyBalance=%d", MoneyBalance);
				FLOGV(" -> MoneyBalanceParDay=%f", MoneyBalanceParDay);*/
			}
		}
	}

	return BestDeal;
}

void UFlareCompanyAI::Tick()
{
	if (Company == Game->GetPC()->GetCompany())
	{
		return;
	}

	ResetShipGroup(EFlareCombatTactic::AttackMilitary);

	SimulateDiplomacy();
}

void UFlareCompanyAI::SimulateDiplomacy()
{
	// Declare war or make peace
	for (int32 CompanyIndex = 0; CompanyIndex < Game->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
	{
		UFlareCompany* OtherCompany = Game->GetGameWorld()->GetCompanies()[CompanyIndex];

		if(OtherCompany == Company)
		{
			continue;
		}

		if(Company->GetHostility(OtherCompany) == EFlareHostility::Hostile && Company->GetReputation(OtherCompany) > -100)
		{
			Company->SetHostilityTo(OtherCompany, false);
		}
		else if(Company->GetHostility(OtherCompany) != EFlareHostility::Hostile && Company->GetReputation(OtherCompany) <= -100)
		{
			Company->SetHostilityTo(OtherCompany, true);
			if (OtherCompany == Game->GetPC()->GetCompany())
			{
				OtherCompany->SetHostilityTo(Company, true);
			}
		}
	}
}

/** Destroy a spacecraft */
void UFlareCompanyAI::DestroySpacecraft(UFlareSimulatedSpacecraft* Spacecraft)
{
	// Don't keep reference on destroyed ship
	ConstructionShips.Remove(Spacecraft);
}

TArray<UFlareSimulatedSpacecraft*> UFlareCompanyAI::FindIdleCargos()
{
	TArray<UFlareSimulatedSpacecraft*> IdleCargos;

	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];


		for (int32 ShipIndex = 0 ; ShipIndex < Sector->GetSectorShips().Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = Sector->GetSectorShips()[ShipIndex];
			if(Ship->GetCompany() != Company || Ship->GetCurrentTradeRoute() != NULL || Ship->GetCargoBay()->GetCapacity() == 0 || ConstructionShips.Contains(Ship))
			{
				continue;
			}

			IdleCargos.Add(Ship);
		}
	}

	return IdleCargos;
}

/*----------------------------------------------------
	Command groups
----------------------------------------------------*/

void UFlareCompanyAI::SetCurrentShipGroup(EFlareCombatGroup::Type Type)
{
	CurrentShipGroup = Type;
}

void UFlareCompanyAI::SetTacticForCurrentShipGroup(EFlareCombatTactic::Type Tactic)
{
	check(CurrentShipGroup < CurrentCombatTactics.Num());
	CurrentCombatTactics[CurrentShipGroup] = Tactic;
	if (CurrentShipGroup == EFlareCombatGroup::AllMilitary)
	{
		CurrentCombatTactics[EFlareCombatGroup::Capitals] = Tactic;
		CurrentCombatTactics[EFlareCombatGroup::Fighters] = Tactic;
	}
}

EFlareCombatGroup::Type UFlareCompanyAI::GetCurrentShipGroup() const
{
	return CurrentShipGroup;
}

EFlareCombatTactic::Type UFlareCompanyAI::GetCurrentTacticForShipGroup(EFlareCombatGroup::Type Type) const
{
	check(Type < CurrentCombatTactics.Num());
	return CurrentCombatTactics[Type];
}

int32 UFlareCompanyAI::GetShipCountForShipGroup(EFlareCombatGroup::Type Type) const
{
	switch (Type)
	{
		case EFlareCombatGroup::AllMilitary:
			return CurrentMilitaryShipCount;

		case EFlareCombatGroup::Capitals:
			return CurrentCapitalShipCount;

		case EFlareCombatGroup::Fighters:
			return CurrentFighterCount;

		case EFlareCombatGroup::Civilan:
		default:
			return CurrentCivilianShipCount;
	}
}

void UFlareCompanyAI::ResetControlGroups(UFlareSimulatedSector* Sector)
{
	// Reset ship count values
	CurrentMilitaryShipCount = 0;
	CurrentCapitalShipCount = 0;
	CurrentFighterCount = 0;
	CurrentCivilianShipCount = 0;

	// Compute the current count of all kinds of ships
	if (Sector)
	{
		TArray<UFlareSimulatedSpacecraft*>& ShipList = Sector->GetSectorShips();
		for (int32 Index = 0; Index < ShipList.Num(); Index++)
		{
			UFlareSimulatedSpacecraft* Ship = ShipList[Index];
			check(Ship);

			if (Ship->GetCompany() != Company)
			{
				continue;
			}

			if (Ship->IsMilitary())
			{
				CurrentMilitaryShipCount++;
				if (Ship->GetDescription()->Size == EFlarePartSize::L)
				{
					CurrentCapitalShipCount++;
				}
				else
				{
					CurrentFighterCount++;
				}
			}
			else
			{
				CurrentCivilianShipCount++;
			}
		}
	}
}

void UFlareCompanyAI::ResetShipGroup(EFlareCombatTactic::Type Tactic)
{
	CurrentCombatTactics.Empty();
	for (int32 Index = EFlareCombatGroup::AllMilitary; Index <= EFlareCombatGroup::Civilan; Index++)
	{
		CurrentCombatTactics.Add(Tactic);
	}
}

TMap<FFlareResourceDescription*, int32> UFlareCompanyAI::ComputeWorldResourceFlow()
{
	TMap<FFlareResourceDescription*, int32> WorldResourceFlow;
	for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;

		WorldResourceFlow.Add(Resource, 0);
	}

	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];
		uint32 CustomerStation = 0;


		for (int32 StationIndex = 0 ; StationIndex < Sector->GetSectorStations().Num(); StationIndex++)
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
					WorldResourceFlow[Resource] -=Flow;
				}

				// Ouput flow
				for (int32 ResourceIndex = 0; ResourceIndex < Factory->GetOutputResourcesCount(); ResourceIndex++)
				{
					FFlareResourceDescription* Resource = Factory->GetOutputResource(ResourceIndex);

					uint32 Flow = Factory->GetOutputResourceQuantity(ResourceIndex) / Factory->GetProductionDuration();
					WorldResourceFlow[Resource] +=Flow;
				}
			}
		}

		if(CustomerStation)
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

SectorVariation UFlareCompanyAI::ComputeSectorResourceVariation(UFlareSimulatedSector* Sector)
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

				if(Factory->IsProducing())
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

				uint32 ResourceQuantity = Station->GetCargoBay()->GetResourceQuantity(Resource);
				if (ResourceQuantity < SlotCapacity)
				{
					int32 Capacity = SlotCapacity - ResourceQuantity;
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

				if(Factory->IsProducing())
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

				uint32 Stock = Station->GetCargoBay()->GetResourceQuantity(Resource);
				if (Company == Station->GetCompany())
				{
					Variation->OwnedStock += Stock;
				}
				else
				{
					Variation->FactoryStock += Stock;
				}
			}


			// TODO habitation

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

				uint32 ResourceQuantity = Station->GetCargoBay()->GetResourceQuantity(Resource);

				// Dept are allowed for sell to customers

				if (ResourceQuantity < SlotCapacity)
				{
					int32 Capacity = SlotCapacity - ResourceQuantity;
					if (Company == Station->GetCompany())
					{
						Variation->OwnedCapacity += Capacity;
					}
					else
					{
						Variation->FactoryCapacity += Capacity;
					}
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

				uint32 ResourceQuantity = Station->GetCargoBay()->GetResourceQuantity(Resource);

				int32 CanBuyQuantity =  (int32) (Station->GetCompany()->GetMoney() / Sector->GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryInput));


				if (ResourceQuantity < SlotCapacity)
				{
					int32 Capacity = SlotCapacity - ResourceQuantity;
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

			}
		}

		// Station construction incitation
		/*if (ConstructionProjectSector == Sector)
		{
			for (int ResourceIndex = 0; ResourceIndex < ConstructionProjectStation->CycleCost.InputResources.Num() ; ResourceIndex++)
			{
				FFlareFactoryResource* Resource = &ConstructionProjectStation->CycleCost.InputResources[ResourceIndex];
				struct ResourceVariation* Variation = &SectorVariation.ResourceVariations[&Resource->Resource->Data];
				Variation->OwnedCapacity += Resource->Quantity;
			}
		}*/
	}

	if(OwnedCustomerStation || NotOwnedCustomerStation)
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


		for (int ShipIndex = 0; ShipIndex < IncomingFleet->GetShips().Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = IncomingFleet->GetShips()[ShipIndex];

			if (Ship->GetCargoBay()->GetSlotCapacity() == 0)
			{
				continue;
			}
			SectorVariation.IncomingCapacity += Ship->GetCargoBay()->GetCapacity() / RemainingTravelDuration;

			TArray<FFlareCargo>& CargoBaySlots = Ship->GetCargoBay()->GetSlots();
			for (int CargoIndex = 0; CargoIndex < CargoBaySlots.Num(); CargoIndex++)
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

	// Consider resource over 10 days of consumption as IncomingResources
	for (int32 StationIndex = 0 ; StationIndex < Sector->GetSectorStations().Num(); StationIndex++)
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

			if(TotalFlow >= 0)
			{
				int32 LongTermConsumption = TotalFlow * 10;
				int32 ResourceQuantity = Station->GetCargoBay()->GetResourceQuantity(Resource);

				if (ResourceQuantity > LongTermConsumption)
				{
					Variation->IncomingResources += ResourceQuantity - LongTermConsumption;
				}
			}
		}


	}

	return SectorVariation;
}

void UFlareCompanyAI::DumpSectorResourceVariation(UFlareSimulatedSector* Sector, TMap<FFlareResourceDescription*, struct ResourceVariation>* SectorVariation)
{
	FLOGV("Sector %s resource variation: ", *Sector->GetSectorName().ToString());
	for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
		struct ResourceVariation* Variation = &(*SectorVariation)[Resource];
		if(Variation->OwnedFlow ||
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
			if(Variation->OwnedFlow)
				FLOGV("   owned flow %d / day", Variation->OwnedFlow);
			if(Variation->FactoryFlow)
				FLOGV("   factory flow %d / day", Variation->FactoryFlow);
			if(Variation->OwnedStock)
				FLOGV("   owned stock %d", Variation->OwnedStock);
			if(Variation->FactoryStock)
				FLOGV("   factory stock %d", Variation->FactoryStock);
			if(Variation->StorageStock)
				FLOGV("   storage stock %d", Variation->StorageStock);
			if(Variation->OwnedCapacity)
				FLOGV("   owned capacity %d", Variation->OwnedCapacity);
			if(Variation->FactoryCapacity)
				FLOGV("   factory capacity %d", Variation->FactoryCapacity);
			if(Variation->StorageCapacity)
				FLOGV("   storage capacity %d", Variation->StorageCapacity);
		}

	}
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/


