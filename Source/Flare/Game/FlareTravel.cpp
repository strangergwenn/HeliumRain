

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

}
