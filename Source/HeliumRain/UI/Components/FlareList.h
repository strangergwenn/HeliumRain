#pragma once

#include "../../Flare.h"
#include "../Components/FlareListItem.h"
#include "../Components/FlareSpacecraftInfo.h"
#include "../Components/FlareFleetInfo.h"

DECLARE_DELEGATE_OneParam(FFlareListItemSelected, TSharedPtr<FInterfaceContainer>)


class SFlareList : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareList)
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

	/** Change title */
	void SetTitle(FText NewTitle);

	/** Set the compact mode status */
	void SetUseCompactDisplay(bool Status);

	/** Remove all entries from the list */
	void Reset();
	
	/** Get the number of items */
	int32 GetItemCount() const
	{
		return ObjectList.Num();
	}


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Show a "no objects" text when the data is empty */
	EVisibility GetNoObjectsVisibility() const;

	/** Show ship filters when ships are present */
	EVisibility GetShipFiltersVisibility() const;

	/** Target item generator */
	TSharedRef<ITableRow> GenerateTargetInfo(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/** Target item selected */
	void OnTargetSelected(TSharedPtr<FInterfaceContainer> Item, ESelectInfo::Type SelectInfo);

	/** Update filters */
	void OnToggleShowFlags();

	/** Ship removed */
	void OnShipRemoved(UFlareSimulatedSpacecraft* Ship);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// HUD reference
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>                      MenuManager;

	// Menu components
	TSharedPtr<STextBlock>                                       Title;
	TSharedPtr< SListView< TSharedPtr<FInterfaceContainer> > >   WidgetList;
	TArray< TSharedPtr<FInterfaceContainer> >                    ObjectList;
	TArray< TSharedPtr<FInterfaceContainer> >                    FilteredObjectList;
	TSharedPtr<FInterfaceContainer>                              SelectedObject;
	TSharedPtr<SFlareListItem>                                   PreviousWidget;

	// Filters
	TSharedPtr<SFlareButton>                                     ShowStationsButton;
	TSharedPtr<SFlareButton>                                     ShowMilitaryButton;
	TSharedPtr<SFlareButton>                                     ShowFreightersButton;

	// State data
	FFlareListItemSelected                                       OnItemSelected;
	bool                                                         UseCompactDisplay;
	bool                                                         HasShips;

};
