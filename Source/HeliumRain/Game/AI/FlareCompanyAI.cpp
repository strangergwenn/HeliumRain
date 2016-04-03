
#include "../../Flare.h"
#include "FlareCompanyAI.h"
#include "../FlareCompany.h"
#include "../FlareGame.h"
#include "../../Spacecrafts/FlareSimulatedSpacecraft.h"

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

	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];
		int32 TransportCapacityBalance = Sector->GetTransportCapacityBalance(Company);
		//FLOGV("Sector %s, transport capacity=%d", *Sector->GetSectorName().ToString(), Sector->GetTransportCapacity(Company));
		//FLOGV("Sector %s, transport needs=%d", *Sector->GetSectorName().ToString(), Sector->GetTransportCapacityNeeds(Company));
		//FLOGV("Sector %s, transport balance=%d", *Sector->GetSectorName().ToString(), Sector->GetTransportCapacityBalance(Company));

		if (TransportCapacityBalance > 0)
		{
			// TODO tolerate few more ship
			UnassignShipsFromSector(Sector, (uint32) TransportCapacityBalance);
			//FLOGV("AI %s ACTION : Unassign ships from sector %s %d units", *Company->GetCompanyName().ToString(), *Sector->GetSectorName().ToString(), TransportCapacityBalance)
		}
		else if (TransportCapacityBalance < 0)
		{
			AssignShipsToSector(Sector, (uint32) (- TransportCapacityBalance));
			//FLOGV("AI %s ACTION : Assign ships to sector %s %d units", *Company->GetCompanyName().ToString(), *Sector->GetSectorName().ToString(), TransportCapacityBalance)
		}
		// TODO reassign large ships


		// Assign ship for trade

		int32 TradeTransportCapacityBalance = Sector->GetTransportCapacityBalance(Company, true);
		if (TradeTransportCapacityBalance < 0)
		{
			AssignShipsToSector(Sector, (uint32) (- TradeTransportCapacityBalance));
			FLOGV("AI %s ACTION : Assign ships to sector for trade %s %d units", *Company->GetCompanyName().ToString(), *Sector->GetSectorName().ToString(), TradeTransportCapacityBalance)
		}
		//FLOGV("Sector %s, final transport capacity=%d", *Sector->GetSectorName().ToString(), Sector->GetTransportCapacity(Company));
	}




	// Sell
	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];


		for (int32 ShipIndex = 0 ; ShipIndex < Sector->GetSectorShips().Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = Sector->GetSectorShips()[ShipIndex];
			if(Ship->GetCompany() != Company || Ship->GetCargoBay()->GetCapacity() == 0)
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

				uint32 SoldResources = Sector->GiveResources(Company, Cargo.Resource , Cargo.Quantity, true);
				Ship->GetCargoBay()->TakeResources(Cargo.Resource, SoldResources);
			}

		}
	}

	// TODO Move unassign ship un sector that have not enough ship


	// Substract ship that are currently traveling to the sector and are not in a trade route


	// Trade route creation


	// TODO IF AT LEAST ONE IDLE SHIP

	TMap<UFlareSimulatedSector*, TMap<FFlareResourceDescription*, struct ResourceVariation>> WorldResourceVariation;

	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];


		// Compute input and output ressource equation (ex: 100 + 10/ day)

		// TODO, cache in sector for all company


		TMap<FFlareResourceDescription*, struct ResourceVariation> Variation = ComputeSectorResourceVariation(Sector);

		WorldResourceVariation.Add(Sector, Variation);
		//DumpSectorResourceVariation(Sector, &Variation);

	}

	TArray<UFlareSimulatedSpacecraft*> IdleCargos = FindIdleCargos();

	for (int32 ShipIndex = 0 ; ShipIndex < IdleCargos.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = IdleCargos[ShipIndex];

	//	FLOGV("Search something to do for %s", *Ship->GetImmatriculation().ToString());

		// TODO first, go for auto assign if needed
		// TODO substract current travelling ships

		float BestMoneyBalanceParDay = 0;
		UFlareSimulatedSector* BestSectorA = NULL;
		UFlareSimulatedSector* BestSectorB = NULL;
		FFlareResourceDescription* BestResource = NULL;
		int32 BestBuyQuantity = 0;

		for (int32 SectorAIndex = 0; SectorAIndex < Company->GetKnownSectors().Num(); SectorAIndex++)
		{
			UFlareSimulatedSector* SectorA = Company->GetKnownSectors()[SectorAIndex];

			for (int32 SectorBIndex = 0; SectorBIndex < Company->GetKnownSectors().Num(); SectorBIndex++)
			{
				UFlareSimulatedSector* SectorB = Company->GetKnownSectors()[SectorBIndex];

				if(SectorA == SectorB)
				{
					// Same sector
					continue;
				}

				// Travel time

				int64 TravelTimeToA = UFlareTravel::ComputeTravelDuration(Game->GetGameWorld(), Ship->GetCurrentSector(), SectorA);
				int64 TravelTimeToB = UFlareTravel::ComputeTravelDuration(Game->GetGameWorld(), SectorA, SectorB);
				int64 TravelTime = TravelTimeToA + TravelTimeToB;


				//FLOGV("Travel %s -> %s -> %s : %lld days", *Ship->GetCurrentSector()->GetSectorName().ToString(),
				//			*SectorA->GetSectorName().ToString(), *SectorB->GetSectorName().ToString(), TravelTime);


				TMap<FFlareResourceDescription*, struct ResourceVariation>* SectorVariationA = &WorldResourceVariation[SectorA];
				TMap<FFlareResourceDescription*, struct ResourceVariation>* SectorVariationB = &WorldResourceVariation[SectorB];

				for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
				{
					FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
					struct ResourceVariation* VariationA = &(*SectorVariationA)[Resource];
					struct ResourceVariation* VariationB = &(*SectorVariationB)[Resource];

					if(VariationA->OwnedFlow &&
							VariationA->FactoryFlow &&
							VariationA->OwnedStock &&
							VariationA->FactoryStock &&
							VariationA->StorageStock &&
							VariationA->OwnedCapacity &&
							VariationA->FactoryCapacity &&
							VariationA->StorageCapacity &&
							VariationB->OwnedFlow &&
							VariationB->FactoryFlow &&
							VariationB->OwnedStock &&
							VariationB->FactoryStock &&
							VariationB->StorageStock &&
							VariationB->OwnedCapacity &&
							VariationB->FactoryCapacity &&
							VariationB->StorageCapacity)
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

					int32 CapacityInBAfterTravel =
							VariationB->OwnedCapacity
							+ VariationB->FactoryCapacity
							+ VariationB->StorageCapacity
							+ VariationB->OwnedFlow * TravelTime
							+ VariationB->FactoryFlow * TravelTime;

					int32 SellQuantity = FMath::Min(CapacityInBAfterTravel, CanBuyQuantity + InitialQuantity);
					int32  BuyQuantity = FMath::Max(0, SellQuantity - InitialQuantity);

					// Use price details

					int32 MoneyGain = 0;
					int32 QuantityToSell = SellQuantity;

					int32 OwnedCapacity = FMath::Max(0, (int32) (VariationB->OwnedCapacity + VariationB->OwnedFlow * TravelTime));
					int32 FactoryCapacity = FMath::Max(0, (int32) (VariationB->FactoryCapacity + VariationB->FactoryFlow * TravelTime));
					int32 StorageCapacity = VariationB->StorageCapacity;

					int32 OwnedSellQuantity = FMath::Min(OwnedCapacity, QuantityToSell);
					MoneyGain += OwnedSellQuantity * SectorB->GetResourcePrice(Resource) * 1.1; // Valorise transport to its own station
					QuantityToSell -= OwnedSellQuantity;

					int32 FactorySellQuantity = FMath::Min(FactoryCapacity, QuantityToSell);
					MoneyGain += FactorySellQuantity * SectorB->GetResourcePrice(Resource) * 1.01;
					QuantityToSell -= FactorySellQuantity;

					int32 StorageSellQuantity = FMath::Min(StorageCapacity, QuantityToSell);
					MoneyGain += StorageSellQuantity * SectorB->GetResourcePrice(Resource);
					QuantityToSell -= StorageSellQuantity;

					int32 MoneySpend = 0;
					int32 QuantityToBuy = BuyQuantity;

					int32 OwnedStock = FMath::Max(0, (int32) (VariationA->OwnedStock - VariationA->OwnedFlow * TravelTimeToA));
					int32 FactoryStock = FMath::Max(0, (int32) (VariationA->FactoryStock - VariationA->FactoryFlow * TravelTimeToA));
					int32 StorageStock = VariationA->StorageStock;


					int32 OwnedBuyQuantity = FMath::Min(OwnedStock, QuantityToBuy);
					MoneySpend += OwnedBuyQuantity * SectorA->GetResourcePrice(Resource) * 0.9; // Valorise buy to self
					QuantityToBuy -= OwnedBuyQuantity;

					int32 FactoryBuyQuantity = FMath::Min(FactoryStock, QuantityToBuy);
					MoneySpend += FactoryBuyQuantity * SectorA->GetResourcePrice(Resource) * 0.99;
					QuantityToBuy -= FactoryBuyQuantity;

					int32 StorageBuyQuantity = FMath::Min(StorageStock, QuantityToBuy);
					MoneySpend += StorageBuyQuantity * SectorA->GetResourcePrice(Resource);
					QuantityToBuy -= StorageBuyQuantity;


					int32 MoneyBalance = MoneyGain - MoneySpend;

					float MoneyBalanceParDay = (float) MoneyBalance / (float) TravelTime;


					if(MoneyBalanceParDay > BestMoneyBalanceParDay)
					{
						BestMoneyBalanceParDay = MoneyBalanceParDay;
						BestSectorA = SectorA;
						BestSectorB = SectorB;
						BestResource = Resource;
						BestBuyQuantity = BuyQuantity;
						/*FLOGV(" Resource %s", *Resource->Name.ToString())

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
		}

		//FLOGV("Best balance for %s : %f credit per day", *Ship->GetImmatriculation().ToString(), BestMoneyBalanceParDay);
		if(BestResource)
		{
			//FLOGV(" -> Transfert %s from %s to %s", *BestResource->Name.ToString(), *BestSectorA->GetSectorName().ToString(), *BestSectorB->GetSectorName().ToString());
			if (Ship->GetCurrentSector() == BestSectorA)
			{
				// Already in A, buy resources and go to B

				uint32 BroughtResource = BestSectorA->TakeUselessResources(Company, BestResource, BestBuyQuantity, false);
				BroughtResource += BestSectorA->TakeUselessResources(Company, BestResource, BestBuyQuantity - BroughtResource, true);
				Ship->GetCargoBay()->GiveResources(BestResource, BroughtResource);

				// TODO reduce computed sector stock

				//FLOGV(" -> Buy %d / %d", BroughtResource, BestBuyQuantity);
				if(BroughtResource == BestBuyQuantity)
				{
					// All wanted resources is get, travel to B
					Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), BestSectorB);
					//FLOGV(" -> Travel to %s", *BestSectorB->GetSectorName().ToString());
				}


			}
			else
			{
				Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), BestSectorA);
				//FLOGV(" -> Travel to %s", *BestSectorA->GetSectorName().ToString());
			}
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








	/*
	TArray<UFlareSimulatedSpacecraft*> CompanyShips = Company->GetCompanyShips();

	// Assign ships to current sector
	for (int32 ShipIndex = 0; ShipIndex < CompanyShips.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = CompanyShips[ShipIndex];
		Ship->AssignToSector(true);
	}*/
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

void UFlareCompanyAI::UnassignShipsFromSector(UFlareSimulatedSector* Sector, uint32 MaxCapacity)
{
	uint32 RemainingCapacity = MaxCapacity;

	for (int32 ShipIndex = 0 ; ShipIndex < Sector->GetSectorShips().Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = Sector->GetSectorShips()[ShipIndex];
		if(Ship->GetCompany() != Company || !Ship->IsAssignedToSector())
		{
			continue;
		}

		if(Ship->GetCargoBay()->GetCapacity() <= RemainingCapacity)
		{
			Ship->AssignToSector(false);
			RemainingCapacity-= Ship->GetCargoBay()->GetCapacity();
		}

		if(RemainingCapacity == 0)
		{
			return;
		}
	}
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
			if(Ship->GetCompany() != Company || Ship->IsAssignedToSector() || Ship->GetCurrentTradeRoute() != NULL || Ship->GetCargoBay()->GetCapacity() == 0)
			{
				continue;
			}

			IdleCargos.Add(Ship);
		}
	}

	return IdleCargos;
}

void UFlareCompanyAI::AssignShipsToSector(UFlareSimulatedSector* Sector, uint32 MinCapacity)
{
	int32 RemainingCapacity = MinCapacity;

	for (int32 ShipIndex = 0 ; ShipIndex < Sector->GetSectorShips().Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = Sector->GetSectorShips()[ShipIndex];
		if(Ship->GetCompany() != Company || Ship->IsAssignedToSector() || Ship->GetCurrentTradeRoute() != NULL || Ship->GetCargoBay()->GetCapacity() == 0)
		{
			continue;
		}


		Ship->AssignToSector(true);
		RemainingCapacity-= Ship->GetCargoBay()->GetCapacity();


		if(RemainingCapacity <= 0)
		{
			return;
		}
	}
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

void UFlareCompanyAI::ResetControlGroups(UFlareSector* Sector)
{
	// Reset ship count values
	CurrentMilitaryShipCount = 0;
	CurrentCapitalShipCount = 0;
	CurrentFighterCount = 0;
	CurrentCivilianShipCount = 0;

	// Compute the current count of all kinds of ships
	if (Sector)
	{
		TArray<IFlareSpacecraftInterface*>& ShipList = Sector->GetSectorShipInterfaces();
		for (int32 Index = 0; Index < ShipList.Num(); Index++)
		{
			IFlareSpacecraftInterface* Ship = ShipList[Index];
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

TMap<FFlareResourceDescription*, ResourceVariation> UFlareCompanyAI::ComputeSectorResourceVariation(UFlareSimulatedSector* Sector)
{
	TMap<FFlareResourceDescription*, ResourceVariation> SectorVariations;
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
		SectorVariations.Add(Resource, ResourceVariation);
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
				struct ResourceVariation* Variation = &SectorVariations[Resource];


				uint32 Flow = Factory->GetInputResourceQuantity(ResourceIndex) / Factory->GetProductionDuration();

				if (Flow == 0)
				{
					continue;
				}

				if (Company == Station->GetCompany())
				{
					Variation->OwnedFlow += Flow;
				}
				else
				{
					Variation->FactoryFlow += Flow;
				}

				uint32 ResourceQuantity = Station->GetCargoBay()->GetResourceQuantity(Resource);
				if (ResourceQuantity < SlotCapacity)
				{
					uint32 Capacity = SlotCapacity - ResourceQuantity;
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

			// Ouput flow
			for (int32 ResourceIndex = 0; ResourceIndex < Factory->GetOutputResourcesCount(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = Factory->GetOutputResource(ResourceIndex);
				struct ResourceVariation* Variation = &SectorVariations[Resource];

				uint32 Flow = Factory->GetOutputResourceQuantity(ResourceIndex) / Factory->GetProductionDuration();

				if (Flow == 0)
				{
					continue;
				}

				if (Company == Station->GetCompany())
				{
					Variation->OwnedFlow -= Flow;
				}
				else
				{
					Variation->FactoryFlow -= Flow;
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
				struct ResourceVariation* Variation = &SectorVariations[Resource];

				uint32 ResourceQuantity = Station->GetCargoBay()->GetResourceQuantity(Resource);
				if (ResourceQuantity < SlotCapacity)
				{
					uint32 Capacity = SlotCapacity - ResourceQuantity;
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
	}

	if(OwnedCustomerStation || NotOwnedCustomerStation)
	{
		float OwnedCustomerRatio = (float) OwnedCustomerStation / (float) (OwnedCustomerStation + NotOwnedCustomerStation);
		float NotOwnedCustomerRatio = (float) NotOwnedCustomerStation / (float) (OwnedCustomerStation + NotOwnedCustomerStation);

		for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->ConsumerResources.Num(); ResourceIndex++)
		{
			FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->ConsumerResources[ResourceIndex]->Data;
			struct ResourceVariation* Variation = &SectorVariations[Resource];


			uint32 Consumption = Sector->GetPeople()->GetRessourceConsumption(Resource);

			Variation->OwnedFlow = OwnedCustomerRatio * Consumption;
			Variation->FactoryFlow = NotOwnedCustomerRatio * Consumption;
		}
	}

	return SectorVariations;
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


