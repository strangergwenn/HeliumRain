#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareDropList.h"


class AFlareMenuManager;
struct FFlareResourceDescription;

class UFlareSimulatedSector;


class SFlareResourcePricesMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareResourcePricesMenu){}

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

	void GenerateResourceList();


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	FText GetSectorName() const;

	FSlateColor GetPriceColor(FFlareResourceDescription* Resource) const;

	/** On show world infos clicked */
	void OnShowWorldInfosClicked(FFlareResourceDescription* Resource);
	
	/** Get the resource production info */
	FText GetResourceProductionInfo(FFlareResourceDescription* Resource) const;

	/** Get the resource consumption info */
	FText GetResourceConsumptionInfo(FFlareResourceDescription* Resource) const;

	/** Get the resource stock info */
	FText GetResourceStockInfo(FFlareResourceDescription* Resource) const;

	/** Get the resource capacity info */
	FText GetResourceCapacityInfo(FFlareResourceDescription* Resource) const;

	/** Get the resource price info */
	FText GetResourcePriceInfo(FFlareResourceDescription* Resource) const;

	/** Get the resource price variation info */
	FText GetResourcePriceVariationInfo(FFlareResourceDescription* Resource) const;

	/** Get the resource transport fee info */
	FText GetResourceTransportFeeInfo(FFlareResourceDescription* Resource) const;

	TSharedRef<SWidget> OnGenerateSectorComboLine(UFlareSimulatedSector* Sector);
	void OnSectorComboLineSelectionChanged(UFlareSimulatedSector* Sector, ESelectInfo::Type SelectInfo);
	FText OnGetCurrentSectorComboLine() const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Target data
	TWeakObjectPtr<class AFlareMenuManager>         MenuManager;
	UFlareSimulatedSector*                          TargetSector;
	TArray<UFlareSimulatedSector*>                  KnownSectors;

	// Slate data
	TSharedPtr<SVerticalBox>                        ResourcePriceList;
	TSharedPtr<SFlareDropList<UFlareSimulatedSector*>> SectorSelector;

};
