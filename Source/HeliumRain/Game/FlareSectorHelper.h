#pragma once
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

	static int32 Trade(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource, int32 MaxQuantity);

	static void GetAvailableFleetSupplyCount(UFlareSimulatedSector* Sector, UFlareCompany* Company, int32& OwnedFS, int32& AvailableFS, int32& AffordableFS);

	static float GetComponentMaxRepairRatio(FFlareSpacecraftComponentDescription* ComponentDescription);

	static void GetRepairFleetSupplyNeeds(UFlareSimulatedSector* Sector, UFlareCompany* Company, int32& CurrentNeededFleetSupply, int32& TotalNeededFleetSupply);

	static void GetRefillFleetSupplyNeeds(UFlareSimulatedSector* Sector, UFlareCompany* Company, int32& CurrentNeededFleetSupply, int32& TotalNeededFleetSupply);


	static void RepairFleets(UFlareSimulatedSector* Sector, UFlareCompany* Company);

	static void RefillFleets(UFlareSimulatedSector* Sector, UFlareCompany* Company);

	static void ConsumeFleetSupply(UFlareSimulatedSector* Sector, UFlareCompany* Company, int32 ConsumedFS);

	static int32 GetArmyCombatPoints(UFlareSimulatedSector* Sector);

	static int32 GetHostileArmyCombatPoints(UFlareSimulatedSector* Sector, UFlareCompany* Company);

	static int32 GetCompanyArmyCombatPoints(UFlareSimulatedSector* Sector, UFlareCompany* Company);

};
