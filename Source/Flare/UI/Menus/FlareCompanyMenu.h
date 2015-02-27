#pragma once

#include "../../Flare.h"
#include "../Components/FlareColorPanel.h"


class SFlareCompanyMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareCompanyMenu){}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareHUD>, OwnerHUD)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Setup the widget */
	void Setup(FFlarePlayerSave& PlayerData);

	/** Enter this menu */
	void Enter(UFlareCompany* Target);

	/** Exit this menu */
	void Exit();


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Go back to the dahsboard */
	void OnDashboardClicked();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** HUD reference */
	UPROPERTY()
	TWeakObjectPtr<class AFlareHUD> OwnerHUD;

	// Menu data
	TSharedPtr<STextBlock>             ColorBoxTitle;
	TSharedPtr<SFlareColorPanel>       ColorBox;


};
