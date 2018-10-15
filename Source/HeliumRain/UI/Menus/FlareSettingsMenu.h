#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareColorPanel.h"
#include "SlateMaterialBrush.h"
#include "../Components/FlareKeyBind.h"
#include "GameFramework/PlayerInput.h"

class AFlareGame;
class AFlareMenuManager;

struct FSimpleBind
{
	FText DisplayName;
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

protected:

	/*----------------------------------------------------
		Internal
	----------------------------------------------------*/

	/** Fill the key binding list */
	TSharedRef<SWidget> BuildKeyBindingBox();

	/** Fill a key binding pane */
	void BuildKeyBindingPane(TArray<TSharedPtr<FSimpleBind> >& BindList, TSharedPtr<SVerticalBox>& Form);

	/** Fill the joystick binding list */
	TSharedRef<SWidget> BuildJoystickBindingBox();

	/** Fill a joystick binding item */
	void BuildJoystickBinding(FText AxisDisplayName, FName AxisMappingName, TSharedPtr<SVerticalBox>& Form);


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	// Culture
	FText OnGetCurrentCultureComboLine() const;
	TSharedRef<SWidget> OnGenerateCultureComboLine(TSharedPtr<FString> Item);
	void OnCultureComboLineSelectionChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo);
	
	// Resolution list
	FText OnGetCurrentResolutionComboLine() const;
	TSharedRef<SWidget> OnGenerateResolutionComboLine(TSharedPtr<FScreenResolutionRHI> Item);
	void OnResolutionComboLineSelectionChanged(TSharedPtr<FScreenResolutionRHI> StringItem, ESelectInfo::Type SelectInfo);

	// Joystick axis list
	FText OnGetCurrentJoystickKeyName(FName AxisName) const;
	TSharedRef<SWidget> OnGenerateJoystickComboLine(TSharedPtr<FKey> Item, FName AxisName);
	void OnJoystickComboLineSelectionChanged(TSharedPtr<FKey> KeyItem, ESelectInfo::Type SelectInfo, FName AxisName);

	// Joystick actions
	void OnInvertAxisClicked(FName AxisName);
	void OnUnbindAxisClicked(FName AxisName);
	bool IsAxisControlsDisabled(FName AxisName) const;

	// Gamepad profile list
	FText OnGetCurrentGamepadComboLine() const;
	TSharedRef<SWidget> OnGenerateGamepadComboLine(TSharedPtr<FText> Item);
	void OnGamepadComboLineSelectionChanged(TSharedPtr<FText> KeyItem, ESelectInfo::Type SelectInfo);

	// FOV
	void OnFOVSliderChanged(float Value);
	FText GetFOVLabel(int32 Value) const;

	// Gamma
	void OnGammaSliderChanged(float Value);
	FText GetGammaLabel(float Value) const;

	// Sensitivity
	void OnSensitivitySliderChanged(float Value);
	FText GetSensitivityLabel(float Value) const;

	// Texture quality
	void OnTextureQualitySliderChanged(float Value);
	FText GetTextureQualityLabel(int32 Value) const;

	// Effects quality
	void OnEffectsQualitySliderChanged(float Value);
	FText GetEffectsQualityLabel(int32 Value) const;

	// AA quality
	void OnAntiAliasingQualitySliderChanged(float Value);
	FText GetAntiAliasingQualityLabel(int32 Value) const;

	// Post-process quality
	void OnPostProcessQualitySliderChanged(float Value);
	FText GetPostProcessQualityLabel(int32 Value) const;

	// Music volume
	void OnMusicVolumeSliderChanged(float Value);
	FText GetMusicVolumeLabel(int32 Value) const;

	// Master volume
	void OnMasterVolumeSliderChanged(float Value);
	FText GetMasterVolumeLabel(int32 Value) const;

	// Ship count
	void OnShipCountSliderChanged(float Value);
	FText GetShipCountLabel(int32 Value) const;

	// Toggles
	void OnFullscreenToggle();
	void OnVSyncToggle();
	void OnMotionBlurToggle();
	void OnTemporalAAToggle();
	void OnSupersamplingToggle();	
	void OnInvertYToggle();
	void OnAnticollisionToggle();
	void OnCockpitToggle();
	void OnLateralVelocityToggle();
	void OnForwardOnlyThrustToggle();
	
	// Dead zone sliders
	void OnRotationDeadZoneSliderChanged(float Value);
	void OnRollDeadZoneSliderChanged(float Value);
	void OnTranslationDeadZoneSliderChanged(float Value);
	
	const FSlateBrush* GetGamepadDrawing() const;
	
	// Bindings
	void ApplyNewBinding(TSharedPtr<FSimpleBind> BindingThatChanged, bool Replace, bool bPrimaryKey);
	void CancelNewBinding(SFlareKeyBind* UIBinding, FKey PreviousKey);
	void OnKeyBindingChanged( FKey PreviousKey, FKey NewKey, SFlareKeyBind* UIBinding, TSharedPtr<FSimpleBind> BindingThatChanged, bool bPrimaryKey);

	/** Get all axis bindings */
	void FillJoystickAxisList();

	/** Get all resolutions */
	void FillResolutionList();

	/** Get all resolutions */
	void FillGameGamepadList();

	/** Update resolutions */
	void UpdateResolutionList(FIntPoint Resolution);

	/** Update the current game state after a resolution change */
	void UpdateResolution(bool CanAdaptResolution);
	
	void CreateBinds();

	bool IsAlreadyUsed(TArray<TSharedPtr<FSimpleBind>> &BindConflicts, FKey Key, FSimpleBind& ExcludeBinding);

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
	TSharedPtr<SFlareButton>                    MotionBlurButton;
	TSharedPtr<SFlareButton>                    TemporalAAButton;
	TSharedPtr<SFlareButton>                    SupersamplingButton;
	TSharedPtr<SSlider>                         FOVSlider;
	TSharedPtr<SSlider>                         GammaSlider;
	TSharedPtr<SSlider>                         SensitivitySlider;
	TSharedPtr<SSlider>                         TextureQualitySlider;
	TSharedPtr<SSlider>                         EffectsQualitySlider;
	TSharedPtr<SSlider>                         AntiAliasingQualitySlider;
	TSharedPtr<SSlider>                         PostProcessQualitySlider;
	TSharedPtr<STextBlock>	        			FOVLabel;
	TSharedPtr<STextBlock>	        			GammaLabel;
	TSharedPtr<STextBlock>	        			SensitivityLabel;
	TSharedPtr<STextBlock>	        			TextureQualityLabel;
	TSharedPtr<STextBlock>	        			EffectsQualityLabel;
	TSharedPtr<STextBlock>	        			AntiAliasingQualityLabel;
	TSharedPtr<STextBlock>	        			PostProcessQualityLabel;

	// Gameplay
	TSharedPtr<SFlareButton>                    InvertYButton;
#if !UE_BUILD_SHIPPING
	TSharedPtr<SFlareButton>                    CockpitButton;
#endif
	TSharedPtr<SFlareButton>                    LateralVelocityButton;
	TSharedPtr<SFlareButton>                    AnticollisionButton;
	TSharedPtr<SSlider>                         ShipCountSlider;
	TSharedPtr<STextBlock>	        			ShipCountLabel;

	// Sound
	TSharedPtr<SSlider>                         MusicVolumeSlider;
	TSharedPtr<SSlider>                         MasterVolumeSlider;
	TSharedPtr<STextBlock>	        			MusicVolumeLabel;
	TSharedPtr<STextBlock>	        			MasterVolumeLabel;
	
	// Controls
	TSharedPtr<SFlareButton>                    ForwardOnlyThrustButton;
	TSharedPtr<SVerticalBox>                    ControlListLeft;
	TSharedPtr<SVerticalBox>                    ControlListRight;
	TArray<TSharedPtr<FSimpleBind> >            Binds;
	TArray<TSharedPtr<FSimpleBind> >            Binds2;

	// Joystick data
	TArray<TSharedPtr<FKey>>                    JoystickAxisKeys;

	// Resolution data
	TSharedPtr<SFlareDropList<TSharedPtr<FScreenResolutionRHI>>> ResolutionSelector;
	TArray<TSharedPtr<FScreenResolutionRHI>>                ResolutionList;

	// Culture data
	TSharedPtr<SFlareDropList<TSharedPtr<FString>>>  CultureSelector;
	TArray<TSharedPtr<FString>>						 CultureList;

	// Gamepad data
	TSharedPtr<SFlareDropList<TSharedPtr<FText>>>    GamepadSelector;
	TArray<TSharedPtr<FText>>					     GamepadList;

};
