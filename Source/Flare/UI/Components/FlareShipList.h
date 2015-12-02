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

	SLATE_BEGIN_ARGS(SFlareShipList){}

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

	/** Add a new ship to the list */
	void AddShip(IFlareSpacecraftInterface* ShipCandidate);

	/** Update the list display from content */
	void RefreshList();

	/** Clear the current selection */
	void ClearSelection();

	/** Remove all entries from the list */
	void Reset();

	/** Get the currently selected spacecraft, or NULL */
	IFlareSpacecraftInterface* GetSelectedSpacecraft() const;

	/** Get the currently selected part, or NULL */
	FFlareSpacecraftComponentDescription* GetSelectedPart() const;

	/** Get the currently selected compay, or NULL */
	UFlareCompany* GetSelectedCompany() const;


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
	

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// HUD reference
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>                      MenuManager;

	// Menu components
	TSharedPtr<SFlareListItem>                                   PreviousSelection;
	TSharedPtr< SListView< TSharedPtr<FInterfaceContainer> > >   TargetList;
	TArray< TSharedPtr<FInterfaceContainer> >                    TargetListData;
	TSharedPtr<FInterfaceContainer>                              SelectedItem;

	// State data
	FFlareListItemSelected                                       OnItemSelected;

};
