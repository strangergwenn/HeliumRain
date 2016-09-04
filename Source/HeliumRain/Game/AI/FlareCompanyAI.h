#pragma once

#include "Object.h"
#include "../FlareGameTypes.h"
#include "FlareCompanyAI.generated.h"


class UFlareCompany;

/* Inter-sector trade deal */
struct SectorDeal
{
	float MoneyBalanceParDay;
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


protected:

	/*----------------------------------------------------
		Internal subsystems
	----------------------------------------------------*/

	/** Update diplomacy changes */
	void UpdateDiplomacy();

	/** Update trading for the company's fleet*/
	int32 UpdateTrading(TMap<UFlareSimulatedSector*, SectorVariation>& WorldResourceVariation);

	/** Manage the construction of stations */
	void UpdateStationConstruction(TMap<UFlareSimulatedSector*, SectorVariation>& WorldResourceVariation, int32 IdleCargoCapacity);

	/** Manage the construction of ships */
	void UpdateShipConstruction(TMap<UFlareSimulatedSector*, SectorVariation>& WorldResourceVariation);

	/** Buy ships at shipyards */
	void UpdateShipAcquisition(int32 IdleCargoCapacity);


	/*----------------------------------------------------
		Helpers
	----------------------------------------------------*/

	/** Get a list of idle cargos */
	TArray<UFlareSimulatedSpacecraft*> FindIdleCargos();

	/** Generate a score for ranking construction projects & the gains per day */
	TPair<float, float> ComputeConstructionScoreForStation(UFlareSimulatedSector* Sector, FFlareSpacecraftDescription* StationDescription, FFlareFactoryDescription* FactoryDescription) const;

	/** Get the resource flow in this sector */
	SectorVariation ComputeSectorResourceVariation(UFlareSimulatedSector* Sector);

	/** Print the resource flow */
	void DumpSectorResourceVariation(UFlareSimulatedSector* Sector, TMap<FFlareResourceDescription*, struct ResourceVariation>* Variation);

	SectorDeal FindBestDealForShipFromSector(UFlareSimulatedSpacecraft* Ship, UFlareSimulatedSector* SectorA, SectorDeal* DealToBeat, TMap<UFlareSimulatedSector*, SectorVariation> *WorldResourceVariation);
	
	TMap<FFlareResourceDescription*, int32> ComputeWorldResourceFlow() const;


protected:

	/*----------------------------------------------------
		Data
	----------------------------------------------------*/

	// Gameplay data
	UFlareCompany*			               Company;
	FFlareCompanyAISave					   AIData;
	AFlareGame*                            Game;
	
	// TODO Save it
	FFlareSpacecraftDescription*			 ConstructionProjectStation;
	UFlareSimulatedSector*         			 ConstructionProjectSector;
	TArray<UFlareSimulatedSpacecraft *>      ConstructionShips;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const
	{
		return Game;
	}

};

