#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareShipList.h"

class IFlareSpacecraftInterface;


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
	void UpdateShipList();


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/
	
	/** Go back to the previous menu*/
	void OnBackClicked();

	/** A spacecraft has been selected*/
	void OnSpacecraftSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer);

	/** A fleet has been selected */
	void OnFleetSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer);

	/** Can we see edit buttons */
	EVisibility GetEditVisibility() const;

	/** Get the text about editing a fleet */
	FText GetSelectText() const;

	/** Get text about merging */
	FText GetAddText() const;

	/** Get text about removing a ship from the fleet */
	FText GetRemoveText() const;

	/** Is the "select fleet" button disabled */
	bool IsSelectDisabled() const;

	/** Is the "add to fleet" button disabled */
	bool IsAddDisabled() const;

	/** Is the "remove from fleet" button disabled */
	bool IsRemoveDisabled() const;

	/** Is the "rename fleet" button disabled */
	bool IsRenameDisabled() const;

	/** Select a fleet */
	void OnSelectFleet();

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

	// Menu components
	TSharedPtr<SFlareShipList>                      ShipList;
	TSharedPtr<SFlareShipList>                      FleetList;
	TSharedPtr<SEditableText>                       EditFleetName;

	// State data
	UFlareFleet*                                    SelectedFleet;
	UFlareFleet*                                    FleetToAdd;
	IFlareSpacecraftInterface*                      ShipToRemove;


};
