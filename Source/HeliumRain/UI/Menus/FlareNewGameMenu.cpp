
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
	ScenarioList.Add(MakeShareable(new FString(TEXT("Transport"))));
	ScenarioList.Add(MakeShareable(new FString(TEXT("Defense"))));
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
				
				// Title
				+ SVerticalBox::Slot()
				.Padding(Theme.TitlePadding)
				.AutoHeight()
				[
					SNew(STextBlock)
					.TextStyle(&Theme.SubTitleFont)
					.Text(LOCTEXT("NewGameTitle", "NEMA COLONIAL ADMINISTRATION"))
				]

				// Scenario
				+ SVerticalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(LOCTEXT("NewGameScenario", "Company trade"))
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					[
						SNew(SBox)
						.WidthOverride(0.4 * Theme.ContentWidth)
						[
							SNew(SBorder)
							.BorderImage(&Theme.BackgroundBrush)
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
						]
					]
				]

				// Company name
				+ SVerticalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(LOCTEXT("NewGameCompany", "Company name (Up to 25 characters)"))
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
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
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(LOCTEXT("NewGameIdentifier", "Hull identifier (Three letters only)"))
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
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

				// Company ID hint
				+ SVerticalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SAssignNew(CompanyIDHint, STextBlock)
					.TextStyle(&Theme.TextFont)
				]

				// Tutorial
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Right)
				[
					SAssignNew(TutorialButton, SFlareButton)
					.Text(LOCTEXT("Tutorial", "Training contract"))
					.HelpText(LOCTEXT("TutorialInfo", "Start with a few training contracts"))
					.Toggle(true)
				]

				// Start
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Right)
				[
					SNew(SFlareButton)
					.Text(LOCTEXT("Start", "Start"))
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
	
	// Get decorator mesh
	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		const FFlareSpacecraftComponentDescription* PartDesc = PC->GetGame()->GetShipPartsCatalog()->Get("object-safe");
		PC->GetMenuPawn()->SetCameraOffset(FVector2D(150, 30));
		PC->GetMenuPawn()->ShowPart(PartDesc);
		PC->GetMenuPawn()->SetCameraDistance(500);
	}

	// Setup colors
	ColorBox->SetupDefault();

	// List forbidden companies
	FString ForbiddenIDsString;
	UFlareCompanyCatalog* CompanyList = Game->GetCompanyCatalog();
	for (const FFlareCompanyDescription& Company : CompanyList->Companies)
	{
		ForbiddenIDs.Add(Company.ShortName);

		if (ForbiddenIDsString.Len())
		{
			ForbiddenIDsString += " - ";
		}
		ForbiddenIDsString += Company.ShortName.ToString();
	}
	FText ForbiddenIdsText = FText::Format(LOCTEXT("ForbiddenIdListFormat", "Some identifiers are forbidden ({0})"), FText::FromString(ForbiddenIDsString));
	CompanyIDHint->SetText(ForbiddenIdsText);
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
	else if (ForbiddenIDs.Find(FName(*CompanyIdentifierData)) != INDEX_NONE)
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

		// Create company data
		const FFlareCompanyDescription* CurrentCompanyData = PC->GetCompanyDescription();
		CompanyData.Name = CompanyNameData;
		CompanyData.ShortName = CompanyIdentifierData;
		CompanyData.Emblem = NULL;
		CompanyData.CustomizationBasePaintColorIndex = CurrentCompanyData->CustomizationBasePaintColorIndex;
		CompanyData.CustomizationPaintColorIndex = CurrentCompanyData->CustomizationPaintColorIndex;
		CompanyData.CustomizationOverlayColorIndex = CurrentCompanyData->CustomizationOverlayColorIndex;
		CompanyData.CustomizationLightColorIndex = CurrentCompanyData->CustomizationLightColorIndex;
		CompanyData.CustomizationPatternIndex = CurrentCompanyData->CustomizationPatternIndex;
		
		// Create game
		FFlareMenuParameterData Data;
		Data.CompanyDescription = &CompanyData;
		Data.ScenarioIndex = ScenarioIndex;
		Data.PlayTutorial = TutorialButton->IsActive();
		MenuManager->OpenMenu(EFlareMenu::MENU_CreateGame, Data);
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

