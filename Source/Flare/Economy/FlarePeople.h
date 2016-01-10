
#pragma once
#include "FlarePeople.generated.h"


class UFlareSimulatedSector;

/** Company reputation save data */
USTRUCT()
struct FFlareCompanyReputationSave
{
	GENERATED_USTRUCT_BODY()

	/** Reputation owner */
	UPROPERTY(EditAnywhere, Category = Save)
	FName CompanyIdentifier;

	/** Reputation points */
	UPROPERTY(EditAnywhere, Category = Save)
	uint32 Reputation;
};

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

	/** Birth point. When reach birth threasold, someone die */
	UPROPERTY(EditAnywhere, Category = Save)
	uint32 BirthPoint;

	/** Death point. When reach death threasold, someone die */
	UPROPERTY(EditAnywhere, Category = Save)
	uint32 DeathPoint;

	/** Birth point */
	UPROPERTY(EditAnywhere, Category = Save)
	uint32 HappinessPoint;

	/** Reputation of each company. By default 10 point per inhabitant. */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareCompanyReputationSave> CompanyReputations;
};



UCLASS()
class FLARE_API UFlarePeople : public UObject
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

	void Simulate(int64 Duration);

	void GiveBirth(uint32 BirthCount);


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
};
