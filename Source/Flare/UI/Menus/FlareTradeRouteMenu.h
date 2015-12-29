#pragma once

#include "../../Flare.h"

class UFlareTradeRoute;

class SFlareTradeRouteMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareTradeRouteMenu){}

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

protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Go back to the previous menu*/
	void OnBackClicked();


	void OnChangeRouteNameClicked();

    void OnConfirmChangeRouteNameClicked();

    void OnCancelChangeRouteNameClicked();


    /** Are the not editing components visible ? */
    EVisibility GetShowingRouteNameVisibility() const;

    /** Are the not editing components visible ? */
    EVisibility GetEditingRouteNameVisibility() const;


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

    void OnLoadResourceClicked(UFlareSimulatedSector* Sector);

    void OnDecreaseLoadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource);

    void OnIncreaseLoadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource);

    void OnClearLoadResourceClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource);

    void OnUnloadResourceClicked(UFlareSimulatedSector* Sector);

    void OnDecreaseUnloadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource);

    void OnIncreaseUnloadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource);

    void OnClearUnloadResourceClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// HUD reference
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>    MenuManager;

	// Menu components
	UFlareTradeRoute*                     TargetTradeRoute;
    bool                                    IsEditingName;

	// Sector data
	TSharedPtr<SComboBox<UFlareSimulatedSector*>> SectorSelector;
    TArray<UFlareSimulatedSector*>                SectorList;

    TSharedPtr<SComboBox<UFlareResourceCatalogEntry*>> ResourceSelector;
    TArray<UFlareResourceCatalogEntry*>                ResourceList;


    TSharedPtr<SEditableText>                   EditRouteName;
    TSharedPtr<SVerticalBox>                    TradeSectorList;

};
