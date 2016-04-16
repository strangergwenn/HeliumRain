
#pragma once

#include "Object.h"
#include "FlareSectorInterface.h"
#include "FlareGameTypes.h"
#include "FlareTravel.h"
#include "Planetarium/FlareSimulatedPlanetarium.h"
#include "FlareWorld.generated.h"

class UFlareSector;
class UFlareFleet;
class UFlareFactory;
class UFlareCompany;

/** Hostility status */
UENUM()
namespace EFlareEventVisibility
{
	enum Type
	{
		Blocking,
		Warning,
		Silent
	};
}

/** World save data */
USTRUCT()
struct FFlareWorldSave
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Save)
	int64                    Date;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareCompanySave> CompanyData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareSectorSave> SectorData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareTravelSave> TravelData;
};


/** World event data */
USTRUCT()
struct FFlareWorldEvent
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Save)
	int64                    Date;

	UPROPERTY(EditAnywhere, Category = Save)
	TEnumAsByte<EFlareEventVisibility::Type>  Visibility;
};

UCLASS()
class HELIUMRAIN_API UFlareWorld: public UObject
{
    GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Save
	----------------------------------------------------*/

	/** Load the world from a save file */
	virtual void Load(const FFlareWorldSave& Data);

	/** Save the company to a save file */
	virtual FFlareWorldSave* Save(UFlareSector* ActiveSector);

	/** Spawn a company from save data */
	virtual UFlareCompany* LoadCompany(const FFlareCompanySave& CompanyData);

	/** Spawn a sector from save data */
	UFlareSimulatedSector* LoadSector(const FFlareSectorDescription* Description, const FFlareSectorSave& SectorData, const FFlareSectorOrbitParameters& OrbitParameters);

	UFlareTravel* LoadTravel(const FFlareTravelSave& TravelData);

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	/** Simulate world for a day */
	void Simulate();

	/** Simulate world from now to the next event */
	void FastForward();

	UFlareTravel* StartTravel(UFlareFleet* TravelingFleet, UFlareSimulatedSector* DestinationSector);

	virtual void DeleteTravel(UFlareTravel* Travel);

	/** Force new date */
	virtual void ForceDate(int64 Date);

	/** Generate all the next events in the world. If PointOfView is set, return the next event this company known */
	TArray<FFlareWorldEvent> GenerateEvents(UFlareCompany* PointOfView = NULL);

	/** Clear all factories associated to ParentSpacecraft */
	void ClearFactories(UFlareSimulatedSpacecraft *ParentSpacecraft);

	/** Add a factory to world */
	void AddFactory(UFlareFactory* Factory);

	// TODO Check docking capabilities
	/** Transfert resource from one spacecraft to another spacecraft */
	bool TransfertResources(IFlareSpacecraftInterface* SourceSpacecraft, IFlareSpacecraftInterface* DestinationSpacecraft, FFlareResourceDescription* Resource, uint32 Quantity);

	void SimulatePriceHomogenization();

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Gameplay data
	FFlareWorldSave                       WorldData;

	/** Sectors */
	UPROPERTY()
	TArray<UFlareSimulatedSector*>                 Sectors;

	/** Companies */
	UPROPERTY()
	TArray<UFlareCompany*>                Companies;

	/** Factories */
	UPROPERTY()
	TArray<UFlareFactory*>                Factories;

	UPROPERTY()
	TArray<UFlareTravel*>                Travels;

	UPROPERTY()
	UFlareSimulatedPlanetarium*			Planetarium;

	AFlareGame*                             Game;

	bool WorldMoneyReferenceInit;

public:
	uint64 WorldMoneyReference;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const
	{
		return Game;
	}

	inline UFlareSimulatedPlanetarium* GetPlanerarium()
	{
		return Planetarium;
	}

	inline TArray<UFlareSimulatedSector*>& GetSectors()
	{
		return Sectors;
	}

	inline TArray<UFlareTravel*>& GetTravels()
	{
		return Travels;
	}

	inline int64 GetDate()
	{
		return WorldData.Date;
	}

	UFlareCompany* FindCompany(FName Identifier) const;

	UFlareCompany* FindCompanyByShortName(FName CompanyShortName) const;

	UFlareSimulatedSector* FindSector(FName Identifier) const;

	UFlareSimulatedSector* FindSectorBySpacecraft(FName SpacecraftIdentifier) const;

	UFlareFleet* FindFleet(FName Identifier) const;

	UFlareTradeRoute* FindTradeRoute(FName Identifier) const;

	UFlareSimulatedSpacecraft* FindSpacecraft(FName ShipImmatriculation);

	inline TArray<UFlareCompany*> GetCompanies() const
	{
		return Companies;
	}

	uint64 GetWorldMoney();

};
