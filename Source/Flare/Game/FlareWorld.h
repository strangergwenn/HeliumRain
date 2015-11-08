
#pragma once

#include "Object.h"
#include "FlareSimulatedSector.h"
#include "FlareCompany.h"
#include "FlareTravel.h"
#include "Planetarium/FlareSimulatedPlanetarium.h"
#include "FlareWorld.generated.h"

class UFlareSector;
class UFlareFleet;


/** World save data */
USTRUCT()
struct FFlareWorldSave
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Save)
	int64                    Time;

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
	int64                    Time;
};

UCLASS()
class FLARE_API UFlareWorld: public UObject
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

	/** Simulate world during a specific duration */
	void Simulate(int64 Duration);

	/** Simulate world from now to the next event */
	void FastForward();

	UFlareTravel* StartTravel(UFlareFleet* TravelingFleet, UFlareSimulatedSector* DestinationSector);

	virtual void DeleteTravel(UFlareTravel* Travel);

	/** Force new time */
	virtual void ForceTime(int64 Time);

	/** Generate all the next events in the world. If PointOfView is set, return the next event this company known */
	TArray<FFlareWorldEvent> GenerateEvents(UFlareCompany* PointOfView = NULL);

	/** Clear all factories associated to ParentSpacecraft */
	void ClearFactories(UFlareSimulatedSpacecraft *ParentSpacecraft);

	/** Add a factory to world */
	void AddFactory(UFlareFactory* Factory);

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

	inline int64 GetTime()
	{
		return WorldData.Time;
	}

	inline UFlareCompany* FindCompany(FName Identifier) const
	{
		for (int i = 0; i < Companies.Num(); i++)
		{
			UFlareCompany* Company = Companies[i];
			if (Company->GetIdentifier() == Identifier)
			{
				return Company;
			}
		}
		return NULL;
	}

	inline UFlareCompany* FindCompanyByShortName(FName CompanyShortName) const
	{
		// Find company
		for (int i = 0; i < Companies.Num(); i++)
		{
			UFlareCompany* Company = Companies[i];
			if (Company->GetShortName() == CompanyShortName)
			{
				return Company;
			}
		}
		return NULL;
	}

	inline UFlareSimulatedSector* FindSector(FName Identifier) const
	{
		for (int i = 0; i < Sectors.Num(); i++)
		{
			UFlareSimulatedSector* Sector = Sectors[i];
			if (Sector->GetIdentifier() == Identifier)
			{
				return Sector;
			}
		}
		return NULL;
	}

	inline UFlareFleet* FindFleet(FName Identifier) const
	{
		for (int i = 0; i < Companies.Num(); i++)
		{
			UFlareCompany* Company = Companies[i];
			UFlareFleet* Fleet = Company->FindFleet(Identifier);
			if (Fleet)
			{
				return Fleet;
			}
		}
		return NULL;
	}

	UFlareSimulatedSpacecraft* FindSpacecraft(FName ShipImmatriculation)
	{
		for (int i = 0; i < Companies.Num(); i++)
		{
			UFlareCompany* Company = Companies[i];
			UFlareSimulatedSpacecraft* Spacecraft = Company->FindSpacecraft(ShipImmatriculation);
			if (Spacecraft)
			{
				return Spacecraft;
			}
		}
		return NULL;
	}

	inline TArray<UFlareCompany*> GetCompanies() const
	{
		return Companies;
	}

};
