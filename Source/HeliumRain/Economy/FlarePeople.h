
#pragma once
#include "../Game/FlareGameTypes.h"
#include "FlarePeople.generated.h"

class AFlareGame;
class UFlareSimulatedSector;
class UFlareSimulatedSpacecraft;
struct FFlareResourceDescription;

/** Sector people save data */
USTRUCT()
struct FFlarePeopleSave
{
	GENERATED_USTRUCT_BODY()

	/** Inhabitant count */
	UPROPERTY(EditAnywhere, Category = Save)
	uint32 Population;

	/** Food stock count */
	UPROPERTY(EditAnywhere, Category = Save)
	uint32 FoodStock;

	/** Money */
	UPROPERTY(EditAnywhere, Category = Save)
	uint32 Money;

	/** Dept. 10% money gain will be use to cancel the dept */
	UPROPERTY(EditAnywhere, Category = Save)
	uint32 Dept;

	/** Birth point. When reach birth threasold, someone die */
	UPROPERTY(EditAnywhere, Category = Save)
	uint32 BirthPoint;

	/** Death point. When reach death threasold, someone die */
	UPROPERTY(EditAnywhere, Category = Save)
	uint32 DeathPoint;

	/** Hunger point. Kill people with time */
	UPROPERTY(EditAnywhere, Category = Save)
	uint32 HungerPoint;

	/** Birth point */
	UPROPERTY(EditAnywhere, Category = Save)
	uint32 HappinessPoint;

	/** Reputation of each company. By default 10 point per inhabitant. */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareCompanyReputationSave> CompanyReputations;

	/** Fuel stock count */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 FuelStock;

	/** Tools stock count */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 ToolStock;

	/** Tech stock count */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 TechStock;

	/** Food consumtion per inhabitant */
	UPROPERTY(EditAnywhere, Category = Save)
	float FoodConsumption;

	/** Fuel consumtion per inhabitant */
	UPROPERTY(EditAnywhere, Category = Save)
	float FuelConsumption;

	/** Tool consumtion per inhabitant */
	UPROPERTY(EditAnywhere, Category = Save)
	float ToolConsumption;

	/** Tech consumtion per inhabitant */
	UPROPERTY(EditAnywhere, Category = Save)
	float TechConsumption;
};



UCLASS()
class HELIUMRAIN_API UFlarePeople : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/*----------------------------------------------------
	   Save
	----------------------------------------------------*/

	/** Load the factory from a save file */
	virtual void Load(UFlareSimulatedSector* ParentSector, const FFlarePeopleSave& Data);

	/** Save the factory to a save file */
	virtual FFlarePeopleSave* Save();

	/*----------------------------------------------------
	   Gameplay
	----------------------------------------------------*/

	void Simulate();

	void SimulateResourcePurchase();

	uint32 BuyResourcesInSector(FFlareResourceDescription* Resource, uint32 Quantity);

	uint32 BuyInStationForCompany(FFlareResourceDescription* Resource, uint32 Quantity, UFlareCompany* Company, TArray<UFlareSimulatedSpacecraft*>& Stations);

	float GetRessourceConsumption(FFlareResourceDescription* Resource, bool WithStock);

	void GiveBirth(uint32 BirthCount);

	void KillPeople(uint32 KillCount);

	int32 GetBasePopulation();

	void Migrate(UFlareSimulatedSector* DestinationSector, int32 ShipCount);

	void IncreaseHappiness(float HappinessPoints);

	void DecreaseHappiness(uint32 SadnessPoints);

	void SetHappiness(float Happiness);

	void Pay(uint32 Amount);

	void TakeMoney(uint32 Amount);

	void ResetPeople();

	void PrintInfo();

	void CheckPopulationDisparition();

protected:

	/*----------------------------------------------------
	   Protected data
	----------------------------------------------------*/

	// Gameplay data
	FFlarePeopleSave                         PeopleData;

	AFlareGame*                              Game;
	UFlareSimulatedSector*   				 Parent;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline AFlareGame* GetGame() const
	{
		return Game;
	}

	inline UFlareSimulatedSector* GetParent()
	{
		return Parent;
	}

	inline int64 GetMoney()
	{
		return PeopleData.Money;
	}

	inline int64 GetDept()
	{
		return PeopleData.Dept;
	}

	inline uint32 GetPopulation()
	{
		return PeopleData.Population;
	}

	FFlarePeopleSave* GetData()
	{
		return &PeopleData;
	}


	float GetHappiness();

	float GetWealth();

	FFlareCompanyReputationSave* GetCompanyReputation(UFlareCompany* Company);
};
