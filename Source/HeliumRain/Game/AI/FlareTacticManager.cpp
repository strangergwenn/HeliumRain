
#include "FlareTacticManager.h"
#include "../../Flare.h"
#include "../FlareCompany.h"
#include "../FlareGame.h"


#define STATION_CONSTRUCTION_PRICE_BONUS 1.2


/*----------------------------------------------------
	Public API
----------------------------------------------------*/

UFlareTacticManager::UFlareTacticManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareTacticManager::Load(UFlareCompany* ParentCompany)
{
	ResetShipGroup(EFlareCombatTactic::AttackMilitary);

	Company = ParentCompany;
}

void UFlareTacticManager::SetCurrentShipGroup(EFlareCombatGroup::Type Type)
{
	CurrentShipGroup = Type;
}

void UFlareTacticManager::SetTacticForCurrentShipGroup(EFlareCombatTactic::Type Tactic)
{
	FCHECK(CurrentShipGroup < CurrentCombatTactics.Num());
	CurrentCombatTactics[CurrentShipGroup] = Tactic;

	if (CurrentShipGroup == EFlareCombatGroup::AllMilitary)
	{
		CurrentCombatTactics[EFlareCombatGroup::Capitals] = Tactic;
		CurrentCombatTactics[EFlareCombatGroup::Fighters] = Tactic;
	}
}

EFlareCombatGroup::Type UFlareTacticManager::GetCurrentShipGroup() const
{
	return CurrentShipGroup;
}

EFlareCombatTactic::Type UFlareTacticManager::GetCurrentTacticForShipGroup(EFlareCombatGroup::Type Type) const
{
	FCHECK(Type < CurrentCombatTactics.Num());
	return CurrentCombatTactics[Type];
}

int32 UFlareTacticManager::GetShipCountForShipGroup(EFlareCombatGroup::Type Type) const
{
	switch (Type)
	{
	case EFlareCombatGroup::AllMilitary:
		return CurrentMilitaryShipCount;

	case EFlareCombatGroup::Capitals:
		return CurrentCapitalShipCount;

	case EFlareCombatGroup::Fighters:
		return CurrentFighterCount;

	case EFlareCombatGroup::Civilan:
	default:
		return CurrentCivilianShipCount;
	}
}

void UFlareTacticManager::ResetControlGroups(UFlareSimulatedSector* Sector)
{
	// Reset ship count values
	CurrentMilitaryShipCount = 0;
	CurrentCapitalShipCount = 0;
	CurrentFighterCount = 0;
	CurrentCivilianShipCount = 0;

	// Compute the current count of all kinds of ships
	if (Sector)
	{
		TArray<UFlareSimulatedSpacecraft*>& ShipList = Sector->GetSectorShips();
		for (int32 Index = 0; Index < ShipList.Num(); Index++)
		{
			UFlareSimulatedSpacecraft* Ship = ShipList[Index];
			FCHECK(Ship);

			if (Ship->GetCompany() != Company)
			{
				continue;
			}

			if (Ship->IsMilitary())
			{
				CurrentMilitaryShipCount++;
				if (Ship->GetDescription()->Size == EFlarePartSize::L)
				{
					CurrentCapitalShipCount++;
				}
				else
				{
					CurrentFighterCount++;
				}
			}
			else
			{
				CurrentCivilianShipCount++;
			}
		}
	}
}

void UFlareTacticManager::ResetShipGroup(EFlareCombatTactic::Type Tactic)
{
	CurrentCombatTactics.Empty();
	for (int32 Index = EFlareCombatGroup::AllMilitary; Index <= EFlareCombatGroup::Civilan; Index++)
	{
		CurrentCombatTactics.Add(Tactic);
	}
}

