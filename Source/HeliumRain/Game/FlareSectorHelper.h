#pragma once

#include "FlareWorldHelper.h"
#include "../Economy/FlareResource.h"
#include "../Game/FlareTradeRoute.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"

struct SectorHelper
{
	struct FlareTradeRequest
	{
		FFlareResourceDescription* Resource;
		EFlareTradeRouteOperation::Type Operation;
		int32 MaxQuantity;
		float CargoLimit;
		UFlareSimulatedSpacecraft *Client;
	};

	static UFlareSimulatedSpacecraft*  FindTradeStation(FlareTradeRequest Request);

	static int32 Trade(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource, int32 MaxQuantity, int64* TransactionPrice = NULL, UFlareTradeRoute* TradeRoute = nullptr);

	static void GetAvailableFleetSupplyCount(UFlareSimulatedSector* Sector, UFlareCompany* Company, int32& OwnedFS, int32& AvailableFS, int32& AffordableFS);

	static float GetComponentMaxRepairRatio(FFlareSpacecraftComponentDescription* ComponentDescription);

	static bool HasShipRefilling(UFlareSimulatedSector* TargetSector, UFlareCompany* Company);

	static bool HasShipRepairing(UFlareSimulatedSector* TargetSector, UFlareCompany* Company);

	static void GetRepairFleetSupplyNeeds(UFlareSimulatedSector* Sector, UFlareCompany* Company, int32& CurrentNeededFleetSupply, int32& TotalNeededFleetSupply, int64& MaxDuration);

	static void GetRefillFleetSupplyNeeds(UFlareSimulatedSector* Sector, UFlareCompany* Company, int32& CurrentNeededFleetSupply, int32& TotalNeededFleetSupply, int64& MaxDuration);

	static void GetRepairFleetSupplyNeeds(UFlareSimulatedSector* Sector, TArray<UFlareSimulatedSpacecraft*>& ships, int32& CurrentNeededFleetSupply, int32& TotalNeededFleetSupply, int64& MaxDuration);

	static void GetRefillFleetSupplyNeeds(UFlareSimulatedSector* Sector, TArray<UFlareSimulatedSpacecraft*>& ships, int32& CurrentNeededFleetSupply, int32& TotalNeededFleetSupply, int64& MaxDuration);


	static void RepairFleets(UFlareSimulatedSector* Sector, UFlareCompany* Company);

	static void RefillFleets(UFlareSimulatedSector* Sector, UFlareCompany* Company);

	static void ConsumeFleetSupply(UFlareSimulatedSector* Sector, UFlareCompany* Company, int32 ConsumedFS, bool ForRepair);

	static int32 GetArmyCombatPoints(UFlareSimulatedSector* Sector, bool ReduceByDamage);

	static int32 GetHostileArmyCombatPoints(UFlareSimulatedSector* Sector, UFlareCompany* Company, bool ReduceByDamage);

	static int32 GetCompanyArmyCombatPoints(UFlareSimulatedSector* Sector, UFlareCompany* Company, bool ReduceByDamage);

	static TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> ComputeSectorResourceStats(UFlareSimulatedSector* Sector);

	static int64 GetMeanResourcePrice(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resouce, bool worlFallBack, UFlareCompany const* Client);

	static int64 GetMeanWorldResourcePrice(UFlareWorld* World, FFlareResourceDescription* Resource, UFlareCompany const* Client);
};
