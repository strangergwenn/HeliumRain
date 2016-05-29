#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"


class AFlareMenuManager;
struct FFlareResourceDescription;


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

	/** Exit this menu */
	void Back();


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	FSlateColor GetPriceColor(FFlareResourceDescription* Resource) const;

	/** On show world infos clicked */
	void OnShowWorldInfosClicked(FFlareResourceDescription* Resource);


	/** Get the resource price info */
	FText GetResourcePriceInfo(FFlareResourceDescription* Resource) const;

	/** Get the resource price variation info */
	FText GetResourcePriceVariationInfo(FFlareResourceDescription* Resource) const;

	/** Get the resource transport fee info */
	FText GetResourceTransportFeeInfo(FFlareResourceDescription* Resource) const;

	FText GetSectorPriceInfo() const;

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Target data
	TWeakObjectPtr<class AFlareMenuManager>         MenuManager;
	UFlareSimulatedSector*                          TargetSector;

	// Slate data
	TSharedPtr<SVerticalBox>                        ResourcePriceList;

};
