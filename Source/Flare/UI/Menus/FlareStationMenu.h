#pragma once

#include "../../Flare.h"
#include "../Components/FlareListItem.h"
#include "../Widgets/FlareShipInstanceInfo.h"
#include "../Widgets/FlareTargetActions.h"
#include "../Widgets/FlareShipList.h"


class SFlareStationMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareStationMenu){}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareHUD>, OwnerHUD)
	
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
	void Enter(IFlareSpacecraftInterface* Target);

	/** Exit this menu */
	void Exit();


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Target item generator */
	TSharedRef<ITableRow> GenerateTargetInfo(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/** Target item selected */
	void OnTargetSelected(TSharedPtr<FInterfaceContainer> Item, ESelectInfo::Type SelectInfo);

	/** Go back to the dahsboard */
	void OnDashboardClicked();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** HUD reference */
	UPROPERTY()
	TWeakObjectPtr<class AFlareHUD> OwnerHUD;

	/** Target station to use */
	UPROPERTY()
	IFlareSpacecraftInterface* CurrentStationTarget;
	
	// Menu components
	TSharedPtr<SFlareTargetActions>    ObjectActionMenu;
	TSharedPtr<STextBlock>             ObjectDescription;
	TSharedPtr<SFlareShipList>         ShipList;

};
