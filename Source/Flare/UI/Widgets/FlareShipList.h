#pragma once

#include "../../Flare.h"
#include "../Components/FlareListItem.h"
#include "../Widgets/FlareShipInstanceInfo.h"
#include "../Widgets/FlareTargetActions.h"


class SFlareShipList : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareShipList){}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareHUD>, OwnerHUD)

	SLATE_ARGUMENT(FText, Title)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Add a new station to the list */
	void AddStation(IFlareSpacecraftInterface* StationCandidate);

	/** Add a new ship to the list */
	void AddShip(IFlareSpacecraftInterface* ShipCandidate);
	
	/** Update the list display from content */
	void RefreshList();

	/** Remove all entries from the list */
	void Reset();


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

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
	TWeakObjectPtr<class AFlareHUD>    OwnerHUD;

	// Menu components
	TSharedPtr<SFlareListItem>                                   PreviousSelection;
	TSharedPtr< SListView< TSharedPtr<FInterfaceContainer> > >   TargetList;
	TArray< TSharedPtr<FInterfaceContainer> >                    TargetListData;


};
