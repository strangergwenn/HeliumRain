

#include "../Flare.h"
#include "FlareTravel.h"
#include "FlareWorld.h"
#include "FlareGame.h"
#include "FlareGameTools.h"

static const double TRAVEL_DURATION_PER_PHASE_KM = 0.4;
static const double TRAVEL_DURATION_PER_ALTITUDE_KM = 6;



/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareTravel::UFlareTravel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareTravel::Load(const FFlareTravelSave& Data)
{
	Game = Cast<UFlareWorld>(GetOuter())->GetGame();
	TravelData = Data;
	TravelShips.Empty();

	Fleet = Game->GetGameWorld()->FindFleet(TravelData.FleetIdentifier);
	DestinationSector = Game->GetGameWorld()->FindSector(TravelData.DestinationSectorIdentifier);

	for (int ShipIndex = 0; ShipIndex < Fleet->GetShips().Num(); ShipIndex++)
	{
		TravelShips.Add(Fleet->GetShips()[ShipIndex]);
	}

	Fleet->SetCurrentTravel(this);
	GenerateTravelDuration();
}

FFlareTravelSave* UFlareTravel::Save()
{
	return &TravelData;
}

/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/


void UFlareTravel::Simulate()
{
	if (GetRemainingTravelDuration() <= 0)
	{
		EndTravel();
	}
}

void UFlareTravel::EndTravel()
{
	Fleet->SetCurrentSector(DestinationSector);
	DestinationSector->AddFleet(Fleet);

	// Place correctly new ships to avoid collision

	Game->GetGameWorld()->DeleteTravel(this);
}

int64 UFlareTravel::GetElapsedTime()
{
	return Game->GetGameWorld()->GetDate() - TravelData.DepartureDate;
}

int64 UFlareTravel::GetRemainingTravelDuration()
{
	return TravelDuration - GetElapsedTime();
}

void UFlareTravel::ChangeDestination(UFlareSimulatedSector* NewDestinationSector)
{
	if (!CanChangeDestination())
	{
		return;
	}

	DestinationSector = NewDestinationSector;

	TravelData.DestinationSectorIdentifier = DestinationSector->GetIdentifier();

	// Reset travel duration
	// TODO intelligent travel remaining duration change
	TravelData.DepartureDate = Game->GetGameWorld()->GetDate();
	GenerateTravelDuration();
}

bool UFlareTravel::CanChangeDestination()
{
	// Travel not possible once started
	return GetElapsedTime() <= 0;
}

void UFlareTravel::GenerateTravelDuration()
{
	TravelDuration = 0;

	double OriginAltitude;
	double DestinationAltitude;
	double OriginPhase;
	double DestinationPhase;
	FName OriginCelestialBodyIdentifier;
	FName DestinationCelestialBodyIdentifier;

	UFlareSimulatedSector* OriginSector = Game->GetGameWorld()->FindSector(TravelData.OriginSectorIdentifier);

	OriginAltitude = OriginSector->GetOrbitParameters()->Altitude;
	OriginCelestialBodyIdentifier = OriginSector->GetOrbitParameters()->CelestialBodyIdentifier;
	OriginPhase = OriginSector->GetOrbitParameters()->Phase;

	DestinationAltitude = DestinationSector->GetOrbitParameters()->Altitude;
	DestinationCelestialBodyIdentifier = DestinationSector->GetOrbitParameters()->CelestialBodyIdentifier;
	DestinationPhase = DestinationSector->GetOrbitParameters()->Phase;

	if (OriginCelestialBodyIdentifier == DestinationCelestialBodyIdentifier && OriginAltitude == DestinationAltitude)
	{
		// Phase change travel
		FFlareCelestialBody* CelestialBody = Game->GetGameWorld()->GetPlanerarium()->FindCelestialBody(OriginCelestialBodyIdentifier);
		TravelDuration = ComputePhaseTravelDuration(CelestialBody, OriginAltitude, OriginPhase, DestinationPhase) / UFlareGameTools::SECONDS_IN_DAY;
	}
	else
	{
		// Altitude change travel
		FFlareCelestialBody* OriginCelestialBody = Game->GetGameWorld()->GetPlanerarium()->FindCelestialBody(OriginCelestialBodyIdentifier);
		FFlareCelestialBody* DestinationCelestialBody = Game->GetGameWorld()->GetPlanerarium()->FindCelestialBody(DestinationCelestialBodyIdentifier);

		TravelDuration = ComputeAltitudeTravelDuration(OriginCelestialBody, OriginAltitude, DestinationCelestialBody, DestinationAltitude) / UFlareGameTools::SECONDS_IN_DAY;
	}
}

double UFlareTravel::ComputeSphereOfInfluenceAltitude(FFlareCelestialBody* CelestialBody)
{
	FFlareCelestialBody* ParentCelestialBody = Game->GetGameWorld()->GetPlanerarium()->FindParent(CelestialBody);
	return CelestialBody->OrbitDistance * pow(CelestialBody->Mass / ParentCelestialBody->Mass, 0.4) - CelestialBody->Radius;
}


int64 UFlareTravel::ComputePhaseTravelDuration(FFlareCelestialBody* CelestialBody, double Altitude, double OriginPhase, double DestinationPhase)
{
	double TravelPhase =  FMath::Abs(FMath::UnwindDegrees(DestinationPhase - OriginPhase));

	double OrbitRadius = CelestialBody->Radius + Altitude;
	double OrbitPerimeter = 2 * PI * OrbitRadius;
	double TravelDistance = OrbitPerimeter * TravelPhase / 360;

	return TRAVEL_DURATION_PER_PHASE_KM * TravelDistance;
}

int64 UFlareTravel::ComputeAltitudeTravelDuration(FFlareCelestialBody* OriginCelestialBody, double OriginAltitude, FFlareCelestialBody* DestinationCelestialBody, double DestinationAltitude)
{
	double TravelAltitude;

	if (OriginCelestialBody == DestinationCelestialBody)
	{
		TravelAltitude = ComputeAltitudeTravelDistance(OriginAltitude, DestinationAltitude);
	}
	else if (Game->GetGameWorld()->GetPlanerarium()->IsSatellite(DestinationCelestialBody, OriginCelestialBody))
	{
		// Planet to moon
		TravelAltitude = ComputeAltitudeTravelToMoonDistance(OriginCelestialBody, OriginAltitude, DestinationCelestialBody) +
			ComputeAltitudeTravelToSoiDistance(DestinationCelestialBody, DestinationAltitude);
	}
	else if (Game->GetGameWorld()->GetPlanerarium()->IsSatellite(OriginCelestialBody, DestinationCelestialBody))
	{
		// Moon to planet
		TravelAltitude = ComputeAltitudeTravelToSoiDistance(OriginCelestialBody, OriginAltitude) +
				ComputeAltitudeTravelToMoonDistance(DestinationCelestialBody, DestinationAltitude, OriginCelestialBody);
	}
	else
	{
		TravelAltitude = ComputeAltitudeTravelToSoiDistance(OriginCelestialBody, OriginAltitude) +
				ComputeAltitudeTravelMoonToMoonDistance(OriginCelestialBody, DestinationCelestialBody) +
				ComputeAltitudeTravelToSoiDistance(DestinationCelestialBody, DestinationAltitude);
	}

	return TRAVEL_DURATION_PER_ALTITUDE_KM * TravelAltitude;
}

double UFlareTravel::ComputeAltitudeTravelDistance(double OriginAltitude, double DestinationAltitude)
{
	// Altitude change in same celestial body
	return FMath::Abs(DestinationAltitude - OriginAltitude);
}

double UFlareTravel::ComputeAltitudeTravelToSoiDistance(FFlareCelestialBody* CelestialBody, double Altitude)
{
	double MoonSoiAltitude = ComputeSphereOfInfluenceAltitude(CelestialBody);
	// Travel from moon SOI to moon target altitude
	return FMath::Abs(Altitude - MoonSoiAltitude);
}

double UFlareTravel::ComputeAltitudeTravelToMoonDistance(FFlareCelestialBody* ParentCelestialBody, double Altitude, FFlareCelestialBody* MoonCelestialBody)
{
	double MoonAltitude = MoonCelestialBody->OrbitDistance - ParentCelestialBody->Radius;
	// Travel to moon altitude
	return FMath::Abs(MoonAltitude - Altitude);
}

double UFlareTravel::ComputeAltitudeTravelMoonToMoonDistance(FFlareCelestialBody* OriginCelestialBody, FFlareCelestialBody* DestinationCelestialBody)
{
	// Moon1 orbit  to moon2 orbit
	return FMath::Abs(DestinationCelestialBody->OrbitDistance - OriginCelestialBody->OrbitDistance);
}

