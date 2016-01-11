#pragma once

#include "../../Flare.h"

class UFlareTradeRoute;

class SFlareTradeRouteMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareTradeRouteMenu) {}

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
	void Enter(UFlareTradeRoute* TradeRoute);

	/** Exit this menu */
	void Exit();

	void GenerateSectorList();

	void GenerateFleetList();

protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Go back to the previous menu*/
	void OnBackClicked();

	/** Confirm a new name for this trade route */
	void OnConfirmChangeRouteNameClicked();

	/** Get route name */
	FText GetTradeRouteName() const;

	TSharedRef<SWidget> OnGenerateSectorComboLine(UFlareSimulatedSector* Item);

	void OnSectorComboLineSelectionChanged(UFlareSimulatedSector* Item, ESelectInfo::Type SelectInfo);

	FText OnGetCurrentSectorComboLine() const;

	void OnAddSectorClicked();

	/** Can Add sector */
	EVisibility GetAddSectorVisibility() const;

	TSharedRef<SWidget> OnGenerateResourceComboLine(UFlareResourceCatalogEntry* Item);

	void OnResourceComboLineSelectionChanged(UFlareResourceCatalogEntry* Item, ESelectInfo::Type SelectInfo);

	FText OnGetCurrentResourceComboLine() const;

	/** Can Add resource*/
	EVisibility GetResourceSelectorVisibility() const;

	/** Get the load text's button */
	FText GetLoadText() const;

	/** Get the unload text's button */
	FText GetUnloadText() const;

	/** Load the current resource */
	void OnLoadResourceClicked(UFlareSimulatedSector* Sector);

	void OnDecreaseLoadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource);

	void OnIncreaseLoadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource);

	void OnClearLoadResourceClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource);

	/** Load the current resource */
	void OnUnloadResourceClicked(UFlareSimulatedSector* Sector);

	void OnDecreaseUnloadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource);

	void OnIncreaseUnloadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource);

	void OnClearUnloadResourceClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource);

	/** Fleet management */

	TSharedRef<SWidget> OnGenerateFleetComboLine(UFlareFleet* Item);

	void OnFleetComboLineSelectionChanged(UFlareFleet* Item, ESelectInfo::Type SelectInfo);

	FText OnGetCurrentFleetComboLine() const;

	void OnAssignFleetClicked();

	/** Can assign a fleet*/
	EVisibility GetAssignFleetVisibility() const;

	void OnUnassignFleetClicked(UFlareFleet* Fleet);


protected:

	/*----------------------------------------------------
	Protected data
	----------------------------------------------------*/

	// HUD reference
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>            MenuManager;

	// Menu components
	UFlareTradeRoute*                                  TargetTradeRoute;

	// Sector data
	TSharedPtr<SComboBox<UFlareSimulatedSector*>>      SectorSelector;
	TArray<UFlareSimulatedSector*>                     SectorList;

	TSharedPtr<SComboBox<UFlareFleet*>>                FleetSelector;
	TArray<UFlareFleet*>                               FleetList;

	TSharedPtr<SComboBox<UFlareResourceCatalogEntry*>> ResourceSelector;


	TSharedPtr<SEditableText>                          EditRouteName;
	TSharedPtr<SHorizontalBox>                         TradeSectorList;
	TSharedPtr<SVerticalBox>                           TradeFleetList;

};
