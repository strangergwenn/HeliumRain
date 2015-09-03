
#include "../../Flare.h"
#include "FlareSettingsMenu.h"
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

	// Game starts


	//const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();


	FIntPoint Resolution = MyGameSettings->GetScreenResolution();



	FLOGV("Resolution: %s", *Resolution.ToString());
	FScreenResolutionArray Resolutions;
	int CurrentResolutionIndex = -1;

	if (RHIGetAvailableResolutions(Resolutions, true))
	{
		for (const FScreenResolutionRHI& EachResolution : Resolutions)
		{
			ResolutionList.Insert(MakeShareable(new FScreenResolutionRHI(EachResolution)),0);

			if (Resolution.X == EachResolution.Width && Resolution.Y == EachResolution.Height)
			{
				CurrentResolutionIndex = ResolutionList.Num() -1 ;
			}
		}
	}
	else
	{
		FLOG("Screen Resolutions could not be obtained");
	}

	if (CurrentResolutionIndex < 0)
	{
		TSharedPtr<FScreenResolutionRHI> InitialResolution = MakeShareable(new FScreenResolutionRHI());
		InitialResolution->Width = Resolution.X;
		InitialResolution->Height = Resolution.Y;

		ResolutionList.Insert(InitialResolution,0);
		CurrentResolutionIndex = 0;
	}

	float CurrentTextureQualityRatio = MyGameSettings->ScalabilityQuality.TextureQuality / 3.f;
	float CurrentEffectsQualityRatio = MyGameSettings->ScalabilityQuality.EffectsQuality / 3.f;
	float CurrentAntiAliasingQualityRatio = MyGameSettings->ScalabilityQuality.AntiAliasingQuality / 3.f;
	float CurrentPostProcessQualityRatio = MyGameSettings->ScalabilityQuality.PostProcessQuality / 3.f;

	FLOGV("MyGameSettings->ScalabilityQuality.TextureQuality=%d CurrentTextureQualityRatio=%f", MyGameSettings->ScalabilityQuality.TextureQuality, CurrentTextureQualityRatio);
	FLOGV("MyGameSettings->ScalabilityQuality.EffectsQuality=%d CurrentEffectsQualityRatio=%f", MyGameSettings->ScalabilityQuality.EffectsQuality, CurrentEffectsQualityRatio);
	FLOGV("MyGameSettings->ScalabilityQuality.AntiAliasingQuality=%d CurrentAntiAliasingQualityRatio=%f", MyGameSettings->ScalabilityQuality.AntiAliasingQuality, CurrentAntiAliasingQualityRatio);
	FLOGV("MyGameSettings->ScalabilityQuality.PostProcessQuality=%d CurrentPostProcessQualityRatio=%f", MyGameSettings->ScalabilityQuality.PostProcessQuality, CurrentPostProcessQualityRatio);


	// Color
	FLinearColor Color = Theme.NeutralColor;
	Color.A = Theme.DefaultAlpha;

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.Padding(Theme.ContentPadding)
		[
			SNew(SHorizontalBox)

			// Icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SImage).Image(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Main))
			]

			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TitleFont)
				.Text(LOCTEXT("Settings", "SETTINGS"))
			]

			// Close
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("Back", "Back"))
				.HelpText(LOCTEXT("BakcInfo", "Go back to the main menu"))
				.Icon(FFlareStyleSet::GetIcon("Ok"))
				.OnClicked(this, &SFlareSettingsMenu::OnExit)
			]
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(200, 20))
		[
			SNew(SImage).Image(&Theme.SeparatorBrush)
		]

		// Main form
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.ContentPadding)
		.HAlign(HAlign_Center)
		[

			SNew(SScrollBox)
			.Style(&Theme.ScrollBoxStyle)
			.ScrollBarStyle(&Theme.ScrollBarStyle)

			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)

				// Info
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("GraphicsSettingsHint", "Graphics"))
					.TextStyle(&Theme.SubTitleFont)
				]

				// Graphic form
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(Theme.ContentWidth / 1.5)
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
							.InitiallySelectedItem(ResolutionList[CurrentResolutionIndex])
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

						// Fullscreen
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						.HAlign(HAlign_Right)
						[
							SAssignNew(FullscreenButton, SFlareButton)
							.Text(LOCTEXT("Fullscreen", "Fullscreen"))
							.HelpText(LOCTEXT("FullscreenInfo", "Enable fullscreen"))
							.Toggle(true)
							.OnClicked(this, &SFlareSettingsMenu::OnFullscreenToggle)
						]

						// VSync
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						.HAlign(HAlign_Right)
						[
							SAssignNew(VSyncButton, SFlareButton)
							.Text(LOCTEXT("V-sync", "V-sync"))
							.HelpText(LOCTEXT("VSyncInfo", "Enable v-sync"))
							.Toggle(true)
							.OnClicked(this, &SFlareSettingsMenu::OnVSyncToggle)
						]

						// Texture quality box
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(SHorizontalBox)

							// Texture quality text
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.ContentPadding)
							[
								SNew(SBox)
								.WidthOverride(150)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("TextureLabel", "Textures"))
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
								.WidthOverride(80)
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
						.Padding(Theme.ContentPadding)
						[
							SNew(SHorizontalBox)

							// Effects quality text
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.ContentPadding)
							[
								SNew(SBox)
								.WidthOverride(150)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("EffectsLabel", "Effects"))
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
								.WidthOverride(80)
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
						.Padding(Theme.ContentPadding)
						[
							SNew(SHorizontalBox)

							// Anti aliasing quality text
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.ContentPadding)
							[
								SNew(SBox)
								.WidthOverride(150)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("AntiAliasingLabel", "Anti Aliasing"))
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
								.WidthOverride(80)
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
						.Padding(Theme.ContentPadding)
						[
							SNew(SHorizontalBox)

							// PostProcess quality text
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.ContentPadding)
							[
								SNew(SBox)
								.WidthOverride(150)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("PostProcessLabel", "Post Process"))
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
								.WidthOverride(80)
								[
									SAssignNew(PostProcessQualityLabel, STextBlock)
									.TextStyle(&Theme.TextFont)
									.Text(GetPostProcessQualityLabel(MyGameSettings->ScalabilityQuality.PostProcessQuality))
								]
							]
						]

						// Supersampling
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						.HAlign(HAlign_Right)
						[
							SAssignNew(SupersamplingButton, SFlareButton)
							.Text(LOCTEXT("Supersampling", "Supersampling"))
							.HelpText(LOCTEXT("SupersamplingInfo", "Enable Supersampling"))
							.Toggle(true)
							.OnClicked(this, &SFlareSettingsMenu::OnSupersamplingToggle)
						]
					]
				]
			]
		]
	];

	FullscreenButton->SetActive(MyGameSettings->GetFullscreenMode() == EWindowMode::Fullscreen);
	VSyncButton->SetActive(MyGameSettings->IsVSyncEnabled());
	SupersamplingButton->SetActive(MyGameSettings->ScalabilityQuality.ResolutionQuality > 100);
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareSettingsMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}

void SFlareSettingsMenu::Enter()
{
	FLOG("SFlareSettingsMenu::Enter");
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		FFlarePlayerSave Data;
		FFlareCompanyDescription Unused;
		PC->Save(Data, Unused);
		//ColorBox->Setup(Data);
	}
}

void SFlareSettingsMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/


FText SFlareSettingsMenu::OnGetCurrentResolutionComboLine() const
{
	TSharedPtr<FScreenResolutionRHI> Item = ResolutionSelector->GetSelectedItem();

	//return ResolutionSelector->GetSelectedItem();

	//return Item.IsValid() ? FText::FromString(*Item) : FText::FromString(*ResolutionList[0]);
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

void SFlareSettingsMenu::OnTextureQualitySliderChanged(float Value)
{
	int32 Step = 3;
	int32 StepValue = FMath::RoundToInt(Step * Value);
	TextureQualitySlider->SetValue((float)StepValue / (float)Step);

	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();
	FLOGV("Set Texture quality to %d (current is %d)", StepValue, MyGameSettings->ScalabilityQuality.TextureQuality);

	if(MyGameSettings->ScalabilityQuality.TextureQuality != StepValue)
	{
		MyGameSettings->ScalabilityQuality.TextureQuality = StepValue;
		MyGameSettings->ApplySettings();
		TextureQualityLabel->SetText(GetTextureQualityLabel(StepValue));
	}
}

void SFlareSettingsMenu::OnEffectsQualitySliderChanged(float Value)
{
	int32 Step = 3;
	int32 StepValue = FMath::RoundToInt(Step * Value);
	EffectsQualitySlider->SetValue((float)StepValue / (float)Step);

	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();
	FLOGV("Set Effects quality to %d (current is %d)", StepValue, MyGameSettings->ScalabilityQuality.EffectsQuality);

	if(MyGameSettings->ScalabilityQuality.EffectsQuality != StepValue)
	{
		MyGameSettings->ScalabilityQuality.EffectsQuality = StepValue;
		MyGameSettings->ApplySettings();
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
	FLOGV("Set AntiAliasing quality to %d (current is %d)", AAValue, MyGameSettings->ScalabilityQuality.AntiAliasingQuality);

	if(MyGameSettings->ScalabilityQuality.AntiAliasingQuality != AAValue)
	{
		MyGameSettings->ScalabilityQuality.AntiAliasingQuality = AAValue;
		MyGameSettings->ApplySettings();
		AntiAliasingQualityLabel->SetText(GetAntiAliasingQualityLabel(AAValue));
	}
}

void SFlareSettingsMenu::OnPostProcessQualitySliderChanged(float Value)
{
	int32 Step = 3;
	int32 StepValue = FMath::RoundToInt(Step * Value);
	PostProcessQualitySlider->SetValue((float)StepValue / (float)Step);

	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();
	FLOGV("Set PostProcess quality to %d (current is %d)", StepValue, MyGameSettings->ScalabilityQuality.PostProcessQuality);

	if(MyGameSettings->ScalabilityQuality.PostProcessQuality != StepValue)
	{
		MyGameSettings->ScalabilityQuality.PostProcessQuality = StepValue;
		MyGameSettings->ApplySettings();
		PostProcessQualityLabel->SetText(GetPostProcessQualityLabel(StepValue));
	}
}

void SFlareSettingsMenu::OnVSyncToggle()
{
	if(VSyncButton->IsActive())
	{
		FLOG("Enable vsync")

	}
	else
	{
		FLOG("Disable vsync")
	}

	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();
	MyGameSettings->SetVSyncEnabled(VSyncButton->IsActive());
	MyGameSettings->ApplySettings();
}

void SFlareSettingsMenu::OnSupersamplingToggle()
{
	if(SupersamplingButton->IsActive())
	{
		FLOG("Enable supersampling")

	}
	else
	{
		FLOG("Disable supersampling")
	}

	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();


	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
	CVar->Set((SupersamplingButton->IsActive() ? 200 : 100), ECVF_SetByScalability);

	MyGameSettings->ApplySettings();

	//FLOGV("MyGameSettings->ScalabilityQuality.ResolutionQuality %d", MyGameSettings->ScalabilityQuality.ResolutionQuality);

}

void SFlareSettingsMenu::OnExit()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Main);
}
/*----------------------------------------------------
	Helpers
----------------------------------------------------*/


void SFlareSettingsMenu::UpdateResolution()
{
	if(GEngine)
	{
		UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();


		FIntPoint Resolution = MyGameSettings->GetScreenResolution();
		FLOGV("GetScreenResolution: %s", *Resolution.ToString());

		TSharedPtr<FScreenResolutionRHI> Item = ResolutionSelector->GetSelectedItem();

		MyGameSettings->SetScreenResolution(FIntPoint(Item->Width, Item->Height));
		MyGameSettings->SetFullscreenMode(FullscreenButton->IsActive() ? EWindowMode::Fullscreen : EWindowMode::Windowed);
		MyGameSettings->RequestResolutionChange(Item->Width, Item->Height, FullscreenButton->IsActive() ? EWindowMode::Fullscreen : EWindowMode::Windowed, false);

		MyGameSettings->ConfirmVideoMode();
		MyGameSettings->ApplySettings();
		/*MyGameSettings->ApplyNonResolutionSettings();
		MyGameSettings->SaveSettings();*/
	}
}

FText SFlareSettingsMenu::GetTextureQualityLabel(int32 Value)
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
			return LOCTEXT("TextureQualityVeryLow", "Very low");
	}
}

FText SFlareSettingsMenu::GetEffectsQualityLabel(int32 Value)
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
			return LOCTEXT("EffectsQualityVeryLow", "Very low");
	}
}

FText SFlareSettingsMenu::GetAntiAliasingQualityLabel(int32 Value)
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

FText SFlareSettingsMenu::GetPostProcessQualityLabel(int32 Value)
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
			return LOCTEXT("PostProcessQualityVeryLow", "Very low");
	}
}

#undef LOCTEXT_NAMESPACE

