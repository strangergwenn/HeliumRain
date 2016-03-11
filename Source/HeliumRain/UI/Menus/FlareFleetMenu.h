#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareShipList.h"

class IFlareSpacecraftInterface;
class IFlareSectorInterface;


class SFlareFleetMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareFleetMenu){}

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
	void Enter(UFlareFleet* TargetFleet);

	/** Fill a content pane with the trading information for Target spacecraft to deal with Other */
	void FillTradeBlock(IFlareSpacecraftInterface* TargetSpacecraft, IFlareSpacecraftInterface* OtherSpacecraft, TSharedPtr<SHorizontalBox> TargetBlock);

	/** Exit this menu */
	void Exit();


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/
	
	/** Go back to the previous menu*/
	void OnBackClicked();

	/** A spacecraft has been selected, hide the list and show the cargo */
	void OnSpacecraftSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// HUD reference
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>         MenuManager;

	// Menu components
	TSharedPtr<SFlareShipList>                      ShipList;


};
