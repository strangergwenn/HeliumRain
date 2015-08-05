

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

	for(int ShipIndex = 0; ShipIndex < Fleet->GetShips().Num(); ShipIndex++)
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
	long TravelDuration = 3600;

	if((Game->GetGameWorld()->GetTime() - TravelData.DepartureTime) > TravelDuration)
	{
		EndTravel();
	}
}

void UFlareTravel::EndTravel()
{
	Fleet->SetCurrentSector(DestinationSector);
	DestinationSector->AddFleet(Fleet);

	Game->GetGameWorld()->DeleteTravel(this);
}
