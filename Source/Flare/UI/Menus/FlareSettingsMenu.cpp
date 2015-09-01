
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

		// Info
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.ContentPadding)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("GraphicsSettingsHint", "Graphics."))
			.TextStyle(&Theme.SubTitleFont)
		]

		// Main form
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.ContentPadding)
		.HAlign(HAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(Theme.ContentWidth / 2)
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
					.OnGenerateWidget(this, &SFlareSettingsMenu::OnGenerateComboLine)
					.OnSelectionChanged(this, &SFlareSettingsMenu::OnComboLineSelectionChanged)
					.ComboBoxStyle(&Theme.ComboBoxStyle)
					.ForegroundColor(FLinearColor::White)
					[
						SNew(STextBlock)
						.Text(this, &SFlareSettingsMenu::OnGetCurrentComboLine)
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
				]
		]
	];

	FullscreenButton->SetActive(MyGameSettings->GetFullscreenMode() == EWindowMode::Fullscreen);
	VSyncButton->SetActive(MyGameSettings->IsVSyncEnabled());
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


FText SFlareSettingsMenu::OnGetCurrentComboLine() const
{
	TSharedPtr<FScreenResolutionRHI> Item = ResolutionSelector->GetSelectedItem();

	//return ResolutionSelector->GetSelectedItem();

	//return Item.IsValid() ? FText::FromString(*Item) : FText::FromString(*ResolutionList[0]);
	return Item.IsValid() ? FText::FromString(FString::Printf(TEXT("%dx%d"), Item->Width, Item->Height)) : FText::FromString("");
}

TSharedRef<SWidget> SFlareSettingsMenu::OnGenerateComboLine(TSharedPtr<FScreenResolutionRHI> Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(STextBlock)
	.Text(FText::FromString(FString::Printf(TEXT("%dx%d"), Item->Width, Item->Height)))
	.TextStyle(&Theme.TextFont);
}

void SFlareSettingsMenu::OnComboLineSelectionChanged(TSharedPtr<FScreenResolutionRHI> StringItem, ESelectInfo::Type SelectInfo)
{
	UpdateResolution();
}

void SFlareSettingsMenu::OnFullscreenToggle()
{
	UpdateResolution();
}

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
	MyGameSettings->ConfirmVideoMode();
	MyGameSettings->ApplySettings();
}

void SFlareSettingsMenu::OnExit()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Main);
}




#undef LOCTEXT_NAMESPACE

