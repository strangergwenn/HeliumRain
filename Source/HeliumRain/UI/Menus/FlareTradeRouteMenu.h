#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareDropList.h"
#include "../../Game/FlareTradeRoute.h"
#include "../../Game/FlareSimulatedSector.h"
#include "../../Game/FlareFleet.h"
#include "../../Data/FlareResourceCatalogEntry.h"
#include "../FlareUITypes.h"

class UFlareTradeRoute;
struct FFlareTradeRouteSectorOperationSave;
typedef TPair<FFlareResourceDescription*, int64> TFlareResourceDeal;

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
		Content callbacks
	----------------------------------------------------*/
	
	/** Get route name */
	FText GetTradeRouteName() const;

	/** Get info for the current fleet */
	FText GetFleetInfo() const;
	
	/** Get info for the selected trade route step */
	FText GetSelectedStepInfo() const;

	/** Get info for the selected sector */
	FText GetSelectedSectorInfo() const;

	/** Get info for the next trade route step */
	FText GetNextStepInfo() const;

	/** Get the icon for the pause button */
	const FSlateBrush* GetPauseIcon() const;

	/** Get a description of a trade operation */
	FText GetOperationInfo(FFlareTradeRouteSectorOperationSave* Operation) const;
	
	// Sector list
	TSharedRef<SWidget> OnGenerateSectorComboLine(UFlareSimulatedSector* TargetSector);
	TArray<TFlareResourceDeal> GetSellableResources(UFlareSimulatedSector* TargetSector) const;
	TArray<TFlareResourceDeal> GetBuyableResources(UFlareSimulatedSector* TargetSector) const;
	void AddResourceDeals(TSharedPtr<SHorizontalBox> ResourcesBox, TArray<TFlareResourceDeal> Resources);
	
	FText OnGetCurrentSectorComboLine() const;
	
	/** Can add sector */
	bool IsAddSectorDisabled() const;

	/** Can rename trade oute */
	bool IsRenameDisabled() const;

	TSharedRef<SWidget> OnGenerateResourceComboLine(UFlareResourceCatalogEntry* Item);

	void OnResourceComboLineSelectionChanged(UFlareResourceCatalogEntry* Item, ESelectInfo::Type SelectInfo);

	FText OnGetCurrentResourceComboLine() const;

	TSharedRef<SWidget> OnGenerateOperationComboLine(TSharedPtr<FText> Item);

	FText OnGetCurrentOperationComboLine() const;

	void OnOperationComboLineSelectionChanged(TSharedPtr<FText> Item, ESelectInfo::Type SelectInfo);

	/** Can edit operation */
	EVisibility GetOperationDetailsVisibility() const;

	/** Can edit trade route */
	EVisibility GetMainVisibility() const;	

	/** Add button */
	FText GetAddSectorText() const;

	/** Get status */
	FText GetOperationStatusText(FFlareTradeRouteSectorOperationSave* Operation, FName SectorName) const;

	/** Highlight color */
	FSlateColor GetOperationHighlight(FFlareTradeRouteSectorOperationSave* Operation) const;

	/** Get the load text's button */
	FText GetLoadText() const;

	/** Get the unload text's button */
	FText GetUnloadText() const;
	
	/** Fleet info line */
	TSharedRef<SWidget> OnGenerateFleetComboLine(UFlareFleet* Item);

	FText OnGetCurrentFleetComboLine() const;
	
	/** Can assign a fleet*/
	EVisibility GetAssignFleetVisibility() const;
	
	bool IsEditOperationDisabled(FFlareTradeRouteSectorOperationSave* Operation) const;
	
	EVisibility GetQuantityLimitVisibility() const;

	EVisibility GetWaitLimitVisibility() const;


	/*----------------------------------------------------
		Actions callbacks
	----------------------------------------------------*/

	/** Reset statistics */
	void OnResetStatistics();

	/** Confirm a new name for this trade route */
	void OnConfirmChangeRouteNameClicked();

	void OnSectorComboLineSelectionChanged(UFlareSimulatedSector* Item, ESelectInfo::Type SelectInfo);

	/** Sector added */
	void OnAddSectorClicked();

	/** Move trade route */
	void OnMoveLeft(UFlareSimulatedSector* Sector);
	void OnMoveRight(UFlareSimulatedSector* Sector);
	bool IsMoveLeftDisabled(UFlareSimulatedSector* Sector) const;
	bool IsMoveRightDisabled(UFlareSimulatedSector* Sector) const;

	/** Sector removed */
	void OnRemoveSectorClicked(UFlareSimulatedSector* Sector);

	/** Edit & Delete */
	void OnEditOperationClicked(FFlareTradeRouteSectorOperationSave* Operation, UFlareSimulatedSector* Sector);
	void OnDeleteOperationClicked(FFlareTradeRouteSectorOperationSave* Operation);

	/** Done editing current operation */
	void OnDoneClicked();

	/** Skip current operation */
	void OnSkipOperationClicked();

	/** Pause current operation */
	void OnPauseTradeRouteClicked();

	/** Limits */
	void OnOperationUpClicked();
	void OnOperationDownClicked();
	void OnQuantityLimitToggle();
	void OnWaitLimitToggle();
	void OnQuantityLimitChanged(float Value);
	void OnQuantityLimitEntered(const FText& TextValue);
	void OnWaitLimitChanged(float Value);
	
	/** Load the current resource */
	void OnAddOperationClicked(UFlareSimulatedSector* Sector);
	
	/** Fleet selection */
	void OnFleetComboLineSelectionChanged(UFlareFleet* Item, ESelectInfo::Type SelectInfo);
	void OnAssignFleetClicked();
	void OnUnassignFleetClicked(UFlareFleet* Fleet);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// HUD reference
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>            MenuManager;

	// Menu data
	int32                                              MaxSectorsInRoute;
	UFlareTradeRoute*                                  TargetTradeRoute;
	FFlareTradeRouteSectorOperationSave*               SelectedOperation;
	UFlareSimulatedSector*                             SelectedSector;

	// Sector data
	TSharedPtr<SFlareDropList<UFlareSimulatedSector*>> SectorSelector;
	TArray<UFlareSimulatedSector*>                     SectorList;

	// Fleet list
	TSharedPtr<SFlareDropList<UFlareFleet*>>           FleetSelector;
	TArray<UFlareFleet*>                               FleetList;

	// Items
	TSharedPtr<SFlareDropList<UFlareResourceCatalogEntry*>> ResourceSelector;
	TSharedPtr<SFlareDropList<TSharedPtr<FText> >>     OperationSelector;
	TArray<TSharedPtr<FText>>                          OperationNameList;
	TArray<EFlareTradeRouteOperation::Type>            OperationList;

	TSharedPtr<SEditableText>                          EditRouteName;
	TSharedPtr<SHorizontalBox>                         TradeSectorList;
	TSharedPtr<SVerticalBox>                           TradeFleetList;

	TSharedPtr<SFlareButton>                           QuantityLimitButton;
	TSharedPtr<SFlareButton>                           WaitLimitButton;
	TSharedPtr<SSlider>                                QuantityLimitSlider;
	TSharedPtr<SSlider>                                WaitLimitSlider;
	TSharedPtr<SEditableText>                          QuantityLimitText;

	// Deals
	TSharedPtr<SHorizontalBox>                         EditSuggestedPurchasesBox;
	TSharedPtr<SHorizontalBox>                         EditSuggestedSalesBox;
	TArray<TFlareResourceDeal>                         CurrentlyBoughtResources;
	TArray<TFlareResourceDeal>                         CurrentlySoldResources;

};
