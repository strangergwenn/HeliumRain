#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"


class UFlareSectorInterface;
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
	void Enter(UFlareSectorInterface* Sector);

	/** Exit this menu */
	void Exit();

	/** Exit this menu */
	void Back();


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Get the resource price info */
	FText GetResourcePriceInfo(FFlareResourceDescription* Resource) const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Target data
	TWeakObjectPtr<class AFlareMenuManager>         MenuManager;
	UFlareSectorInterface*                          TargetSector;

	// Slate data
	TSharedPtr<SVerticalBox>                        ResourcePriceList;

};
