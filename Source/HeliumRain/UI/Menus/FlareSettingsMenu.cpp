
#include "../../Flare.h"
#include "FlareSettingsMenu.h"
#include "../../Game/FlareGameUserSettings.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareSaveGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "STextComboBox.h"


#define LOCTEXT_NAMESPACE "FlareSettingsMenu"


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

	// Current settings
	float CurrentTextureQualityRatio = MyGameSettings->ScalabilityQuality.TextureQuality / 3.f;
	float CurrentEffectsQualityRatio = MyGameSettings->ScalabilityQuality.EffectsQuality / 3.f;
	float CurrentAntiAliasingQualityRatio = MyGameSettings->ScalabilityQuality.AntiAliasingQuality / 6.f;
	float CurrentPostProcessQualityRatio = MyGameSettings->ScalabilityQuality.PostProcessQuality / 3.f;
	FLOGV("MyGameSettings->ScalabilityQuality.TextureQuality=%d CurrentTextureQualityRatio=%f", MyGameSettings->ScalabilityQuality.TextureQuality, CurrentTextureQualityRatio);
	FLOGV("MyGameSettings->ScalabilityQuality.EffectsQuality=%d CurrentEffectsQualityRatio=%f", MyGameSettings->ScalabilityQuality.EffectsQuality, CurrentEffectsQualityRatio);
	FLOGV("MyGameSettings->ScalabilityQuality.AntiAliasingQuality=%d CurrentAntiAliasingQualityRatio=%f", MyGameSettings->ScalabilityQuality.AntiAliasingQuality, CurrentAntiAliasingQualityRatio);
	FLOGV("MyGameSettings->ScalabilityQuality.PostProcessQuality=%d CurrentPostProcessQualityRatio=%f", MyGameSettings->ScalabilityQuality.PostProcessQuality, CurrentPostProcessQualityRatio);

	CreateBinds();

	// General data
	FLinearColor Color = Theme.NeutralColor;
	Color.A = Theme.DefaultAlpha;
	int32 LabelSize = 200;
	int32 ValueSize = 100;

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SScrollBox)
		.Style(&Theme.ScrollBoxStyle)
		.ScrollBarStyle(&Theme.ScrollBarStyle)

		+ SScrollBox::Slot()
		.HAlign(HAlign_Fill)
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
						SAssignNew(ResolutionSelector, SComboBox<TSharedPtr<FScreenResolutionRHI>>)
						.OptionsSource(&ResolutionList)
						.OnGenerateWidget(this, &SFlareSettingsMenu::OnGenerateResolutionComboLine)
						.OnSelectionChanged(this, &SFlareSettingsMenu::OnResolutionComboLineSelectionChanged)
						.ComboBoxStyle(&Theme.ComboBoxStyle)
						.ForegroundColor(FLinearColor::White)
						[
							SNew(STextBlock)
							.Text(this, &SFlareSettingsMenu::OnGetCurrentResolutionComboLine)
							.TextStyle(&Theme.TextFont)
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
								.Text(LOCTEXT("PostProcessLabel", "Post-processing quality"))
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

			// Gameplay options
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.TitlePadding)
			.HAlign(HAlign_Left)
			[
				SNew(SBox)
				.WidthOverride(Theme.ContentWidth)
				.HAlign(HAlign_Fill)
				[
					SNew(SVerticalBox)

					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("GameplayHint", "Gameplay"))
						.TextStyle(&Theme.SubTitleFont)
					]
				
					// Options
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					[
						SNew(SHorizontalBox)
					
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
					
						// Anticollision
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SAssignNew(AnticollisionButton, SFlareButton)
							.Text(LOCTEXT("Anticollision", "Use anticollision"))
							.HelpText(LOCTEXT("AnticollisionInfo", "Anti-collision will prevent your ship from crahsing into objects and forbid close fly-bys."))
							.Toggle(true)
							.OnClicked(this, &SFlareSettingsMenu::OnAnticollisionToggle)
						]
					
						// Pause in menus
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SAssignNew(PauseInMenusButton, SFlareButton)
							.Text(LOCTEXT("PauseInMenus", "Pause in menus"))
							.HelpText(LOCTEXT("PauseInMenusInfo", "Pause the game when entering a full-screen menu."))
							.Toggle(true)
							.OnClicked(this, &SFlareSettingsMenu::OnPauseInMenusToggle)
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
							.Value(MyGameSettings->MusicVolume / 10.0f)
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

			// Controls
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.TitlePadding)
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ControlsSettingsHint", "Controls"))
				.TextStyle(&Theme.SubTitleFont)
			]

			// Controls form
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.ContentPadding)
			.HAlign(HAlign_Fill)
			[
				BuildKeyBindingBox()
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
		]
	];

	// Default settings
	VSyncButton->SetActive(MyGameSettings->IsVSyncEnabled());
	MotionBlurButton->SetActive(MyGameSettings->UseMotionBlur);
	TemporalAAButton->SetActive(MyGameSettings->UseTemporalAA);
	FullscreenButton->SetActive(MyGameSettings->GetFullscreenMode() == EWindowMode::Fullscreen);
	SupersamplingButton->SetActive(MyGameSettings->ScreenPercentage > 100);
	CockpitButton->SetActive(MyGameSettings->UseCockpit);
	AnticollisionButton->SetActive(MyGameSettings->UseAnticollision);
	PauseInMenusButton->SetActive(MyGameSettings->PauseGameInMenus);

	// Music volume
	int32 MusicVolume = MyGameSettings->MusicVolume;
	int32 MasterVolume = MyGameSettings->MusicVolume;
	ShipCountSlider->SetValue((float)MyGameSettings->MaxShipsInSector / 100.0f);
	ShipCountLabel->SetText(GetShipCountLabel((MyGameSettings->MaxShipsInSector - 10) / 5.0f));
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
					.Text(FText::FromString(Bind->DisplayName))
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
					.Text(FText::FromString(Bind->DisplayName))
				]

				// Key 1
				+ SHorizontalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				[
					SAssignNew(Bind->KeyWidget, SFlareKeyBind)
					.Key(Bind->Key)
					.DefaultKey(Bind->DefaultKey)
					.OnKeyBindingChanged( this, &SFlareSettingsMenu::OnKeyBindingChanged, Bind, true)
				]

				// Key 2
				+ SHorizontalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				[
					SAssignNew(Bind->AltKeyWidget, SFlareKeyBind)
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
		.WidthOverride(Theme.ContentWidth)
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
			.Width(2)
			.Text(LOCTEXT("InvertAxisDirection", "Invert"))
			.HelpText(LOCTEXT("InvertAxisDirectionInfo", "Toggle the axis inversion to reverse the joystick axis."))
			.OnClicked(this, &SFlareSettingsMenu::OnInvertAxisClicked, AxisName)
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
}

void SFlareSettingsMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FText SFlareSettingsMenu::OnGetCurrentResolutionComboLine() const
{
	TSharedPtr<FScreenResolutionRHI> Item = ResolutionSelector->GetSelectedItem();
	return Item.IsValid() ? FText::FromString(FString::Printf(TEXT("%dx%d"), Item->Width, Item->Height)) : FText::FromString("");
}

TSharedRef<SWidget> SFlareSettingsMenu::OnGenerateResolutionComboLine(TSharedPtr<FScreenResolutionRHI> Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(STextBlock)
	.Text(FText::FromString(FString::Printf(TEXT("%dx%d"), Item->Width, Item->Height)))
	.TextStyle(&Theme.TextFont);
}

void SFlareSettingsMenu::OnResolutionComboLineSelectionChanged(TSharedPtr<FScreenResolutionRHI> StringItem, ESelectInfo::Type SelectInfo)
{
	UpdateResolution();
}

FText SFlareSettingsMenu::OnGetCurrentJoystickKeyName(FName AxisName) const
{
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
	for (auto& Bind : InputSettings->AxisMappings)
	{
		if (Bind.AxisName == AxisName)
		{
			FText InvertedText = (Bind.Scale < 0) ? LOCTEXT("AxisInvertedHint", "(Inverted)") : FText();
			return FText::Format(FText::FromString("{0} {1}"), Bind.Key.GetDisplayName(), InvertedText);
		}
	}

	return FText();
}

TSharedRef<SWidget> SFlareSettingsMenu::OnGenerateJoystickComboLine(TSharedPtr<FKey> Item, FName AxisName)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(STextBlock)
		.Text(Item->GetDisplayName())
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

void SFlareSettingsMenu::OnFullscreenToggle()
{
	UpdateResolution();
}

void SFlareSettingsMenu::OnCockpitToggle()
{
	bool New = CockpitButton->IsActive();
	MenuManager->GetPC()->SetUseCockpit(New);

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	MyGameSettings->UseCockpit = New;
	MyGameSettings->ApplySettings(false);
}

void SFlareSettingsMenu::OnAnticollisionToggle()
{
	bool New = AnticollisionButton->IsActive();

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	MyGameSettings->UseAnticollision = New;
	MyGameSettings->ApplySettings(false);
}

void SFlareSettingsMenu::OnPauseInMenusToggle()
{
	bool New = PauseInMenusButton->IsActive();
	MenuManager->GetPC()->SetPauseGameInMenus(New);

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	MyGameSettings->PauseGameInMenus = New;
	MyGameSettings->ApplySettings(false);
}

void SFlareSettingsMenu::OnShipCountSliderChanged(float Value)
{
	int32 Step = 18;
	int32 StepValue = FMath::RoundToInt(Step * Value);
	ShipCountSlider->SetValue((float)StepValue / (float)Step);

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	int32 NewMaxShipValue = 10 + 5 * StepValue;

	if (MyGameSettings->MaxShipsInSector != NewMaxShipValue)
	{
		FLOGV("SFlareSettingsMenu::OnShipCountSliderChanged : Set max ship count to %d (current is %d)", NewMaxShipValue, MyGameSettings->MaxShipsInSector);
		MyGameSettings->MaxShipsInSector = NewMaxShipValue;
		MyGameSettings->ApplySettings(false);
		ShipCountLabel->SetText(GetShipCountLabel(StepValue));
	}
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

void SFlareSettingsMenu::OnAntiAliasingQualitySliderChanged(float Value)
{
	int32 Step = 6;
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

	MenuManager->GetPC()->SetUseTemporalAA(TemporalAAButton->IsActive());

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	MyGameSettings->UseTemporalAA = TemporalAAButton->IsActive();
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

void SFlareSettingsMenu::OnKeyBindingChanged( FKey PreviousKey, FKey NewKey, TSharedPtr<FSimpleBind> BindingThatChanged, bool bPrimaryKey )
{
	// Primary or Alt key changed to a valid state.
	// TODO Duplicate

	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
	BindingThatChanged->WriteBind();
	InputSettings->SaveKeyMappings();
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
	int CurrentResolutionIndex = -1;

	// Get all resolution settings
	if (RHIGetAvailableResolutions(Resolutions, true))
	{
		for (const FScreenResolutionRHI& EachResolution : Resolutions)
		{
			if (EachResolution.Width >= 1280 && EachResolution.Height>= 720)
			{
				ResolutionList.Insert(MakeShareable(new FScreenResolutionRHI(EachResolution)), 0);
			}
		}
	}
	else
	{
		FLOG("SFlareSettingsMenu::FillResolutionList : screen resolutions could not be obtained");
	}

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
	ResolutionSelector->SetSelectedItem(ResolutionList[CurrentResolutionIndex]);
	ResolutionSelector->RefreshOptions();
}

void SFlareSettingsMenu::UpdateResolution()
{
	if (GEngine)
	{
		FIntPoint Resolution = GEngine->GameViewport->Viewport->GetSizeXY();
		FLOGV("SFlareSettingsMenu::UpdateResolution : current resolution is %s", *Resolution.ToString());

		TSharedPtr<FScreenResolutionRHI> Item = ResolutionSelector->GetSelectedItem();
		FLOGV("SFlareSettingsMenu::UpdateResolution : new resolution is %dx%d", Item->Width, Item->Height);

		FLOGV("SFlareSettingsMenu::UpdateResolution : Need fullscreen ? %d", FullscreenButton->IsActive());

		FString Command = FString::Printf(TEXT("setres %dx%d%s"), Item->Width, Item->Height, FullscreenButton->IsActive() ? TEXT("f") : TEXT("w"));
		FLOGV("SFlareSettingsMenu::UpdateResolution : Command=%s", *Command);

		GEngine->GameViewport->ConsoleCommand(*Command);

		UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();
		MyGameSettings->SetScreenResolution(FIntPoint(Item->Width, Item->Height));
		MyGameSettings->SetFullscreenMode(FullscreenButton->IsActive() ? EWindowMode::Fullscreen : EWindowMode::Windowed);
		MyGameSettings->ConfirmVideoMode();
		MyGameSettings->SaveConfig();

		GEngine->SaveConfig();
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

FText SFlareSettingsMenu::GetAntiAliasingQualityLabel(int32 Value) const
{
	switch(Value)
	{
		case 1:
			return LOCTEXT("AntiAliasingQualityVeryLow", "Very low");
		case 2:
			return LOCTEXT("AntiAliasingQualityLow", "Low");
		case 3:
			return LOCTEXT("AntiAliasingQualityMedium", "Medium");
		case 4:
			return LOCTEXT("AntiAliasingQualityHigh", "High");
		case 5:
			return LOCTEXT("AntiAliasingQualityVeryHigh", "Very high");
		case 6:
			return LOCTEXT("AntiAliasingQualityUltra", "Ultra");
		case 0:
		default:
			return LOCTEXT("AntiAliasingQualityDisabled", "Off");		
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

FText SFlareSettingsMenu::GetShipCountLabel(int32 Value) const
{
	return FText::AsNumber(10 + 5 * Value);
}

FText SFlareSettingsMenu::GetMusicVolumeLabel(int32 Value) const
{
	if (Value == 0)
		return LOCTEXT("Muted", "Muted");
	else
		return FText::Format(LOCTEXT("MusicVolumeFormat", "{0}%"), FText::AsNumber(10 * Value));
}

FText SFlareSettingsMenu::GetMasterVolumeLabel(int32 Value) const
{
	if (Value == 0)
		return LOCTEXT("Muted", "Muted");
	else
		return FText::Format(LOCTEXT("MasterVolumeFormat", "{0}%"), FText::AsNumber(10 * Value));
}

void SFlareSettingsMenu::CreateBinds()
{
	// Piloting
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Flying", "FLYING")))->MakeHeader()));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("MoveForward", "Move Forward")))
		->AddAxisMapping("NormalThrustInput", 1.0f)
		->AddDefaults(EKeys::W)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("MoveBackward", "Move Backward")))
		->AddAxisMapping("NormalThrustInput", -1.0f)
		->AddDefaults(EKeys::S)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("MoveLeft", "Move Left")))
		->AddAxisMapping("MoveHorizontalInput", -1.0f)
		->AddDefaults(EKeys::A)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("MoveRight", "Move Right")))
		->AddAxisMapping("MoveHorizontalInput", 1.0f)
		->AddDefaults(EKeys::D)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("MoveUp", "Move Up")))
		->AddAxisMapping("MoveVerticalInput", 1.0f)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("MoveDown", "Move Down")))
		->AddAxisMapping("MoveVerticalInput", -1.0f)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("RollCW", "Roll Right")))
		->AddAxisMapping("NormalRollInput", 1.0f)
		->AddDefaults(EKeys::A)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("RollCCW", "Roll Left")))
		->AddAxisMapping("NormalRollInput", -1.0f)
		->AddDefaults(EKeys::E)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Quick Ship Switch", "Quick Ship Switch")))
		->AddActionMapping("QuickSwitch")
		->AddDefaults(EKeys::N)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Toogle Camera", "Toogle Camera")))
		->AddActionMapping("ToggleCamera")
		->AddDefaults(EKeys::C)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("LockDirection", "Lock direction")))
		->AddActionMapping("LockDirection")
		->AddDefaults(EKeys::SpaceBar)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Toogle HUD", "Toogle HUD")))
		->AddActionMapping("ToggleHUD")
		->AddDefaults(EKeys::H)));

	// Auto pilot
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Autopilot", "AUTOPILOT")))->MakeHeader()));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("AlignPrograde", "Align to Speed")))
		->AddActionMapping("FaceForward")
		->AddDefaults(EKeys::X)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("AlignRetrograde", "Align to Reverse")))
		->AddActionMapping("FaceBackward")
		->AddDefaults(EKeys::Z)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Disengage Autopilot", "Disengage Autopilot")))
		->AddActionMapping("Manual")
		->AddDefaults(EKeys::M)));

	// Weapons
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Weapons", "WEAPONS")))->MakeHeader()));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Deactivate Weapons", "Deactivate Weapons")))
		->AddActionMapping("DeactivateWeapon")
		->AddDefaults(EKeys::One)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Activate primary weapons", "Weapon Group 1")))
		->AddActionMapping("WeaponGroup1")
		->AddDefaults(EKeys::Two)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Activate secondary weapons", "Weapon Group 2")))
		->AddActionMapping("WeaponGroup2")
		->AddDefaults(EKeys::Three)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Activate tertiary weapons", "Weapon Group 3")))
		->AddActionMapping("WeaponGroup3")
		->AddDefaults(EKeys::Four)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Fire Weapon", "Fire")))
		->AddActionMapping("StartFire")
		->AddDefaults(EKeys::LeftControl)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Toogle Weapons", "Toogle Weapons")))
		->AddActionMapping("ToggleCombat")
		->AddDefaults(EKeys::F)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Find best target", "Find best target")))
		->AddActionMapping("FindTarget")
		->AddDefaults(EKeys::T)));

	// Menus
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Menus", "MENUS")))->MakeHeader()));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Toggle menus", "Open / close menus")))
		->AddActionMapping("ToggleMenu")
		->AddDefaults(EKeys::Enter)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Toggle overlay", "Open / close flight overlay")))
		->AddActionMapping("ToggleOverlay")
		->AddDefaults(EKeys::RightMouseButton)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Toggle wheel menu", "Open / close wheel menu")))
		->AddActionMapping("Wheel")
		->AddDefaults(EKeys::MiddleMouseButton)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Back", "Previous menu")))
		->AddActionMapping("BackMenu")
		->AddDefaults(EKeys::Escape)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open ship menu", "Open ship menu")))
		->AddActionMapping("ShipMenu")
		->AddDefaults(EKeys::F2)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open sector menu", "Open sector menu")))
		->AddActionMapping("SectorMenu")
		->AddDefaults(EKeys::F3)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open orbital map", "Open orbital map")))
		->AddActionMapping("OrbitMenu")
		->AddDefaults(EKeys::F4)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open leaderboard", "Open leaderboard")))
		->AddActionMapping("LeaderboardMenu")
		->AddDefaults(EKeys::F5)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open company menu", "Open company menu")))
		->AddActionMapping("CompanyMenu")
		->AddDefaults(EKeys::F6)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open fleet menu", "Open fleet menu")))
		->AddActionMapping("FleetMenu")
		->AddDefaults(EKeys::F7)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open quest menu", "Open quest menu")))
		->AddActionMapping("QuestMenu")
		->AddDefaults(EKeys::F8)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open main menu", "Open main menu")))
		->AddActionMapping("MainMenu")
		->AddDefaults(EKeys::F9)));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open settings menu", "Open settings menu")))
		->AddActionMapping("SettingsMenu")
		->AddDefaults(EKeys::F10)));

	// Strategy
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Strategy", "STRATEGY")))->MakeHeader()));
	Binds2.Add(MakeShareable((new FSimpleBind(LOCTEXT("Fast forward single", "Fast forward a day")))
		->AddActionMapping("Simulate")
		->AddDefaults(EKeys::J)));

}


/*----------------------------------------------------
	FSimpleBind
----------------------------------------------------*/

FSimpleBind::FSimpleBind(const FText& InDisplayName)
{
	DisplayName = InDisplayName.ToString();
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
		if (Mapping == Action.ActionName && !Action.Key.IsGamepadKey())
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

	// Collapse the keys if the main key is missing.
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

	//Set our new keys and readd them
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

