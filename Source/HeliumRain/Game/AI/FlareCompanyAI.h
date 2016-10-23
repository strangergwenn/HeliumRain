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

	int32 MinCapacity;
};

/* Local list of resource flows */
struct SectorVariation
{
	int32 IncomingCapacity;
	TMap<FFlareResourceDescription*, ResourceVariation> ResourceVariations;
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

	/** Destroy a spacecraft */
	virtual void DestroySpacecraft(UFlareSimulatedSpacecraft* Spacecraft);


/*----------------------------------------------------
	Behavior API
----------------------------------------------------*/

	/** Update diplomacy changes */
	void UpdateDiplomacy();

	/** Update trading for the company's fleet*/
	int32 UpdateTrading();

	/** Manage the construction of stations */
	void UpdateStationConstruction(int32& IdleCargoCapacity);

	/** Buy / build ships at shipyards */
	void UpdateShipAcquisition(int32& IdleCargoCapacity);

	/** Repair then refill all ships and stations */
	void RepairAndRefill();

	/** Buy / build ships at shipyards */
	void UpdateMilitaryMovement(bool DefendOnly);

	/** Buy war ships */
	void UpdateWarShipAcquisition();

protected:

	/*----------------------------------------------------
		Internal subsystems
	----------------------------------------------------*/



	/** Try to muster resources to build stations */
	void FindResourcesForStationConstruction();


	/** Buy cargos ships */
	void UpdateCargoShipAcquisition();



	/*----------------------------------------------------
		Helpers
	----------------------------------------------------*/

	/** Order one ship at any shipyard */
	bool OrderOneShip(FFlareSpacecraftDescription* ShipDescription);

	FFlareSpacecraftDescription* FindBestShipToBuild(bool Military);


	/** Return if a ship is currently build for the company */
	bool IsBuildingShip(bool Military);

	/** Get a list of shipyard */
	TArray<UFlareSimulatedSpacecraft*> FindShipyards();

	/** Get a list of idle cargos */
	TArray<UFlareSimulatedSpacecraft*> FindIdleCargos() const;

	int32 GetDamagedCargosCapacity();

	/** Get a list of idle military */
	TArray<UFlareSimulatedSpacecraft*> FindIdleMilitaryShips() const;

	/** Generate a score for ranking construction projects, version 2 */
	float ComputeConstructionScoreForStation(UFlareSimulatedSector* Sector, FFlareSpacecraftDescription* StationDescription, FFlareFactoryDescription* FactoryDescription, UFlareSimulatedSpacecraft* Station) const;

	float ComputeStationPrice(UFlareSimulatedSector* Sector, FFlareSpacecraftDescription* StationDescription, UFlareSimulatedSpacecraft* Station) const;

	/** Get the resource flow in this sector */
	SectorVariation ComputeSectorResourceVariation(UFlareSimulatedSector* Sector) const;

	/** Print the resource flow */
	void DumpSectorResourceVariation(UFlareSimulatedSector* Sector, TMap<FFlareResourceDescription*, struct ResourceVariation>* Variation) const;

	SectorDeal FindBestDealForShipFromSector(UFlareSimulatedSpacecraft* Ship, UFlareSimulatedSector* SectorA, SectorDeal* DealToBeat);
	
	TMap<FFlareResourceDescription*, int32> ComputeWorldResourceFlow() const;


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
	
	// Construction project
	FFlareSpacecraftDescription*			 ConstructionProjectStationDescription;
	UFlareSimulatedSector*         			 ConstructionProjectSector;
	UFlareSimulatedSpacecraft *              ConstructionProjectStation;
	TArray<UFlareSimulatedSpacecraft *>      ConstructionShips;
	TArray<UFlareSimulatedSpacecraft *>      ConstructionStaticShips;
	int32                                    ConstructionProjectNeedCapacity;

	// Cache
	TMap<FFlareResourceDescription*, int32>  ResourceFlow;
	TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> WorldStats;
	TArray<UFlareSimulatedSpacecraft*>       Shipyards;
	TMap<UFlareSimulatedSector*, SectorVariation> WorldResourceVariation;
	TMap<FFlareResourceDescription *, int32> MissingResourcesQuantity;
	TMap<FFlareResourceDescription *, int32> MissingStaticResourcesQuantity;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const
	{
		return Game;
	}

};

