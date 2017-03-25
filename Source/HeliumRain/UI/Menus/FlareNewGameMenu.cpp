
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
	ScenarioList.Add(MakeShareable(new FString(TEXT("Freighter"))));
	ScenarioList.Add(MakeShareable(new FString(TEXT("Fighter"))));
	//ScenarioList.Add(MakeShareable(new FString(TEXT("Debug"))));

	// Color
	FLinearColor Color = Theme.NeutralColor;
	Color.A = Theme.DefaultAlpha;

	// Name
	FText DefaultName = LOCTEXT("CompanyName", "Player Inc");
	FText DefaultIdentifier = LOCTEXT("CompanyName", "PLY");
	FString PlayerName = MenuManager->GetPC()->PlayerState->PlayerName;
	if (PlayerName.Len())
	{
		DefaultName = FText::Format(LOCTEXT("CompanyNameFormat", "{0} Corp"), FText::FromString(PlayerName));
	}

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SHorizontalBox)

		// Content block
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.AutoWidth()
		[
			SNew(SVerticalBox)
		
			// Main form
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.ContentPadding)
			.HAlign(HAlign_Right)
			[
				SNew(SVerticalBox)
				
				// Company info
				+ SVerticalBox::Slot()
				.Padding(Theme.TitlePadding)
				.AutoHeight()
				[
					SNew(STextBlock)
					.TextStyle(&Theme.SubTitleFont)
					.Text(LOCTEXT("NewGameTitle", "NEMA COLONIAL ADMINISTRATION"))
				]
				
				// Company info
				+ SVerticalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoHeight()
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(LOCTEXT("NewGameHint", "Please enter the details of your new company."))
				]

				// Company name
				+ SVerticalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.ContentPadding)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(LOCTEXT("NewGameCompany", "Company name (Up to 25 characters)"))
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(0.4 * Theme.ContentWidth)
						[
							SNew(SBorder)
							.BorderImage(&Theme.BackgroundBrush)
							.Padding(Theme.ContentPadding)
							[
								SAssignNew(CompanyName, SEditableText)
								.Text(DefaultName)
								.Style(&Theme.TextInputStyle)
							]
						]
					]
				]
				
				// Company ID
				+ SVerticalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.ContentPadding)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(LOCTEXT("NewGameIdentifier", "Hull identifier (Three letters only)"))
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(0.4 * Theme.ContentWidth)
						[
							SNew(SBorder)
							.BorderImage(&Theme.BackgroundBrush)
							.Padding(Theme.ContentPadding)
							[
								SAssignNew(CompanyIdentifier, SEditableText)
								.Text(DefaultIdentifier)
								.Style(&Theme.TextInputStyle)
							]
						]
					]
				]

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
					.IsDisabled(this, &SFlareNewGameMenu::IsLaunchDisabled)
				]
			]
		]

		// Color box
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.Padding(Theme.ContentPadding)
		[
			SAssignNew(ColorBox, SFlareColorPanel)
			.MenuManager(MenuManager)
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
	SetVisibility(EVisibility::Collapsed);
}

void SFlareNewGameMenu::Enter()
{
	FLOG("SFlareNewGameMenu::Enter");

	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	AFlarePlayerController* PC = MenuManager->GetPC();
	
	// Default colors
	FFlareCompanyDescription Data;
	Data.CustomizationBasePaintColorIndex = 0;
	Data.CustomizationPaintColorIndex = 8;
	Data.CustomizationOverlayColorIndex = 4;
	Data.CustomizationLightColorIndex = 13;
	Data.CustomizationPatternIndex = 0;
	ColorBox->Setup(Data);

	// Get decorator mesh
	if (PC)
	{
		const FFlareSpacecraftComponentDescription* PartDesc = PC->GetGame()->GetShipPartsCatalog()->Get("object-safe");
		PC->GetMenuPawn()->SetCameraOffset(FVector2D(150, 30));
		PC->GetMenuPawn()->ShowPart(PartDesc);
		PC->GetMenuPawn()->SetCameraDistance(500);
	}
}

void SFlareNewGameMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

bool SFlareNewGameMenu::IsLaunchDisabled() const
{
	FString CompanyNameData = CompanyName->GetText().ToString();
	FString CompanyIdentifierData = CompanyIdentifier->GetText().ToString();

	if (CompanyNameData.Len() > 25 || CompanyIdentifierData.Len() != 3)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void SFlareNewGameMenu::OnLaunch()
{
	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC && Game && !Game->IsLoadedOrCreated())
	{
		// Get data
		FText CompanyNameData = FText::FromString(CompanyName->GetText().ToString().Left(25)); // FString needed here
		FName CompanyIdentifierData = FName(*CompanyIdentifier->GetText().ToString().ToUpper().Left(3)); // FString needed here
		int32 ScenarioIndex = ScenarioList.Find(ScenarioSelector->GetSelectedItem());
		FLOGV("SFlareNewGameMenu::OnLaunch '%s', ID '%s', ScenarioIndex %d", *CompanyNameData.ToString(), *CompanyIdentifierData.ToString(), ScenarioIndex);

		// Create company
		const FFlareCompanyDescription* CurrentCompanyData = PC->GetCompanyDescription();
		FFlareCompanyDescription CompanyData;
		CompanyData.Name = CompanyNameData;
		CompanyData.ShortName = CompanyIdentifierData;
		CompanyData.Emblem = NULL;
		CompanyData.CustomizationBasePaintColorIndex = CurrentCompanyData->CustomizationBasePaintColorIndex;
		CompanyData.CustomizationPaintColorIndex = CurrentCompanyData->CustomizationPaintColorIndex;
		CompanyData.CustomizationOverlayColorIndex = CurrentCompanyData->CustomizationOverlayColorIndex;
		CompanyData.CustomizationLightColorIndex = CurrentCompanyData->CustomizationLightColorIndex;
		CompanyData.CustomizationPatternIndex = CurrentCompanyData->CustomizationPatternIndex;

		// Create game
		Game->CreateGame(PC, CompanyData, ScenarioIndex, TutorialButton->IsActive());

		// Launch
		UFlareSimulatedSpacecraft* CurrentShip = PC->GetPlayerShip();
		if (CurrentShip)
		{
			UFlareSimulatedSector* Sector = CurrentShip->GetCurrentSector();
			PC->GetGame()->ActivateCurrentSector();

			FCHECK(PC->GetPlayerShip()->GetActive());

			FFlareMenuParameterData Data;
			Data.Spacecraft = PC->GetPlayerShip();
			MenuManager->OpenMenu(EFlareMenu::MENU_FlyShip, Data);
		}
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


#undef LOCTEXT_NAMESPACE

