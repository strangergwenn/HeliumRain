#pragma once

#include "Object.h"
#include "../FlareGameTypes.h"

#define DEBUG_NEW_AI_TRADING 0


class UFlareWorld;

/* Inter-sector trade deal */
struct SectorDeal
{
	float Score;
	UFlareSimulatedSector* SectorA;
	UFlareSimulatedSector* SectorB;
	FFlareResourceDescription* Resource;
	int32 BuyQuantity;
	UFlareSimulatedSpacecraft* BuyStation;
	UFlareSimulatedSpacecraft* SellStation;
};

/* Resource flow */
struct ResourceVariation
{
	int32 OwnedFlow;
	int32 FactoryFlow;

	int32 OwnedStock;
	int32 FactoryStock;
	int32 StorageStock;
	int32 IncomingResources;

	int32 OwnedCapacity;
	int32 FactoryCapacity;
	int32 StorageCapacity;
	int32 MaintenanceCapacity;

	int32 MinCapacity;
	int32 ConsumerMaxStock;
	int32 MaintenanceMaxStock;

	int32 HighPriority;
};

/* Local list of resource flows */
struct SectorVariation
{
	int32 IncomingCapacity;
	TMap<FFlareResourceDescription*, ResourceVariation> ResourceVariations;
};

struct AITradeNeed
{
	float Ratio;
	int32 Quantity;
	int32 TotalCapacity; // Used to update ratio
	FFlareResourceDescription* Resource;
	UFlareCompany* Company;
	UFlareSimulatedSector* Sector;
	UFlareSimulatedSpacecraft* Station;
	size_t SourceFunctionIndex;
	bool Maintenance;
	bool HighPriority;

	void Consume(int UsedQuantity);
};


struct AICompaniesMoney
{
	void ConsumeMoney(UFlareCompany* Company, int64 Amount);
	bool HasMoney(UFlareCompany* Company);

	TMap<UFlareCompany*, int64> CompaniesMoney;
};

struct AITradeNeeds
{
	TArray<AITradeNeed> List;

	void Print();
};


struct AITradeSource
{
	UFlareSimulatedSpacecraft* Ship;
	UFlareSimulatedSpacecraft* Station;
	FFlareResourceDescription* Resource;
	UFlareCompany* Company;
	UFlareSimulatedSector* Sector;
	int32 Quantity;
	bool Stranded;
	bool Traveling;
};

inline bool operator==(const AITradeSource& lhs, const AITradeSource& rhs){
	return lhs.Ship == rhs.Ship
			&& lhs.Station == rhs.Station
			&& lhs.Company == rhs.Company
			&& lhs.Sector == rhs.Sector;
}

struct AITradeSourcesByResourceLocation
{
	AITradeSourcesByResourceLocation(UFlareWorld* World);

	TArray<AITradeSource*>& GetSourcePerCompany(UFlareCompany* Company);

	TArray<AITradeSource*>& GetSources();

	void ConsumeSource(AITradeSource*);

	void Add(AITradeSource* Source);

	TMap<UFlareCompany*, TArray<AITradeSource*>> SourcesPerCompany;
	TArray<AITradeSource*> Sources;
};


struct AITradeSourcesByResource
{
	AITradeSourcesByResource(UFlareWorld* World);

	AITradeSourcesByResourceLocation& GetSourcesPerSector(UFlareSimulatedSector* Sector);

	AITradeSourcesByResourceLocation& GetSourcesPerMoon(FName Moon);

	TArray<AITradeSource*>& GetSourcePerCompany(UFlareCompany* Company);

	TArray<AITradeSource*>& GetSources();

	void ConsumeSource(AITradeSource*);

	void Add(AITradeSource* Source);

	TMap<UFlareSimulatedSector*, AITradeSourcesByResourceLocation> SourcesPerSector;
	TMap<FName, AITradeSourcesByResourceLocation> SourcesPerMoon;
	TMap<UFlareCompany*, TArray<AITradeSource*>> SourcesPerCompany;
	TArray<AITradeSource*> Sources;
};


struct AITradeSources
{
	AITradeSources(UFlareWorld* World);

	void ConsumeSource(AITradeSource*);

	void ConsumeSource(AITradeSource*, int32 Quantity);

	AITradeSourcesByResource& GetSourcesPerResource(FFlareResourceDescription* Resource);

	void Add(AITradeSource const& Source);
	void GenerateCache();


	TArray<AITradeSource> Sources;
#if DEBUG_NEW_AI_TRADING
	TArray<AITradeSource*> SourcesPtr;
	void Print();
#endif
	size_t SourceCount;



	TMap<FFlareResourceDescription*, AITradeSourcesByResource> SourcesPerResource;
};

struct AIIdleShip
{
	UFlareSimulatedSpacecraft* Ship;
	UFlareCompany* Company;
	UFlareSimulatedSector* Sector;
	int32 Capacity;
	bool Traveling;
	bool Stranded;

};

inline bool operator==(const AIIdleShip& lhs, const AIIdleShip& rhs){ return lhs.Ship == rhs.Ship;}

struct AITradeIdleShipsByLocation
{
	AITradeIdleShipsByLocation(UFlareWorld* World);

	TArray<AIIdleShip*>& GetShipsPerCompany(UFlareCompany* Company);

	TArray<AIIdleShip*>& GetShips();

	void ConsumeShip(AIIdleShip* Ship);

	void Add(AIIdleShip* Ship);

	TMap<UFlareCompany*, TArray<AIIdleShip*>> ShipsPerCompany;
	TArray<AIIdleShip*> Ships;
};

struct AITradeIdleShips
{
	AITradeIdleShips(UFlareWorld* World);

	void ConsumeShip(AIIdleShip* Ship);

	AITradeIdleShipsByLocation& GetShipsPerSector(UFlareSimulatedSector* Sector);

	AITradeIdleShipsByLocation& GetShipsPerMoon(FName Moon);

	TArray<AIIdleShip*>& GetShipsPerCompany(UFlareCompany* Company);

	TMap<UFlareSimulatedSector*, AITradeIdleShipsByLocation> ShipsPerSector;
	TMap<FName, AITradeIdleShipsByLocation> ShipsPerMoon;
	TMap<UFlareCompany*, TArray<AIIdleShip*>> ShipsPerCompany;

	void Add(AIIdleShip const& Ship);
	void GenerateCache();

	void Print();

	TArray<AIIdleShip*>& GetShips();

	TArray<AIIdleShip*> ShipsPtr;
	TArray<AIIdleShip> Ships;
};

struct AITradeHelper
{
	static void CompanyAutoTrade(UFlareCompany* Company);

	static void FleetAutoTrade(UFlareFleet* Fleet, TMap<UFlareSimulatedSector*, SectorVariation>& WorldResourceVariation);

	static TArray<UFlareSimulatedSpacecraft*> FindIdleCargos(UFlareCompany* Company);

	static SectorDeal FindBestDealForShipFromSector(UFlareSimulatedSpacecraft* Ship, UFlareSimulatedSector* SectorA, SectorDeal* DealToBeat, TMap<UFlareSimulatedSector*, SectorVariation> const& WorldResourceVariation, UFlareSimulatedSector* SectorBRestiction);

	static SectorDeal FindBestDealForShip(UFlareSimulatedSpacecraft* Ship, TMap<UFlareSimulatedSector*, SectorVariation>& WorldResourceVariation, UFlareSimulatedSector* SectorARestiction, UFlareSimulatedSector* SectorBRestiction);

	static UFlareSimulatedSpacecraft* FindBestMasterShip(int32& UsableShipCount,UFlareFleet* Fleet, TArray<UFlareSimulatedSpacecraft*>& ExcludeList);


	static void ApplyDeal(UFlareSimulatedSpacecraft* Ship, SectorDeal const&Deal, TMap<UFlareSimulatedSector*, SectorVariation>* WorldResourceVariation, bool AllowTravel = true);

	/** Get the resource flow in this sector */
	static SectorVariation ComputeSectorResourceVariation(UFlareCompany* Company, UFlareSimulatedSector* Sector);

	static void GenerateTradingNeeds(AITradeNeeds& Needs, AITradeNeeds& MaintenanceNeeds, UFlareWorld* World);

	static void GenerateTradingSources(AITradeSources& Sources, AITradeSources& MaintenanceSources, UFlareWorld* World);

	static void GenerateIdleShips(AITradeIdleShips& Ships, UFlareWorld* World);

	static void ComputeGlobalTrading(UFlareWorld* World, AITradeNeeds& Needs, AITradeSources& Sources, AITradeSources& MaintenanceSources, AITradeIdleShips& IdleShips, AICompaniesMoney& CompaniesMoney);

	static bool ProcessNeed(AITradeNeed& Need, AITradeSources& Sources, AITradeSources& MaintenanceSources, AITradeIdleShips& IdleShips, AICompaniesMoney& CompaniesMoney);

	static AITradeSource* FindBestSource(AITradeSources& Sources, FFlareResourceDescription* Resource, UFlareSimulatedSector* Sector, UFlareCompany* Company, int32 NeededQuantity, size_t FunctionIndex, AICompaniesMoney& CompaniesMoney);

	static AIIdleShip* FindBestShip(AITradeIdleShips& IdleShips, UFlareSimulatedSector* Sector, UFlareCompany* SourceCompany, UFlareCompany* NeedCompany, int32 NeedQuantity);
};
