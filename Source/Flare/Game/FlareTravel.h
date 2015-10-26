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

	int64 GetRemainingTravelDuration();

	void ChangeDestination(UFlareSimulatedSector* NewDestinationSector);

	bool CanChangeDestination();

	void GenerateTravelDuration();

	FFlareSectorOrbitParameters ComputeCurrentTravelLocation();

	double ComputeSphereOfInfluenceAltitude(FFlareCelestialBody* CelestialBody);

	int64 ComputePhaseTravelDuration(FFlareCelestialBody* CelestialBody, double Altitude, double OriginPhase, double DestinationPhase);

	int64 ComputeAltitudeTravelDuration(FFlareCelestialBody* OriginCelestialBody, double OriginAltitude, FFlareCelestialBody* DestinationCelestialBody, double DestinationAltitude);

	double ComputeAltitudeTravelDistance(double OriginAltitude, double DestinationAltitude);

	double ComputeAltitudeTravelToSoiDistance(FFlareCelestialBody* CelestialBody, double Altitude);

	double ComputeAltitudeTravelToMoonDistance(FFlareCelestialBody* ParentCelestialBody, double Altitude, FFlareCelestialBody*MoonCelestialBody);

	double ComputeAltitudeTravelMoonToMoonDistance(FFlareCelestialBody* OriginCelestialBody, FFlareCelestialBody* DestinationCelestialBody);

	FFlareSectorOrbitParameters ComputeAltitudeTravelLocation(FFlareCelestialBody* CelestialBody, double OriginAltitude, double DestinationAltitude, int64 ElapsedTime);

protected:

	void EndTravel();


	TArray<UFlareSimulatedSpacecraft*>      TravelShips;

	UFlareFleet*                            Fleet;
	UFlareSimulatedSector*                  DestinationSector;


	FFlareTravelSave                        TravelData;
	AFlareGame*                             Game;
	int64                                   TravelDuration;

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
