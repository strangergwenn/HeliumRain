

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
	// Load catalog
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinder<UFlareAsteroidCatalog> RockCatalog;
		ConstructorHelpers::FObjectFinder<UFlareAsteroidCatalog> DebrisCatalog;
		FConstructorStatics()
			: RockCatalog(TEXT("/Game/ThirdParty/RocksDebris/RockDebrisCatalog.RockDebrisCatalog"))
			, DebrisCatalog(TEXT("/Game/Environment/Debris/MetalDebrisCatalog.MetalDebrisCatalog"))
		{}
	};
	static FConstructorStatics ConstructorStatics;

	// Push catalog data into storage
	RockCatalog = ConstructorStatics.RockCatalog.Object;
	DebrisCatalog = ConstructorStatics.DebrisCatalog.Object;

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

	// Sector
	Data.SectorDescription.Identifier = FName("sector-skirmish");
	Data.SectorDescription.Name = LOCTEXT("SkirmishSectorName", "Skirmish");
	Data.SectorDescription.Description = LOCTEXT("SkirmishSectorDescription", "Unknown sector");
	Data.SectorDescription.CelestialBodyIdentifier = "nema";
	Data.SectorDescription.Altitude = 6000;
	Data.SectorDescription.Phase = 0;
	Data.SectorDescription.IsIcy = true;
	Data.SectorDescription.LevelName = FName("GenericIcySector");

	// Debris field
	Data.SectorDescription.DebrisFieldInfo.DebrisFieldDensity = 25;
	Data.SectorDescription.DebrisFieldInfo.MinDebrisSize = 3;
	Data.SectorDescription.DebrisFieldInfo.MaxDebrisSize = 7;

	// Asteroids
	Data.AsteroidCount = 100;

	CurrentPhase = EFlareSkirmishPhase::Setup;
}

void UFlareSkirmishManager::StartPlay()
{
	FCHECK(CurrentPhase == EFlareSkirmishPhase::Setup);
	
	// Use the appropriate debris
	if (Data.MetallicDebris)
	{
		Data.SectorDescription.DebrisFieldInfo.DebrisCatalog = DebrisCatalog;
	}
	else
	{
		Data.SectorDescription.DebrisFieldInfo.DebrisCatalog = RockCatalog;
	}

	// Set level name
	if (Data.SectorDescription.IsIcy)
	{
		Data.SectorDescription.LevelName = FName("GenericIcySector");
	}
	else
	{
		Data.SectorDescription.LevelName = FName("GenericDustySector");
	}

	// Set common world elements
	Data.PlayerCompanyData.Emblem = GetGame()->GetCustomizationCatalog()->GetEmblem(0);
	Data.PlayerCompanyData.Name = FText::FromString("Player");
	Data.PlayerCompanyData.ShortName = "PLY";
	
	// Set phase
	CurrentPhase = EFlareSkirmishPhase::Play;

	// Start the game
	FFlareMenuParameterData MenuData;
	MenuData.ScenarioIndex = -1;
	MenuData.Skirmish = this;
	AFlareMenuManager::GetSingleton()->OpenMenu(EFlareMenu::MENU_CreateGame, MenuData);
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
