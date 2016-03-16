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
		case EFlareCombatGroup::Civilan:       Result = LOCTEXT("AllCivilians", "Freighters");           break;
	}

	return Result;
}

FText UFlareGameTypes::GetCombatTacticDescription(EFlareCombatTactic::Type Type)
{
	FText Result;

	switch (Type)
	{
		case EFlareCombatTactic::ProtectMe:        Result = LOCTEXT("ProtectMe",       "Protect me");         break;
		case EFlareCombatTactic::AttackMilitary:   Result = LOCTEXT("AttackMilitary",  "Attack military");    break;
		case EFlareCombatTactic::AttackStations:   Result = LOCTEXT("AttackStations",  "Attack stations");    break;
		case EFlareCombatTactic::AttackCivilians:  Result = LOCTEXT("AttackCivilians", "Attack freighters");  break;
		case EFlareCombatTactic::StandDown:        Result = LOCTEXT("StandDown",       "Stand down");         break;
	}

	return Result;
}

const FSlateBrush* UFlareGameTypes::GetCombatGroupIcon(EFlareCombatGroup::Type Type)
{
	const FSlateBrush* Result = NULL;

	switch (Type)
	{
		case EFlareCombatGroup::AllMilitary:   Result = FFlareStyleSet::GetIcon("AllMilitary");   break;
		case EFlareCombatGroup::Capitals:      Result = FFlareStyleSet::GetIcon("AllCapitals");   break;
		case EFlareCombatGroup::Fighters:      Result = FFlareStyleSet::GetIcon("AllFighters");   break;
		case EFlareCombatGroup::Civilan:       Result = FFlareStyleSet::GetIcon("AllFreighters"); break;
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE
