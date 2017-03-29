#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareList.h"

class UFlareSimulatedSpacecraft;


class SFlareFleetMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareFleetMenu){}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareMenuManager>, MenuManager)

	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Setup the widget */
	void Setup();

	/** Enter this menu */
	void Enter(UFlareFleet* TargetFleet);
	
	/** Exit this menu */
	void Exit();

	/** Update the fleet list */
	void UpdateFleetList();

	/** Update the ship list */
	void UpdateShipList(UFlareFleet* Fleet);


protected:

	/*----------------------------------------------------
		Content callbacks
	----------------------------------------------------*/

	/** Can we see edit buttons */
	EVisibility GetEditVisibility() const;

	/** Is the "add to fleet" button disabled */
	bool IsAddDisabled() const;
	
	/** Get hint text about merging */
	FText GetAddHintText() const;

	/** Is the "remove from fleet" button disabled */
	bool IsRemoveDisabled() const;

	/** Get hint text about removing a ship from the fleet */
	FText GetRemoveHintText() const;

	/** Is the "rename fleet" button disabled */
	bool IsRenameDisabled() const;

	/** Get hint text about renaming a fleet */
	FText GetRenameHintText() const;


	/*----------------------------------------------------
		Action callbacks
	----------------------------------------------------*/
	
	/** A spacecraft has been selected*/
	void OnSpacecraftSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer);

	/** A fleet has been selected */
	void OnFleetSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer);
	
	/** Editing done */
	void OnEditFinished();

	/** Add the selected ship to the selected fleet */
	void OnAddToFleet();

	/** Remove the selected ship from the selected fleet */
	void OnRemoveFromFleet();

	/** Rename the selected fleet */
	void OnRenameFleet();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// HUD reference
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>         MenuManager;
	
	// Game data
	UFlareFleet*                                    FleetToEdit;
	UFlareFleet*                                    FleetToAdd;
	UFlareSimulatedSpacecraft*                      ShipToRemove;

	// Menu components
	TSharedPtr<SFlareList>                          ShipList;
	TSharedPtr<SFlareList>                          FleetList;
	TSharedPtr<SEditableText>                       EditFleetName;
	

};
