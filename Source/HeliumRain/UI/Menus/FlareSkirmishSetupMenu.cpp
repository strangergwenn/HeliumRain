
#include "FlareSkirmishSetupMenu.h"
#include "../../Flare.h"

#include "../Components/FlareButton.h"

#include "../../Data/FlareCustomizationCatalog.h"

#include "../../Game/FlareGame.h"
#include "../../Game/FlareSaveGame.h"
#include "../../Game/FlareSkirmishManager.h"

#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareSkirmishSetupMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSkirmishSetupMenu::Construct(const FArguments& InArgs)
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
			SNew(STextBlock)
			.Text(LOCTEXT("SkirmishSetupTitle", "New skirmish"))
			.TextStyle(&Theme.SpecialTitleFont)
		]

		// Add ship
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.AutoHeight()
		[
			SNew(SFlareButton)
			.Transparent(true)
			.Width(3)
			.Text(LOCTEXT("AddPlayerShip", "Add player ship"))
			.OnClicked(this, &SFlareSkirmishSetupMenu::OnOrderShip, true)
		]

		// Add ship
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.AutoHeight()
		[
			SNew(SFlareButton)
			.Transparent(true)
			.Width(3)
			.Text(LOCTEXT("AddEnemyShip", "Add enemy ship"))
			.OnClicked(this, &SFlareSkirmishSetupMenu::OnOrderShip, false)
		]

		// Start
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.AutoHeight()
		[
			SNew(SFlareButton)
			.Transparent(true)
			.Width(3)
			.Text(LOCTEXT("Start", "Start"))
			.OnClicked(this, &SFlareSkirmishSetupMenu::OnStartSkirmish)
		]

		// Back
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.AutoHeight()
		[
			SNew(SFlareButton)
			.Transparent(true)
			.Width(3)
			.Text(LOCTEXT("Back", "Back"))
			.OnClicked(this, &SFlareSkirmishSetupMenu::OnMainMenu)
		]
	];

	// TODO 1075 : real menu, spacecraft lists...
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareSkirmishSetupMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareSkirmishSetupMenu::Enter()
{
	FLOG("SFlareSkirmishSetupMenu::Enter");
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	MenuManager->GetGame()->GetSkirmishManager()->StartSetup();
	MenuManager->GetGame()->GetSkirmishManager()->SetAllowedValue(true, 100);
	MenuManager->GetGame()->GetSkirmishManager()->SetAllowedValue(false, 100);
}

void SFlareSkirmishSetupMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareSkirmishSetupMenu::OnOrderShip(bool ForPlayer)
{
	IsOrderingForPlayer = ForPlayer;

	FFlareMenuParameterData Data;
	Data.OrderForPlayer = ForPlayer;
	Data.Skirmish = MenuManager->GetGame()->GetSkirmishManager();
	MenuManager->OpenSpacecraftOrder(Data, FOrderDelegate::CreateSP(this, &SFlareSkirmishSetupMenu::OnOrderShipConfirmed));
}

void SFlareSkirmishSetupMenu::OnOrderShipConfirmed(FFlareSpacecraftDescription* Spacecraft)
{
	MenuManager->GetGame()->GetSkirmishManager()->AddShip(IsOrderingForPlayer, Spacecraft);
}

void SFlareSkirmishSetupMenu::OnStartSkirmish()
{
	MenuManager->GetGame()->GetSkirmishManager()->StartPlay();

	// Get company defaults
	AFlarePlayerController* PC = MenuManager->GetPC();
	UFlareCustomizationCatalog* CustomizationCatalog = PC->GetGame()->GetCustomizationCatalog();
	const FFlareCompanyDescription* CurrentCompanyData = PC->GetCompanyDescription();

	// Override defaults
	PlayerCompanyData.Name = FText::FromString("Player");
	PlayerCompanyData.ShortName = "PLY";
	PlayerCompanyData.Emblem = CustomizationCatalog->GetEmblem(0);
	PlayerCompanyData.CustomizationBasePaintColor = CurrentCompanyData->CustomizationBasePaintColor;
	PlayerCompanyData.CustomizationPaintColor = CurrentCompanyData->CustomizationPaintColor;
	PlayerCompanyData.CustomizationOverlayColor = CurrentCompanyData->CustomizationOverlayColor;
	PlayerCompanyData.CustomizationLightColor = CurrentCompanyData->CustomizationLightColor;
	PlayerCompanyData.CustomizationPatternIndex = CurrentCompanyData->CustomizationPatternIndex;

	// Create the game
	FFlareMenuParameterData Data;
	Data.Skirmish = MenuManager->GetGame()->GetSkirmishManager();
	Data.CompanyDescription = &PlayerCompanyData;
	MenuManager->OpenMenu(EFlareMenu::MENU_CreateGame, Data);
}

void SFlareSkirmishSetupMenu::OnMainMenu()
{
	MenuManager->GetGame()->GetSkirmishManager()->EndSkirmish();

	MenuManager->OpenMenu(EFlareMenu::MENU_Main);
}


#undef LOCTEXT_NAMESPACE

