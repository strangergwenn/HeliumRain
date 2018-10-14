
#include "FlareSettingsMenu.h"

#include "../../Flare.h"
#include "../../Game/FlareGameUserSettings.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareSaveGame.h"

#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Data/FlareSpacecraftComponentsCatalog.h"

#include "../Components/FlareTabView.h"

#include "STextComboBox.h"
#include "Internationalization/Culture.h"
#include "GameFramework/InputSettings.h"
#include "Engine.h"


#define LOCTEXT_NAMESPACE "FlareSettingsMenu"

#define MIN_MAX_SHIPS 20
#define MAX_MAX_SHIPS 100
#define STEP_MAX_SHIPS 5

#define MIN_GAMMA 1.5f
#define MAX_GAMMA 3.0f

#define MIN_SENSITIVITY 0.5f
#define MAX_SENSITIVITY 1.5f


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSettingsMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	Game = MenuManager->GetPC()->GetGame();
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());

	// Minima
	float MinFOV = MenuManager->GetPC()->GetMinVerticalFOV();

	// Current settings
	float CurrentGammaRatio = (MyGameSettings->Gamma - MIN_GAMMA) / (MAX_GAMMA - MIN_GAMMA);
	float CurrentSensitivityRatio = (MyGameSettings->InputSensitivity - MIN_SENSITIVITY) / (MAX_SENSITIVITY - MIN_SENSITIVITY);
	float CurrentFOVRatio = (MyGameSettings->VerticalFOV - MinFOV) / (MenuManager->GetPC()->GetMaxVerticalFOV() - MinFOV);
	float CurrentTextureQualityRatio = MyGameSettings->ScalabilityQuality.TextureQuality / 3.f;
	float CurrentEffectsQualityRatio = MyGameSettings->ScalabilityQuality.EffectsQuality / 3.f;
	float CurrentAntiAliasingQualityRatio = MyGameSettings->ScalabilityQuality.AntiAliasingQuality / 2.f;
	float CurrentPostProcessQualityRatio = MyGameSettings->ScalabilityQuality.PostProcessQuality / 3.f;
	//FLOGV("MyGameSettings->ScalabilityQuality.TextureQuality=%d CurrentTextureQualityRatio=%f", MyGameSettings->ScalabilityQuality.TextureQuality, CurrentTextureQualityRatio);
	//FLOGV("MyGameSettings->ScalabilityQuality.EffectsQuality=%d CurrentEffectsQualityRatio=%f", MyGameSettings->ScalabilityQuality.EffectsQuality, CurrentEffectsQualityRatio);
	//FLOGV("MyGameSettings->ScalabilityQuality.AntiAliasingQuality=%d CurrentAntiAliasingQualityRatio=%f", MyGameSettings->ScalabilityQuality.AntiAliasingQuality, CurrentAntiAliasingQualityRatio);
	//FLOGV("MyGameSettings->ScalabilityQuality.PostProcessQuality=%d CurrentPostProcessQualityRatio=%f", MyGameSettings->ScalabilityQuality.PostProcessQuality, CurrentPostProcessQualityRatio);

	CreateBinds();

	// General data
	FLinearColor Color = Theme.NeutralColor;
	Color.A = Theme.DefaultAlpha;
	int32 LabelSize = 200;
	int32 ValueSize = 100;

	// Build culture list
	TArray<FString> CultureNames = FTextLocalizationManager::Get().GetLocalizedCultureNames(ELocalizationLoadFlags::Game);
	TArray<FCultureRef> CultureRefList = FInternationalization::Get().GetAvailableCultures(CultureNames, false);
	for(FCultureRef Culture: CultureRefList)
	{
		CultureList.Add(MakeShareable(new FString(Culture->GetName())));
	}

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SFlareTabView)
			
		// Gameplay options
		+ SFlareTabView::Slot()
		.Header(LOCTEXT("GameplayTab", "Gameplay"))
		.HeaderHelp(LOCTEXT("GameplayTabInfo", "Tweak the gameplay options"))
		[
			SNew(SVerticalBox)

			// Title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.TitlePadding)
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("GameplayHint", "Gameplay"))
				.TextStyle(&Theme.SubTitleFont)
			]
		
			// Content
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			[
				SNew(SBox)
				.WidthOverride(Theme.ContentWidth)
				.HAlign(HAlign_Fill)
				[
					SNew(SVerticalBox)

					// Culture
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SAssignNew(CultureSelector, SFlareDropList<TSharedPtr<FString>>)
							.OptionsSource(&CultureList)
							.OnGenerateWidget(this, &SFlareSettingsMenu::OnGenerateCultureComboLine)
							.OnSelectionChanged(this, &SFlareSettingsMenu::OnCultureComboLineSelectionChanged)
							.HeaderWidth(10.1)
							.ItemWidth(10.1)
							[
								SNew(SBox)
								.Padding(Theme.ListContentPadding)
								[
									SNew(STextBlock)
									.Text(this, &SFlareSettingsMenu::OnGetCurrentCultureComboLine)
									.TextStyle(&Theme.TextFont)
								]
							]
						]
					]
				
					// Options line 1
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					[
						SNew(SHorizontalBox)
					
						// Invert vertical
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SAssignNew(InvertYButton, SFlareButton)
							.Text(LOCTEXT("InvertY", "Invert vertical"))
							.HelpText(LOCTEXT("InvertYInfo", "Invert the vertical axis of the mouse while flying"))
							.Toggle(true)
							.OnClicked(this, &SFlareSettingsMenu::OnInvertYToggle)
							.Width(6)
						]
					
						// Disable mouse
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SAssignNew(DisableMouseButton, SFlareButton)
							.Text(LOCTEXT("DisableMouse", "Disable mouse"))
							.HelpText(LOCTEXT("DisableMouseInfo", "Disable mouse input while flying to avoid unwanted input with a joystick or gamepad "))
							.Toggle(true)
							.OnClicked(this, &SFlareSettingsMenu::OnDisableMouseToggle)
							.Width(6)
						]
					]
				
					// Options line 2
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					[
						SNew(SHorizontalBox)
					
						// Anticollision
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SAssignNew(AnticollisionButton, SFlareButton)
							.Text(LOCTEXT("Anticollision", "Use anticollision"))
							.HelpText(LOCTEXT("AnticollisionInfo", "Anti-collision will prevent your ship from crashing into objects and forbid close fly-bys."))
							.Toggle(true)
							.OnClicked(this, &SFlareSettingsMenu::OnAnticollisionToggle)
							.Width(6)
						]

						// Lateral velocity
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SAssignNew(LateralVelocityButton, SFlareButton)
							.Text(LOCTEXT("LateralVelocity", "Show drift"))
							.HelpText(LOCTEXT("LateralVelocityInfo", "Show the current drift velocity of your ship on the HUD."))
							.Toggle(true)
							.OnClicked(this, &SFlareSettingsMenu::OnLateralVelocityToggle)
							.Width(6)
						]

#if !UE_BUILD_SHIPPING
						// Cockpit
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SAssignNew(CockpitButton, SFlareButton)
							.Text(LOCTEXT("Cockpit", "Use cockpit"))
							.HelpText(LOCTEXT("CockpitInfo", "Use the immersive 3D cockpit instead of a flat interface."))
							.Toggle(true)
							.OnClicked(this, &SFlareSettingsMenu::OnCockpitToggle)
						]
#endif
					]

					// FOV box
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// FOV text
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(LabelSize)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("FOVLabel", "Field of view"))
								.TextStyle(&Theme.TextFont)
							]
						]

						// FOV slider
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(FOVSlider, SSlider)
							.Value(CurrentFOVRatio)
							.Style(&Theme.SliderStyle)
							.OnValueChanged(this, &SFlareSettingsMenu::OnFOVSliderChanged)
						]

						// FOV label
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ValueSize)
							[
								SAssignNew(FOVLabel, STextBlock)
								.TextStyle(&Theme.TextFont)
								.Text(GetFOVLabel(MyGameSettings->VerticalFOV))
							]
						]
					]

					// Gamma box
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Gamma text
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(LabelSize)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("GammaLabel", "Gamma"))
								.TextStyle(&Theme.TextFont)
							]
						]

						// Gamma slider
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(GammaSlider, SSlider)
							.Value(CurrentGammaRatio)
							.Style(&Theme.SliderStyle)
							.OnValueChanged(this, &SFlareSettingsMenu::OnGammaSliderChanged)
						]

						// Gamma label
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ValueSize)
							[
								SAssignNew(GammaLabel, STextBlock)
								.TextStyle(&Theme.TextFont)
								.Text(GetGammaLabel(MyGameSettings->Gamma))
							]
						]
					]

					// Sensitivity box
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Sensitivity text
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(LabelSize)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("SensitivityLabel", "Input sensitivity"))
								.TextStyle(&Theme.TextFont)
							]
						]

						// Sensitivity slider
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(SensitivitySlider, SSlider)
							.Value(CurrentSensitivityRatio)
							.Style(&Theme.SliderStyle)
							.OnValueChanged(this, &SFlareSettingsMenu::OnSensitivitySliderChanged)
						]

						// Sensitivity label
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ValueSize)
							[
								SAssignNew(SensitivityLabel, STextBlock)
								.TextStyle(&Theme.TextFont)
								.Text(GetSensitivityLabel(MyGameSettings->InputSensitivity))
							]
						]
					]
				
					// Ship count level box
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Text
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(LabelSize)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("ShipCountLabel", "Max ships in sector"))
								.TextStyle(&Theme.TextFont)
							]
						]

						// Slider
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(ShipCountSlider, SSlider)
							.Value(MyGameSettings->MaxShipsInSector / 100.0f)
							.Style(&Theme.SliderStyle)
							.OnValueChanged(this, &SFlareSettingsMenu::OnShipCountSliderChanged)
						]

						// Label
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ValueSize)
							[
								SAssignNew(ShipCountLabel, STextBlock)
								.TextStyle(&Theme.TextFont)
								.Text(GetShipCountLabel(MyGameSettings->MaxShipsInSector))
							]
						]
					]
				
					// Master sound level box
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Master volume text
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(LabelSize)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("MasterLabel", "Master volume"))
								.TextStyle(&Theme.TextFont)
							]
						]

						// Master volume slider
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(MasterVolumeSlider, SSlider)
							.Value(MyGameSettings->MasterVolume / 10.0f)
							.Style(&Theme.SliderStyle)
							.OnValueChanged(this, &SFlareSettingsMenu::OnMasterVolumeSliderChanged)
						]

						// Master volume label
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ValueSize)
							[
								SAssignNew(MasterVolumeLabel, STextBlock)
								.TextStyle(&Theme.TextFont)
								.Text(GetMusicVolumeLabel(MyGameSettings->MasterVolume))
							]
						]
					]
				
					// Music level box
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Music volume text
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(LabelSize)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("MusicLabel", "Music volume"))
								.TextStyle(&Theme.TextFont)
							]
						]

						// Music volume slider
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(MusicVolumeSlider, SSlider)
							.Value(MyGameSettings->MusicVolume / 10.0f)
							.Style(&Theme.SliderStyle)
							.OnValueChanged(this, &SFlareSettingsMenu::OnMusicVolumeSliderChanged)
						]

						// Music volume label
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ValueSize)
							[
								SAssignNew(MusicVolumeLabel, STextBlock)
								.TextStyle(&Theme.TextFont)
								.Text(GetMusicVolumeLabel(MyGameSettings->MusicVolume))
							]
						]
					]
				]
			]
		]

		// Graphics
		+ SFlareTabView::Slot()
		.Header(LOCTEXT("GraphicsTab", "Graphics"))
		.HeaderHelp(LOCTEXT("GraphicsTabInfo", "Tweak the graphics settings"))
		[
			SNew(SVerticalBox)
				
			// Title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.TitlePadding)
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("GraphicsSettingsHint", "Graphics"))
				.TextStyle(&Theme.SubTitleFont)
			]

			// Graphic form
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			[
				SNew(SBox)
				.WidthOverride(Theme.ContentWidth)
				.HAlign(HAlign_Fill)
				[
					SNew(SVerticalBox)

					// Resolution
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SAssignNew(ResolutionSelector, SFlareDropList<TSharedPtr<FScreenResolutionRHI>>)
							.OptionsSource(&ResolutionList)
							.OnGenerateWidget(this, &SFlareSettingsMenu::OnGenerateResolutionComboLine)
							.OnSelectionChanged(this, &SFlareSettingsMenu::OnResolutionComboLineSelectionChanged)
							.HeaderWidth(10.1)
							.ItemWidth(10.1)
							[
								SNew(SBox)
								.Padding(Theme.ListContentPadding)
								[
									SNew(STextBlock)
									.Text(this, &SFlareSettingsMenu::OnGetCurrentResolutionComboLine)
									.TextStyle(&Theme.TextFont)
								]
							]
						]
					]

					// Buttons
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Left)
					[
						SNew(SHorizontalBox)
							
						// Fullscreen
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SAssignNew(FullscreenButton, SFlareButton)
							.Text(LOCTEXT("Fullscreen", "Fullscreen"))
							.HelpText(LOCTEXT("FullscreenInfo", "Show the game in full screen"))
							.Toggle(true)
							.OnClicked(this, &SFlareSettingsMenu::OnFullscreenToggle)
						]

						// VSync
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SAssignNew(VSyncButton, SFlareButton)
							.Text(LOCTEXT("V-sync", "V-Sync"))
							.HelpText(LOCTEXT("VSyncInfo", "Vertical synchronization ensures that every image is consistent, even with low performance."))
							.Toggle(true)
							.OnClicked(this, &SFlareSettingsMenu::OnVSyncToggle)
						]
					]
					

					// Buttons 2
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Left)
					[
						SNew(SHorizontalBox)

						// Motion blur
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SAssignNew(MotionBlurButton, SFlareButton)
							.Text(LOCTEXT("MotionBlur", "Use motion blur"))
							.HelpText(LOCTEXT("MotionBlurInfo", "Motion blur makes the game feel much more responsive and fluid."))
							.Toggle(true)
							.OnClicked(this, &SFlareSettingsMenu::OnMotionBlurToggle)
						]

						// Temporal AA
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SAssignNew(TemporalAAButton, SFlareButton)
							.Text(LOCTEXT("TemporalAA", "Use Temporal AA"))
							.HelpText(LOCTEXT("TemporalAAInfo", "Temporal Anti-Aliasing is a cleaner AA method, but might create ghosting artifacts on some computers."))
							.Toggle(true)
							.OnClicked(this, &SFlareSettingsMenu::OnTemporalAAToggle)
						]

						// Supersampling
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SAssignNew(SupersamplingButton, SFlareButton)
							.Text(LOCTEXT("Supersampling", "2x Supersampling (!)"))
							.HelpText(LOCTEXT("SupersamplingInfo", "Supersampling will render the scenes at double the resolution. This is a very demanding feature."))
							.Toggle(true)
							.OnClicked(this, &SFlareSettingsMenu::OnSupersamplingToggle)
						]
					]

					// Texture quality box
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Texture quality text
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(LabelSize)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("TextureLabel", "Texture quality"))
								.TextStyle(&Theme.TextFont)
							]
						]

						// Texture quality slider
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(TextureQualitySlider, SSlider)
							.Value(CurrentTextureQualityRatio)
							.Style(&Theme.SliderStyle)
							.OnValueChanged(this, &SFlareSettingsMenu::OnTextureQualitySliderChanged)
						]

						// Texture quality label
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ValueSize)
							[
								SAssignNew(TextureQualityLabel, STextBlock)
								.TextStyle(&Theme.TextFont)
								.Text(GetTextureQualityLabel(MyGameSettings->ScalabilityQuality.TextureQuality))
							]
						]
					]

					// Effets quality box
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Effects quality text
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(LabelSize)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("EffectsLabel", "Effects quality"))
								.TextStyle(&Theme.TextFont)
							]
						]

						// Effects quality slider
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(EffectsQualitySlider, SSlider)
							.Value(CurrentEffectsQualityRatio)
							.Style(&Theme.SliderStyle)
							.OnValueChanged(this, &SFlareSettingsMenu::OnEffectsQualitySliderChanged)
						]

						// Effects quality label
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ValueSize)
							[
								SAssignNew(EffectsQualityLabel, STextBlock)
								.TextStyle(&Theme.TextFont)
								.Text(GetEffectsQualityLabel(MyGameSettings->ScalabilityQuality.EffectsQuality))
							]
						]
					]

					// AntiAliasing quality box
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Anti aliasing quality text
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(LabelSize)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("AntiAliasingLabel", "Anti-aliasing quality"))
								.TextStyle(&Theme.TextFont)
							]
						]

						// AntiAliasing quality slider
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(AntiAliasingQualitySlider, SSlider)
							.Value(CurrentAntiAliasingQualityRatio)
							.Style(&Theme.SliderStyle)
							.OnValueChanged(this, &SFlareSettingsMenu::OnAntiAliasingQualitySliderChanged)
						]

						// AntiAliasing quality label
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ValueSize)
							[
								SAssignNew(AntiAliasingQualityLabel, STextBlock)
								.TextStyle(&Theme.TextFont)
								.Text(GetAntiAliasingQualityLabel(MyGameSettings->ScalabilityQuality.AntiAliasingQuality))
							]
						]
					]

					// PostProcess quality box
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// PostProcess quality text
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(LabelSize)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("PostProcessLabel", "Post-process quality"))
								.TextStyle(&Theme.TextFont)
							]
						]

						// PostProcess quality slider
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(PostProcessQualitySlider, SSlider)
							.Value(CurrentPostProcessQualityRatio)
							.Style(&Theme.SliderStyle)
							.OnValueChanged(this, &SFlareSettingsMenu::OnPostProcessQualitySliderChanged)
						]

						// PostProcess quality label
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ValueSize)
							[
								SAssignNew(PostProcessQualityLabel, STextBlock)
								.TextStyle(&Theme.TextFont)
								.Text(GetPostProcessQualityLabel(MyGameSettings->ScalabilityQuality.PostProcessQuality))
							]
						]
					]
				]
			]
		]
			
		// Keyboard
		+ SFlareTabView::Slot()
		.Header(LOCTEXT("KeyboardTab", "Controls"))
		.HeaderHelp(LOCTEXT("KeyboardTabInfo", "Change your key bindings"))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.TitlePadding)
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ControlsSettingsHint", "Controls"))
				.TextStyle(&Theme.SubTitleFont)
			]
			
			// Keyboard hint
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("KeyboardBindingsHint", "The left mouse button is always used to fire, but you can add another binding."))
				.TextStyle(&Theme.TextFont)
			]

			// Controls form
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.ContentPadding)
			.HAlign(HAlign_Fill)
			[
				BuildKeyBindingBox()
			]
		]

		// Gamepad and joystick
		+ SFlareTabView::Slot()
		.Header(LOCTEXT("GamepadJoystickTab", "Gamepad / Joystick"))
		.HeaderHelp(LOCTEXT("GamepadJoystickTabInfo", "Configure your gamepad or joystick"))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.TitlePadding)
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ControlsSettings2Hint", "Controls"))
				.TextStyle(&Theme.SubTitleFont)
			]

			// Gamepad
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.TitlePadding)
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("GamepadSettingsHint", "GAMEPAD"))
				.TextStyle(&Theme.NameFont)
			]
			
			// Gamepad
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.TitlePadding)
			.HAlign(HAlign_Left)
			[
				SNew(SImage)
				.Image(this, &SFlareSettingsMenu::GetGamepadDrawing)
			]

			// Gamepad options
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.ContentPadding)
			.HAlign(HAlign_Left)
			[
				SNew(SHorizontalBox)

				// Profile selector
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(Theme.SmallContentPadding)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					[
						SAssignNew(GamepadSelector, SFlareDropList<TSharedPtr<FText>>)
						.OptionsSource(&GamepadList)
						.OnGenerateWidget(this, &SFlareSettingsMenu::OnGenerateGamepadComboLine)
						.OnSelectionChanged(this, &SFlareSettingsMenu::OnGamepadComboLineSelectionChanged)
						.HeaderWidth(10.1)
						.ItemWidth(10.1)
						[
							SNew(SBox)
							.Padding(Theme.ListContentPadding)
							[
								SNew(STextBlock)
								.Text(this, &SFlareSettingsMenu::OnGetCurrentGamepadComboLine)
								.TextStyle(&Theme.TextFont)
							]
						]
					]
				]
			]

			// Joystick
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.TitlePadding)
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("JoystickSettingsHint", "JOYSTICK"))
				.TextStyle(&Theme.NameFont)
			]

			// Controls form
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.ContentPadding)
			.HAlign(HAlign_Fill)
			[
				BuildJoystickBindingBox()
			]

			// Joystick options
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.ContentPadding)
			.HAlign(HAlign_Left)
			[
				SNew(SHorizontalBox)
							
				// Forward only
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(Theme.SmallContentPadding)
				[
					SAssignNew(ForwardOnlyThrustButton, SFlareButton)
					.Text(LOCTEXT("ForwardOnlyThrust", "Forward-only thrust"))
					.HelpText(LOCTEXT("ForwardOnlyThrustInfo", "Prevent the thrust control on the joystick from creating backward thrust"))
					.Toggle(true)
					.OnClicked(this, &SFlareSettingsMenu::OnForwardOnlyThrustToggle)
				]
			]
			
			// Joystick dead zone (rotation)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			[
				SNew(SBox)
				.WidthOverride(Theme.ContentWidth)
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					// Text
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.ContentPadding)
					[
						SNew(SBox)
						.WidthOverride(LabelSize)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("JoystickRotationDeadZone", "Rotation dead zone"))
							.TextStyle(&Theme.TextFont)
						]
					]

					// Slider
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.Padding(Theme.ContentPadding)
					[
						SNew(SSlider)
						.Value(MyGameSettings->RotationDeadZone)
						.Style(&Theme.SliderStyle)
						.OnValueChanged(this, &SFlareSettingsMenu::OnRotationDeadZoneSliderChanged)
					]
				]
			]
			
			// Joystick dead zone (roll)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			[
				SNew(SBox)
				.WidthOverride(Theme.ContentWidth)
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					// Text
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.ContentPadding)
					[
						SNew(SBox)
						.WidthOverride(LabelSize)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("JoystickRollDeadZone", "Roll dead zone"))
							.TextStyle(&Theme.TextFont)
						]
					]

					// Slider
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.Padding(Theme.ContentPadding)
					[
						SNew(SSlider)
						.Value(MyGameSettings->RollDeadZone)
						.Style(&Theme.SliderStyle)
						.OnValueChanged(this, &SFlareSettingsMenu::OnRollDeadZoneSliderChanged)
					]
				]
			]
			
			// Joystick dead zone (translation)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			[
				SNew(SBox)
				.WidthOverride(Theme.ContentWidth)
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					// Text
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.ContentPadding)
					[
						SNew(SBox)
						.WidthOverride(LabelSize)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("JoystickTranslationDeadZone", "Translation dead zone"))
							.TextStyle(&Theme.TextFont)
						]
					]

					// Slider
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.Padding(Theme.ContentPadding)
					[
						SNew(SSlider)
						.Value(MyGameSettings->TranslationDeadZone)
						.Style(&Theme.SliderStyle)
						.OnValueChanged(this, &SFlareSettingsMenu::OnTranslationDeadZoneSliderChanged)
					]
				]
			]
		]
	];

	// Default settings
	ForwardOnlyThrustButton->SetActive(MyGameSettings->ForwardOnlyThrust);
	VSyncButton->SetActive(MyGameSettings->IsVSyncEnabled());
	MotionBlurButton->SetActive(MyGameSettings->UseMotionBlur);
	TemporalAAButton->SetActive(MyGameSettings->UseTemporalAA);
	FullscreenButton->SetActive(MyGameSettings->GetFullscreenMode() == EWindowMode::Fullscreen);
	SupersamplingButton->SetActive(MyGameSettings->ScreenPercentage > 100);

	// Gameplay defaults
	InvertYButton->SetActive(MyGameSettings->InvertY);
	DisableMouseButton->SetActive(MyGameSettings->DisableMouse);
#if !UE_BUILD_SHIPPING
	CockpitButton->SetActive(MyGameSettings->UseCockpit);
#endif
	LateralVelocityButton->SetActive(MyGameSettings->ShowLateralVelocity);
	AnticollisionButton->SetActive(MyGameSettings->UseAnticollision);

	float MaxShipRatio = ((float) MyGameSettings->MaxShipsInSector - MIN_MAX_SHIPS) / ((float) MAX_MAX_SHIPS - MIN_MAX_SHIPS);

	// Music volume
	int32 MusicVolume = MyGameSettings->MusicVolume;
	int32 MasterVolume = MyGameSettings->MasterVolume;
	ShipCountSlider->SetValue(MaxShipRatio);
	ShipCountLabel->SetText(GetShipCountLabel(MyGameSettings->MaxShipsInSector));
	MusicVolumeSlider->SetValue((float)MusicVolume / 10.0f);
	MusicVolumeLabel->SetText(GetMusicVolumeLabel(MusicVolume));
	MasterVolumeSlider->SetValue((float)MasterVolume / 10.0f);
	MasterVolumeLabel->SetText(GetMasterVolumeLabel(MasterVolume));
}

TSharedRef<SWidget> SFlareSettingsMenu::BuildKeyBindingBox()
{
	// Get data
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	TSharedPtr<SVerticalBox> KeyboardBox;
	SAssignNew(KeyboardBox, SVerticalBox)
	
	// Key bind list
	+ SVerticalBox::Slot()
	.AutoHeight()
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)

		// Left key pane (flying)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		[
			SNew(SBox)
			.WidthOverride(Theme.ContentWidth)
			[
				SAssignNew(ControlListLeft, SVerticalBox)
			]
		]

		// Right key pane (strategy & menus)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		.Padding(FMargin(50, 0, 0, 0))
		[
			SNew(SBox)
			.WidthOverride(Theme.ContentWidth)
			[
				SAssignNew(ControlListRight, SVerticalBox)
			]
		]
	];

	// Add contents
	if (KeyboardBox.IsValid())
	{
		BuildKeyBindingPane(Binds, ControlListLeft);
		BuildKeyBindingPane(Binds2, ControlListRight);
	}
	return KeyboardBox.ToSharedRef();
}

void SFlareSettingsMenu::BuildKeyBindingPane(TArray<TSharedPtr<FSimpleBind> >& BindList, TSharedPtr<SVerticalBox>& Form)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	for (const auto& Bind : BindList)
	{
		// Header block
		if (Bind->bHeader)
		{
			Form->AddSlot()
			.AutoHeight()
			.Padding(FMargin(0, 20, 0, 10))
			[
				SNew(SHorizontalBox)

				// Title
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.Padding(Theme.SmallContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.NameFont)
					.Text(Bind->DisplayName)
				]

				// Key 1
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
					.Padding(Theme.SmallContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.SmallFont)
					.Text(LOCTEXT("ControlSettingsKeyBinds", "Key"))
				]

				// Key 2
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Padding(Theme.SmallContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.SmallFont)
					.Text(LOCTEXT("ControlSettingsAlternateKeyBinds", "Alternate Key"))
				]
			];
		}
		else
		{
			Form->AddSlot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// Action label
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.Padding(Theme.SmallContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(Bind->DisplayName)
				]

				// Key 1
				+ SHorizontalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				[
					SAssignNew(Bind->KeyWidget, SFlareKeyBind)
					.PC(MenuManager->GetPC())
					.Key(Bind->Key)
					.DefaultKey(Bind->DefaultKey)
					.OnKeyBindingChanged( this, &SFlareSettingsMenu::OnKeyBindingChanged, Bind, true)
				]

				// Key 2
				+ SHorizontalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				[
					SAssignNew(Bind->AltKeyWidget, SFlareKeyBind)
					.PC(MenuManager->GetPC())
					.Key(Bind->AltKey)
					.DefaultKey(Bind->DefaultAltKey)
					.OnKeyBindingChanged( this, &SFlareSettingsMenu::OnKeyBindingChanged, Bind, false)
				]
			];
		}
	}
}

TSharedRef<SWidget> SFlareSettingsMenu::BuildJoystickBindingBox()
{
	// Get data
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	TSharedPtr<SVerticalBox> JoystickBindingsBox;
	TSharedPtr<SVerticalBox> JoystickBox;

	// Create box
	SAssignNew(JoystickBox, SVerticalBox)
	+ SVerticalBox::Slot()
	.AutoHeight()
	.HAlign(HAlign_Left)
	[
		SNew(SBox)
		.WidthOverride(1.5 * Theme.ContentWidth)
		.HAlign(HAlign_Fill)
		[
			SAssignNew(JoystickBindingsBox, SVerticalBox)
		]
	];
	
	// Add bindings
	BuildJoystickBinding(LOCTEXT("JoyBindYaw",        "Yaw"),        "JoystickYawInput",            JoystickBindingsBox);
	BuildJoystickBinding(LOCTEXT("JoyBindPitch",      "Pitch"),      "JoystickPitchInput",          JoystickBindingsBox);
	BuildJoystickBinding(LOCTEXT("JoyBindRoll",       "Roll"),       "JoystickRollInput",           JoystickBindingsBox);
	BuildJoystickBinding(LOCTEXT("JoyBindThrust",     "Thrust"),     "JoystickThrustInput",         JoystickBindingsBox);
	BuildJoystickBinding(LOCTEXT("JoyBindHorizontal", "Horizontal"), "JoystickMoveHorizontalInput", JoystickBindingsBox);
	BuildJoystickBinding(LOCTEXT("JoyBindVertical",   "Vertical"),   "JoystickMoveVerticalInput",   JoystickBindingsBox);

	return JoystickBox.ToSharedRef();
}

void SFlareSettingsMenu::BuildJoystickBinding(FText AxisDisplayName, FName AxisName, TSharedPtr<SVerticalBox>& Form)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	int32 LabelSize = Theme.ContentWidth / 3 - Theme.SmallContentPadding.Left - Theme.SmallContentPadding.Right;

	Form->AddSlot()
	.AutoHeight()
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)

		// Label
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		.Padding(Theme.SmallContentPadding)
		[
			SNew(SBox)
			.WidthOverride(LabelSize)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TextFont)
				.Text(AxisDisplayName)
			]
		]

		// Combo box for selection
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.Padding(Theme.SmallContentPadding)
		[
			SNew(SComboBox<TSharedPtr<FKey>>)
			.OptionsSource(&JoystickAxisKeys)
			.OnGenerateWidget(this, &SFlareSettingsMenu::OnGenerateJoystickComboLine, AxisName)
			.OnSelectionChanged(this, &SFlareSettingsMenu::OnJoystickComboLineSelectionChanged, AxisName)
			.ComboBoxStyle(&Theme.ComboBoxStyle)
			.ForegroundColor(FLinearColor::White)
			[
				SNew(STextBlock)
				.Text(this, &SFlareSettingsMenu::OnGetCurrentJoystickKeyName, AxisName)
				.TextStyle(&Theme.TextFont)
			]
		]

		// Inversion switch
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.Padding(Theme.SmallContentPadding)
		[
			SNew(SFlareButton)
			.Width(3)
			.Text(LOCTEXT("InvertAxisDirection", "Invert"))
			.HelpText(LOCTEXT("InvertAxisDirectionInfo", "Toggle the axis inversion to reverse the joystick axis."))
			.OnClicked(this, &SFlareSettingsMenu::OnInvertAxisClicked, AxisName)
			.IsDisabled(this, &SFlareSettingsMenu::IsAxisControlsDisabled, AxisName)
		]

		// Unbind
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.Padding(Theme.SmallContentPadding)
		[
			SNew(SFlareButton)
			.Width(3)
			.Text(LOCTEXT("UnbindAxis", "Unbind"))
			.HelpText(LOCTEXT("UnbindAxisInfo", "Remove this control binding."))
			.OnClicked(this, &SFlareSettingsMenu::OnUnbindAxisClicked, AxisName)
			.IsDisabled(this, &SFlareSettingsMenu::IsAxisControlsDisabled, AxisName)
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareSettingsMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);

	// Get current culture
	FString CurrentCultureString = GConfig->GetStr(TEXT("Internationalization"), TEXT("NativeGameCulture"), GGameUserSettingsIni);
	if (CurrentCultureString.Len() == 0)
	{
		FLOGV("SFlareSettingsMenu::Setup : no user-set culture, using OS default", *CurrentCultureString);
		CurrentCultureString = FInternationalization::Get().GetCurrentCulture().Get().GetName();
	}
	FLOGV("SFlareSettingsMenu::Setup : current culture is '%s'", *CurrentCultureString);
	FInternationalization::Get().SetCurrentCulture(CurrentCultureString);
}

void SFlareSettingsMenu::Enter()
{
	// Main
	FLOG("SFlareSettingsMenu::Enter");
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	AFlarePlayerController* PC = MenuManager->GetPC();

	// Get decorator mesh
	if (PC)
	{
		const FFlareSpacecraftComponentDescription* PartDesc = PC->GetGame()->GetShipPartsCatalog()->Get("object-safe");
		PC->GetMenuPawn()->SetCameraOffset(FVector2D(150, 30));
		PC->GetMenuPawn()->ShowPart(PartDesc);
		PC->GetMenuPawn()->SetCameraDistance(500);
	}

	// Get a list of joystick axis keys
	FillJoystickAxisList();

	// Get a list of resolutions
	FillResolutionList();

	// Fill gamepad profile list
	FillGameGamepadList();

	// Set list of cultures to the current one
	CultureSelector->RefreshOptions();
	FString CurrentCultureString = FInternationalization::Get().GetCurrentCulture().Get().GetName();
	for (TSharedPtr<FString> Culture : CultureList)
	{
		if (*Culture.Get() == CurrentCultureString)
		{
			CultureSelector->SetSelectedItem(Culture);
			break;
		}
	}
}

void SFlareSettingsMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FText SFlareSettingsMenu::OnGetCurrentCultureComboLine() const
{
	TSharedPtr<FString> Item = CultureSelector->GetSelectedItem();

	if(!Item.IsValid())
	{
		 return FText::FromString("");
	}

	FCulturePtr Culture = FInternationalization::Get().GetCulture(*Item);
	FString NativeName = Culture->GetNativeName();
	NativeName = NativeName.Left(1).ToUpper() + NativeName.RightChop(1); // FString needed here

	return FText::FromString(NativeName);
}

TSharedRef<SWidget> SFlareSettingsMenu::OnGenerateCultureComboLine(TSharedPtr<FString> Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	FCulturePtr Culture = FInternationalization::Get().GetCulture(*Item);
	FString NativeName = Culture->GetNativeName();
	NativeName = NativeName.Left(1).ToUpper() + NativeName.RightChop(1); // FString needed here

	return SNew(SBox)
	.Padding(Theme.ListContentPadding)
	[
		SNew(STextBlock)
		.Text(FText::FromString(NativeName))
		.TextStyle(&Theme.TextFont)
	];
}

void SFlareSettingsMenu::OnCultureComboLineSelectionChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo)
{
	FInternationalization::Get().SetCurrentCulture(*Item.Get());

	GConfig->SetString(TEXT("Internationalization"), TEXT("NativeGameCulture"), **Item.Get(), GGameUserSettingsIni);
	GConfig->Flush(false, GEditorSettingsIni);
	FTextLocalizationManager::Get().RefreshResources();
}

FText SFlareSettingsMenu::OnGetCurrentResolutionComboLine() const
{
	TSharedPtr<FScreenResolutionRHI> Item = ResolutionSelector->GetSelectedItem();
	return Item.IsValid() ? FText::FromString(FString::Printf(TEXT("%dx%d"), Item->Width, Item->Height)) : FText::FromString("");
}

TSharedRef<SWidget> SFlareSettingsMenu::OnGenerateResolutionComboLine(TSharedPtr<FScreenResolutionRHI> Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(SBox)
	.Padding(Theme.ListContentPadding)
	[
		SNew(STextBlock)
		.Text(FText::FromString(FString::Printf(TEXT("%dx%d"), Item->Width, Item->Height)))
		.TextStyle(&Theme.TextFont)
	];
}

void SFlareSettingsMenu::OnResolutionComboLineSelectionChanged(TSharedPtr<FScreenResolutionRHI> StringItem, ESelectInfo::Type SelectInfo)
{
	UpdateResolution(false);
}

FText SFlareSettingsMenu::OnGetCurrentJoystickKeyName(FName AxisName) const
{
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
	for (auto& Bind : InputSettings->AxisMappings)
	{
		if (Bind.AxisName == AxisName)
		{
			FText InvertedText = (Bind.Scale < 0) ? LOCTEXT("AxisInvertedHint", "(Inverted)") : FText();
			FString DisplayName = Bind.Key.GetDisplayName().ToString();
			DisplayName = DisplayName.Replace(TEXT("Joystick_"), TEXT(""));
			DisplayName = DisplayName.Replace(TEXT("_"), TEXT(" "));
			return FText::Format(FText::FromString("{0} {1}"), FText::FromString(DisplayName), InvertedText);
		}
	}

	return FText();
}

TSharedRef<SWidget> SFlareSettingsMenu::OnGenerateJoystickComboLine(TSharedPtr<FKey> Item, FName AxisName)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	FString DisplayName = Item->GetDisplayName().ToString();
	DisplayName = DisplayName.Replace(TEXT("Joystick_"), TEXT(""));
	DisplayName = DisplayName.Replace(TEXT("_"), TEXT(" "));

	return SNew(STextBlock)
		.Text(FText::FromString(DisplayName))
		.TextStyle(&Theme.TextFont);
}

void SFlareSettingsMenu::OnJoystickComboLineSelectionChanged(TSharedPtr<FKey> KeyItem, ESelectInfo::Type SelectInfo, FName AxisName)
{
	if (KeyItem.IsValid())
	{
		FLOGV("SFlareSettingsMenu::OnJoystickComboLineSelectionChanged : '%s' bound to '%s'",
			*AxisName.ToString(), *KeyItem->GetDisplayName().ToString());

		// Remove the original binding
		UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
		for (int i = 0; i < InputSettings->AxisMappings.Num();)
		{
			if (InputSettings->AxisMappings[i].AxisName == AxisName)
			{
				InputSettings->RemoveAxisMapping(InputSettings->AxisMappings[i]);
			}
			else
			{
				i++;
			}
		}

		// Add new binding
		FInputAxisKeyMapping Bind;
		Bind.AxisName = AxisName;
		Bind.Key = *KeyItem;
		Bind.Scale = 1.0f;
		InputSettings->AddAxisMapping(Bind);
		InputSettings->SaveKeyMappings();
	}
}

FText SFlareSettingsMenu::OnGetCurrentGamepadComboLine() const
{
	return FText::FromString(GamepadList[GamepadSelector->GetSelectedIndex()]->ToString());
}

TSharedRef<SWidget> SFlareSettingsMenu::OnGenerateGamepadComboLine(TSharedPtr<FText> Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	FText Content = FText::FromString(Item->ToString());

	return SNew(STextBlock)
		.Text(Content)
		.TextStyle(&Theme.TextFont);
}

void SFlareSettingsMenu::OnGamepadComboLineSelectionChanged(TSharedPtr<FText> KeyItem, ESelectInfo::Type SelectInfo)
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());

	int32 Index = GamepadList.Find(KeyItem);
	MyGameSettings->GamepadProfileLayout = Index != INDEX_NONE ? Index : EFlareGamepadLayout::GL_Default;
	MyGameSettings->ApplySettings(false);

	MenuManager->GetPC()->SetupGamepad();
}


/*----------------------------------------------------
	Sliders
----------------------------------------------------*/

void SFlareSettingsMenu::OnShipCountSliderChanged(float Value)
{
	float NotSteppedValue = Value * (MAX_MAX_SHIPS - MIN_MAX_SHIPS) + MIN_MAX_SHIPS;
	int32 NewMaxShipValue = FMath::RoundToInt(NotSteppedValue / STEP_MAX_SHIPS) * STEP_MAX_SHIPS;
	float SteppedRatio = ((float) NewMaxShipValue - MIN_MAX_SHIPS) / ((float) MAX_MAX_SHIPS - MIN_MAX_SHIPS);

	ShipCountSlider->SetValue(SteppedRatio);

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	if (MyGameSettings->MaxShipsInSector != NewMaxShipValue)
	{
		FLOGV("SFlareSettingsMenu::OnShipCountSliderChanged : Set max ship count to %d (current is %d)", NewMaxShipValue, MyGameSettings->MaxShipsInSector);
		MyGameSettings->MaxShipsInSector = NewMaxShipValue;
		MyGameSettings->ApplySettings(false);
		ShipCountLabel->SetText(GetShipCountLabel(NewMaxShipValue));
	}
}

FText SFlareSettingsMenu::GetShipCountLabel(int32 Value) const
{
	return FText::AsNumber(Value);
}

void SFlareSettingsMenu::OnFOVSliderChanged(float Value)
{
	int32 Step = MenuManager->GetPC()->GetMaxVerticalFOV() - MenuManager->GetPC()->GetMinVerticalFOV();
	int32 StepValue = FMath::RoundToInt(Step * Value);
	FOVSlider->SetValue((float)StepValue / (float)Step);
	StepValue += MenuManager->GetPC()->GetMinVerticalFOV();

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	if (MyGameSettings->VerticalFOV != StepValue)
	{
		FLOGV("SFlareSettingsMenu::OnFOVSliderChanged : Set fov to %d (current is %d)", StepValue, MyGameSettings->VerticalFOV);
		MyGameSettings->VerticalFOV = StepValue;
		MyGameSettings->ApplySettings(false);
		FOVLabel->SetText(GetFOVLabel(StepValue));
	}
}

FText SFlareSettingsMenu::GetFOVLabel(int32 Value) const
{
	return FText::Format(LOCTEXT("FOVFormat", "{0}"), FText::AsNumber(Value));
}

void SFlareSettingsMenu::OnGammaSliderChanged(float Value)
{
	float Step = 1 / 0.1f;
	float NewValue = MIN_GAMMA + Value * (MAX_GAMMA - MIN_GAMMA);
	int SteppedNewValue = FMath::RoundToInt(Step * NewValue);
	NewValue = SteppedNewValue / Step;

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	if (MyGameSettings->Gamma != NewValue)
	{
		FLOGV("SFlareSettingsMenu::OnGammaSliderChanged : Set gamma to %f (current is %f)", NewValue, MyGameSettings->Gamma);
		MyGameSettings->Gamma = NewValue;
		MyGameSettings->ApplySettings(false);

		FString GammaCommand = FString::Printf(TEXT("gamma %f"), NewValue);
		GEngine->GameViewport->ConsoleCommand(GammaCommand);

		GammaLabel->SetText(GetGammaLabel(NewValue));
	}
}

FText SFlareSettingsMenu::GetGammaLabel(float Value) const
{
	return FText::Format(LOCTEXT("GammaFormat", "{0}"), FText::AsNumber(Value));
}

void SFlareSettingsMenu::OnSensitivitySliderChanged(float Value)
{
	float Step = 1 / 0.1f;
	float NewValue = MIN_SENSITIVITY + Value * (MAX_SENSITIVITY - MIN_SENSITIVITY);
	int SteppedNewValue = FMath::RoundToInt(Step * NewValue);
	NewValue = SteppedNewValue / Step;

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	if (MyGameSettings->InputSensitivity != NewValue)
	{
		FLOGV("SFlareSettingsMenu::OnSensitivitySliderChanged : Set sensitivity to %f (current is %f)", NewValue, MyGameSettings->InputSensitivity);
		MyGameSettings->InputSensitivity = NewValue;
		MyGameSettings->ApplySettings(false);
		SensitivityLabel->SetText(GetSensitivityLabel(NewValue));
	}
}

FText SFlareSettingsMenu::GetSensitivityLabel(float Value) const
{
	return FText::Format(LOCTEXT("SensitivityFormat", "{0}"), FText::AsNumber(Value));
}

void SFlareSettingsMenu::OnTextureQualitySliderChanged(float Value)
{
	int32 Step = 3;
	int32 StepValue = FMath::RoundToInt(Step * Value);
	TextureQualitySlider->SetValue((float)StepValue / (float)Step);

	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();

	if (MyGameSettings->ScalabilityQuality.TextureQuality != StepValue)
	{
		FLOGV("SFlareSettingsMenu::OnTextureQualitySliderChanged : Set Texture quality to %d (current is %d)", StepValue, MyGameSettings->ScalabilityQuality.TextureQuality);
		MyGameSettings->ScalabilityQuality.TextureQuality = StepValue;
		MyGameSettings->ApplySettings(false);
		TextureQualityLabel->SetText(GetTextureQualityLabel(StepValue));
	}
}

FText SFlareSettingsMenu::GetTextureQualityLabel(int32 Value) const
{
	switch(Value)
	{
		case 1:
			return LOCTEXT("TextureQualityMedium", "Medium");
		case 2:
			return LOCTEXT("TextureQualityHigh", "High");
		case 3:
			return LOCTEXT("TextureQualityUltra", "Ultra");
		case 0:
		default:
			return LOCTEXT("TextureQualityLow", "Low");
	}
}

void SFlareSettingsMenu::OnEffectsQualitySliderChanged(float Value)
{
	int32 Step = 3;
	int32 StepValue = FMath::RoundToInt(Step * Value);
	EffectsQualitySlider->SetValue((float)StepValue / (float)Step);

	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();

	if (MyGameSettings->ScalabilityQuality.EffectsQuality != StepValue)
	{
		FLOGV("SFlareSettingsMenu::OnEffectsQualitySliderChanged : Set Effects quality to %d (current is %d)", StepValue, MyGameSettings->ScalabilityQuality.EffectsQuality);
		MyGameSettings->ScalabilityQuality.EffectsQuality = StepValue;
		MyGameSettings->ApplySettings(false);
		EffectsQualityLabel->SetText(GetEffectsQualityLabel(StepValue));
	}
}

FText SFlareSettingsMenu::GetEffectsQualityLabel(int32 Value) const
{
	switch(Value)
	{
		case 1:
			return LOCTEXT("EffectsQualityMedium", "Medium");
		case 2:
			return LOCTEXT("EffectsQualityHigh", "High");
		case 3:
			return LOCTEXT("EffectsQualityUltra", "Ultra");
		case 0:
		default:
			return LOCTEXT("EffectsQualityLow", "Low");
	}
}

void SFlareSettingsMenu::OnAntiAliasingQualitySliderChanged(float Value)
{
	int32 Step = 2;
	int32 StepValue = FMath::RoundToInt(Step * Value);
	AntiAliasingQualitySlider->SetValue((float)StepValue / (float)Step);

	int32 AAValue = StepValue;
	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();

	if (MyGameSettings->ScalabilityQuality.AntiAliasingQuality != AAValue)
	{
		FLOGV("SFlareSettingsMenu::OnAntiAliasingQualitySliderChanged : set AntiAliasing quality to %d (current is %d)", AAValue, MyGameSettings->ScalabilityQuality.AntiAliasingQuality);
		MyGameSettings->ScalabilityQuality.AntiAliasingQuality = AAValue;
		MyGameSettings->ApplySettings(false);
		AntiAliasingQualityLabel->SetText(GetAntiAliasingQualityLabel(AAValue));
	}
}

FText SFlareSettingsMenu::GetAntiAliasingQualityLabel(int32 Value) const
{
	switch(Value)
	{
		case 1:
			return LOCTEXT("AntiAliasingQualityMedium", "Medium");
		case 2:
		case 3:
			return LOCTEXT("AntiAliasingQualityUltra", "Ultra");
		case 0:
		default:
			return LOCTEXT("AntiAliasingQualityDisabled", "Off");		
	}
}

void SFlareSettingsMenu::OnPostProcessQualitySliderChanged(float Value)
{
	int32 Step = 3;
	int32 StepValue = FMath::RoundToInt(Step * Value);
	PostProcessQualitySlider->SetValue((float)StepValue / (float)Step);

	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();

	if (MyGameSettings->ScalabilityQuality.PostProcessQuality != StepValue)
	{
		FLOGV("SFlareSettingsMenu::OnAntiAliasingQualitySliderChanged : set PostProcess quality to %d (current is %d)", StepValue, MyGameSettings->ScalabilityQuality.PostProcessQuality);
		MyGameSettings->ScalabilityQuality.PostProcessQuality = StepValue;
		MyGameSettings->ApplySettings(false);
		PostProcessQualityLabel->SetText(GetPostProcessQualityLabel(StepValue));
	}
}

FText SFlareSettingsMenu::GetPostProcessQualityLabel(int32 Value) const
{
	switch(Value)
	{
		case 1:
			return LOCTEXT("PostProcessQualityMedium", "Medium");
		case 2:
			return LOCTEXT("PostProcessQualityHigh", "High");
		case 3:
			return LOCTEXT("PostProcessQualityUltra", "Ultra");
		case 0:
		default:
			return LOCTEXT("PostProcessQualityLow", "Low");
	}
}

void SFlareSettingsMenu::OnMusicVolumeSliderChanged(float Value)
{
	int32 Step = 10;
	int32 StepValue = FMath::RoundToInt(Step * Value);
	MusicVolumeSlider->SetValue((float)StepValue / (float)Step);

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	AFlarePlayerController* PC = MenuManager->GetPC();
	int32 CurrentVolume = MyGameSettings->MusicVolume;
	
	if (PC && CurrentVolume != StepValue)
	{
		FLOGV("SFlareSettingsMenu::OnMusicVolumeSliderChanged : set music volume to %d (current is %d)", StepValue, CurrentVolume);
		MusicVolumeLabel->SetText(GetMusicVolumeLabel(StepValue));
		MyGameSettings->MusicVolume = StepValue;
		MyGameSettings->ApplySettings(false);
		PC->SetMusicVolume(StepValue);
	}
}

FText SFlareSettingsMenu::GetMusicVolumeLabel(int32 Value) const
{
	if (Value == 0)
		return LOCTEXT("Muted", "Muted");
	else
		return FText::Format(LOCTEXT("MusicVolumeFormat", "{0}%"), FText::AsNumber(10 * Value));
}

void SFlareSettingsMenu::OnMasterVolumeSliderChanged(float Value)
{
	int32 Step = 10;
	int32 StepValue = FMath::RoundToInt(Step * Value);
	MasterVolumeSlider->SetValue((float)StepValue / (float)Step);

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	AFlarePlayerController* PC = MenuManager->GetPC();
	int32 CurrentVolume = MyGameSettings->MasterVolume;

	if (PC && CurrentVolume != StepValue)
	{
		FLOGV("SFlareSettingsMenu::OnMasterVolumeSliderChanged : set music volume to %d (current is %d)", StepValue, CurrentVolume);
		MasterVolumeLabel->SetText(GetMasterVolumeLabel(StepValue));
		MyGameSettings->MasterVolume = StepValue;
		MyGameSettings->ApplySettings(false);
		PC->SetMasterVolume(StepValue);
	}
}

FText SFlareSettingsMenu::GetMasterVolumeLabel(int32 Value) const
{
	if (Value == 0)
		return LOCTEXT("Muted", "Muted");
	else
		return FText::Format(LOCTEXT("MasterVolumeFormat", "{0}%"), FText::AsNumber(10 * Value));
}


/*----------------------------------------------------
	Toggles
----------------------------------------------------*/

void SFlareSettingsMenu::OnVSyncToggle()
{
	if (VSyncButton->IsActive())
	{
		FLOG("SFlareSettingsMenu::OnVSyncToggle : Enable vsync")
	}
	else
	{
		FLOG("SFlareSettingsMenu::OnVSyncToggle : Disable vsync")
	}

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	MyGameSettings->SetVSyncEnabled(VSyncButton->IsActive());
	MyGameSettings->ApplySettings(false);
}

void SFlareSettingsMenu::OnMotionBlurToggle()
{
	if (MotionBlurButton->IsActive())
	{
		FLOG("SFlareSettingsMenu::OnMotionBlurToggle : Enable motion blur")
	}
	else
	{
		FLOG("SFlareSettingsMenu::OnMotionBlurToggle : Disable motion blur")
	}

	MenuManager->GetPC()->SetUseMotionBlur(MotionBlurButton->IsActive());

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	MyGameSettings->UseMotionBlur = MotionBlurButton->IsActive();
	MyGameSettings->ApplySettings(false);
}

void SFlareSettingsMenu::OnTemporalAAToggle()
{
	if (TemporalAAButton->IsActive())
	{
		FLOG("SFlareSettingsMenu::OnTemporalAAToggle : Enable TAA")
	}
	else
	{
		FLOG("SFlareSettingsMenu::OnTemporalAAToggle : Enable FXAA")
	}

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	MyGameSettings->SetUseTemporalAA(TemporalAAButton->IsActive());
	MyGameSettings->ApplySettings(false);
}

void SFlareSettingsMenu::OnSupersamplingToggle()
{
	if (SupersamplingButton->IsActive())
	{
		FLOG("SFlareSettingsMenu::OnSupersamplingToggle : Enable supersampling")
	}
	else
	{
		FLOG("SFlareSettingsMenu::OnSupersamplingToggle : Disable supersampling")
	}

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	MyGameSettings->SetScreenPercentage(SupersamplingButton->IsActive() ? 142 : 100);
	MyGameSettings->ApplySettings(false);

}

void SFlareSettingsMenu::OnFullscreenToggle()
{
	UpdateResolution(true);
}

void SFlareSettingsMenu::OnInvertYToggle()
{
	bool New = InvertYButton->IsActive();

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	MyGameSettings->InvertY = New;
	MyGameSettings->ApplySettings(false);
}

void SFlareSettingsMenu::OnDisableMouseToggle()
{
	bool New = DisableMouseButton->IsActive();

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	MyGameSettings->DisableMouse = New;
	MyGameSettings->ApplySettings(false);
}

void SFlareSettingsMenu::OnCockpitToggle()
{
#if !UE_BUILD_SHIPPING
	bool New = CockpitButton->IsActive();
	MenuManager->GetPC()->SetUseCockpit(New);

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	MyGameSettings->UseCockpit = New;
	MyGameSettings->ApplySettings(false);
#endif
}

void SFlareSettingsMenu::OnLateralVelocityToggle()
{
	bool New = LateralVelocityButton->IsActive();

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	MyGameSettings->ShowLateralVelocity = New;
	MyGameSettings->ApplySettings(false);
}

void SFlareSettingsMenu::OnForwardOnlyThrustToggle()
{
	bool New = ForwardOnlyThrustButton->IsActive();

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	MyGameSettings->ForwardOnlyThrust = New;
	MyGameSettings->ApplySettings(false);
}

void SFlareSettingsMenu::OnAnticollisionToggle()
{
	bool New = AnticollisionButton->IsActive();

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	MyGameSettings->UseAnticollision = New;
	MyGameSettings->ApplySettings(false);
}

const FSlateBrush* SFlareSettingsMenu::GetGamepadDrawing() const
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());

	switch (MyGameSettings->GamepadProfileLayout)
	{
		case EFlareGamepadLayout::GL_TurnWithLeftStick:
			return FFlareStyleSet::GetImage("Pad_TurnWithLeft");
			break;

		case EFlareGamepadLayout::GL_LeftHanded:
			return FFlareStyleSet::GetImage("Pad_LeftHanded");
			break;

		case EFlareGamepadLayout::GL_RollWithShoulders:
			return FFlareStyleSet::GetImage("Pad_RollWithShoulders");
			break;

		default:
		case EFlareGamepadLayout::GL_Default:
			return FFlareStyleSet::GetImage("Pad");
			break;
	}
}

void SFlareSettingsMenu::OnInvertAxisClicked(FName AxisName)
{
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
	for (int i = 0; i < InputSettings->AxisMappings.Num(); i++)
	{
		if (InputSettings->AxisMappings[i].AxisName == AxisName && i != InputSettings->AxisMappings.Num() - 1)
		{
			// Remove the original binding
			FInputAxisKeyMapping Bind = InputSettings->AxisMappings[i];
			InputSettings->RemoveAxisMapping(Bind);

			// Re-add to force update
			Bind.Scale = (Bind.Scale < 0) ? 1 : -1;
			InputSettings->AddAxisMapping(Bind);
		}
	}

	InputSettings->SaveKeyMappings();
}

void SFlareSettingsMenu::OnUnbindAxisClicked(FName AxisName)
{
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
	for (int i = 0; i < InputSettings->AxisMappings.Num(); i++)
	{
		if (InputSettings->AxisMappings[i].AxisName == AxisName && i != InputSettings->AxisMappings.Num() - 1)
		{
			FInputAxisKeyMapping Bind = InputSettings->AxisMappings[i];
			InputSettings->RemoveAxisMapping(Bind);
		}
	}
}

bool SFlareSettingsMenu::IsAxisControlsDisabled(FName AxisName) const
{
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
	for (int i = 0; i < InputSettings->AxisMappings.Num(); i++)
	{
		if (InputSettings->AxisMappings[i].AxisName == AxisName && i != InputSettings->AxisMappings.Num() - 1)
		{
			return false;
		}
	}

	return true;
}

void SFlareSettingsMenu::OnRotationDeadZoneSliderChanged(float Value)
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	MyGameSettings->RotationDeadZone = Value;
	MyGameSettings->ApplySettings(false);
}

void SFlareSettingsMenu::OnRollDeadZoneSliderChanged(float Value)
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	MyGameSettings->RollDeadZone = Value;
	MyGameSettings->ApplySettings(false);
}

void SFlareSettingsMenu::OnTranslationDeadZoneSliderChanged(float Value)
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	MyGameSettings->TranslationDeadZone = Value;
	MyGameSettings->ApplySettings(false);
}

void  SFlareSettingsMenu::ApplyNewBinding(TSharedPtr<FSimpleBind> BindingThatChanged, bool Replace, bool bPrimaryKey)
{
	if(Replace)
	{
		FKey KeyToErase = *BindingThatChanged->Key;

		if(!bPrimaryKey)
		{
			KeyToErase = *BindingThatChanged->AltKey;
		}

		auto EraseIfNeeded = [&KeyToErase, &BindingThatChanged](TSharedPtr<FSimpleBind> Bind)
		{
			if (!Bind->bHeader && &(*BindingThatChanged) != &(*Bind))
			{
				if (*(Bind->Key) == KeyToErase && Bind->KeyWidget.IsValid())
				{
					Bind->KeyWidget->SetKey(FKey(), true, false);
					Bind->WriteBind();
				}

				if (*(Bind->AltKey) == KeyToErase && Bind->AltKeyWidget.IsValid())
				{
					Bind->AltKeyWidget->SetKey(FKey(), true, false);
					Bind->WriteBind();
				}
			}
		};

		for (TSharedPtr<FSimpleBind> Bind : Binds)
		{
			EraseIfNeeded(Bind);
		}

		for (TSharedPtr<FSimpleBind> Bind : Binds2)
		{
			EraseIfNeeded(Bind);
		}
	}

	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
	BindingThatChanged->WriteBind();
	InputSettings->SaveKeyMappings();
}

void  SFlareSettingsMenu::CancelNewBinding(SFlareKeyBind* UIBinding, FKey PreviousKey)
{
	UIBinding->SetKey(PreviousKey, true, false);
}

void SFlareSettingsMenu::OnKeyBindingChanged(FKey PreviousKey, FKey NewKey, SFlareKeyBind* UIBinding, TSharedPtr<FSimpleBind> BindingThatChanged, bool bPrimaryKey)
{
	// Primary or Alt key changed to a valid state.
	// TODO Duplicate

	TArray<TSharedPtr<FSimpleBind>> BindConflicts;

	if (NewKey.ToString() == "None")
	{
		UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
		BindingThatChanged->WriteBind();
		InputSettings->SaveKeyMappings();
		return;
	}
	else if (IsAlreadyUsed(BindConflicts, NewKey, *BindingThatChanged))
	{
		FText ConflictKeys;

		for(TSharedPtr<FSimpleBind> Bind: BindConflicts)
		{
			if(ConflictKeys.IsEmpty())
			{
				ConflictKeys = FText::Format(LOCTEXT("ConflictsKeysFirst", "'{0}'"), Bind->DisplayName);
			}
			else
			{
				ConflictKeys = FText::Format(LOCTEXT("ConflictsKeysOthers", "{0}, '{1}'"), ConflictKeys, Bind->DisplayName);
			}
		}

		AFlareMenuManager::GetSingleton()->Confirm(LOCTEXT("ConfirmKeyConflict", "KEY ALREADY USED"),
			FText::Format(LOCTEXT("ConfirmKeyConflictInfo", "'{0}' is already used for {1}. Do you want to assign this key for '{2}' only ?\nIgnore to keep both assignements"),
						  FText::FromString(NewKey.ToString()),
						  ConflictKeys,
						  BindingThatChanged->DisplayName),
			FSimpleDelegate::CreateSP(this, &SFlareSettingsMenu::ApplyNewBinding, BindingThatChanged, true, bPrimaryKey),
			FSimpleDelegate::CreateSP(this, &SFlareSettingsMenu::CancelNewBinding, UIBinding, PreviousKey),
			FSimpleDelegate::CreateSP(this, &SFlareSettingsMenu::ApplyNewBinding, BindingThatChanged, false, bPrimaryKey));
	}
	else
	{
		ApplyNewBinding(BindingThatChanged, false, bPrimaryKey);
	}
}


/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

void SFlareSettingsMenu::FillJoystickAxisList()
{
	// Get keys
	JoystickAxisKeys.Empty();
	TArray<FKey> AllKeys;
	EKeys::GetAllKeys(AllKeys);
	for (FKey Key : AllKeys)
	{
		if (Key.GetFName().ToString().StartsWith("Joystick")
		&& !Key.GetFName().ToString().Contains("Button"))
		{
			JoystickAxisKeys.Add(MakeShareable(new FKey(Key)));
		}
	}
}

void SFlareSettingsMenu::FillResolutionList()
{
	// Get the current res
	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();
	FIntPoint Resolution = GEngine->GameViewport->Viewport->GetSizeXY();
	FLOGV("SFlareSettingsMenu::FillResolutionList : current resolution is %s", *Resolution.ToString());

	// Setup the list data
	ResolutionList.Empty();
	FScreenResolutionArray Resolutions;

	// Get all resolution settings
	if (RHIGetAvailableResolutions(Resolutions, true))
	{
		for (const FScreenResolutionRHI& EachResolution : Resolutions)
		{
			if (EachResolution.Width >= 1280 && EachResolution.Height >= 720)
			{
				ResolutionList.Insert(MakeShareable(new FScreenResolutionRHI(EachResolution)), 0);
			}
		}
	}
	else
	{
		FLOG("SFlareSettingsMenu::FillResolutionList : screen resolutions could not be obtained");
	}

	UpdateResolutionList(Resolution);
}

void SFlareSettingsMenu::UpdateResolutionList(FIntPoint Resolution)
{
	int CurrentResolutionIndex = -1;

	// Look for current resolution
	for (int i = 0; i < ResolutionList.Num(); i++)
	{
		if (Resolution.X == ResolutionList[i]->Width && Resolution.Y == ResolutionList[i]->Height)
		{
			CurrentResolutionIndex = i;
			break;
		}
	}

	// Didn't find our current res...
	if (CurrentResolutionIndex < 0)
	{
		TSharedPtr<FScreenResolutionRHI> InitialResolution = MakeShareable(new FScreenResolutionRHI());
		InitialResolution->Width = Resolution.X;
		InitialResolution->Height = Resolution.Y;

		ResolutionList.Insert(InitialResolution, 0);
		CurrentResolutionIndex = 0;
	}

	// Update the list
	ResolutionSelector->RefreshOptions();
	if (CurrentResolutionIndex >= 0 && CurrentResolutionIndex < ResolutionList.Num())
	{
		ResolutionSelector->SetSelectedItem(ResolutionList[CurrentResolutionIndex]);
	}
}

void SFlareSettingsMenu::UpdateResolution(bool CanAdaptResolution)
{
	if (GEngine)
	{
		int32 Width = 0, Height = 0;
		FIntPoint CurrentResolution = GEngine->GameViewport->Viewport->GetSizeXY();

		// We clicked "full screen" to set it on, force a supported resolution
		if (CanAdaptResolution && FullscreenButton->IsActive())
		{
			FLOG("SFlareSettingsMenu::UpdateResolution : enabled full screen, switching res");

			// Get all resolution settings
			FScreenResolutionArray Resolutions;
			RHIGetAvailableResolutions(Resolutions, true);

			// Find the lowest resolution that's >= our current one
			for (const FScreenResolutionRHI& Resolution : Resolutions)
			{
				Width = Resolution.Width;
				Height = Resolution.Height;
				if (Resolution.Width > (uint32)CurrentResolution.X && Resolution.Height > (uint32)CurrentResolution.Y)
				{
					break;
				}
			}

			UpdateResolutionList(FIntPoint(Width, Height));
		}

		// We changed the resolution only
		else
		{
			TSharedPtr<FScreenResolutionRHI> Item = ResolutionSelector->GetSelectedItem();
			Width = Item->Width;
			Height = Item->Height;
		}

		FLOGV("SFlareSettingsMenu::UpdateResolution : new resolution is %dx%d (was %s)", Width, Height, *CurrentResolution.ToString());		
		FLOGV("SFlareSettingsMenu::UpdateResolution : fullscreen = %d", FullscreenButton->IsActive());

		// Apply video settings
		FString Command = FString::Printf(TEXT("setres %dx%d%s"), Width, Height, FullscreenButton->IsActive() ? TEXT("f") : TEXT("w"));
		GEngine->GameViewport->ConsoleCommand(*Command);

		// Save settings
		UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();
		MyGameSettings->SetScreenResolution(FIntPoint(Width, Height));
		MyGameSettings->SetFullscreenMode(FullscreenButton->IsActive() ? EWindowMode::Fullscreen : EWindowMode::Windowed);
		MyGameSettings->ConfirmVideoMode();
		MyGameSettings->SaveConfig();
		GEngine->SaveConfig();
	}
}

void SFlareSettingsMenu::FillGameGamepadList()
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());

	GamepadList.Empty();
	GamepadList.Add(MakeShareable(new FText(LOCTEXT("GamepadDefault", "Default layout"))));
	GamepadList.Add(MakeShareable(new FText(LOCTEXT("GamepadTurnLeft", "Turn with left stick"))));
	GamepadList.Add(MakeShareable(new FText(LOCTEXT("GamepadLeftHanded", "Left-handed"))));
	GamepadList.Add(MakeShareable(new FText(LOCTEXT("RollWithShoulders", "Roll with shoulders"))));

	GamepadSelector->RefreshOptions();
	GamepadSelector->SetSelectedIndex(MyGameSettings->GamepadProfileLayout);
}

void SFlareSettingsMenu::CreateBinds()
{
	// Piloting
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Flying", "FLYING")))->MakeHeader()));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("MoveForward", "Move forward")))
		->AddAxisMapping("NormalThrustInput", 1.0f)
		->AddDefaults(EKeys::W)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("MoveBackward", "Move backward")))
		->AddAxisMapping("NormalThrustInput", -1.0f)
		->AddDefaults(EKeys::S)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("MoveLeft", "Move left")))
		->AddAxisMapping("MoveHorizontalInput", -1.0f)
		->AddDefaults(EKeys::A)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("MoveRight", "Move right")))
		->AddAxisMapping("MoveHorizontalInput", 1.0f)
		->AddDefaults(EKeys::D)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("MoveUp", "Move up")))
		->AddAxisMapping("MoveVerticalInput", 1.0f)
		->AddDefaults(EKeys::LeftShift)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("MoveDown", "Move down")))
		->AddAxisMapping("MoveVerticalInput", -1.0f)
		->AddDefaults(EKeys::LeftControl)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("RollCCW", "Roll left")))
		->AddAxisMapping("NormalRollInput", -1.0f)
		->AddDefaults(EKeys::E)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("RollCW", "Roll right")))
		->AddAxisMapping("NormalRollInput", 1.0f)
		->AddDefaults(EKeys::Q)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("NextTarget", "Next target")))
		->AddActionMapping("AlternateNextTarget")));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("PreviousTarget", "Previous target")))
		->AddActionMapping("AlternatePreviousTarget")));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Quick Ship Switch", "Quick ship switch")))
		->AddActionMapping("QuickSwitch")
		->AddDefaults(EKeys::N)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Toogle Camera", "Toggle camera")))
		->AddActionMapping("ToggleCamera")
		->AddDefaults(EKeys::C)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("LockDirection", "Lock direction")))
		->AddActionMapping("LockDirection")
		->AddDefaults(EKeys::CapsLock)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Toogle HUD", "Toggle HUD")))
		->AddActionMapping("ToggleHUD")
		->AddDefaults(EKeys::H)));

	// Auto pilot
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Autopilot", "AUTOPILOT")))->MakeHeader()));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("AlignPrograde", "Align to speed")))
		->AddActionMapping("FaceForward")));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("AlignRetrograde", "Align to reverse")))
		->AddActionMapping("FaceBackward")
		->AddDefaults(EKeys::X)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("CutSpeed", "Cut speed")))
		->AddActionMapping("Brake")
		->AddDefaults(EKeys::B)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Start Autopilot", "Engage autopilot")))
		->AddActionMapping("EnablePilot")
		->AddDefaults(EKeys::P)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Disengage Autopilot", "Disengage autopilot")))
		->AddActionMapping("DisengagePilot")
		->AddDefaults(EKeys::M)));

	// Weapons
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Weapons", "WEAPONS")))->MakeHeader()));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Deactivate Weapons", "Menu 1 / Stand down")))
		->AddActionMapping("SpacecraftKey1")
		->AddDefaults(EKeys::One)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Activate primary weapons", "Menu 2 / Weapon 1")))
		->AddActionMapping("SpacecraftKey2")
		->AddDefaults(EKeys::Two)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Activate secondary weapons", "Menu 3 / Weapon 2")))
		->AddActionMapping("SpacecraftKey3")
		->AddDefaults(EKeys::Three)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Activate tertiary weapons", "Menu 4 / Weapon 3")))
		->AddActionMapping("SpacecraftKey4")
		->AddDefaults(EKeys::Four)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Menu 5", "Menu 5")))
		->AddActionMapping("SpacecraftKey5")
		->AddDefaults(EKeys::Five)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Menu 6", "Menu 6")))
		->AddActionMapping("SpacecraftKey6")
		->AddDefaults(EKeys::Six)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Menu 7", "Menu 7")))
		->AddActionMapping("SpacecraftKey7")
		->AddDefaults(EKeys::Seven)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Fire Weapon", "Fire")))
		->AddActionMapping("StartFire")));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Toogle Weapons", "Toggle weapons")))
		->AddActionMapping("ToggleCombat")
		->AddDefaults(EKeys::F)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Find best target", "Find best target")))
		->AddActionMapping("FindTarget")
		->AddDefaults(EKeys::R)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Combat zoom", "Combat zoom")))
		->AddActionMapping("CombatZoom")
		->AddDefaults(EKeys::SpaceBar)));

	// Menus
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Menus", "MENUS")))->MakeHeader()));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Toggle menus", "Open / close menus")))
		->AddActionMapping("ToggleMenu")));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Toggle overlay", "Open / close flight overlay")))
		->AddActionMapping("ToggleOverlay")
		->AddDefaults(EKeys::RightMouseButton)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Toggle wheel menu", "Open / close wheel menu")))
		->AddActionMapping("Wheel")
		->AddDefaults(EKeys::MiddleMouseButton)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Enter menu", "Enter menu")))
		->AddActionMapping("EnterMenu")
		->AddDefaults(EKeys::Enter)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Back", "Previous menu")))
		->AddActionMapping("BackMenu")
		->AddDefaults(EKeys::Escape)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open ship menu", "Open ship menu")))
		->AddActionMapping("ShipMenu")
		->AddDefaults(EKeys::F1)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open sector menu", "Open sector menu")))
		->AddActionMapping("SectorMenu")
		->AddDefaults(EKeys::F2)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open orbital map", "Open orbital map")))
		->AddActionMapping("OrbitMenu")
		->AddDefaults(EKeys::F3)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open economy menu", "Open economy menu")))
		->AddActionMapping("EconomyMenu")
		->AddDefaults(EKeys::F4)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open company menu", "Open company menu")))
		->AddActionMapping("CompanyMenu")
		->AddDefaults(EKeys::F5)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open fleet menu", "Open fleet menu")))
		->AddActionMapping("FleetMenu")
		->AddDefaults(EKeys::F6)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open leaderboard", "Open diplomacy menu")))
		->AddActionMapping("LeaderboardMenu")
		->AddDefaults(EKeys::F7)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open technology menu", "Open technology menu")))
		->AddActionMapping("TechnologyMenu")
		->AddDefaults(EKeys::F8)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open quest menu", "Open quest menu")))
		->AddActionMapping("QuestMenu")
		->AddDefaults(EKeys::F9)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open main menu", "Open main menu")))
		->AddActionMapping("MainMenu")
		->AddDefaults(EKeys::F10)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open settings menu", "Open settings menu")))
		->AddActionMapping("SettingsMenu")
		->AddDefaults(EKeys::F12)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open trade menu", "Open trade menu")))
		->AddActionMapping("TradeMenu")
		->AddDefaults(EKeys::T)));

	// Strategy
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Strategy", "STRATEGY")))->MakeHeader()));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Fast forward single", "Fast forward a day")))
		->AddActionMapping("Simulate")
		->AddDefaults(EKeys::J)));

	// Others
	/*Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Others", "OTHERS")))->MakeHeader()));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Take high quality screenshot", "Take high quality screenshot")))
		->AddActionMapping("HighResShot")));*/

}

bool SFlareSettingsMenu::IsAlreadyUsed(TArray<TSharedPtr<FSimpleBind>> &BindConflicts, FKey Key, FSimpleBind& ExcludeBinding)
{
	for(TSharedPtr<FSimpleBind> Bind : Binds)
	{
		if((*(Bind->Key) == Key || *Bind->AltKey == Key) && &ExcludeBinding != &(*Bind))
		{
			BindConflicts.Add(Bind);
		}
	}

	for(TSharedPtr<FSimpleBind> Bind : Binds2)
	{
		if((*Bind->Key == Key || *Bind->AltKey == Key) && &ExcludeBinding != &(*Bind))
		{
			BindConflicts.Add(Bind);
		}
	}

	return BindConflicts.Num() > 0;
}

/*----------------------------------------------------
	FSimpleBind
----------------------------------------------------*/

FSimpleBind::FSimpleBind(const FText& InDisplayName)
{
	DisplayName = InDisplayName;
	bHeader = false;
	Key = MakeShareable(new FKey());
	AltKey = MakeShareable(new FKey());
}

FSimpleBind* FSimpleBind::AddActionMapping(const FName& Mapping)
{
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
	
	bool bFound = false;
	for (int32 i = 0; i < InputSettings->ActionMappings.Num(); i++)
	{
		FInputActionKeyMapping Action = InputSettings->ActionMappings[i];
		bool IsJoystickKey = Action.Key.GetDisplayName().ToString().StartsWith("Joystick");
		bool IsGamePadKey = Action.Key.IsGamepadKey() && !IsJoystickKey;

		if (Mapping == Action.ActionName && !IsGamePadKey)
		{
			ActionMappings.Add(Action);

			//Fill in the first 2 keys we find from the ini
			if (*Key == FKey())
			{
				*Key = Action.Key;
			}
			else if (*AltKey == FKey() && Action.Key != *Key)
			{
				*AltKey = Action.Key;
			}
			bFound = true;
		}
	}

	if (!bFound)
	{
		FInputActionKeyMapping Action;
		Action.ActionName = Mapping;
		ActionMappings.Add(Action);
	}
	return this;
}

FSimpleBind* FSimpleBind::AddAxisMapping(const FName& Mapping, float Scale)
{
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();

	bool bFound = false;
	for (int32 i = 0; i < InputSettings->AxisMappings.Num(); i++)
	{
		FInputAxisKeyMapping Axis = InputSettings->AxisMappings[i];
		if (Mapping == Axis.AxisName && Axis.Scale == Scale && !Axis.Key.IsGamepadKey())
		{
			AxisMappings.Add(Axis);

			//Fill in the first 2 keys we find from the ini
			if (*Key == FKey())
			{
				*Key = Axis.Key;
			}
			else if (*AltKey == FKey() && Axis.Key != *Key)
			{
				*AltKey = Axis.Key;
			}
			bFound = true;
		}
	}

	if (!bFound)
	{
		FInputAxisKeyMapping Action;
		Action.AxisName = Mapping;
		Action.Scale = Scale;
		AxisMappings.Add(Action);
	}
	return this;
}

FSimpleBind* FSimpleBind::AddSpecialBinding(const FName& Mapping)
{
	if (Mapping == FName(TEXT("Console")))
	{
		UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
		if (InputSettings->ConsoleKeys.IsValidIndex(0))
		{
			*Key = InputSettings->ConsoleKeys[0];
		}
		if (InputSettings->ConsoleKeys.IsValidIndex(1))
		{
			*AltKey = InputSettings->ConsoleKeys[1];
		}
	}

	SpecialBindings.Add(Mapping);

	return this;
}

void FSimpleBind::WriteBind()
{
	if (bHeader)
	{
		return;
	}
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();

	// Collapse the keys if the main key is missing
	if (*Key == FKey() && *AltKey != FKey())
	{
		Key = AltKey;
		AltKey = MakeShareable(new FKey());
	}

	//Remove the original bindings
	for (auto& Bind : ActionMappings)
	{
		InputSettings->RemoveActionMapping(Bind);
	}
	for (auto& Bind : AxisMappings)
	{
		InputSettings->RemoveAxisMapping(Bind);
	}

	// Set our new keys and read them
	TArray<FName> ActionMappingNames;
	TArray<FName> AxisMappingNames;
	TArray<float> AxisMappingScales;


	for (auto Bind : ActionMappings)
	{
		Bind.Key = *Key;
		InputSettings->AddActionMapping(Bind);
		if (*AltKey != FKey())
		{
			Bind.Key = *AltKey;
			InputSettings->AddActionMapping(Bind);
		}

		ActionMappingNames.AddUnique(Bind.ActionName);
	}
	for (auto Bind : AxisMappings)
	{
		Bind.Key = *Key;
		InputSettings->AddAxisMapping(Bind);
		if (*AltKey != FKey())
		{
			Bind.Key = *AltKey;
			InputSettings->AddAxisMapping(Bind);
		}

		AxisMappingNames.Add(Bind.AxisName);
		AxisMappingScales.Add(Bind.Scale);
	}

	for (FName Bind : SpecialBindings)
	{
		if (Bind == FName(TEXT("Console")))
		{
			InputSettings->ConsoleKeys.Empty();
			InputSettings->ConsoleKeys.Add(*Key);
			InputSettings->ConsoleKeys.Add(*AltKey);
		}
	}

	// Reload Mappings
	ActionMappings.Empty();
	AxisMappings.Empty();

	for (FName MappingName : ActionMappingNames)
	{
		AddActionMapping(MappingName);
	}

	for (int i = 0; i < AxisMappingNames.Num(); i++)
	{
		AddAxisMapping(AxisMappingNames[i], AxisMappingScales[i]);
	}
}

#undef LOCTEXT_NAMESPACE

