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

	void UpdateStationCost();


protected:

	/*----------------------------------------------------
		Content callbacks
	----------------------------------------------------*/

	/** Can we build stations */
	FText GetBuildStationText() const;

	/** Can we build stations */
	EVisibility GetBuildStationVisibility() const;

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

	FText OnGetStationCost() const;

	/** Get the text of a station combo box line*/
	TSharedRef<SWidget> OnGenerateStationComboLine(UFlareSpacecraftCatalogEntry* Item);

	bool IsBuildStationDisabled() const;

	FText OnGetCurrentStationComboLine() const;


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

	void OnStartTravelConfirmed();

	void OnStationComboLineSelectionChanged(UFlareSpacecraftCatalogEntry* Item, ESelectInfo::Type SelectInfo);

	void OnBuildStationClicked();


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
	TSharedPtr<SComboBox<UFlareSpacecraftCatalogEntry*>> StationSelector;
	TArray<UFlareSpacecraftCatalogEntry*>                StationList;
	FText                                                           StationCost;
	bool                                                            StationBuildable;
};
