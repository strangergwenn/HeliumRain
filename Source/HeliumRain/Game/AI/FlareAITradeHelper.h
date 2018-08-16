#pragma once

#include "Object.h"
#include "../FlareGameTypes.h"

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

	void Consume(int UsedQuantity);
};


struct AITradeNeeds
{
	TArray<AITradeNeed> List;
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

struct AITradeSourcesByResourceSector
{
	TArray<AITradeSource*>& GetSourcePerCompany(UFlareCompany* Company);

	void ConsumeSource(AITradeSource*);

	void Add(AITradeSource* Source);

	TMap<UFlareCompany*, TArray<AITradeSource*>> SourcesPerCompany;
};


struct AITradeSourcesByResource
{
	AITradeSourcesByResourceSector& GetSourcesPerSector(UFlareSimulatedSector* Sector);

	void ConsumeSource(AITradeSource*);

	void Add(AITradeSource* Source);

	TMap<UFlareSimulatedSector*, AITradeSourcesByResourceSector> SourcesPerSector;
};


struct AITradeSources
{
	void ConsumeSource(AITradeSource*);

	void ConsumeSource(AITradeSource*, int32 Quantity);

	AITradeSourcesByResource& GetSourcesPerResource(FFlareResourceDescription* Resource);

	void Add(AITradeSource const& Source);

	TArray<AITradeSource> Sources;

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

struct AITradeIdleShipsBySector
{
	TArray<AIIdleShip*>& GetSourcePerCompany(UFlareCompany* Company);

	void ConsumeShip(AIIdleShip* Ship);

	void Add(AIIdleShip* Ship);

	TMap<UFlareCompany*, TArray<AIIdleShip*>> ShipsPerCompany;
};

struct AITradeIdleShips
{
	void ConsumeShip(AIIdleShip* Ship);

	AITradeIdleShipsBySector& GetShipsPerSector(UFlareSimulatedSector* Sector);

	TMap<UFlareSimulatedSector*, AITradeIdleShipsBySector> ShipsPerSector;

	void Add(AIIdleShip const& Ship);

	TArray<AIIdleShip> Ships;
};

struct AITradeHelper
{
	static TArray<UFlareSimulatedSpacecraft*> FindIdleCargos(UFlareCompany* Company);

	static SectorDeal FindBestDealForShipFromSector(UFlareSimulatedSpacecraft* Ship, UFlareSimulatedSector* SectorA, SectorDeal* DealToBeat, TMap<UFlareSimulatedSector*, SectorVariation> const& WorldResourceVariation);

	static SectorDeal FindBestDealForShip(UFlareSimulatedSpacecraft* Ship, TMap<UFlareSimulatedSector*, SectorVariation>& WorldResourceVariation);

	static void ApplyDeal(UFlareSimulatedSpacecraft* Ship, SectorDeal const&Deal, TMap<UFlareSimulatedSector*, SectorVariation>* WorldResourceVariation);

	/** Get the resource flow in this sector */
	static SectorVariation ComputeSectorResourceVariation(UFlareCompany* Company, UFlareSimulatedSector* Sector);

	static AITradeNeeds GenerateTradingNeeds(UFlareWorld* World);

	static AITradeSources GenerateTradingSources(UFlareWorld* World);

	static AITradeIdleShips GenerateIdleShips(UFlareWorld* World);

	static void ComputeGlobalTrading(UFlareWorld* World, AITradeNeeds& Needs, AITradeSources& Sources, AITradeIdleShips& IdleShips);

	static bool ProcessNeed(AITradeNeed& Need, AITradeSources& Sources, AITradeIdleShips& IdleShips);

	static AITradeSource* FindBestSource(AITradeSources& Sources, FFlareResourceDescription* Resource, UFlareSimulatedSector* Sector, UFlareCompany* Company, int32 NeededQuantity, size_t FunctionIndex);

	static AIIdleShip* FindBestShip(AITradeIdleShips& IdleShips, UFlareSimulatedSector* Sector, UFlareCompany* SourceCompany, UFlareCompany* NeedCompany, int32 NeedQuantity);
};
