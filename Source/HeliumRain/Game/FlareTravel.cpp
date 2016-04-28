

#include "../Flare.h"
#include "FlareTravel.h"
#include "FlareWorld.h"
#include "FlareGame.h"
#include "FlareGameTools.h"

static const double TRAVEL_DURATION_PER_PHASE_KM = 0.4;
static const double TRAVEL_DURATION_PER_ALTITUDE_KM = 6;

#define LOCTEXT_NAMESPACE "FlareTravelInfos"


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
	OriginSector = Game->GetGameWorld()->FindSector(TravelData.OriginSectorIdentifier);

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

	// TODO Money migration
	// TODO People migration

	// Price migration
	float ContaminationFactor = 0.01f;
	for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;

		float OriginPrice = OriginSector->GetPreciseResourcePrice(Resource);
		float DestinationPrice = DestinationSector->GetPreciseResourcePrice(Resource);

		float Mean = (OriginPrice + DestinationPrice) / 2.f;


		float NewOriginPrice = (OriginPrice * (1 - ContaminationFactor)) + (ContaminationFactor * Mean);
		float NewDestinationPrice = (DestinationPrice * (1 - ContaminationFactor)) + (ContaminationFactor * Mean);

		//FLOGV("Travel start from %s. %s price ajusted from %f to %f (Mean: %f)", *OriginSector->GetSectorName().ToString(), *Resource->Name.ToString(), OriginPrice, NewOriginPrice, Mean);
		//FLOGV("Travel end from %s. %s price ajusted from %f to %f (Mean: %f)", *DestinationSector->GetSectorName().ToString(), *Resource->Name.ToString(), DestinationPrice, NewDestinationPrice, Mean);

		OriginSector->SetPreciseResourcePrice(Resource, NewOriginPrice);
		DestinationSector->SetPreciseResourcePrice(Resource, NewDestinationPrice);

	}


	Game->GetGameWorld()->DeleteTravel(this);

	if(Fleet->GetFleetCompany() == Game->GetPC()->GetCompany() && Fleet->GetCurrentTradeRoute() == NULL)
	{
		 Game->GetPC()->Notify(LOCTEXT("TravelEnded", "Travel ended"),
			FText::Format(LOCTEXT("TravelEndedFormat", "{0} just arrived at {1}"),
			Fleet->GetFleetName(),
			DestinationSector->GetSectorName()),
			FName("travel-end"),
			EFlareNotification::NT_Economy);
	}
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
	TravelDuration = ComputeTravelDuration(Game->GetGameWorld(), OriginSector, DestinationSector);
}

int64 UFlareTravel::ComputeTravelDuration(UFlareWorld* World, UFlareSimulatedSector* OriginSector, UFlareSimulatedSector* DestinationSector)
{
	int64 TravelDuration = 0;

	if (OriginSector == DestinationSector)
	{
		return 0;
	}

	double OriginAltitude;
	double DestinationAltitude;
	double OriginPhase;
	double DestinationPhase;
	FName OriginCelestialBodyIdentifier;
	FName DestinationCelestialBodyIdentifier;


	OriginAltitude = OriginSector->GetOrbitParameters()->Altitude;
	OriginCelestialBodyIdentifier = OriginSector->GetOrbitParameters()->CelestialBodyIdentifier;
	OriginPhase = OriginSector->GetOrbitParameters()->Phase;

	DestinationAltitude = DestinationSector->GetOrbitParameters()->Altitude;
	DestinationCelestialBodyIdentifier = DestinationSector->GetOrbitParameters()->CelestialBodyIdentifier;
	DestinationPhase = DestinationSector->GetOrbitParameters()->Phase;

	if (OriginCelestialBodyIdentifier == DestinationCelestialBodyIdentifier && OriginAltitude == DestinationAltitude)
	{
		// Phase change travel
		FFlareCelestialBody* CelestialBody = World->GetPlanerarium()->FindCelestialBody(OriginCelestialBodyIdentifier);
		TravelDuration = ComputePhaseTravelDuration(World, CelestialBody, OriginAltitude, OriginPhase, DestinationPhase) / UFlareGameTools::SECONDS_IN_DAY;
	}
	else
	{
		// Altitude change travel
		FFlareCelestialBody* OriginCelestialBody = World->GetPlanerarium()->FindCelestialBody(OriginCelestialBodyIdentifier);
		FFlareCelestialBody* DestinationCelestialBody = World->GetPlanerarium()->FindCelestialBody(DestinationCelestialBodyIdentifier);

		TravelDuration = ComputeAltitudeTravelDuration(World, OriginCelestialBody, OriginAltitude, DestinationCelestialBody, DestinationAltitude) / UFlareGameTools::SECONDS_IN_DAY;
	}

	return FMath::Max((int64) 1, TravelDuration);
}

double UFlareTravel::ComputeSphereOfInfluenceAltitude(UFlareWorld* World, FFlareCelestialBody* CelestialBody)
{
	FFlareCelestialBody* ParentCelestialBody = World->GetPlanerarium()->FindParent(CelestialBody);
	return CelestialBody->OrbitDistance * pow(CelestialBody->Mass / ParentCelestialBody->Mass, 0.4) - CelestialBody->Radius;
}


int64 UFlareTravel::ComputePhaseTravelDuration(UFlareWorld* World, FFlareCelestialBody* CelestialBody, double Altitude, double OriginPhase, double DestinationPhase)
{
	double TravelPhase =  FMath::Abs(FMath::UnwindDegrees(DestinationPhase - OriginPhase));

	double OrbitRadius = CelestialBody->Radius + Altitude;
	double OrbitPerimeter = 2 * PI * OrbitRadius;
	double TravelDistance = OrbitPerimeter * TravelPhase / 360;

	return TRAVEL_DURATION_PER_PHASE_KM * TravelDistance;
}

int64 UFlareTravel::ComputeAltitudeTravelDuration(UFlareWorld* World, FFlareCelestialBody* OriginCelestialBody, double OriginAltitude, FFlareCelestialBody* DestinationCelestialBody, double DestinationAltitude)
{
	double TravelAltitude;

	if (OriginCelestialBody == DestinationCelestialBody)
	{
		TravelAltitude = ComputeAltitudeTravelDistance(World, OriginAltitude, DestinationAltitude);
	}
	else if (World->GetPlanerarium()->IsSatellite(DestinationCelestialBody, OriginCelestialBody))
	{
		// Planet to moon
		TravelAltitude = ComputeAltitudeTravelToMoonDistance(World, OriginCelestialBody, OriginAltitude, DestinationCelestialBody) +
			ComputeAltitudeTravelToSoiDistance(World, DestinationCelestialBody, DestinationAltitude);
	}
	else if (World->GetPlanerarium()->IsSatellite(OriginCelestialBody, DestinationCelestialBody))
	{
		// Moon to planet
		TravelAltitude = ComputeAltitudeTravelToSoiDistance(World, OriginCelestialBody, OriginAltitude) +
				ComputeAltitudeTravelToMoonDistance(World, DestinationCelestialBody, DestinationAltitude, OriginCelestialBody);
	}
	else
	{
		TravelAltitude = ComputeAltitudeTravelToSoiDistance(World, OriginCelestialBody, OriginAltitude) +
				ComputeAltitudeTravelMoonToMoonDistance(World, OriginCelestialBody, DestinationCelestialBody) +
				ComputeAltitudeTravelToSoiDistance(World, DestinationCelestialBody, DestinationAltitude);
	}

	return TRAVEL_DURATION_PER_ALTITUDE_KM * TravelAltitude;
}

double UFlareTravel::ComputeAltitudeTravelDistance(UFlareWorld* World, double OriginAltitude, double DestinationAltitude)
{
	// Altitude change in same celestial body
	return FMath::Abs(DestinationAltitude - OriginAltitude);
}

double UFlareTravel::ComputeAltitudeTravelToSoiDistance(UFlareWorld* World, FFlareCelestialBody* CelestialBody, double Altitude)
{
	double MoonSoiAltitude = ComputeSphereOfInfluenceAltitude(World, CelestialBody);
	// Travel from moon SOI to moon target altitude
	return FMath::Abs(Altitude - MoonSoiAltitude);
}

double UFlareTravel::ComputeAltitudeTravelToMoonDistance(UFlareWorld* World, FFlareCelestialBody* ParentCelestialBody, double Altitude, FFlareCelestialBody* MoonCelestialBody)
{
	double MoonAltitude = MoonCelestialBody->OrbitDistance - ParentCelestialBody->Radius;
	// Travel to moon altitude
	return FMath::Abs(MoonAltitude - Altitude);
}

double UFlareTravel::ComputeAltitudeTravelMoonToMoonDistance(UFlareWorld* World, FFlareCelestialBody* OriginCelestialBody, FFlareCelestialBody* DestinationCelestialBody)
{
	// Moon1 orbit  to moon2 orbit
	return FMath::Abs(DestinationCelestialBody->OrbitDistance - OriginCelestialBody->OrbitDistance);
}

#undef LOCTEXT_NAMESPACE
