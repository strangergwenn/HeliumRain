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
	FText OnGetCurrentResolutionComboLine() const;

	/** Generate a combo box line */
	TSharedRef<SWidget> OnGenerateResolutionComboLine(TSharedPtr<FScreenResolutionRHI> Item);

	/** Combo line selection changed */
	void OnResolutionComboLineSelectionChanged(TSharedPtr<FScreenResolutionRHI> StringItem, ESelectInfo::Type SelectInfo);

	void OnTextureQualitySliderChanged(float Value);

	void OnEffectsQualitySliderChanged(float Value);

	void OnAntiAliasingQualitySliderChanged(float Value);

	void OnPostProcessQualitySliderChanged(float Value);

	void OnFullscreenToggle();

	void OnVSyncToggle();

	void OnSupersamplingToggle();

	/** Exit this menu */
	void OnExit();

	/*----------------------------------------------------
		Helpers
	----------------------------------------------------*/

	/** Get all resolutions */
	void FillResolutionList();

	/** Update the current game state after a resolution change */
	void UpdateResolution();

	FText GetTextureQualityLabel(int32 Value) const;
	FText GetEffectsQualityLabel(int32 Value) const;
	FText GetAntiAliasingQualityLabel(int32 Value) const;
	FText GetPostProcessQualityLabel(int32 Value) const;


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
	TSharedPtr<SFlareButton>                    SupersamplingButton;
	TSharedPtr<SSlider>                         TextureQualitySlider;
	TSharedPtr<SSlider>                         EffectsQualitySlider;
	TSharedPtr<SSlider>                         AntiAliasingQualitySlider;
	TSharedPtr<SSlider>                         PostProcessQualitySlider;
	TSharedPtr<STextBlock>	        			TextureQualityLabel;
	TSharedPtr<STextBlock>	        			EffectsQualityLabel;
	TSharedPtr<STextBlock>	        			AntiAliasingQualityLabel;
	TSharedPtr<STextBlock>	        			PostProcessQualityLabel;

	// Resolution data
	TSharedPtr<SComboBox<TSharedPtr<FScreenResolutionRHI>>> ResolutionSelector;
	TArray<TSharedPtr<FScreenResolutionRHI>>                ResolutionList;


};
