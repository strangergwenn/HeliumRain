#pragma once

#include "../../Flare.h"
#include "../Components/FlareListItem.h"
#include "../Components/FlareSpacecraftInfo.h"
#include "../Components/FlareShipList.h"


class SFlareSectorMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareSectorMenu){}

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
	void Enter(UFlareSimulatedSector* Sector);

	/** Exit this menu */
	void Exit();


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Visibility setting for the travel button */
	EVisibility GetTravelVisibility() const;

	/** Get the sector's name */
	FText GetSectorName() const;

	/** Get the sector's description */
	FText GetSectorDescription() const;

	/** Go back to the previous menu*/
	void OnBackClicked();

	/** Move the selected fleet here */
	void OnTravelHereClicked();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// HUD reference
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>    MenuManager;

	// Menu components
	TSharedPtr<SFlareShipList>                 ShipList;
	UFlareSimulatedSector*                     TargetSector;

};
