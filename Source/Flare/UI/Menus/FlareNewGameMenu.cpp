
#include "../../Flare.h"
#include "FlareNewGameMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareSaveGame.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"


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
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(200, 30))
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

		// UI
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.ContentPadding)
		[
			SNew(SFlareButton)
			.Text(LOCTEXT("Start", "Start the game"))
			.OnClicked(this, &SFlareNewGameMenu::OnLaunch)
		]
	];

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
	if (PC && Game)
	{
		Game->CreateWorld(PC);
		MenuManager->OpenMenu(EFlareMenu::MENU_FlyShip, PC->GetShipPawn());
	}
}


#undef LOCTEXT_NAMESPACE

