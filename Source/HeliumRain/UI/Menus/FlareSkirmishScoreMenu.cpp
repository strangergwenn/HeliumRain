
#include "FlareSkirmishScoreMenu.h"
#include "../../Flare.h"

#include "../Components/FlareButton.h"

#include "../../Game/FlareGame.h"
#include "../../Game/FlareSaveGame.h"
#include "../../Game/FlareSkirmishManager.h"

#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareSkirmishScoreMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSkirmishScoreMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	int32 Width = Theme.ContentWidth;

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SVerticalBox)
		
		// Title
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(Theme.ContentPadding)
		[
			SAssignNew(SkirmishResultText, STextBlock)
			.TextStyle(&Theme.SpecialTitleFont)
		]
		
		// Time
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(Theme.ContentPadding)
		[
			SAssignNew(SkirmishTimeText, STextBlock)
			.TextStyle(&Theme.SubTitleFont)
		]
	
		// Main
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Fill)
		.Padding(Theme.ContentPadding)
		[
			SNew(SHorizontalBox)
			
			// Player
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(Width)
				[
					SAssignNew(PlayerResults, SVerticalBox)
				]
			]
			
			// Enemy
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(Width)
				[
					SAssignNew(EnemyResults, SVerticalBox)
				]
			]
		]
		
		// Back
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(Theme.ContentPadding)
		[
			SNew(SHorizontalBox)

			// Retry
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SFlareButton)
				.Transparent(true)
				.Width(3)
				.Text(LOCTEXT("Retry", "Retry"))
				.OnClicked(this, &SFlareSkirmishScoreMenu::OnRetry)
			]
			
			// Back
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SFlareButton)
				.Transparent(true)
				.Width(3)
				.Text(LOCTEXT("Exit", "Exit"))
				.OnClicked(this, &SFlareSkirmishScoreMenu::OnMainMenu)
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareSkirmishScoreMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareSkirmishScoreMenu::Enter()
{
	FLOG("SFlareSkirmishScoreMenu::Enter");

	// Setup
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	PlayerResults->ClearChildren();
	EnemyResults->ClearChildren();

	// Get data
	UFlareSkirmishManager* SkirmishManager = MenuManager->GetGame()->GetSkirmishManager();
	FCHECK(SkirmishManager);
	FFlareSkirmishResultData Result = SkirmishManager->GetResult();

	// Set title
	if (Result.PlayerVictory)
	{
		SkirmishResultText->SetText(LOCTEXT("SkirmishVictory", "Skirmish won"));
	}
	else
	{
		SkirmishResultText->SetText(LOCTEXT("SkirmishLoss", "Skirmish lost"));
	}

	// Set time
	int32 Minutes = FMath::Max(FMath::RoundToInt(Result.GameTime / 60), 1);
	FText MinutesText = (Minutes > 1) ? LOCTEXT("Minutes", "minutes") : LOCTEXT("Minute", "minute");
	SkirmishTimeText->SetText(
		FText::Format(LOCTEXT("TimeFormat", "Skirmish completed in {0} {1}"),
		FText::AsNumber(Minutes), MinutesText));

	// Set results
	FillResults(PlayerResults, LOCTEXT("PlayerTitle", "Player results"), Result.Player);
	FillResults(EnemyResults,  LOCTEXT("EnemyTitle",  "Enemy results"),  Result.Enemy);
}

void SFlareSkirmishScoreMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);

	PlayerResults->ClearChildren();
	EnemyResults->ClearChildren();
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareSkirmishScoreMenu::FillResults(TSharedPtr<SVerticalBox> Box, FText Title, FFlareSkirmishPlayerResult Result)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Title
	Box->AddSlot()
	.AutoHeight()
	.HAlign(HAlign_Left)
	.Padding(Theme.TitlePadding)
	[
		SNew(STextBlock)
		.Text(Title)
		.TextStyle(&Theme.SubTitleFont)
	];

	// Results
	AddResult(Box, LOCTEXT("ResultDisabled",  "Ships disabled"),    Result.ShipsDisabled);
	AddResult(Box, LOCTEXT("ResultDestroyed", "Ships destroyed"),   Result.ShipsDestroyed);
	AddResult(Box, LOCTEXT("ResultFired",     "Projectiles fired"), Result.AmmoFired);
	AddResult(Box, LOCTEXT("ResultHit",       "Successful hits"),   Result.AmmoHit);
}

void SFlareSkirmishScoreMenu::AddResult(TSharedPtr<SVerticalBox> Box, FText Name, int32 Result)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	// Slot
	Box->AddSlot()
	.AutoHeight()
	.HAlign(HAlign_Left)
	.Padding(Theme.ContentPadding)
	[
		SNew(SHorizontalBox)

		// Stat name
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(Theme.ContentWidth / 3)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TextFont)
				.Text(Name)
			]
		]

		// Stat value
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(Theme.ContentWidth / 3)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TextFont)
				.Text(FText::AsNumber(Result))
			]
		]
	];
}

void SFlareSkirmishScoreMenu::OnRetry()
{
	UFlareSkirmishManager* SkirmishManager = MenuManager->GetGame()->GetSkirmishManager();
	FCHECK(SkirmishManager);
	SkirmishManager->RestartPlay();
}

void SFlareSkirmishScoreMenu::OnMainMenu()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Main);
}


#undef LOCTEXT_NAMESPACE

