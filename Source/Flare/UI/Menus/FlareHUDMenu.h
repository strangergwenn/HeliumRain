#pragma once

#include "../../Flare.h"
#include "../../Ships/FlareShip.h"
#include "../Components/FlareSubsystemStatus.h"


class SFlareHUDMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareHUDMenu){}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareHUD>, OwnerHUD)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);
	
	/** Set the ship to display data for */
	void SetTargetShip(AFlareShip* Target);


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/



protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** HUD reference */
	UPROPERTY()
	TWeakObjectPtr<class AFlareHUD>      OwnerHUD;

	// Menu components
	TSharedPtr<SFlareSubsystemStatus>    HullStatus;
	TSharedPtr<SFlareSubsystemStatus>    PropulsionStatus;
	TSharedPtr<SFlareSubsystemStatus>    RCSStatus;
	TSharedPtr<SFlareSubsystemStatus>    LifeSupportStatus;
	TSharedPtr<SHorizontalBox>           WeaponContainer;

};
