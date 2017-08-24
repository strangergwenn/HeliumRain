#pragma once

#include "../../Flare.h"
#include "../Components/FlareListItem.h"
#include "../Components/FlareSpacecraftInfo.h"
#include "../Components/FlareList.h"
#include "../../Game/FlareGame.h"


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

	/** Can we build stations */
	FText GetBuildStationHelpText() const;
	
	/** Do we have more than the max stations already ? */
	bool IsBuildStationDisabled() const;

	/** Fleet info line */
	TSharedRef<SWidget> OnGenerateFleetComboLine(UFlareFleet* Item);

	/** Current fleet info line */
	FText OnGetCurrentFleetComboLine() const;
	
	/** Get the travel text */
	FText GetTravelText() const;
	
	/** Visibility setting for the travel button */
	bool IsTravelDisabled() const;

	/** Visibility setting for the owned reserve list */
	EVisibility GetOwnedReserveVisibility() const;

	/** Visibility setting for the other reserve list */
	EVisibility GetOtherReserveVisibility() const;

	/** Visibility of lists */
	EVisibility GetVisitedListVisibility() const;

	/** Visibility unkwown sector text */
	EVisibility GetUnknownSectorVisibility() const;

	/** Get the refill text */
	FText GetRefillText() const;

	/** Visibility setting for the resource prices button */
	bool IsResourcePricesDisabled() const;

	/** Visibility setting for the refill button */
	bool IsRefillDisabled() const;

	/** Get the repair text */
	FText GetRepairText() const;

	/** Visibility setting for the repair button */
	bool IsRepairDisabled() const;

	/** Get the sector's name */
	FText GetSectorName() const;

	/** Get the sector's description */
	FText GetSectorDescription() const;

	/** Get the sector's location */
	FText GetSectorLocation() const;

	/** Get the combat value visibility */
	EVisibility GetCombatValueVisibility() const;

	/** Get the combat value */
	FText GetOwnCombatValue() const;

	/** Get the combat value */
	FText GetFullCombatValue() const;

	/** Get the combat value */
	FText GetHostileCombatValue() const;
	

	/*----------------------------------------------------
		Action callbacks
	----------------------------------------------------*/
	
	/** See the resource prices */
	void OnResourcePrices();

	/** Fleet selection */
	void OnFleetComboLineSelectionChanged(UFlareFleet* Item, ESelectInfo::Type SelectInfo);

	/** Move the selected fleet here */
	void OnTravelHereClicked();

	/** Refill all fleets */
	void OnRefillClicked();

	/** Repair all fleets */
	void OnRepairClicked();
	
	/** Start travel */
	void OnStartTravelConfirmed(UFlareFleet* SelectedFleet);

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

	// Fleet list
	TSharedPtr<SFlareDropList<UFlareFleet*>>   FleetSelector;
	TArray<UFlareFleet*>                       FleetList;
	FName                                      LastSelectedFleetName;

	// Menu components
	TSharedPtr<SFlareList>                     OwnedShipList;
	TSharedPtr<SFlareList>                     OtherShipList;
	TSharedPtr<SFlareList>                     OwnedReserveShipList;
	TSharedPtr<SFlareList>                     OtherReserveShipList;
	UFlareSimulatedSector*                     TargetSector;

	// Station data
	FFlareSpacecraftDescription*               StationDescription;

};
