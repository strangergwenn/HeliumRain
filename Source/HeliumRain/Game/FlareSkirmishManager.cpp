

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

void UFlareSkirmishManager::Update(float DeltaSeconds)
{
	if (CurrentPhase == EFlareSkirmishPhase::Play)
	{
		Result.GameTime += DeltaSeconds;
	}
}

void UFlareSkirmishManager::StartSetup()
{
	FCHECK(CurrentPhase == EFlareSkirmishPhase::Idle);

	Data = FFlareSkirmishData();
	Result = FFlareSkirmishResultData();

	CurrentPhase = EFlareSkirmishPhase::Setup;
}

void UFlareSkirmishManager::StartPlay()
{
	FCHECK(CurrentPhase == EFlareSkirmishPhase::Setup);
	
	// Set common world elements
	Data.PlayerCompanyData.Emblem = GetGame()->GetCustomizationCatalog()->GetEmblem(0);
	Data.PlayerCompanyData.Name = FText::FromString("Player");
	Data.PlayerCompanyData.ShortName = "PLY";
	
	// Set phase
	CurrentPhase = EFlareSkirmishPhase::Play;

	// Start the game
	FFlareMenuParameterData Data;
	Data.ScenarioIndex = -1;
	Data.Skirmish = this;
	AFlareMenuManager::GetSingleton()->OpenMenu(EFlareMenu::MENU_CreateGame, Data);
}

void UFlareSkirmishManager::EndPlay()
{
	// Start skirmish countdown
	if (CurrentPhase == EFlareSkirmishPhase::Play)
	{
		AFlareMenuManager::GetSingleton()->PrepareSkirmishEnd();
	}

	// Detect victory
	AFlarePlayerController* PC = GetGame()->GetPC();
	FFlareSectorBattleState State = GetGame()->GetActiveSector()->GetSimulatedSector()->GetSectorBattleState(PC->GetCompany());
	if (State.BattleWon)
	{
		Result.PlayerVictory = true;
	}
	else
	{
		Result.PlayerVictory = false;
	}

	// Reset phase
	CurrentPhase = EFlareSkirmishPhase::End;
}

void UFlareSkirmishManager::EndSkirmish()
{
	CurrentPhase = EFlareSkirmishPhase::Idle;

	Data = FFlareSkirmishData();
	Result = FFlareSkirmishResultData();
}

bool UFlareSkirmishManager::IsPlaying() const
{
	return (CurrentPhase == EFlareSkirmishPhase::Play || CurrentPhase == EFlareSkirmishPhase::End);
}


/*----------------------------------------------------
	Setup
----------------------------------------------------*/

void UFlareSkirmishManager::SetAllowedValue(bool ForPlayer, uint32 Budget)
{
	FFlareSkirmishPlayerData& Belligerent = ForPlayer ? Data.Player : Data.Enemy;

	Belligerent.AllowedValue = Budget;

	while (GetCurrentCombatValue(ForPlayer) > Budget)
	{
		Belligerent.OrderedSpacecrafts.Pop();
	}
}

inline uint32 UFlareSkirmishManager::GetCurrentCombatValue(bool ForPlayer) const
{
	const FFlareSkirmishPlayerData& Belligerent = ForPlayer ? Data.Player : Data.Enemy;

	uint32 Value = 0;
	for (const FFlareSkirmishSpacecraftOrder& Order : Belligerent.OrderedSpacecrafts)
	{
		Value += Order.Description->CombatPoints;
	}

	return Value;
}

void UFlareSkirmishManager::AddShip(bool ForPlayer, FFlareSkirmishSpacecraftOrder Order)
{
	FFlareSkirmishPlayerData& Belligerent = ForPlayer ? Data.Player : Data.Enemy;

	Belligerent.OrderedSpacecrafts.Add(Order);
}


/*----------------------------------------------------
	Scoring
----------------------------------------------------*/

void UFlareSkirmishManager::ShipDisabled(bool ForPlayer)
{
	FFlareSkirmishPlayerResult& Belligerent = ForPlayer ? Result.Player : Result.Enemy;

	Belligerent.ShipsDisabled++;
}

void UFlareSkirmishManager::ShipDestroyed(bool ForPlayer)
{
	FFlareSkirmishPlayerResult& Belligerent = ForPlayer ? Result.Player : Result.Enemy;

	Belligerent.ShipsDestroyed++;
}

void UFlareSkirmishManager::AmmoFired(bool ForPlayer)
{
	FFlareSkirmishPlayerResult& Belligerent = ForPlayer ? Result.Player : Result.Enemy;

	Belligerent.AmmoFired++;
}

void UFlareSkirmishManager::AmmoHit(bool ForPlayer)
{
	FFlareSkirmishPlayerResult& Belligerent = ForPlayer ? Result.Player : Result.Enemy;

	Belligerent.AmmoHit++;
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

AFlareGame* UFlareSkirmishManager::GetGame() const
{
	return Cast<AFlareGame>(GetOuter());
}


#undef LOCTEXT_NAMESPACE
