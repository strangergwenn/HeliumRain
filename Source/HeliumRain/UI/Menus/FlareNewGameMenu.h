#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareColorPanel.h"
#include "SlateMaterialBrush.h"


class AFlareMenuManager;
class AFlareGame;

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

	/** Get the combo line headline */
	FText OnGetCurrentComboLine() const;

	/** Generate a combo box line */
	TSharedRef<SWidget> OnGenerateComboLine(TSharedPtr<FString> Item);

	/** Combo line selection changed */
	void OnComboLineSelectionChanged(TSharedPtr<FString> StringItem, ESelectInfo::Type SelectInfo);
	

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	// Game data
	AFlareGame*                                 Game;
	TWeakObjectPtr<class AFlareMenuManager>     MenuManager;
	
	// Slate widgets
	TSharedPtr<SEditableText>                   CompanyName;
	TSharedPtr<SFlareColorPanel>                ColorBox;
	TSharedPtr<SFlareButton>                    TutorialButton;

	// Scenario data
	TSharedPtr<SComboBox<TSharedPtr<FString> >> ScenarioSelector;
	TArray<TSharedPtr<FString>>                 ScenarioList;


};
