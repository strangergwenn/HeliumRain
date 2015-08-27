#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareColorPanel.h"
#include "SlateMaterialBrush.h"


class AFlareMenuManager;


class SFlareSettingsMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareSettingsMenu){}

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

	/** Get the combo line headline */
	FText OnGetCurrentComboLine() const;

	/** Generate a combo box line */
	TSharedRef<SWidget> OnGenerateComboLine(TSharedPtr<FString> Item);

	/** Combo line selection changed */
	void OnComboLineSelectionChanged(TSharedPtr<FString> StringItem, ESelectInfo::Type SelectInfo);

	/** Exit this menu */
	void OnExit();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Game data
	AFlareGame*                                 Game;
	TWeakObjectPtr<class AFlareMenuManager>     MenuManager;

	// Slate widgets
	TSharedPtr<SFlareButton>                    FullscreenButton;
	TSharedPtr<SFlareButton>                    VSyncButton;

	// Resolution data
	TSharedPtr<SComboBox<TSharedPtr<FString> >> ResolutionSelector;
	TArray<TSharedPtr<FString>>                 ResolutionList;


};
