#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "SlateMaterialBrush.h"


class AFlareMenuManager;
class AFlareGame;

class SFlareMainMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareMainMenu){}

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

	/** Get the current background */
	const FSlateBrush* GetBackgroundBrush() const;

	/** Get a save slot's description */
	FText GetText(int32 Index) const;

	/** Get the main image for a save slot */
	const FSlateBrush* GetSaveIcon(int32 Index) const;

	/** Is the delete button visible ? */
	EVisibility GetDeleteButtonVisibility(int32 Index) const;

	/** Get a button text */
	FText GetButtonText(int32 Index) const;

	/** Get a button icon */
	const FSlateBrush* GetButtonIcon(int32 Index) const;

	/** Start the game */
	void OnOpenSlot(TSharedPtr<int32> Index);

	/** Delete a game */
	void OnDeleteSlot(TSharedPtr<int32> Index);

	/** Delete a game */
	void OnDeleteSlotConfirmed();

	/*** Start a skirmish */
	void OnOpenSkirmish();
	
	/*** Open credits */
	void OnOpenCredits();

	/*** Open EULA */
	void OnOpenEULA();
	

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	TWeakObjectPtr<class AFlareMenuManager>    MenuManager;
	AFlareGame*                                Game;

	TSharedPtr<SHorizontalBox>                 SaveBox;

	int32                                      SaveSlotToDelete;

};
