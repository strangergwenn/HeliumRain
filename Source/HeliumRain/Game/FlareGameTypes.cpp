#include "../Flare.h"
#include "FlareGameTypes.h"

#define LOCTEXT_NAMESPACE "FlareNavigationHUD"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareGameTypes::UFlareGameTypes(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


/*----------------------------------------------------
	Text
----------------------------------------------------*/

FText UFlareGameTypes::GetCombatGroupDescription(EFlareCombatGroup::Type Type)
{
	FText Result;

	switch (Type)
	{
		case EFlareCombatGroup::AllMilitary:   Result = LOCTEXT("AllMilitary",  "All military ships");   break;
		case EFlareCombatGroup::Capitals:      Result = LOCTEXT("AllCapitals",  "Capital ships");        break;
		case EFlareCombatGroup::Fighters:      Result = LOCTEXT("AllFighters",  "Fighters");             break;
		case EFlareCombatGroup::Bombers:       Result = LOCTEXT("AllBombers",   "Bombers");              break;
		case EFlareCombatGroup::Civilan:       Result = LOCTEXT("AllCivilians", "Freighters");           break;
	}

	return Result;
}

FText UFlareGameTypes::GetCombatTacticDescription(EFlareCombatTactic::Type Type)
{
	FText Result;

	switch (Type)
	{
		case EFlareCombatTactic::AttackAllEnemies:   Result = LOCTEXT("AttackAll",       "Attack all enemies"); break;
		case EFlareCombatTactic::AttackFighters:     Result = LOCTEXT("AttackFighters",  "Attack fighters");    break;
		case EFlareCombatTactic::AttackCapitals:     Result = LOCTEXT("AttackCapitals",  "Attack capitals");    break;
		case EFlareCombatTactic::AttackStations:     Result = LOCTEXT("AttackStations",  "Attack stations");    break;
		case EFlareCombatTactic::AttackCivilians:    Result = LOCTEXT("AttackCivilians", "Attack civilians");   break;
		case EFlareCombatTactic::StandDown:          Result = LOCTEXT("StandDown",       "Stand down");         break;
		case EFlareCombatTactic::Flee:               Result = LOCTEXT("Flee",            "Flee");               break;
	}

	return Result;
}


#undef LOCTEXT_NAMESPACE
