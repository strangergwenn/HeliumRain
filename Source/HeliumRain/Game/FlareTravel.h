#pragma once

#include "Object.h"
#include "FlareSimulatedSector.h"
#include "FlareTravel.generated.h"


struct FFlareSectorDescription;
struct FFlareSectorOrbitParameters;
struct FFlareCelestialBody;

class UFlareWorld;
class UFlareSimulatedSector;


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
	int64 DepartureDate;

	UPROPERTY(EditAnywhere, Category = Save)
	FFlareSectorSave SectorData;
};


UCLASS()
class HELIUMRAIN_API UFlareTravel : public UObject
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

	void Simulate();

	int64 GetRemainingTravelDuration();

	void ChangeDestination(UFlareSimulatedSector* NewDestinationSector);

	bool CanChangeDestination();

	void GenerateTravelDuration();

	UFlareSimulatedSector* GetTravelSector()
	{
		return TravelSector;
	}

	FFlareSectorOrbitParameters ComputeCurrentTravelLocation();

	static int64 ComputeTravelDuration(UFlareWorld* World, UFlareSimulatedSector* OriginSector, UFlareSimulatedSector* DestinationSector, UFlareCompany* Company);

	static int64 ComputePhaseTravelDuration(UFlareWorld* World, FFlareCelestialBody* CelestialBody, double Altitude, double OriginPhase, double DestinationPhase);

	static int64 ComputeAltitudeTravelDuration(UFlareWorld* World, FFlareCelestialBody* OriginCelestialBody, double OriginAltitude, FFlareCelestialBody* DestinationCelestialBody, double DestinationAltitude);

	static double ComputeSphereOfInfluenceAltitude(UFlareWorld* World, FFlareCelestialBody* CelestialBody);

	static double ComputeAltitudeTravelDistance(UFlareWorld* World, double OriginAltitude, double DestinationAltitude);

	static double ComputeAltitudeTravelToSoiDistance(UFlareWorld* World, FFlareCelestialBody* CelestialBody, double Altitude);

	static double ComputeAltitudeTravelToMoonDistance(UFlareWorld* World, FFlareCelestialBody* ParentCelestialBody, double Altitude, FFlareCelestialBody*MoonCelestialBody);

	static double ComputeAltitudeTravelMoonToMoonDistance(UFlareWorld* World, FFlareCelestialBody* OriginCelestialBody, FFlareCelestialBody* DestinationCelestialBody);

	FFlareSectorOrbitParameters ComputeAltitudeTravelLocation(UFlareWorld* World, FFlareCelestialBody* CelestialBody, double OriginAltitude, double DestinationAltitude, int64 ElapsedTime);

	static void InitTravelSector(FFlareSectorSave& NewSectorData);

protected:

	void EndTravel();


	TArray<UFlareSimulatedSpacecraft*>      TravelShips;

	UFlareFleet*                            Fleet;
	UFlareSimulatedSector*                  DestinationSector;
	UFlareSimulatedSector*                  OriginSector;

	UPROPERTY()
	UFlareSimulatedSector*                  TravelSector;

	UPROPERTY()
	FFlareSectorDescription          SectorDescription;

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

	UFlareSimulatedSector* GetSourceSector()
	{
		return OriginSector;
	}

	UFlareSimulatedSector* GetDestinationSector()
	{
		return DestinationSector;
	}

	UFlareFleet* GetFleet()
	{
		return Fleet;
	}

	int64 GetDepartureDate()
	{
		return TravelData.DepartureDate;
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
