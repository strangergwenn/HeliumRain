

#include "FlareSkirmishManager.h"
#include "../Flare.h"

#include "FlareWorld.h"
#include "FlareGame.h"
#include "FlareGameTools.h"

#include "../Data/FlareCustomizationCatalog.h"

#include "../Player/FlareMenuManager.h"
#include "../Player/FlarePlayerController.h"

#include "../Spacecrafts/FlareSpacecraftTypes.h"

#include "../UI/FlareUITypes.h"


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

	Data = FFlareSkirmishData();

	CurrentPhase = EFlareSkirmishPhase::Setup;
}

void UFlareSkirmishManager::StartPlay()
{
	FCHECK(CurrentPhase == EFlareSkirmishPhase::Setup);
	
	// Set common world elements
	Data.PlayerCompanyData.Emblem = GetGame()->GetCustomizationCatalog()->GetEmblem(0);
	Data.PlayerCompanyData.Name = FText::FromString("Player");
	Data.PlayerCompanyData.ShortName = "PLY";

	// TODO 1075 : from menu
	Data.EnemyCompanyName = "PIR";

	// Set phase
	CurrentPhase = EFlareSkirmishPhase::Play;

	// TODO 1075 : reset counters, start time... 

	// Start the game
	FFlareMenuParameterData Data;
	Data.ScenarioIndex = -1;
	Data.Skirmish = this;
	AFlareMenuManager::GetSingleton()->OpenMenu(EFlareMenu::MENU_CreateGame, Data);
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

	Data = FFlareSkirmishData();
}


/*----------------------------------------------------
	Tools
----------------------------------------------------*/

void UFlareSkirmishManager::SetAllowedValue(bool ForPlayer, uint32 Budget)
{
	FFlareSkirmishPlayer& Belligerent = ForPlayer ? Data.Player : Data.Enemy;

	Belligerent.AllowedValue = Budget;

	while (GetCurrentCombatValue(ForPlayer) > Budget)
	{
		Belligerent.OrderedSpacecrafts.Pop();
	}
}

inline uint32 UFlareSkirmishManager::GetCurrentCombatValue(bool ForPlayer) const
{
	const FFlareSkirmishPlayer& Belligerent = ForPlayer ? Data.Player : Data.Enemy;

	uint32 Value = 0;
	for (FFlareSpacecraftDescription* Desc : Belligerent.OrderedSpacecrafts)
	{
		Value += Desc->CombatPoints;
	}

	return Value;
}

void UFlareSkirmishManager::AddShip(bool ForPlayer, FFlareSpacecraftDescription* Desc)
{
	FFlareSkirmishPlayer& Belligerent = ForPlayer ? Data.Player : Data.Enemy;

	Belligerent.OrderedSpacecrafts.Add(Desc);
}

bool UFlareSkirmishManager::IsPlaying() const
{
	return (CurrentPhase == EFlareSkirmishPhase::Play);
}

AFlareGame* UFlareSkirmishManager::GetGame() const
{
	return Cast<AFlareGame>(GetOuter());
}


#undef LOCTEXT_NAMESPACE
