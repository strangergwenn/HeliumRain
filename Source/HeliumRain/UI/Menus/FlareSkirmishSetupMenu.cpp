
#include "FlareSkirmishSetupMenu.h"
#include "../../Flare.h"

#include "../Components/FlareButton.h"

#include "../../Data/FlareCompanyCatalog.h"
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
	
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.AutoHeight()
		[
			SAssignNew(CompanySelector, SFlareDropList<FFlareCompanyDescription>)
			.OptionsSource(&MenuManager->GetPC()->GetGame()->GetCompanyCatalog()->Companies)
			.OnGenerateWidget(this, &SFlareSkirmishSetupMenu::OnGenerateCompanyComboLine)
			.OnSelectionChanged(this, &SFlareSkirmishSetupMenu::OnCompanyComboLineSelectionChanged)
			.HeaderWidth(5)
			.ItemWidth(5)
			[
				SNew(SBox)
				.Padding(Theme.ListContentPadding)
				[
					SNew(STextBlock)
					.Text(this, &SFlareSkirmishSetupMenu::OnGetCurrentCompanyComboLine)
					.TextStyle(&Theme.TextFont)
				]
			]
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

	// TODO 1075 : budget picker
	// TODO 1075 : spacecraft lists
	// TODO 1075 : spacecraft customization
	// TODO 1075 : company customization
	// TODO 1075 : sector settings

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

	CompanySelector->RefreshOptions();

	// Start doing the setup
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
	Content callbacks
----------------------------------------------------*/

TSharedRef<SWidget> SFlareSkirmishSetupMenu::OnGenerateCompanyComboLine(FFlareCompanyDescription Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(SBox)
	.Padding(Theme.ListContentPadding)
	[
		SNew(STextBlock)
		.Text(Item.Name)
		.TextStyle(&Theme.TextFont)
	];
}

void SFlareSkirmishSetupMenu::OnCompanyComboLineSelectionChanged(FFlareCompanyDescription Item, ESelectInfo::Type SelectInfo)
{

}

FText SFlareSkirmishSetupMenu::OnGetCurrentCompanyComboLine() const
{
	const FFlareCompanyDescription Item = CompanySelector->GetSelectedItem();
	return Item.Name;
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
	UFlareSkirmishManager* Skirmish = MenuManager->GetGame()->GetSkirmishManager();

	// Override defaults with current color settings
	FFlareCompanyDescription& PlayerCompanyData = Skirmish->GetData().PlayerCompanyData;
	const FFlareCompanyDescription* CurrentCompanyData = MenuManager->GetPC()->GetCompanyDescription();
	PlayerCompanyData.CustomizationBasePaintColor = CurrentCompanyData->CustomizationBasePaintColor;
	PlayerCompanyData.CustomizationPaintColor = CurrentCompanyData->CustomizationPaintColor;
	PlayerCompanyData.CustomizationOverlayColor = CurrentCompanyData->CustomizationOverlayColor;
	PlayerCompanyData.CustomizationLightColor = CurrentCompanyData->CustomizationLightColor;
	PlayerCompanyData.CustomizationPatternIndex = CurrentCompanyData->CustomizationPatternIndex;

	// Set enemy name
	Skirmish->GetData().EnemyCompanyName = CompanySelector->GetSelectedItem().ShortName;

	// Create the game
	Skirmish->StartPlay();
}

void SFlareSkirmishSetupMenu::OnMainMenu()
{
	MenuManager->GetGame()->GetSkirmishManager()->EndSkirmish();

	MenuManager->OpenMenu(EFlareMenu::MENU_Main);
}


#undef LOCTEXT_NAMESPACE

