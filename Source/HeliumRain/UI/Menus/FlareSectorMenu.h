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
		Content callbacks
	----------------------------------------------------*/

	/** Can we build stations */
	FText GetBuildStationText() const;
	
	/** Dp we have more than the max stations already ? */
	bool IsBuildStationDisabled() const;

	/** Get the travel text */
	FText GetTravelText() const;

	/** Visibility setting for the travel button */
	bool IsTravelDisabled() const;

	/** Get the refuel text */
	FText GetRefuelText() const;

	/** Visibility setting for the refuel button */
	bool IsRefuelDisabled() const;

	/** Get the repair text */
	FText GetRepairText() const;

	/** Visibility setting for the repair button */
	bool IsRepairDisabled() const;

	/** Get the sector's name */
	FText GetSectorName() const;

	/** Get the sector's description */
	FText GetSectorDescription() const;

	/** Get the sector's transport data */
	FText GetSectorTransportInfo() const;

	/** Get the sector's location */
	FText GetSectorLocation() const;

	/** Get the station' cost */
	FText OnGetStationCost() const;


	/*----------------------------------------------------
		Action callbacks
	----------------------------------------------------*/

	/** Go back to the previous menu*/
	void OnBackClicked();

	/** Move the selected fleet here */
	void OnTravelHereClicked();

	/** Refuel all fleets */
	void OnRefuelClicked();

	/** Repair all fleets */
	void OnRepairClicked();

	/** Start travel */
	void OnStartTravelConfirmed();

	/** Build a station */
	void OnBuildStationClicked();

	/** Station selected */
	void OnBuildStationSelected(FFlareSpacecraftDescription* NewStationDescription);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// HUD reference
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>    MenuManager;

	// Menu components
	TSharedPtr<SFlareShipList>                 OwnedShipList;
	TSharedPtr<SFlareShipList>                 OtherShipList;
	UFlareSimulatedSector*                     TargetSector;

	// Station data
	FFlareSpacecraftDescription*               StationDescription;

};
