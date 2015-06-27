#pragma once

#include "../../Flare.h"
#include "../../Player/FlareHUD.h"
#include "../../Player/FlarePlayerController.h"

#include "../Components/FlareButton.h"


class SFlareMainMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareMainMenu){}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareHUD>, OwnerHUD)
	
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
	void Enter();

	/** Exit this menu */
	void Exit();


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Start the game */
	void OnStartGame() const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** HUD reference */
	UPROPERTY()
	TWeakObjectPtr<class AFlareHUD> OwnerHUD;
		

};
