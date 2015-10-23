

#include "../Flare.h"
#include "FlareTravel.h"
#include "FlareWorld.h"
#include "FlareGame.h"

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
	long TravelDuration = 5;

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
