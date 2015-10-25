

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


void UFlareTravel::Simulate(long Duration)
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
	return Game->GetGameWorld()->GetTime() - TravelData.DepartureTime;
}

int64 UFlareTravel::GetRemainingTravelDuration()
{
	return TravelDuration - GetElapsedTime();
}

void UFlareTravel::ChangeDestination(UFlareSimulatedSector* NewDestinationSector)
{
	if(GetElapsedTime() > 0)
	{
		TravelData.CustomOriginIdentifier = ComputeCurrentTravelLocation();
		TravelData.OriginSectorIdentifier = NAME_None;
	}

	DestinationSector = NewDestinationSector;

	TravelData.DestinationSectorIdentifier = DestinationSector->GetIdentifier();

	// Reset travel duration
	// TODO intelligent travel remaining duration change
	TravelData.DepartureTime = Game->GetGameWorld()->GetTime();
	GenerateTravelDuration();
	Simulate(0);
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
	if(OriginSector)
	{
		OriginAltitude = OriginSector->GetOrbitParameters()->Altitude;
		OriginCelestialBodyIdentifier = OriginSector->GetOrbitParameters()->CelestialBodyIdentifier;
		OriginPhase = OriginSector->GetOrbitParameters()->Phase;
	}
	else
	{
		OriginAltitude = TravelData.CustomOriginIdentifier.Altitude;
		OriginCelestialBodyIdentifier = TravelData.CustomOriginIdentifier.CelestialBodyIdentifier;
		OriginPhase = TravelData.CustomOriginIdentifier.Phase;
	}

	DestinationAltitude = DestinationSector->GetOrbitParameters()->Altitude;
	DestinationCelestialBodyIdentifier = DestinationSector->GetOrbitParameters()->CelestialBodyIdentifier;
	DestinationPhase = DestinationSector->GetOrbitParameters()->Phase;

	if(OriginCelestialBodyIdentifier == DestinationCelestialBodyIdentifier && OriginAltitude == DestinationAltitude)
	{
		// Phase change travel
		FFlareCelestialBody* CelestialBody = Game->GetGameWorld()->GetPlanerarium()->FindCelestialBody(OriginCelestialBodyIdentifier);
		TravelDuration = ComputePhaseTravelDuration(CelestialBody, OriginAltitude, OriginPhase, DestinationPhase);
	}
	else
	{
		// Altitude change travel
		FFlareCelestialBody* OriginCelestialBody = Game->GetGameWorld()->GetPlanerarium()->FindCelestialBody(OriginCelestialBodyIdentifier);
		FFlareCelestialBody* DestinationCelestialBody = Game->GetGameWorld()->GetPlanerarium()->FindCelestialBody(DestinationCelestialBodyIdentifier);

		TravelDuration = ComputeAltitudeTravelDuration(OriginCelestialBody, OriginAltitude, DestinationCelestialBody, DestinationAltitude);
	}
}

FFlareSectorOrbitParameters UFlareTravel::ComputeCurrentTravelLocation()
{
	FFlareSectorOrbitParameters NewTravelOrigin;

	double OriginAltitude;
	double OriginPhase;
	FName OriginCelestialBodyIdentifier;
	double DestinationAltitude = DestinationSector->GetOrbitParameters()->Altitude;
	double DestinationPhase = DestinationSector->GetOrbitParameters()->Phase;
	FName DestinationCelestialBodyIdentifier = DestinationSector->GetOrbitParameters()->CelestialBodyIdentifier;

	UFlareSimulatedSector* OriginSector = Game->GetGameWorld()->FindSector(TravelData.OriginSectorIdentifier);
	if(OriginSector)
	{
		OriginAltitude = OriginSector->GetOrbitParameters()->Altitude;
		OriginCelestialBodyIdentifier = OriginSector->GetOrbitParameters()->CelestialBodyIdentifier;
		OriginPhase = OriginSector->GetOrbitParameters()->Phase;
	}
	else
	{
		OriginAltitude = TravelData.CustomOriginIdentifier.Altitude;
		OriginCelestialBodyIdentifier = TravelData.CustomOriginIdentifier.CelestialBodyIdentifier;
		OriginPhase = TravelData.CustomOriginIdentifier.Phase;
	}

	if (GetElapsedTime() == 0)
	{
		NewTravelOrigin.Phase = OriginPhase;
		NewTravelOrigin.Altitude = OriginAltitude;
		NewTravelOrigin.CelestialBodyIdentifier = OriginCelestialBodyIdentifier;
	}
	else if(OriginCelestialBodyIdentifier == DestinationCelestialBodyIdentifier && OriginAltitude == DestinationAltitude)
	{
		// Phase change travel
		float TravelRatio = (float) GetElapsedTime() / (float) TravelDuration;

		NewTravelOrigin.Phase = OriginPhase + ( FMath::UnwindDegrees(DestinationPhase) - FMath::UnwindDegrees(OriginPhase)) * TravelRatio;
		NewTravelOrigin.Altitude = OriginAltitude;
		NewTravelOrigin.CelestialBodyIdentifier = OriginCelestialBodyIdentifier;
	}
	else
	{
		// Altitude change travel
		FFlareCelestialBody* OriginCelestialBody = Game->GetGameWorld()->GetPlanerarium()->FindCelestialBody(OriginCelestialBodyIdentifier);
		FFlareCelestialBody* DestinationCelestialBody = Game->GetGameWorld()->GetPlanerarium()->FindCelestialBody(DestinationCelestialBodyIdentifier);

		if (OriginCelestialBody == DestinationCelestialBody)
		{
			return ComputeAltitudeTravelLocation(OriginCelestialBody, OriginAltitude, DestinationAltitude, GetElapsedTime());
		}
		else if (Game->GetGameWorld()->GetPlanerarium()->IsSatellite(DestinationCelestialBody, OriginCelestialBody))
		{
			// Planet to moon
			int64 FirstPartDuration = ComputeAltitudeTravelToMoonDistance(OriginCelestialBody, OriginAltitude, DestinationCelestialBody) * TRAVEL_DURATION_PER_ALTITUDE_KM;
			if (FirstPartDuration > GetElapsedTime())
			{
				double MoonAltitude = DestinationCelestialBody->OrbitDistance - OriginCelestialBody->Radius;
				return ComputeAltitudeTravelLocation(OriginCelestialBody, OriginAltitude, MoonAltitude, GetElapsedTime());
			}
			else
			{
				int64 RemainingDuration = GetElapsedTime() - FirstPartDuration;
				return ComputeAltitudeTravelLocation(DestinationCelestialBody, ComputeSphereOfInfluenceAltitude(DestinationCelestialBody), DestinationAltitude, RemainingDuration);
			}
		}
		else if (Game->GetGameWorld()->GetPlanerarium()->IsSatellite(OriginCelestialBody, DestinationCelestialBody))
		{
			// Moon to planet
			int64 FirstPartDuration = ComputeAltitudeTravelToSoiDistance(OriginCelestialBody, OriginAltitude) * TRAVEL_DURATION_PER_ALTITUDE_KM;
			if (FirstPartDuration > GetElapsedTime())
			{
				return ComputeAltitudeTravelLocation(OriginCelestialBody, OriginAltitude, ComputeSphereOfInfluenceAltitude(OriginCelestialBody), GetElapsedTime());
			}
			else
			{
				int64 RemainingDuration = GetElapsedTime() - FirstPartDuration;
				double MoonAltitude = OriginCelestialBody->OrbitDistance - DestinationCelestialBody->Radius;
				return ComputeAltitudeTravelLocation(DestinationCelestialBody, MoonAltitude, DestinationAltitude, RemainingDuration);
			}
		}
		else
		{
			int64 RemainingDuration = GetElapsedTime();
			int64 PartDuration = ComputeAltitudeTravelToSoiDistance(OriginCelestialBody, OriginAltitude) * TRAVEL_DURATION_PER_ALTITUDE_KM;
			if (PartDuration > RemainingDuration)
			{
				return ComputeAltitudeTravelLocation(OriginCelestialBody, OriginAltitude, ComputeSphereOfInfluenceAltitude(OriginCelestialBody), RemainingDuration);
			}

			RemainingDuration -= PartDuration;
			FFlareCelestialBody* ParentCelestialBody = Game->GetGameWorld()->GetPlanerarium()->FindParent(OriginCelestialBody);
			double Moon1Altitude = OriginCelestialBody->OrbitDistance - ParentCelestialBody->Radius;
			double Moon2Altitude = DestinationCelestialBody->OrbitDistance - ParentCelestialBody->Radius;

			PartDuration = ComputeAltitudeTravelDistance(Moon1Altitude, Moon2Altitude) * TRAVEL_DURATION_PER_ALTITUDE_KM;
			if (PartDuration > RemainingDuration)
			{
				return ComputeAltitudeTravelLocation(ParentCelestialBody, Moon1Altitude, Moon2Altitude, RemainingDuration);
			}

			RemainingDuration -= PartDuration;
			return ComputeAltitudeTravelLocation(DestinationCelestialBody, ComputeSphereOfInfluenceAltitude(DestinationCelestialBody), DestinationAltitude, RemainingDuration);
		}

	}
	return NewTravelOrigin;
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

FFlareSectorOrbitParameters UFlareTravel::ComputeAltitudeTravelLocation(FFlareCelestialBody* CelestialBody, double OriginAltitude, double DestinationAltitude, int64 ElapsedTime)
{
	double Altitude = ComputeAltitudeTravelDistance(OriginAltitude, DestinationAltitude);
	int64 Duration = TRAVEL_DURATION_PER_ALTITUDE_KM * Altitude;
	float TravelRatio = (float) ElapsedTime / (float) Duration;

	FFlareSectorOrbitParameters OrbitParameters;
	OrbitParameters.CelestialBodyIdentifier = CelestialBody->Identifier;
	OrbitParameters.Phase = 0;
	OrbitParameters.Altitude = OriginAltitude + (DestinationAltitude - OriginAltitude) * TravelRatio;

	return OrbitParameters;
}
