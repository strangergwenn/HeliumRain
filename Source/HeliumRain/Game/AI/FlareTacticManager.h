#pragma once

#include <UObject/Object.h>
#include "../FlareGameTypes.h"
#include "FlareTacticManager.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareTacticManager : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public API
	----------------------------------------------------*/

	/** Setup the tactic manager */
	void Load(UFlareCompany* ParentCompany);

	/** Set the current ship group to give orders to */
	void SetCurrentShipGroup(EFlareCombatGroup::Type Type);

	/** Set the current order for the currently selected ship group */
	void SetTacticForCurrentShipGroup(EFlareCombatTactic::Type Tactic);

	/** Get the current ship group */
	EFlareCombatGroup::Type GetCurrentShipGroup() const;

	/* Get the current order */
	EFlareCombatTactic::Type GetCurrentTacticForShipGroup(EFlareCombatGroup::Type Type) const;

	/** Get the ship count in this group */
	int32 GetShipCountForShipGroup(EFlareCombatGroup::Type Type) const;

	/** Reset all controls */
	void ResetControlGroups(UFlareSimulatedSector* Sector);

	/** Reset this ship group */
	void ResetShipGroup(EFlareCombatTactic::Type Tactic);


protected:

	/*----------------------------------------------------
		Data
	----------------------------------------------------*/

	// Gameplay data
	UFlareCompany*			                          Company;

	// Command groups
	TEnumAsByte<EFlareCombatGroup::Type>              CurrentShipGroup;
	TArray<TEnumAsByte<EFlareCombatTactic::Type>>     CurrentCombatTactics;
	int32                                             CurrentMilitaryShipCount;
	int32                                             CurrentCapitalShipCount;
	int32                                             CurrentFighterCount;
	int32                                             CurrentCivilianShipCount;
	

};

