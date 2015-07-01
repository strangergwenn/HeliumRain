#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "SlateMaterialBrush.h"


class AFlareMenuManager;


class SFlareNewGameMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareNewGameMenu){}

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
	void Enter();

	/** Exit this menu */
	void Exit();


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/
	
	/** Start the game */
	void OnLaunch();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	AFlareGame*                                Game;
	TWeakObjectPtr<class AFlareMenuManager>    MenuManager;

	int32                                      SlotIndex;

};
