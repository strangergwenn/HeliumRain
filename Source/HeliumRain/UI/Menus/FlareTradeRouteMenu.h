#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../../Game/FlareTradeRoute.h"

class UFlareTradeRoute;
struct FFlareTradeRouteSectorOperationSave;

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

	TSharedRef<SWidget> OnGenerateSectorComboLine(UFlareSimulatedSector* Item);
	
	FText OnGetCurrentSectorComboLine() const;
	
	/** Can add sector */
	bool IsAddSectorDisabled() const;

	TSharedRef<SWidget> OnGenerateResourceComboLine(UFlareResourceCatalogEntry* Item);

	void OnResourceComboLineSelectionChanged(UFlareResourceCatalogEntry* Item, ESelectInfo::Type SelectInfo);

	FText OnGetCurrentResourceComboLine() const;

	TSharedRef<SWidget> OnGenerateOperationComboLine(TSharedPtr<FString> Item);

	FText OnGetCurrentOperationComboLine() const;

	void OnOperationComboLineSelectionChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo);

	/** Can edit operation*/
	EVisibility GetOperationDetailsVisibility() const;

	/** Add button */
	FText GetAddSectorText() const;

	FText GetOperationStatusText(FFlareTradeRouteSectorOperationSave* Operation) const;

	/** Get the load text's button */
	FText GetLoadText() const;

	/** Get the unload text's button */
	FText GetUnloadText() const;
	
	/** Fleet info line */
	TSharedRef<SWidget> OnGenerateFleetComboLine(UFlareFleet* Item);

	FText OnGetCurrentFleetComboLine() const;
	
	/** Can assign a fleet*/
	EVisibility GetAssignFleetVisibility() const;


	EVisibility GetEditOperationVisibility(FFlareTradeRouteSectorOperationSave* Operation) const;

	EVisibility GetSkipOperationVisibility(FFlareTradeRouteSectorOperationSave* Operation) const;

	EVisibility GetQuantityLimitVisibility() const;

	EVisibility GetWaitLimitVisibility() const;

	/*----------------------------------------------------
		Actions callbacks
	----------------------------------------------------*/
	
	/** Confirm a new name for this trade route */
	void OnConfirmChangeRouteNameClicked();

	void OnSectorComboLineSelectionChanged(UFlareSimulatedSector* Item, ESelectInfo::Type SelectInfo);

	/** Sector added */
	void OnAddSectorClicked();

	/** Sector removed */
	void OnRemoveSectorClicked(UFlareSimulatedSector* Sector);

	void OnEditOperationClicked(FFlareTradeRouteSectorOperationSave* Operation);
	void OnSkipOperationClicked(FFlareTradeRouteSectorOperationSave* Operation);



	void OnDeleteOperationClicked();
	void OnOperationUpClicked();
	void OnOperationDownClicked();
	void OnQuantityLimitToggle();
	void OnWaitLimitToggle();

	void OnQuantityLimitChanged(float Value);
	void OnWaitLimitChanged(float Value);


	// Load the current resource
	void OnAddOperationClicked(UFlareSimulatedSector* Sector);
	void OnDecreaseLoadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource);
	void OnIncreaseLoadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource);
	void OnClearLoadResourceClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource);

	// Unload the current resource
	void OnUnloadResourceClicked(UFlareSimulatedSector* Sector);
	void OnDecreaseUnloadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource);
	void OnIncreaseUnloadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource);
	void OnClearUnloadResourceClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource);

	// Fleet selection
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

	// Sector data
	TSharedPtr<SComboBox<UFlareSimulatedSector*>>      SectorSelector;
	TArray<UFlareSimulatedSector*>                     SectorList;

	// Fleet list
	TSharedPtr<SComboBox<UFlareFleet*>>                FleetSelector;
	TArray<UFlareFleet*>                               FleetList;

	// Items
	TSharedPtr<SComboBox<UFlareResourceCatalogEntry*>> ResourceSelector;
	TSharedPtr<SComboBox<TSharedPtr<FString> >>        OperationSelector;
	TArray<TSharedPtr<FString>>                        OperationNameList;
	TArray<EFlareTradeRouteOperation::Type>            OperationList;

	TSharedPtr<SEditableText>                          EditRouteName;
	TSharedPtr<SHorizontalBox>                         TradeSectorList;
	TSharedPtr<SVerticalBox>                           TradeFleetList;

	TSharedPtr<SFlareButton>                           QuantityLimitButton;
	TSharedPtr<SFlareButton>                           WaitLimitButton;
	TSharedPtr<SSlider>                                QuantityLimitSlider;
	TSharedPtr<SSlider>                                WaitLimitSlider;
};
