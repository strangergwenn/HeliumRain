

#include "../Flare.h"
#include "FlareTravel.h"
#include "FlareWorld.h"
#include "FlareGame.h"
#include "FlareGameTools.h"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareTravel::UFlareTravel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

static int64 DebugTravelDuration;

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

	DebugTravelDuration = 0;
	while(DebugTravelDuration == 0)
	{
		DebugTravelDuration = (FMath::FRand() > 0.16666 ? 0 : FMath::RandRange(1,50)) * UFlareGameTools::YEAR_IN_SECONDS +
							  (FMath::FRand() > 0.2 ? 0: FMath::RandRange(1,365)) * UFlareGameTools::DAY_IN_SECONDS +
							  (FMath::FRand() > 0.25 ? 0: FMath::RandRange(1,24)) * UFlareGameTools::HOUR_IN_SECONDS +
							  (FMath::FRand() > 0.333 ? 0 : FMath::RandRange(1,60)) * UFlareGameTools::MINUTE_IN_SECONDS +
							  (FMath::FRand() > 0.5 ? 0 : FMath::RandRange(1,60));
	}
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
	long TravelDuration = DebugTravelDuration;

	return TravelDuration - GetElapsedTime();
}

void UFlareTravel::ChangeDestination(UFlareSimulatedSector* NewDestinationSector)
{
	DestinationSector = NewDestinationSector;

	TravelData.DestinationSectorIdentifier = DestinationSector->GetIdentifier();
	// Reset travel duration
	// TODO intelligent travel remaining duration change
	TravelData.DepartureTime = Game->GetGameWorld()->GetTime();
}
