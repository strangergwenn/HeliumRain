#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareColorPanel.h"
#include "SlateMaterialBrush.h"
#include "../Components/FlareKeyBind.h"

class AFlareGame;
class AFlareMenuManager;

struct FSimpleBind
{
	FString DisplayName;
	TSharedPtr<FKey> Key;
	TSharedPtr<FKey> AltKey;
	FKey DefaultKey;
	FKey DefaultAltKey;
	TSharedPtr<SFlareKeyBind> KeyWidget;
	TSharedPtr<SFlareKeyBind> AltKeyWidget;
	bool bHeader;

	explicit FSimpleBind(const FText& InDisplayName);

	TArray<FInputActionKeyMapping> ActionMappings;
	TArray<FInputAxisKeyMapping> AxisMappings;
	TArray<FName>  SpecialBindings;

	FSimpleBind* AddActionMapping(const FName& Mapping);
	FSimpleBind* AddAxisMapping(const FName& Mapping, float Scale);
	FSimpleBind* AddSpecialBinding(const FName& Mapping);
	FSimpleBind* AddDefaults(FKey InDefaultKey, FKey InDefaultAltKey = FKey())
	{
		DefaultKey = InDefaultKey;
		DefaultAltKey = InDefaultAltKey;
		return this;
	}
	FSimpleBind* MakeHeader()
	{
		bHeader = true;
		return this;
	}
	void WriteBind();
};

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

	TSharedRef<SWidget> BuildKeyBindingBox();

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

	void OnMusicVolumeSliderChanged(float Value);

	void OnMasterVolumeSliderChanged(float Value);

	void OnFullscreenToggle();

	void OnVSyncToggle();

	void OnSupersamplingToggle();
		
	void OnCockpitToggle();

	void OnPauseInMenusToggle();

	void OnKeyBindingChanged( FKey PreviousKey, FKey NewKey, TSharedPtr<FSimpleBind> BindingThatChanged, bool bPrimaryKey );


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
	FText GetMusicVolumeLabel(int32 Value) const;
	FText GetMasterVolumeLabel(int32 Value) const;

	void CreateBinds();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Game data
	AFlareGame*                                 Game;
	TWeakObjectPtr<class AFlareMenuManager>     MenuManager;

	// Graphics settings
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

	// Gameplay
	TSharedPtr<SFlareButton>                    CockpitButton;
	TSharedPtr<SFlareButton>                    PauseInMenusButton;

	// Sound
	TSharedPtr<SSlider>                         MusicVolumeSlider;
	TSharedPtr<SSlider>                         MasterVolumeSlider;
	TSharedPtr<STextBlock>	        			MusicVolumeLabel;
	TSharedPtr<STextBlock>	        			MasterVolumeLabel;
	
	// Controls
	TSharedPtr<SVerticalBox>                    ControlList;
	TArray<TSharedPtr<FSimpleBind> >            Binds;

	// Resolution data
	TSharedPtr<SComboBox<TSharedPtr<FScreenResolutionRHI>>> ResolutionSelector;
	TArray<TSharedPtr<FScreenResolutionRHI>>                ResolutionList;


};
