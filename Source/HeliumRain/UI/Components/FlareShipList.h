#pragma once

#include "../../Flare.h"
#include "../Components/FlareListItem.h"
#include "../Components/FlareSpacecraftInfo.h"

DECLARE_DELEGATE_OneParam(FFlareListItemSelected, TSharedPtr<FInterfaceContainer>)


class SFlareShipList : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareShipList)
	 : _UseCompactDisplay(false)
	{}

	SLATE_ARGUMENT(bool, UseCompactDisplay)
	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareMenuManager>, MenuManager)
	SLATE_EVENT(FFlareListItemSelected, OnItemSelected)
	SLATE_ARGUMENT(FText, Title)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Add a new fleet to the list */
	void AddFleet(UFlareFleet* Fleet);

	/** Add a new ship to the list */
	void AddShip(UFlareSimulatedSpacecraft* Ship);

	/** Update the list display from content */
	void RefreshList();

	/** Clear the current selection */
	void ClearSelection();

	/** Remove all entries from the list */
	void Reset();
	
	int32 GetItemCount() const
	{
		return SpacecraftList.Num();
	}


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Show a "no objects" text when the data is empty */
	EVisibility GetNoObjectsVisibility() const;

	/** Target item generator */
	TSharedRef<ITableRow> GenerateTargetInfo(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/** Target item selected */
	void OnTargetSelected(TSharedPtr<FInterfaceContainer> Item, ESelectInfo::Type SelectInfo);

	/** Update filters */
	void OnToggleShowFlags();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// HUD reference
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>                      MenuManager;

	// Menu components
	TSharedPtr<SFlareListItem>                                   PreviousSelection;
	TSharedPtr< SListView< TSharedPtr<FInterfaceContainer> > >   ListWidget;
	TArray< TSharedPtr<FInterfaceContainer> >                    FilteredList;
	TArray< TSharedPtr<FInterfaceContainer> >                    SpacecraftList;
	TSharedPtr<FInterfaceContainer>                              SelectedItem;

	// Filters
	TSharedPtr<SFlareButton>                                     ShowStationsButton;
	TSharedPtr<SFlareButton>                                     ShowMilitaryButton;
	TSharedPtr<SFlareButton>                                     ShowFreightersButton;

	// State data
	FFlareListItemSelected                                       OnItemSelected;
	bool                                                         UseCompactDisplay;

};
