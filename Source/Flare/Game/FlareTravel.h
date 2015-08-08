#pragma once

#include "Object.h"
#include "FlareTravel.generated.h"


/** Travel save data */
USTRUCT()
struct FFlareTravelSave
{
	GENERATED_USTRUCT_BODY()

	/** Fleet identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName FleetIdentifier;

	/** Origin sector identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName OriginSectorIdentifier;

	/** Origin sector parameters (usefull for custom origin) */
	UPROPERTY(EditAnywhere, Category = Save)
	FFlareSectorOrbitParameters CustomOriginIdentifier;

	/** Destination sector identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName DestinationSectorIdentifier;

	UPROPERTY(EditAnywhere, Category = Save)
	int64 DepartureTime;

};


UCLASS()
class FLARE_API UFlareTravel : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
	  Save
	----------------------------------------------------*/

	/** Load the travel from a save file */
	virtual void Load(const FFlareTravelSave& Data);

	/** Save the travel to a save file */
	virtual FFlareTravelSave* Save();

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	void Simulate(long Duration);


protected:

	void EndTravel();


	TArray<UFlareSimulatedSpacecraft*>      TravelShips;

	UFlareFleet*      Fleet;
	UFlareSimulatedSector*      DestinationSector;


	FFlareTravelSave        TravelData;
	AFlareGame*                   Game;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const
	{
		return Game;
	}

	UFlareSimulatedSector* GetDestinationSector()
	{
		return DestinationSector;
	}

	UFlareFleet* GetFleet()
	{
		return Fleet;
	}

	int64 GetDepartureTime()
	{
		return TravelData.DepartureTime;
	}

	int64 GetElapsedTime();

	FFlareTravelSave* GetData()
	{
		return &TravelData;
	}

	inline TArray<UFlareSimulatedSpacecraft*>& GetShips()
	{
		return TravelShips;
	}

};
