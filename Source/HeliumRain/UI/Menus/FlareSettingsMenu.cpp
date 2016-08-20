
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
	float CurrentAntiAliasingQualityRatio = MyGameSettings->ScalabilityQuality.AntiAliasingQuality / 3.f;
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
	int32 ValueSize = 80;

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SVerticalBox)

		// Main form
		+ SVerticalBox::Slot()
		.Padding(Theme.ContentPadding)
		.HAlign(HAlign_Left)
		[
			SNew(SScrollBox)
			.Style(&Theme.ScrollBoxStyle)
			.ScrollBarStyle(&Theme.ScrollBarStyle)

			+ SScrollBox::Slot()
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
				.HAlign(HAlign_Center)
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

							// Supersampling
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.SmallContentPadding)
							[
								SAssignNew(SupersamplingButton, SFlareButton)
								.Text(LOCTEXT("Supersampling", "Supersampling (!)"))
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

				// Gameplay info
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.TitlePadding)
				.HAlign(HAlign_Left)
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
						.HelpText(LOCTEXT("CockpitInfo", "Use the 3D cockpit instead of a flat interface"))
						.Toggle(true)
						.OnClicked(this, &SFlareSettingsMenu::OnCockpitToggle)
					]
					
					// Pause in menus
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					[
						SAssignNew(PauseInMenusButton, SFlareButton)
						.Text(LOCTEXT("PauseInMenus", "Pause game in menus"))
						.HelpText(LOCTEXT("PauseInMenusInfo", "Pause the game when entering a full-screen menu"))
						.Toggle(true)
						.OnClicked(this, &SFlareSettingsMenu::OnPauseInMenusToggle)
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

				// Controls Info
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
			]
		]
	];

	// Default settings
	VSyncButton->SetActive(MyGameSettings->IsVSyncEnabled());
	FullscreenButton->SetActive(MyGameSettings->GetFullscreenMode() == EWindowMode::Fullscreen);
	SupersamplingButton->SetActive(MyGameSettings->ScreenPercentage > 100);
	CockpitButton->SetActive(MyGameSettings->UseCockpit);
	PauseInMenusButton->SetActive(MyGameSettings->PauseGameInMenus);

	// Music volume
	int32 MusicVolume = MyGameSettings->MusicVolume;
	MusicVolumeSlider->SetValue((float)MusicVolume / 10.0f);
	MusicVolumeLabel->SetText(GetMusicVolumeLabel(MusicVolume));
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
	[
		SAssignNew(ControlList, SVerticalBox)
	];

	if (KeyboardBox.IsValid())
	{
		// Create the bind list
		for (const auto& Bind : Binds)
		{
			// Header block
			if (Bind->bHeader)
			{
				ControlList->AddSlot()
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
				ControlList->AddSlot()
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

	return KeyboardBox.ToSharedRef();
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

	if (PC)
	{
		// Decorator mesh
		const FFlareSpacecraftComponentDescription* PartDesc = PC->GetGame()->GetShipPartsCatalog()->Get("object-safe");
		PC->GetMenuPawn()->SetCameraOffset(FVector2D(100, -30));
		PC->GetMenuPawn()->ShowPart(PartDesc);
	}

	// Resolutions
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

void SFlareSettingsMenu::OnPauseInMenusToggle()
{
	bool New = PauseInMenusButton->IsActive();
	MenuManager->GetPC()->SetPauseGameInMenus(New);

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	MyGameSettings->PauseGameInMenus = New;
	MyGameSettings->ApplySettings(false);
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
	int32 Step = 3;
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

	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();
	MyGameSettings->SetVSyncEnabled(VSyncButton->IsActive());
	MyGameSettings->ApplySettings(false);
}

void SFlareSettingsMenu::OnSupersamplingToggle()
{
	if (SupersamplingButton->IsActive())
	{
		FLOG("SFlareSettingsMenu::OnSupersamplingToggle : Enable supersampling (2K)")
	}
	else
	{
		FLOG("SFlareSettingsMenu::OnSupersamplingToggle : Disable supersampling (2K)")
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
			return LOCTEXT("TextureQualityLow", "Low");
		case 2:
			return LOCTEXT("TextureQualityMedium", "Medium");
		case 3:
			return LOCTEXT("TextureQualityHigh", "High");
		case 0:
		default:
			return LOCTEXT("TextureQualityVeryLow", "Very Low");
	}
}

FText SFlareSettingsMenu::GetEffectsQualityLabel(int32 Value) const
{
	switch(Value)
	{
		case 1:
			return LOCTEXT("EffectsQualityLow", "Low");
		case 2:
			return LOCTEXT("EffectsQualityMedium", "Medium");
		case 3:
			return LOCTEXT("EffectsQualityHigh", "High");
		case 0:
		default:
			return LOCTEXT("EffectsQualityVeryLow", "Very Low");
	}
}

FText SFlareSettingsMenu::GetAntiAliasingQualityLabel(int32 Value) const
{
	switch(Value)
	{
		case 1:
			return LOCTEXT("AntiAliasingQualityLow", "Low");
		case 2:
			return LOCTEXT("AntiAliasingQualityMedium", "Medium");
		case 3:
			return LOCTEXT("AntiAliasingQualityHigh", "High");
		case 0:
		default:
			return LOCTEXT("AntiAliasingQualityDisabled", "Disabled");		
	}
}

FText SFlareSettingsMenu::GetPostProcessQualityLabel(int32 Value) const
{
	switch(Value)
	{
		case 1:
			return LOCTEXT("PostProcessQualityLow", "Low");
		case 2:
			return LOCTEXT("PostProcessQualityMedium", "Medium");
		case 3:
			return LOCTEXT("PostProcessQualityHigh", "High");
		case 0:
		default:
			return LOCTEXT("PostProcessQualityVeryLow", "Very Low");
	}
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
	//Piloting
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Flying", "FLYING")))->MakeHeader()));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("MoveForward", "Move Forward")))
		->AddAxisMapping("Thrust", 1.0f)
		->AddDefaults(EKeys::W)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("MoveBackward", "Move Backward")))
		->AddAxisMapping("Thrust", -1.0f)
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
		->AddAxisMapping("RollInput", 1.0f)
		->AddDefaults(EKeys::A)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("RollCCW", "Roll Left")))
		->AddAxisMapping("RollInput", -1.0f)
		->AddDefaults(EKeys::E)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("LockDirection", "Lock direction")))
		->AddActionMapping("LockDirection")
		->AddDefaults(EKeys::SpaceBar)));

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
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Toogle Weapons", "Toogle Weapons")))
		->AddActionMapping("ToggleCombat")
		->AddDefaults(EKeys::F)));

	//Misc
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("End day", "End day")))
		->AddActionMapping("Simulate")
		->AddDefaults(EKeys::F6)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Misc", "MISCELLANEOUS")))->MakeHeader()));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Quick Ship Switch", "Quick Ship Switch")))
		->AddActionMapping("QuickSwitch")
		->AddDefaults(EKeys::N)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Toogle Camera", "Toogle Camera")))
		->AddActionMapping("ToggleCamera")
		->AddDefaults(EKeys::C)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Toogle HUD", "Toogle HUD")))
		->AddActionMapping("ToggleHUD")
		->AddDefaults(EKeys::H)));
	Binds.Add(MakeShareable((new FSimpleBind(LOCTEXT("Open settings menu", "Open settings menu")))
		->AddActionMapping("SettingsMenu")
		->AddDefaults(EKeys::F10)));

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

