
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

static uint32 BIRTH_POINT_TRESHOLD = 7120;
static uint32 DEATH_POINT_TRESHOLD = 29200;
static uint32 MONETARY_CREATION = 10000;

void UFlarePeople::Simulate(int64 Duration)
{
	FLOGV("Simulate people for sector %s. Population=%u", *Parent->GetSectorName().ToString(), PeopleData.Population)



	float Happiness = GetHappiness();

	// Death of old age : 1 death for 80 years per inhabitant = 1 death per 29200 inhabitant days
	// Sickness increase with hunger and sadness
	//	- 4 times normal sickness if happiness is 0
	//  - normal sickness if happiness is 1.5
	//  - half sickness if happiness 2
	//  Formula : sickness = 0.5 + (happiness -2)^2
	//
	//  Hunger is add to population death point
	//
	float Sickness = 0.5 + FMath::Square(Happiness - 2);
	PeopleData.DeathPoint += (PeopleData.Population + PeopleData.HungerPoint * 2) * Sickness;
	KillPeople(PeopleData.DeathPoint / DEATH_POINT_TRESHOLD);
	PeopleData.DeathPoint = PeopleData.DeathPoint % DEATH_POINT_TRESHOLD;

	// Births : 1 birth for 20 years per inhabitant = 1 birth per 7120 inhabitant days
	// Fertility increase with happiness :
	//	 - no fertility if hapinness is less of 50%
	//   - normal fertility at 100 % happiness
	//   - triple fertility at 200%
	//   Formula : fertility = 2 * happiness -1
	float Fertility = FMath::Max(2 * Happiness -1, 0.0f);

	PeopleData.BirthPoint += PeopleData.Population * Fertility;
	GiveBirth(PeopleData.BirthPoint / BIRTH_POINT_TRESHOLD);
	PeopleData.BirthPoint = PeopleData.BirthPoint % BIRTH_POINT_TRESHOLD;


	// Eat
	// Each inhabitant eat 1 kg of food a day as vital food.
	// TODO dynamic consumtion
	// If an inhabitant don't heat, happiness decrease heavily and  hunger is increase
	// If some inhabitant heat, the hunger deaseapear and some happiness is gain
	uint32 FoodConsumption = PeopleData.Population;
	uint32 EatenFood = FMath::Min(FoodConsumption, PeopleData.FoodStock);
	// Reduce stock
	PeopleData.FoodStock = EatenFood;

	// Reduce hunger (100% if everybody eat)
	float FeedPeopleRatio = (float) EatenFood / (float) FoodConsumption;
	if (PeopleData.HungerPoint > 0)
	{
		IncreaseHappiness(FeedPeopleRatio * PeopleData.Population * 2);
	}
	PeopleData.HungerPoint *= 1 - FeedPeopleRatio;


	// Add hunger (0 if everybody eat)
	uint32 Hunger = FoodConsumption - EatenFood;
	PeopleData.HungerPoint += Hunger + PeopleData.HungerPoint / 10;
	DecreaseHappiness(Hunger * 10);



	/*FLOGV(" - happiness: %f", Happiness);
	FLOGV(" - Sickness: %f", Sickness);
	FLOGV(" - Fertility: %f", Fertility);

	FLOGV(" - Food: %u", PeopleData.FoodStock);
	FLOGV(" - FoodConsumption: %u", FoodConsumption);
	FLOGV(" - EatenFood: %u", EatenFood);
	FLOGV(" - FeedPeopleRatio: %f", FeedPeopleRatio);
	FLOGV(" - Delta Hunger: %u", Hunger);
	FLOGV(" - Hunger: %u", PeopleData.HungerPoint);

	FLOGV(" - Money: %u", PeopleData.Money);
	FLOGV(" - Detp: %u", PeopleData.Dept);*/
}

void UFlarePeople::GiveBirth(uint32 BirthCount)
{
	if(BirthCount == 0)
	{
		return;
	}

	FLOGV("Give birth %u people for sector %s", BirthCount, *Parent->GetSectorName().ToString());

	// Increase population
	PeopleData.Population += BirthCount;

	// Money creation
	PeopleData.Money += BirthCount * MONETARY_CREATION;

	IncreaseHappiness(BirthCount * 100 * 2);
	PeopleData.HappinessPoint += BirthCount * 100 * 2; // Birth happiness bonus
}

void UFlarePeople::KillPeople(uint32 KillCount)
{

	uint32 PeopleToKill = FMath::Min(KillCount, PeopleData.Population);
	if(PeopleToKill == 0)
	{
		return;
	}

	FLOGV("Kill %u people for sector %s", KillCount, *Parent->GetSectorName().ToString());


	float KillRatio = (float) PeopleToKill / (float)PeopleData.Population;
	// Decrease population
	PeopleData.Population -= KillCount;

	// Money destruction
	PeopleData.Dept += KillCount * MONETARY_CREATION;

	DecreaseHappiness(KillCount * 100 * 2); // Death happiness malus

	//Cancel dead hunger
	PeopleData.HungerPoint = (1 - KillRatio) * PeopleData.HungerPoint;

	if(PeopleToKill == 0)
	{
		ResetPeople();
	}
}

void UFlarePeople::IncreaseHappiness(uint32 HappinessPoints)
{
	// Max happiness is 2 so Happiness max is 200 x population
	// Gain happiness is boost when sad and difficult when happy
	//  - Normal gain for 1 as happiness
	//  - 4 times gain for 0 as happiness
	//  - No gain for 2 as happiness
	// Formula: gain = (happiness - 2) ^ 2
	float Happiness = GetHappiness();
	float Gain = FMath::Square(Happiness - 2);
	PeopleData.HappinessPoint += HappinessPoints * Gain;
	PeopleData.HappinessPoint = FMath::Min(PeopleData.HappinessPoint, PeopleData.Population * 200);
}

void UFlarePeople::DecreaseHappiness(uint32 SadnessPoints)
{
	// Same as for increase but gain are inverted
	float Happiness = GetHappiness();
	float Gain = FMath::Square(Happiness);
	PeopleData.HappinessPoint -= SadnessPoints * Gain;
	PeopleData.HappinessPoint = FMath::Max(PeopleData.HappinessPoint, (uint32) 0);
}

void UFlarePeople::SetHappiness(float Happiness)
{
	PeopleData.HappinessPoint = PeopleData.Population * 100 * Happiness;
}

void UFlarePeople::Pay(uint32 Amount)
{
	FLOGV("Pay to people for sector %s Amount=%u", *Parent->GetSectorName().ToString(), Amount)

	uint32 Repayment = 0;
	if(PeopleData.Dept > 0)
	{
		Repayment = FMath::Min(PeopleData.Dept, Amount / 10);
		PeopleData.Dept -= Repayment;
	}
	PeopleData.Money += Amount - Repayment;
}

void UFlarePeople::ResetPeople()
{
	PeopleData.Population = 0;
	PeopleData.BirthPoint = 0;
	PeopleData.DeathPoint = 0;
	PeopleData.FoodStock = 0;
	PeopleData.HappinessPoint = 0;
	PeopleData.HungerPoint = 0;

	// Don't reset money to avoid moyen lost

}

/*----------------------------------------------------
	Getters
----------------------------------------------------*/

float UFlarePeople::GetHappiness()
{
	if(PeopleData.Population == 0)
	{
		return 0;
	}
	return (float) PeopleData.HappinessPoint / (100 * (float) PeopleData.Population);
}


#undef LOCTEXT_NAMESPACE
