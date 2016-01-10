
#include "../Flare.h"
#include "../Game/FlareWorld.h"
#include "../Game/FlareGame.h"
#include "../Game/FlareSimulatedSector.h"
#include "FlarePeople.h"


#define LOCTEXT_NAMESPACE "FlarePeopleInfo"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlarePeople::UFlarePeople(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/*----------------------------------------------------
   Save
----------------------------------------------------*/


void UFlarePeople::Load(UFlareSimulatedSector* ParentSector, const FFlarePeopleSave& Data)
{
	Game = ParentSector->GetGame();

	PeopleData = Data;
	Parent = ParentSector;
}

FFlarePeopleSave* UFlarePeople::Save()
{
	return &PeopleData;
}

/*----------------------------------------------------
   Gameplay
----------------------------------------------------*/

static uint32 BIRTH_TRESHOLD = 7120;

void UFlarePeople::Simulate(int64 Duration)
{
	FLOGV("Simulate people for sector %s. Population=%u", *Parent->GetSectorName().ToString(), PeopleData.Population)

	// Births : 1 birth for 20 years per inhabitant = 1 birth per 7120 inhabitant days

	PeopleData.BirthPoint += PeopleData.Population;
	GiveBirth(PeopleData.BirthPoint / BIRTH_TRESHOLD);
	PeopleData.BirthPoint = PeopleData.BirthPoint % BIRTH_TRESHOLD;
}

void UFlarePeople::GiveBirth(uint32 BirthCount)
{
	// Increase population
	PeopleData.Population += BirthCount;

	// Money creation
	PeopleData.Money += BirthCount * 10000; // TODO game settings constant

	PeopleData.HappinessPoint += BirthCount * 100;
}

#undef LOCTEXT_NAMESPACE
