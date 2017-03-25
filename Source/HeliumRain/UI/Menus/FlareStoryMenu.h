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

	/** Skip to next */
	void OnNext();

	/** Back to previous */
	void OnPrevious();

	/** Start playing */
	void OnStartPlaying();

	/** Get previous visibility */
	EVisibility GetPreviousButtonVisibility() const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	// Settings
	float                                      TextShowTime;
	float                                      TextHideTime;
	float                                      TransitionTime;

	// Gameplay data
	bool                                       FadingIn;
	bool                                       GoingToNext;
	float                                      CurrentTime;
	float                                      CurrentAlpha;
	int32                                      CurrentIndex;
	TArray<FText>                              TitleList;
	TArray<FText>                              TextList;
	TArray<FText>                              SubTextList;
	TArray<const FSlateBrush*>                 ImageList;

	// Slate data
	TWeakObjectPtr<class AFlareMenuManager>    MenuManager;
	TSharedPtr<SBorder>                        Image;
	TSharedPtr<STextBlock>                     Title;
	TSharedPtr<STextBlock>                     Text;
	TSharedPtr<STextBlock>                     SubText;


};
