#pragma once

#include "Object.h"
#include "../FlareGameTypes.h"
#include "../FlareWorldHelper.h"
#include "FlareCompanyAI.generated.h"


class UFlareCompany;
class UFlareAIBehavior;

/* Inter-sector trade deal */
struct SectorDeal
{
	float Score;
	UFlareSimulatedSector* SectorA;
	UFlareSimulatedSector* SectorB;
	FFlareResourceDescription* Resource;
	int32 BuyQuantity;
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

	bool HighPriority;
};

/* Local list of resource flows */
struct SectorVariation
{
	int32 IncomingCapacity;
	TMap<FFlareResourceDescription*, ResourceVariation> ResourceVariations;
};

struct FFlareDiplomacyStats
{

};


struct DefenseSector
{
	UFlareSimulatedSector* Sector;
	UFlareSimulatedSector* TempBaseSector;
	int32 CombatPoints;
	int64 ArmyAntiSCombatPoints;
	int64 ArmyAntiLCombatPoints;
	int32 ArmyLargeShipCombatPoints;
	int32 ArmySmallShipCombatPoints;
	int32 LargeShipArmyCount;
	int32 SmallShipArmyCount;
	bool CapturingStation;

	bool operator==(const DefenseSector& lhs)
	{
		return lhs.Sector == Sector;
	}

	bool operator!=(const DefenseSector& lhs)
	{
		return !(*this == lhs);
	}
};


UCLASS()
class HELIUMRAIN_API UFlareCompanyAI : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public API
	----------------------------------------------------*/

	/** Load the company AI from a save file */
	virtual void Load(UFlareCompany* ParentCompany, const FFlareCompanyAISave& Data);

	/** Save the company AI to a save file */
	virtual FFlareCompanyAISave* Save();

	/** Real-time tick */
	virtual void Tick();

	/** Simulate a day */
	virtual void Simulate();

	/** Try to purchase research */
	virtual void PurchaseResearch();

	/** Destroy a spacecraft */
	virtual void DestroySpacecraft(UFlareSimulatedSpacecraft* Spacecraft);


	/*----------------------------------------------------
		Behavior API
	----------------------------------------------------*/

	void CargosEvasion();

	/** Update diplomacy changes */
	void UpdateDiplomacy();

	/** Update trading for the company's fleet*/
	void UpdateTrading();

	void UpdateBestScore(float Score,
						  UFlareSimulatedSector* Sector,
						  FFlareSpacecraftDescription* StationDescription,
						  UFlareSimulatedSpacecraft *Station,
						  float* BestScore,
						  FFlareSpacecraftDescription** BestStationDescription,
						  UFlareSimulatedSpacecraft** BestStation,
						  UFlareSimulatedSector** BestSector);

	void SpendBudget(EFlareBudget::Type Type, int64 Amount);

	void ModifyBudget(EFlareBudget::Type Type, int64 Amount);

	int64 GetBudget(EFlareBudget::Type Type);

	void ProcessBudget(TArray<EFlareBudget::Type> BudgetToProcess);

	void ProcessBudgetMilitary(int64 BudgetAmount, bool& Lock, bool& Idle);

	void ProcessBudgetTrade(int64 BudgetAmount, bool& Lock, bool& Idle);

	void ProcessBudgetStation(int64 BudgetAmount, bool Technology, bool& Lock, bool& Idle);

	/** Buy war ships */
	int64 UpdateWarShipAcquisition(bool limitToOne);

	/** Repair then refill all ships and stations */
	void RepairAndRefill();

	/*----------------------------------------------------
		Military AI
	----------------------------------------------------*/

	/** Move military ships */
	void UpdateMilitaryMovement();

	/** Move military ships while at war */
	void UpdateWarMilitaryMovement();

	/** Move military ships while in peace */
	void UpdatePeaceMilitaryMovement();

	UFlareSimulatedSector* FindNearestSectorWithPeace(UFlareSimulatedSector* OriginSector);

	UFlareSimulatedSector* FindNearestSectorWithFS(UFlareSimulatedSector* OriginSector);

	UFlareSimulatedSector* FindNearestSectorWithUpgradePossible(UFlareSimulatedSector* OriginSector);

	bool UpgradeShip(UFlareSimulatedSpacecraft* Ship, EFlarePartSize::Type WeaponTargetSize);

	bool UpgradeMilitaryFleet(WarTarget Target, DefenseSector& Sector, TArray<UFlareSimulatedSpacecraft*> &MovableShips);

	TArray<WarTargetIncomingFleet> GenerateWarTargetIncomingFleets(UFlareSimulatedSector* DestinationSector);

	TArray<WarTarget> GenerateWarTargetList();

	TArray<UFlareSimulatedSpacecraft*> GenerateWarShipList(UFlareSimulatedSector* Sector);

	TArray<DefenseSector> SortSectorsByDistance(UFlareSimulatedSector* BaseSector, TArray<DefenseSector> SectorsToSort);

	int64 GetDefenseSectorTravelDuration(TArray<DefenseSector>& DefenseSectorList, const DefenseSector& OriginSector);

	TArray<DefenseSector> GetDefenseSectorListInRange(TArray<DefenseSector>& DefenseSectorList, const DefenseSector& OriginSector, int64 MaxTravelDuration);

	TArray<DefenseSector> GenerateDefenseSectorList();

	void CheckBattleResolution();

	void CheckBattleState();

	bool HasHealthyTradeFleet() const;


protected:

	/*----------------------------------------------------
		Internal subsystems
	----------------------------------------------------*/
	
	/** Try to muster resources to build stations */
	void FindResourcesForStationConstruction();

	void ClearConstructionProject();
	
	/** Buy cargos ships */
	int64 UpdateCargoShipAcquisition();

	
	/*----------------------------------------------------
		Helpers
	----------------------------------------------------*/
	
	/** Order one ship at any shipyard */
	int64 OrderOneShip(const FFlareSpacecraftDescription* ShipDescription);

	const FFlareSpacecraftDescription* FindBestShipToBuild(bool Military);
	
	/** Return if a ship is currently build for the company */
	bool IsBuildingShip(bool Military);

	/** Get a list of shipyard */
	TArray<UFlareSimulatedSpacecraft*> FindShipyards();

	/** Get a list of wrecked cargos */
	TArray<UFlareSimulatedSpacecraft*> FindIncapacitatedCargos() const;
	
	/** Get a list of idle cargos */
	TArray<UFlareSimulatedSpacecraft*> FindIdleCargos() const;

	int32 GetDamagedCargosCapacity();

	int32 GetCargosCapacity();

	/** Get a list of idle military */
	TArray<UFlareSimulatedSpacecraft*> FindIdleMilitaryShips() const;

	float GetShipyardUsageRatio() const;

	/** Generate a score for ranking construction projects, version 2 */
	float ComputeConstructionScoreForStation(UFlareSimulatedSector* Sector, FFlareSpacecraftDescription* StationDescription, FFlareFactoryDescription* FactoryDescription, UFlareSimulatedSpacecraft* Station, bool Technology) const;

	float ComputeStationPrice(UFlareSimulatedSector* Sector, FFlareSpacecraftDescription* StationDescription, UFlareSimulatedSpacecraft* Station) const;

	/** Get the resource flow in this sector */
	SectorVariation ComputeSectorResourceVariation(UFlareSimulatedSector* Sector) const;

	/** Print the resource flow */
	void DumpSectorResourceVariation(UFlareSimulatedSector* Sector, TMap<FFlareResourceDescription*, struct ResourceVariation>* Variation) const;

	SectorDeal FindBestDealForShipFromSector(UFlareSimulatedSpacecraft* Ship, UFlareSimulatedSector* SectorA, SectorDeal* DealToBeat);

protected:

	/*----------------------------------------------------
		Data
	----------------------------------------------------*/

	// Gameplay data
	UFlareCompany*			               Company;
	FFlareCompanyAISave					   AIData;
	AFlareGame*                            Game;
	UPROPERTY()
	UFlareAIBehavior*                      Behavior;
	
	// Cache
	TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> WorldStats;
	TArray<UFlareSimulatedSpacecraft*>       Shipyards;
	TMap<UFlareSimulatedSector*, SectorVariation> WorldResourceVariation;

	TArray<UFlareSimulatedSector*>            SectorWithBattle;

	int32 IdleCargoCapacity;

public:

	TArray<EFlareBudget::Type> AllBudgets;

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const
	{
		return Game;
	}

	UFlareAIBehavior* GetBehavior()
	{
		return Behavior;
	}

	FFlareCompanyAISave* GetData()
	{
		return &AIData;
	}

};

