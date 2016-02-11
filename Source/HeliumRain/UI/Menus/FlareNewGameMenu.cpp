
#include "../../Flare.h"
#include "FlareNewGameMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareSaveGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "STextComboBox.h"


#define LOCTEXT_NAMESPACE "FlareNewGameMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareNewGameMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	Game = MenuManager->GetPC()->GetGame();

	// Game starts
	ScenarioList.Add(MakeShareable(new FString(TEXT("Freigther"))));
	ScenarioList.Add(MakeShareable(new FString(TEXT("Fighter"))));
	ScenarioList.Add(MakeShareable(new FString(TEXT("Debug"))));

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
				.Text(LOCTEXT("NewGame", "NEW GAME"))
			]

			// Close
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("Cancel", "Cancel"))
				.HelpText(LOCTEXT("CancelInfo", "Cancel and go back to the main menu"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Exit, true))
				.OnClicked(this, &SFlareNewGameMenu::OnExit)
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
			.Text(LOCTEXT("NewGameHint", "Please describe your company."))
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
				
				// Company name
				+ SVerticalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoHeight()
				[
					SNew(SBorder)
					.BorderImage(&Theme.BackgroundBrush)
					.Padding(Theme.ContentPadding)
					[
						SAssignNew(CompanyName, SEditableText)
						.Text(LOCTEXT("CompanyName", "Player Inc"))
						.Style(&Theme.TextInputStyle)
					]
				]

				// Color picker
				//+ SVerticalBox::Slot()
				//.Padding(Theme.ContentPadding)
				//.AutoHeight()
				//[
				//	SAssignNew(ColorBox, SFlareColorPanel)
				//	.MenuManager(MenuManager)
				//]

				// Scenario
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SAssignNew(ScenarioSelector, SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&ScenarioList)
					.InitiallySelectedItem(ScenarioList[0])
					.OnGenerateWidget(this, &SFlareNewGameMenu::OnGenerateComboLine)
					.OnSelectionChanged(this, &SFlareNewGameMenu::OnComboLineSelectionChanged)
					.ComboBoxStyle(&Theme.ComboBoxStyle)
					.ForegroundColor(FLinearColor::White)
					[
						SNew(STextBlock)
						.Text(this, &SFlareNewGameMenu::OnGetCurrentComboLine)
						.TextStyle(&Theme.TextFont)
					]
				]

				// Tutorial
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Right)
				[
					SAssignNew(TutorialButton, SFlareButton)
					.Text(LOCTEXT("Tutorial", "Play tutorial"))
					.HelpText(LOCTEXT("TutorialInfo", "Start with a few tutorial missions"))
					.Toggle(true)
				]

				// Start
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Right)
				[
					SNew(SFlareButton)
					.Text(LOCTEXT("Start", "Start the game"))
					.HelpText(LOCTEXT("StartInfo", "Confirm the creation of a new game and start playing"))
					.Icon(FFlareStyleSet::GetIcon("Load"))
					.OnClicked(this, &SFlareNewGameMenu::OnLaunch)
				]
			]
		]
	];

	TutorialButton->SetActive(true);
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareNewGameMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}

void SFlareNewGameMenu::Enter()
{
	FLOG("SFlareNewGameMenu::Enter");
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

void SFlareNewGameMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareNewGameMenu::OnLaunch()
{
	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC && Game && !Game->IsLoadedOrCreated())
	{
		// Get data
		FText CompanyNameData = FText::FromString(CompanyName->GetText().ToString().ToUpper().Left(60)); // FString needed here
		int32 ScenarioIndex = ScenarioList.Find(ScenarioSelector->GetSelectedItem());
		FLOGV("SFlareNewGameMenu::OnLaunch : '%s', scenario %d", *CompanyNameData.ToString(), ScenarioIndex);

		// Launch
		Game->CreateGame(PC, CompanyNameData, ScenarioIndex, TutorialButton->IsActive());
		MenuManager->OpenMenu(EFlareMenu::MENU_Orbit);
	}
}

FText SFlareNewGameMenu::OnGetCurrentComboLine() const
{
	TSharedPtr<FString> Item = ScenarioSelector->GetSelectedItem();
	return Item.IsValid() ? FText::FromString(*Item) : FText::FromString(*ScenarioList[0]);
}

TSharedRef<SWidget> SFlareNewGameMenu::OnGenerateComboLine(TSharedPtr<FString> Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(STextBlock)
	.Text(FText::FromString(*Item)) // FString needed here
	.TextStyle(&Theme.TextFont);
}

void SFlareNewGameMenu::OnComboLineSelectionChanged(TSharedPtr<FString> StringItem, ESelectInfo::Type SelectInfo)
{
}

void SFlareNewGameMenu::OnExit()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Main);
}




#undef LOCTEXT_NAMESPACE

