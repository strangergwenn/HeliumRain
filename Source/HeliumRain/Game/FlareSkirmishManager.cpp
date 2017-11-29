

#include "FlareSkirmishManager.h"
#include "../Flare.h"

#include "FlareWorld.h"
#include "FlareGame.h"
#include "FlareGameTools.h"

#include "../Spacecrafts/FlareSpacecraftTypes.h"


#define LOCTEXT_NAMESPACE "FlareSkirmishManager"


/*----------------------------------------------------
	Gameplay phases
----------------------------------------------------*/

UFlareSkirmishManager::UFlareSkirmishManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CurrentPhase = EFlareSkirmishPhase::Idle;
}

void UFlareSkirmishManager::StartSetup()
{
	FCHECK(CurrentPhase == EFlareSkirmishPhase::Idle);

	Player = FFlareSkirmishPlayer();
	Enemy = FFlareSkirmishPlayer();

	CurrentPhase = EFlareSkirmishPhase::Setup;
}

void UFlareSkirmishManager::StartPlay()
{
	FCHECK(CurrentPhase == EFlareSkirmishPhase::Setup);

	CurrentPhase = EFlareSkirmishPhase::Play;

	// TODO 1075 : reset counters, start time... 
}

void UFlareSkirmishManager::EndPlay()
{
	FCHECK(CurrentPhase == EFlareSkirmishPhase::Play);

	CurrentPhase = EFlareSkirmishPhase::End;

	// TODO 1075 : stop counters
}

void UFlareSkirmishManager::EndSkirmish()
{
	CurrentPhase = EFlareSkirmishPhase::Idle;

	// TODO 1075 : endgame detection, use FlareSkirmishScoreMenu

	Player = FFlareSkirmishPlayer();
	Enemy = FFlareSkirmishPlayer();
}


/*----------------------------------------------------
	Tools
----------------------------------------------------*/

void UFlareSkirmishManager::SetAllowedValue(bool ForPlayer, uint32 Budget)
{
	FFlareSkirmishPlayer& Belligerent = ForPlayer ? Player : Enemy;

	Belligerent.AllowedValue = Budget;

	while (GetCurrentCombatValue(ForPlayer) > Budget)
	{
		Belligerent.OrderedSpacecrafts.Pop();
	}
}

inline uint32 UFlareSkirmishManager::GetCurrentCombatValue(bool ForPlayer) const
{
	const FFlareSkirmishPlayer& Belligerent = ForPlayer ? Player : Enemy;

	uint32 Value = 0;
	for (FFlareSpacecraftDescription* Desc : Belligerent.OrderedSpacecrafts)
	{
		Value += Desc->CombatPoints;
	}

	return Value;
}

void UFlareSkirmishManager::AddShip(bool ForPlayer, FFlareSpacecraftDescription* Desc)
{
	FFlareSkirmishPlayer& Belligerent = ForPlayer ? Player : Enemy;

	Belligerent.OrderedSpacecrafts.Add(Desc);
}

bool UFlareSkirmishManager::IsPlaying() const
{
	return (CurrentPhase == EFlareSkirmishPhase::Play);
}

AFlareGame* UFlareSkirmishManager::GetGame() const
{
	return Cast<UFlareWorld>(GetOuter())->GetGame();
}


#undef LOCTEXT_NAMESPACE
