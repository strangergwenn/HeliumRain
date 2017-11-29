

#include "FlareSkirmishManager.h"
#include "../Flare.h"

#include "FlareWorld.h"
#include "FlareGame.h"
#include "FlareGameTools.h"


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

	CurrentPhase = EFlareSkirmishPhase::Setup;
}

void UFlareSkirmishManager::StartPlay()
{
	FCHECK(CurrentPhase == EFlareSkirmishPhase::Setup);

	CurrentPhase = EFlareSkirmishPhase::Play;

	// TODO : reset counters, start time... 
}

void UFlareSkirmishManager::EndPlay()
{
	FCHECK(CurrentPhase == EFlareSkirmishPhase::Play);

	CurrentPhase = EFlareSkirmishPhase::End;

	// TODO : stop counters
}

void UFlareSkirmishManager::EndSkirmish()
{
	CurrentPhase = EFlareSkirmishPhase::Idle;

	// TODO : cleanup
}


/*----------------------------------------------------
	Tools
----------------------------------------------------*/

void UFlareSkirmishManager::AddShip(FFlareSpacecraftDescription*, bool ForPlayer)
{
	// TODO : store decription, decrease budget... 

	// TODO : getter, storage, etc
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
