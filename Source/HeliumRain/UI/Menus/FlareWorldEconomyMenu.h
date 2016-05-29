#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"


class UFlareSimulatedSector;
class AFlareMenuManager;
struct FFlareResourceDescription;

/** Sector resources prices*/
struct FFlareWorldEconomyMenuParam
{
	/** Resource */
	FFlareResourceDescription* Resource;

	/** Sector */
	UFlareSimulatedSector* Sector;
};

class SFlareWorldEconomyMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareWorldEconomyMenu){}

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
	void Enter(FFlareResourceDescription* Resource, UFlareSimulatedSector* Sector);

	/** Exit this menu */
	void Exit();

	/** Exit this menu */
	void Back();

	void GenerateSectorList();

protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	FSlateColor GetPriceColor(UFlareSimulatedSector* Sector) const;

	/** Get the resource price info */
	FText GetResourceDescription() const;

	/** Get the sector info */
	FText GetSectorText(UFlareSimulatedSector* Sector) const;

	/** Get the sector info color */
	FSlateColor GetSectorTextColor(UFlareSimulatedSector* Sector) const;

	/** Get the resource price info */
	FText GetResourcePriceInfo(UFlareSimulatedSector* Sector) const;

	/** Get the resource price variation info */
	FText GetResourcePriceVariationInfo(UFlareSimulatedSector* Sector, TSharedPtr<int32> MeanDuration) const;

	TSharedRef<SWidget> OnGenerateResourceComboLine(UFlareResourceCatalogEntry* Item);

	void OnResourceComboLineSelectionChanged(UFlareResourceCatalogEntry* Item, ESelectInfo::Type SelectInfo);

	FText OnGetCurrentResourceComboLine() const;

	void OnOpenSector(UFlareSimulatedSector* Sector);

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Target data
	TWeakObjectPtr<class AFlareMenuManager>         MenuManager;
	UFlareSimulatedSector*                          BackSector;
	FFlareResourceDescription*                      TargetResource;

	// Slate data
	TSharedPtr<SVerticalBox>                        SectorList;
	TSharedPtr<SComboBox<UFlareResourceCatalogEntry*>> ResourceSelector;

};
