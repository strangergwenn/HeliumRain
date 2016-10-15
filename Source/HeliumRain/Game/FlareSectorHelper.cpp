#include "../Flare.h"
#include "FlareSectorHelper.h"

#include "FlareCompany.h"
#include "../Game/FlareGame.h"
#include "../Economy/FlareCargoBay.h"
#include "../Player/FlarePlayerController.h"
#include "../Game/FlareScenarioTools.h"

#define MAX_REPAIR_RATIO_BY_DAY 0.1f
#define MAX_REFILL_RATIO_BY_DAY 0.4f

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

void SectorHelper::GetAvailableFleetSupplyCount(UFlareSimulatedSector* Sector, UFlareCompany* Company, int32& OwnedFS, int32& AvailableFS, int32& AffordableFS)
{
	OwnedFS = 0;
	int32 NotOwnedFS = 0;

	FFlareResourceDescription* FleetSupply = Sector->GetGame()->GetScenarioTools()->FleetSupply;

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorSpacecrafts().Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Sector->GetSectorSpacecrafts()[SpacecraftIndex];

		if (Company->GetWarState(Spacecraft->GetCompany()) == EFlareHostility::Hostile)
		{
			// At war, no trade possible
			continue;
		}

		int32 AvailableQuantity = Spacecraft->GetCargoBay()->GetResourceQuantity(FleetSupply, Company);

		if (Company == Spacecraft->GetCompany())
		{
			OwnedFS += AvailableQuantity;
		}
		else
		{
			NotOwnedFS += AvailableQuantity;
		}
	}

	AvailableFS = OwnedFS + NotOwnedFS;
	int32 ResourcePrice = Sector->GetResourcePrice(FleetSupply, EFlareResourcePriceContext::MaintenanceConsumption);

	int32 MaxAffordableQuantity = Company->GetMoney() / ResourcePrice;

	AffordableFS = OwnedFS + FMath::Min(MaxAffordableQuantity, NotOwnedFS);
	/*FLOGV("GetAvailableFleetSupplyCount for %s in %s: AvailableFS=%d AffordableFS=%d",
		  *Company->GetCompanyName().ToString(),
		  *Sector->GetSectorName().ToString(),
		  AvailableFS,
		  AffordableFS);*/
}

void SectorHelper::GetRepairFleetSupplyNeeds(UFlareSimulatedSector* Sector, UFlareCompany* Company, int32& CurrentNeededFleetSupply, int32& TotalNeededFleetSupply)
{
	float PreciseCurrentNeededFleetSupply = 0;
	float PreciseTotalNeededFleetSupply = 0;
	UFlareSpacecraftComponentsCatalog* Catalog = Company->GetGame()->GetShipPartsCatalog();

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorSpacecrafts().Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Sector->GetSectorSpacecrafts()[SpacecraftIndex];

		if (Company != Spacecraft->GetCompany()) {
			continue;
		}

		// List components
		for (int32 ComponentIndex = 0; ComponentIndex < Spacecraft->GetData().Components.Num(); ComponentIndex++)
		{
			FFlareSpacecraftComponentSave* ComponentData = &Spacecraft->GetData().Components[ComponentIndex];
			FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

			float DamageRatio = Spacecraft->GetDamageSystem()->GetDamageRatio(ComponentDescription, ComponentData);

			float CurrentRepairRatio = FMath::Min(MAX_REPAIR_RATIO_BY_DAY, (1.f - DamageRatio));
			float TotalRepairRatio = 1.f - DamageRatio;

			if(!Spacecraft->IsRepairing()) {
				PreciseCurrentNeededFleetSupply += CurrentRepairRatio * UFlareSimulatedSpacecraftDamageSystem::GetRepairCost(ComponentDescription);
			}
			PreciseTotalNeededFleetSupply += TotalRepairRatio * UFlareSimulatedSpacecraftDamageSystem::GetRepairCost(ComponentDescription);
		}
	}




	// Round to ceil
	CurrentNeededFleetSupply = FMath::CeilToInt(PreciseCurrentNeededFleetSupply);
	TotalNeededFleetSupply = FMath::CeilToInt(PreciseTotalNeededFleetSupply);

	/*FLOGV("GetRepairFleetSupplyNeeds for %s in %s: CurrentNeededFleetSupply=%f %d TotalNeededFleetSupply=%f %d",
		  *Company->GetCompanyName().ToString(),
		  *Sector->GetSectorName().ToString(),
		  PreciseCurrentNeededFleetSupply,
		  CurrentNeededFleetSupply,
			PreciseTotalNeededFleetSupply,
			TotalNeededFleetSupply);*/
}

void SectorHelper::GetRefillFleetSupplyNeeds(UFlareSimulatedSector* Sector, UFlareCompany* Company, int32& CurrentNeededFleetSupply, int32& TotalNeededFleetSupply)
{
	float PreciseCurrentNeededFleetSupply = 0;
	float PreciseTotalNeededFleetSupply = 0;
	UFlareSpacecraftComponentsCatalog* Catalog = Company->GetGame()->GetShipPartsCatalog();

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorSpacecrafts().Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Sector->GetSectorSpacecrafts()[SpacecraftIndex];

		if (Company != Spacecraft->GetCompany()) {
			continue;
		}

		// List components
		for (int32 ComponentIndex = 0; ComponentIndex < Spacecraft->GetData().Components.Num(); ComponentIndex++)
		{
			FFlareSpacecraftComponentSave* ComponentData = &Spacecraft->GetData().Components[ComponentIndex];
			FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

			if(ComponentDescription->Type == EFlarePartType::Weapon)
			{

				int32 MaxAmmo = ComponentDescription->WeaponCharacteristics.AmmoCapacity;
				int32 CurrentAmmo = MaxAmmo - ComponentData->Weapon.FiredAmmo;

				float FillRatio = (float) CurrentAmmo / (float) MaxAmmo;


				float CurrentRefillRatio = FMath::Min(MAX_REFILL_RATIO_BY_DAY, (1.f - FillRatio));
				float TotalRefillRatio = 1.f - FillRatio;

				if(!Spacecraft->IsRefilling()) {
					PreciseCurrentNeededFleetSupply += CurrentRefillRatio * UFlareSimulatedSpacecraftDamageSystem::GetRefillCost(ComponentDescription);
				}
				PreciseTotalNeededFleetSupply += TotalRefillRatio * (float) FMath::Max(1, ComponentDescription->WeaponCharacteristics.RefillCost);
			}
		}
	}

	// Round to ceil
	CurrentNeededFleetSupply = FMath::CeilToInt(PreciseCurrentNeededFleetSupply);
	TotalNeededFleetSupply = FMath::CeilToInt(PreciseTotalNeededFleetSupply);
}

void SectorHelper::RepairFleets(UFlareSimulatedSector* Sector, UFlareCompany* Company)
{
	int32 CurrentNeededFleetSupply;
	int32 TotalNeededFleetSupply;
	int32 OwnedFS;
	int32 AvailableFS;
	int32 AffordableFS;

	GetRepairFleetSupplyNeeds(Sector, Company, CurrentNeededFleetSupply, TotalNeededFleetSupply);
	GetAvailableFleetSupplyCount(Sector, Company, OwnedFS, AvailableFS, AffordableFS);

	if(Sector->IsInDangerousBattle(Company) || AffordableFS == 0 || CurrentNeededFleetSupply == 0)
	{
		// No repair possible
		return;
	}

	float MaxRepairRatio = MAX_REPAIR_RATIO_BY_DAY * FMath::Min(1.f,(float) AffordableFS /  (float) CurrentNeededFleetSupply);
	float RemainingFS = (float) AffordableFS;
	UFlareSpacecraftComponentsCatalog* Catalog = Company->GetGame()->GetShipPartsCatalog();


	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorSpacecrafts().Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Sector->GetSectorSpacecrafts()[SpacecraftIndex];

		if (Company != Spacecraft->GetCompany() || Spacecraft->IsRepairing()) {
			continue;
		}

		// List components
		for (int32 ComponentIndex = 0; ComponentIndex < Spacecraft->GetData().Components.Num(); ComponentIndex++)
		{
			FFlareSpacecraftComponentSave* ComponentData = &Spacecraft->GetData().Components[ComponentIndex];
			FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

			float ConsumedFS = Spacecraft->GetDamageSystem()->Repair(ComponentDescription,ComponentData, MaxRepairRatio, RemainingFS);

			RemainingFS -= ConsumedFS;

			if(RemainingFS == 0)
			{
				break;
			}
		}

		if(RemainingFS == 0)
		{
			break;
		}
	}

	int32 ConsumedFS = FMath::CeilToInt((float) AffordableFS - RemainingFS);

	FLOGV("Repair consumed %d FS", ConsumedFS);
	ConsumeFleetSupply(Sector, Company, ConsumedFS);
}

void SectorHelper::RefillFleets(UFlareSimulatedSector* Sector, UFlareCompany* Company)
{
	int32 CurrentNeededFleetSupply;
	int32 TotalNeededFleetSupply;
	int32 OwnedFS;
	int32 AvailableFS;
	int32 AffordableFS;

	GetRefillFleetSupplyNeeds(Sector, Company, CurrentNeededFleetSupply, TotalNeededFleetSupply);
	GetAvailableFleetSupplyCount(Sector, Company, OwnedFS, AvailableFS, AffordableFS);

	if(Sector->IsInDangerousBattle(Company) || AffordableFS == 0 || CurrentNeededFleetSupply == 0)
	{
		// No refill possible
		return;
	}

	float MaxRefillRatio = MAX_REFILL_RATIO_BY_DAY * FMath::Min(1.f,(float) AffordableFS /  (float) CurrentNeededFleetSupply);
	float RemainingFS = (float) AffordableFS;
	UFlareSpacecraftComponentsCatalog* Catalog = Company->GetGame()->GetShipPartsCatalog();


	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorSpacecrafts().Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Sector->GetSectorSpacecrafts()[SpacecraftIndex];

		if (Company != Spacecraft->GetCompany() || Spacecraft->IsRefilling()) {
			continue;
		}

		// List components
		for (int32 ComponentIndex = 0; ComponentIndex < Spacecraft->GetData().Components.Num(); ComponentIndex++)
		{
			FFlareSpacecraftComponentSave* ComponentData = &Spacecraft->GetData().Components[ComponentIndex];
			FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

			float ConsumedFS = Spacecraft->GetDamageSystem()->Refill(ComponentDescription,ComponentData, MaxRefillRatio, RemainingFS);

			RemainingFS -= ConsumedFS;

			if(RemainingFS == 0)
			{
				break;
			}
		}

		if(RemainingFS == 0)
		{
			break;
		}
	}

	int32 ConsumedFS = FMath::CeilToInt((float) AffordableFS - RemainingFS);

	FLOGV("Refill consumed %d FS", ConsumedFS);
	ConsumeFleetSupply(Sector, Company, ConsumedFS);
}

void SectorHelper::ConsumeFleetSupply(UFlareSimulatedSector* Sector, UFlareCompany* Company, int32 ConsumedFS)
{
	// First check for owned FS

	FFlareResourceDescription* FleetSupply = Sector->GetGame()->GetScenarioTools()->FleetSupply;

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorSpacecrafts().Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Sector->GetSectorSpacecrafts()[SpacecraftIndex];

		if (Company != Spacecraft->GetCompany())
		{
			continue;
		}

		int TakenQuantity = Spacecraft->GetCargoBay()->TakeResources(FleetSupply, ConsumedFS, Company);
		ConsumedFS -= TakenQuantity;

		if(ConsumedFS == 0)
		{
			return;
		}
	}

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorSpacecrafts().Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Sector->GetSectorSpacecrafts()[SpacecraftIndex];

		if (Company == Spacecraft->GetCompany())
		{
			continue;
		}

		if (Company->GetWarState(Spacecraft->GetCompany()) == EFlareHostility::Hostile)
		{
			// At war, no trade possible
			continue;
		}

		int TakenQuantity = Spacecraft->GetCargoBay()->TakeResources(FleetSupply, ConsumedFS, Company);

		if(TakenQuantity > 0)
		{
			int32 ResourcePrice = Sector->GetResourcePrice(FleetSupply, EFlareResourcePriceContext::MaintenanceConsumption);

			int64 Cost = TakenQuantity * ResourcePrice;
			Company->TakeMoney(Cost, true);
			Spacecraft->GetCompany()->GiveMoney(Cost);

			ConsumedFS -= TakenQuantity;

			if(ConsumedFS == 0)
			{
				return;
			}
		}
	}
}
