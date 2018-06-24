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
	FFlareResourceDescription* Resource;
	UFlareCompany* Company;
	UFlareSimulatedSector* Sector;
	UFlareSimulatedSpacecraft* Station;
	size_t SourceFunctionIndex;
};


struct AITradeNeeds
{
	TArray<AITradeNeed> List;
};


struct AITradeSource
{
	UFlareSimulatedSpacecraft* Ship;
	UFlareSimulatedSpacecraft* Station;
	UFlareCompany* Company;
	UFlareSimulatedSector* Sector;
	int32 Quantity;
};

struct AITradeSourcesByResourceSector
{
	TArray<AITradeSource*> GetSourcePerCompany(UFlareCompany* Company);
};


struct AITradeSourcesByResource
{
	AITradeSourcesByResourceSector& GetSourcesPerSector(UFlareSimulatedSector* Sector);
};


struct AITradeSources
{
	void ConsumeSource(AITradeSource*);

	void ConsumeSource(AITradeSource*, int32 Quantity);

	AITradeSourcesByResource& GetSourcesPerResource(FFlareResourceDescription* Resource);
};

struct AITradeIdleShips
{
	void ConsumeShip(UFlareSimulatedSpacecraft* Ship);
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

	static UFlareSimulatedSpacecraft* FindBestShip(AITradeIdleShips& IdleShips, UFlareSimulatedSector* Sector, UFlareCompany* SourceCompany, UFlareCompany* NeedCompany);
};
