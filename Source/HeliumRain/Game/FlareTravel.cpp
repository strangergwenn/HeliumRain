

#include "../Flare.h"
#include "FlareTravel.h"
#include "FlareWorld.h"
#include "FlareGame.h"
#include "FlareGameTools.h"
#include "../Player/FlarePlayerController.h"
#include "../UI/Components/FlareNotification.h"
#include "../Economy/FlareCargoBay.h"

static const double TRAVEL_DURATION_PER_PHASE_KM = 0.4;
static const double TRAVEL_DURATION_PER_ALTITUDE_KM = 1.5;

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


	FFlareSectorOrbitParameters OrbitParameters;
	OrbitParameters = *OriginSector->GetOrbitParameters();


	SectorDescription.Name = LOCTEXT("TravelSectorName", "Traveling ...");
	SectorDescription.Description = LOCTEXT("TravelSectorDescription", "Travel local sector");
	SectorDescription.Identifier=Fleet->GetIdentifier();
	SectorDescription.Phase = 0;
	SectorDescription.IsIcy = false;
	SectorDescription.IsGeostationary = false;
	SectorDescription.IsSolarPoor = false;
	SectorDescription.LevelName = NAME_None;
	SectorDescription.DebrisFieldInfo.DebrisCatalog = NULL;
	SectorDescription.LevelTrack = EFlareMusicTrack::Travel;
	SectorDescription.DebrisFieldInfo.DebrisFieldDensity = 0;
	SectorDescription.DebrisFieldInfo.MaxDebrisSize = 0;
	SectorDescription.DebrisFieldInfo.MinDebrisSize = 0;

	TravelSector = NewObject<UFlareSimulatedSector>(this, UFlareSimulatedSector::StaticClass());
	TravelSector->Load(&SectorDescription, Data.SectorData, OrbitParameters);

	UpdateTravelParameters();

	Fleet->SetCurrentSector(TravelSector);
	TravelSector->AddFleet(Fleet);

	NeedNotification = true;

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
	int64 RemainingTime = GetRemainingTravelDuration();

	UpdateTravelParameters();

	// Incoming player notification. This doesn't work for incoming enemies, as they don't travel in fleets.
	if (RemainingTime == 1)
	{
		UFlareCompany* PlayerCompany = Fleet->GetGame()->GetPC()->GetCompany();

		if (Fleet->GetFleetCompany() == PlayerCompany
			&& DestinationSector->GetSectorFriendlyness(PlayerCompany) >= EFlareSectorFriendlyness::Contested)
		{
			FFlareMenuParameterData Data;
			Data.Sector = DestinationSector;

			Game->GetPC()->Notify(LOCTEXT("TravelAttackingSoon", "Destination is defended"),
				FText::Format(LOCTEXT("TravelAttackingSoonFormat", "Your fleet is arriving at {0}, but this sector is defended. Prepare for battle."),
					DestinationSector->GetSectorName()),
				FName("travel-raid-soon"),
				EFlareNotification::NT_Military,
				false,
				EFlareMenu::MENU_Sector,
				Data);
		}
	}

	// Trael is done
	else if (RemainingTime <= 0)
	{
		EndTravel();
	}
}

void UFlareTravel::UpdateTravelParameters()
{


	float TravelRatio = GetElapsedTime() /  float(TravelDuration);

	FFlareSectorOrbitParameters OrbitParameters;
	OrbitParameters.Phase = FMath::Lerp(OriginSector->GetOrbitParameters()->Phase, DestinationSector->GetOrbitParameters()->Phase, TravelRatio);

	if (OriginSector->GetOrbitParameters()->CelestialBodyIdentifier == DestinationSector->GetOrbitParameters()->CelestialBodyIdentifier)
	{
		OrbitParameters.CelestialBodyIdentifier = OriginSector->GetOrbitParameters()->CelestialBodyIdentifier;

		OrbitParameters.Altitude = FMath::Lerp(OriginSector->GetOrbitParameters()->Altitude, DestinationSector->GetOrbitParameters()->Altitude, TravelRatio);
	}
	else
	{
		FFlareCelestialBody* OriginBody = GetGame()->GetGameWorld()->GetPlanerarium()->FindCelestialBody(OriginSector->GetOrbitParameters()->CelestialBodyIdentifier);
		FFlareCelestialBody* DestinationBody = GetGame()->GetGameWorld()->GetPlanerarium()->FindCelestialBody(DestinationSector->GetOrbitParameters()->CelestialBodyIdentifier);

		double OriginBaseAltitude;
		double OriginLocalAltitude;
		double DestinationBaseAltitude;
		double DestinationLocalAltitude;

		if(OriginBody->Sattelites.Contains(DestinationBody))
		{
			// Nema to moon

			OriginBaseAltitude = 0;
			OriginLocalAltitude = OriginSector->GetOrbitParameters()->Altitude;

			DestinationBaseAltitude = DestinationBody->OrbitDistance;
			DestinationLocalAltitude = DestinationSector->GetOrbitParameters()->Altitude;

		}
		else if (DestinationBody->Sattelites.Contains(OriginBody))
		{
			// Moon to Nema
			OriginBaseAltitude = OriginBody->OrbitDistance;
			OriginLocalAltitude = OriginSector->GetOrbitParameters()->Altitude;

			DestinationBaseAltitude = 0;
			DestinationLocalAltitude = DestinationSector->GetOrbitParameters()->Altitude;
		}
		else
		{
			// Moon to moon
			OriginBaseAltitude = OriginBody->OrbitDistance;
			OriginLocalAltitude = OriginSector->GetOrbitParameters()->Altitude;

			DestinationBaseAltitude = DestinationBody->OrbitDistance;
			DestinationLocalAltitude = DestinationSector->GetOrbitParameters()->Altitude;
		}


		double Sign = FMath::Sign(DestinationBaseAltitude - OriginBaseAltitude);

		double OriginStartAltitude = OriginBaseAltitude + Sign * OriginLocalAltitude;
		double DestinationStartAltitude = DestinationBaseAltitude - Sign * OriginLocalAltitude;

		double LimitAltitude = (OriginStartAltitude + DestinationStartAltitude)/2;


		if(TravelRatio < 0.5)
		{
			OrbitParameters.CelestialBodyIdentifier = OriginSector->GetOrbitParameters()->CelestialBodyIdentifier;

			double CurrentAltitude = FMath::Lerp(OriginStartAltitude, LimitAltitude, TravelRatio*2);
			OrbitParameters.Altitude = FMath::Abs(CurrentAltitude - OriginBaseAltitude);
		}
		else
		{
			OrbitParameters.CelestialBodyIdentifier = DestinationSector->GetOrbitParameters()->CelestialBodyIdentifier;

			double CurrentAltitude = FMath::Lerp(OriginStartAltitude, LimitAltitude, TravelRatio*2);
			OrbitParameters.Altitude = FMath::Abs(CurrentAltitude - DestinationBaseAltitude);
		}
	}

	TravelSector->SetSectorOrbitParameters(OrbitParameters);
}


void UFlareTravel::EndTravel()
{
	// Each company has a chance of discovering the source sector
	if (GetSourceSector() && DestinationSector != GetSourceSector())
	{
		float DiscoveryChance = 0.005;
		TArray<UFlareCompany*> SectorCompanies;
		UFlareSimulatedSector* Source = GetSourceSector();

		// List companies
		for (auto Spacecraft : DestinationSector->GetSectorSpacecrafts())
		{
			SectorCompanies.AddUnique(Spacecraft->GetCompany());
		}

		for (auto Company : SectorCompanies)
		{
			if (!Company->IsKnownSector(Source) && Company != Fleet->GetFleetCompany())
			{
				if (FMath::FRand() < DiscoveryChance)
				{
					if (Company == Game->GetPC()->GetCompany())
					{
						FLOGV("UFlareTravel::EndTravel : player discovered '%s'", *Source->GetSectorName().ToString());
						Game->GetPC()->DiscoverSector(Source, false, true);
					}
					else
					{
						FLOGV("UFlareTravel::EndTravel : '%s' discovered '%s'", *Company->GetCompanyName().ToString(), *Source->GetSectorName().ToString());
						Company->DiscoverSector(Source);
					}
				}
			}
		}
	}

	// Make fleet arrive
	Fleet->SetCurrentSector(DestinationSector);	
	DestinationSector->AddFleet(Fleet);

	// People migration
	OriginSector->GetPeople()->Migrate(DestinationSector, Fleet->GetShipCount());
	
	// Price migration
	for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;

		float ContaminationFactor = 0.00f;
		for (int ShipIndex = 0; ShipIndex < Fleet->GetShips().Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = Fleet->GetShips()[ShipIndex];
			if (Ship->GetCargoBay()->GetResourceQuantity(Resource, Ship->GetCompany()) > 0)
			{
				ContaminationFactor += Ship->GetCargoBay()->GetResourceQuantity(Resource, Ship->GetCompany()) / 1000.f;
				//TODO scale bay world stock/flow
			}
		}

		ContaminationFactor = FMath::Min(ContaminationFactor, 1.f);

		float OriginPrice = OriginSector->GetPreciseResourcePrice(Resource);
		float DestinationPrice = DestinationSector->GetPreciseResourcePrice(Resource);
		float Mean = (OriginPrice + DestinationPrice) / 2.f;
		
		float NewOriginPrice = (OriginPrice * (1 - ContaminationFactor)) + (ContaminationFactor * Mean);
		float NewDestinationPrice = (DestinationPrice * (1 - ContaminationFactor)) + (ContaminationFactor * Mean);

		//FLOGV("Travel start from %s. %s price ajusted from %f to %f (Mean: %f)", *OriginSector->GetSectorName().ToString(), *Resource->Name.ToString(), OriginPrice/100., NewOriginPrice/100., Mean/100.);
		//FLOGV("Travel end from %s. %s price ajusted from %f to %f (Mean: %f)", *DestinationSector->GetSectorName().ToString(), *Resource->Name.ToString(), DestinationPrice/100., NewDestinationPrice/100., Mean/100.);

		OriginSector->SetPreciseResourcePrice(Resource, NewOriginPrice);
		DestinationSector->SetPreciseResourcePrice(Resource, NewDestinationPrice);
	}
	
	// Notify travel ended
	if (Fleet->GetFleetCompany() == Game->GetPC()->GetCompany() && (Fleet->GetCurrentTradeRoute() == NULL || Fleet->GetCurrentTradeRoute()->IsPaused()))
	{
		FFlareMenuParameterData Data;
		Data.Sector = DestinationSector;
		Game->GetPC()->Notify(LOCTEXT("TravelEnded", "Travel ended"),
			FText::Format(LOCTEXT("TravelEndedFormat", "{0} arrived at {1}"),
				Fleet->GetFleetName(),
				DestinationSector->GetSectorName()),
			FName("travel-end"),
			EFlareNotification::NT_Info,
			false,
			EFlareMenu::MENU_Sector,
			Data);

		Game->GetQuestManager()->OnEvent(FFlareBundle().PutTag("travel-end").PutName("sector", DestinationSector->GetIdentifier()));
	}

	// Update game
	Game->GetGameWorld()->DeleteTravel(this);
	if (Game->GetQuestManager())
	{
		Game->GetQuestManager()->OnTravelEnded(Fleet);
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

	if(NewDestinationSector->IsTravelSector())
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
	TravelDuration = ComputeTravelDuration(Game->GetGameWorld(), OriginSector, DestinationSector, Fleet->GetFleetCompany());
}

int64 UFlareTravel::ComputeTravelDuration(UFlareWorld* World, UFlareSimulatedSector* OriginSector, UFlareSimulatedSector* DestinationSector, UFlareCompany* Company)
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

		TravelDuration = (UFlareGameTools::SECONDS_IN_DAY/2 + ComputeAltitudeTravelDuration(World, OriginCelestialBody, OriginAltitude, DestinationCelestialBody, DestinationAltitude)) / UFlareGameTools::SECONDS_IN_DAY;
	}

	if(Company && Company->IsTechnologyUnlocked("fast-travel"))
	{
		TravelDuration /= 2;
	}

	return FMath::Max((int64) 2, TravelDuration+1);
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
	return TRAVEL_DURATION_PER_ALTITUDE_KM * TravelAltitude + UFlareGameTools::SECONDS_IN_DAY;
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

void UFlareTravel::InitTravelSector(FFlareSectorSave& NewSectorData)
{
	// Init new travel sector
	NewSectorData.GivenName = FText();
	NewSectorData.Identifier = TEXT("Travel");
	NewSectorData.LocalTime = 0;
	NewSectorData.IsTravelSector = true;

	// Init population
	NewSectorData.PeopleData.Population = 0;
	NewSectorData.PeopleData.BirthPoint = 0;
	NewSectorData.PeopleData.DeathPoint = 0;
	NewSectorData.PeopleData.FoodStock = 0;
	NewSectorData.PeopleData.FuelStock = 0;
	NewSectorData.PeopleData.ToolStock = 0;
	NewSectorData.PeopleData.TechStock = 0;
	NewSectorData.PeopleData.FoodConsumption = 0;
	NewSectorData.PeopleData.FuelConsumption = 0;
	NewSectorData.PeopleData.ToolConsumption = 0;
	NewSectorData.PeopleData.TechConsumption = 0;
	NewSectorData.PeopleData.HappinessPoint = 0;
	NewSectorData.PeopleData.HungerPoint = 0;
	NewSectorData.PeopleData.Money = 0;
	NewSectorData.PeopleData.Dept = 0;
}

#undef LOCTEXT_NAMESPACE
