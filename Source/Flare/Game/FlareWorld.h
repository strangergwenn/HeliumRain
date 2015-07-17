
#pragma once

#include "Object.h"
#include "FlareSimulatedSector.h"
#include "FlareCompany.h"
#include "FlareWorld.generated.h"


/** World save data */
USTRUCT()
struct FFlareWorldSave
{
	GENERATED_USTRUCT_BODY()


	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareCompanySave> CompanyData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareSectorSave> SectorData;

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
	virtual FFlareWorldSave* Save();

	/** Spawn a company from save data */
	virtual UFlareCompany* LoadCompany(const FFlareCompanySave& CompanyData);

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

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
public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const
	{
		return Cast<AFlareGame>(GetOuter()->GetWorld()->GetAuthGameMode());
	}


	inline UFlareCompany* FindCompany(FName Identifier) const
	{
		for(int i = 0; i < Companies.Num(); i++)
		{
			UFlareCompany* Company = Companies[i];
			if (Company && Company->GetIdentifier() == Identifier)
			{
				return Company;
			}
		}
		return NULL;
	}

	inline UFlareCompany* FindCompanyByShortName(FName CompanyShortName) const
	{
		// Find company
		for(int i = 0; i < Companies.Num(); i++)
		{
			UFlareCompany* Company = Companies[i];
			if (Company && Company->GetShortName() == CompanyShortName)
			{
				return Company;
			}
		}
		return NULL;
	}

	inline TArray<UFlareCompany*> GetCompanies() const
	{
		return Companies;
	}

};
