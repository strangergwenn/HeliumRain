
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
	int32                    Time;

	UPROPERTY(EditAnywhere, Category = Save)
	float                    Time2;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareCompanySave> CompanyData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareSectorSave> SectorData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareTravelSave> TravelData;
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

	/** Force new time */
	virtual void ForceTime(int64 Time);

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	/** Simulate world during a specific duration */
	void Simulate(long Duration);

	UFlareTravel* StartTravel(UFlareFleet* TravelingFleet, UFlareSimulatedSector* DestinationSector);

	virtual void DeleteTravel(UFlareTravel* Travel);

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

	inline int64 GetTime()
	{
		return WorldData.Time;
	}

	inline UFlareCompany* FindCompany(FName Identifier) const
	{
		for(int i = 0; i < Companies.Num(); i++)
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
		FLOGV("FindCompanyByShortName %s",*CompanyShortName.ToString());
		// Find company
		for(int i = 0; i < Companies.Num(); i++)
		{
			UFlareCompany* Company = Companies[i];
			FLOGV("  %s",*Company->GetShortName().ToString());
			if (Company->GetShortName() == CompanyShortName)
			{
				FLOG("  OK");
				return Company;
			}
		}
		return NULL;
	}

	inline UFlareSimulatedSector* FindSector(FName Identifier) const
	{
		for(int i = 0; i < Sectors.Num(); i++)
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
		for(int i = 0; i < Companies.Num(); i++)
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
		for(int i = 0; i < Companies.Num(); i++)
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
