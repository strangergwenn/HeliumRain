#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "SlateMaterialBrush.h"
#include "FlareMainMenu.generated.h"


class UFlareSaveGame;
class AFlareMenuManager;


USTRUCT()
struct FFlareSaveSlotInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY() UFlareSaveGame*            Save;
	UPROPERTY() UMaterialInstanceDynamic*  Emblem;

	FSlateBrush                EmblemBrush;

	int32                      CompanyShipCount;
	int32                      CompanyMoney;
	FText                      CompanyName;
};


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

	/** Open the settings menu */
	void OnOpenSettings();

	/** Quit the game */
	void OnQuitGame();


	/*----------------------------------------------------
		Helpers
	----------------------------------------------------*/

	/** Update all slot contents */
	void UpdateSaveSlots();

	/** Is this an existing game */
	bool IsExistingGame(int32 Index) const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	bool                                       Initialized;

	int32                                      SaveSlotCount;

	TWeakObjectPtr<class AFlareMenuManager>    MenuManager;
	TSharedPtr<SHorizontalBox>                 SaveBox;

	TArray<FFlareSaveSlotInfo>                 SaveSlots;


};
