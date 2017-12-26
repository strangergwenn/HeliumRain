
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
	int32 Width = 1.75 * Theme.ContentWidth;
	int32 TextWidth = Width - 2 * Theme.ContentPadding.Left - 2 * Theme.ContentPadding.Right;

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
	
		// Main
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Fill)
		.Padding(Theme.ContentPadding)
		
		// Back
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(Theme.ContentPadding)
		[
			SNew(SFlareButton)
			.Transparent(true)
			.Width(3)
			.Text(LOCTEXT("Back", "Back"))
			.OnClicked(this, &SFlareSkirmishScoreMenu::OnMainMenu)
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
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	UFlareSkirmishManager* SkirmishManager = MenuManager->GetGame()->GetSkirmishManager();
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
}

void SFlareSkirmishScoreMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareSkirmishScoreMenu::OnMainMenu()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Main);
}


#undef LOCTEXT_NAMESPACE

