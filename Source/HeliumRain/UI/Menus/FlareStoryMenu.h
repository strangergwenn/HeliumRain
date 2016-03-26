#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"


class AFlareMenuManager;


class SFlareStoryMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareStoryMenu){}

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

	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	/** Get the text color & alpha */
	FSlateColor GetTextColor() const;

	/** Start playing */
	void OnStartPlaying();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	// Settings
	float                                      TextShowTime;
	float                                      TextHideTime;
	float                                      TransitionTime;

	// Gameplay data
	float                                      CurrentTime;
	float                                      CurrentTextAlpha;
	int32                                      CurrentTextIndex;
	TArray<FText>                              TitleList;
	TArray<FText>                              TextList;

	// Slate data
	TWeakObjectPtr<class AFlareMenuManager>    MenuManager;
	TSharedPtr<STextBlock>                     Title;
	TSharedPtr<STextBlock>                     Text;


};
