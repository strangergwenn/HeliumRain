
#include "../../Flare.h"
#include "FlareDashboard.h"
#include "../../Game/FlareGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlarePlayerController.h"
#include "../Components/FlareObjectiveInfo.h"


#define LOCTEXT_NAMESPACE "FlareDashboard"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareDashboard::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	AFlarePlayerController* PC = MenuManager->GetPC();

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
				SNew(SImage).Image(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Dashboard))
			]

			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TitleFont)
				.Text(LOCTEXT("Dashboard", "DASHBOARD"))
			]

			// Ship
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("InspectShip", "Ship"))
				.HelpText(LOCTEXT("InspectShipInfo", "Inspect your current ship"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Ship, true))
				.OnClicked(this, &SFlareDashboard::OnInspectShip)
			]

			// Ship upgrade
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("UpgradeShip", "Upgrade ship"))
				.HelpText(LOCTEXT("UpgradeShipInfo", "Upgrade your current ship"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_ShipConfig, true))
				.OnClicked(this, &SFlareDashboard::OnConfigureShip)
				.Visibility(this, &SFlareDashboard::GetDockedVisibility)
			]

			// Orbit
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("GoOrbit", "Orbital map"))
				.HelpText(LOCTEXT("GoOrbitInfo", "Exit the navigation mode and go back to the orbital map"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Orbit, true))
				.OnClicked(this, &SFlareDashboard::OnOrbit)
			]

			// Close
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("Close", "Close"))
				.HelpText(LOCTEXT("CloseInfo", "Close the menu and go back to flying the ship"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Exit, true))
				.OnClicked(this, &SFlareDashboard::OnExit)
			]
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(200, 20))
		[
			SNew(SImage).Image(&Theme.SeparatorBrush)
		]

		// Action list
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.Padding(Theme.ContentPadding)
		[
			SNew(SHorizontalBox)

			// Objective
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.ContentPadding)
			[
				SNew(SFlareObjectiveInfo).PC(PC)
			]
		]

		// Content block
		+ SVerticalBox::Slot()
		[
			SNew(SScrollBox)
			.Style(&Theme.ScrollBoxStyle)
			.ScrollBarStyle(&Theme.ScrollBarStyle)

			+ SScrollBox::Slot()
			[
				SNew(SHorizontalBox)

				// Owned
				+ SHorizontalBox::Slot()
				.Padding(Theme.ContentPadding)
				[
					SAssignNew(OwnedShipList, SFlareShipList)
					.MenuManager(MenuManager)
					.Title(LOCTEXT("DashboardMyListTitle", "OWNED SPACECRAFTS IN SECTOR"))
				]

				// Others
				+ SHorizontalBox::Slot()
				.Padding(Theme.ContentPadding)
				[
					SAssignNew(OtherShipList, SFlareShipList)
					.MenuManager(MenuManager)
					.Title(LOCTEXT("DashboardOtherListTitle", "OTHER SPACECRAFTS IN SECTOR"))
				]
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareDashboard::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}

void SFlareDashboard::Enter()
{
	FLOG("SFlareDashboard::Enter");
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	// Find all ships here
	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{

		for (int32 SpacecraftIndex = 0; SpacecraftIndex < PC->GetGame()->GetActiveSector()->GetSpacecrafts().Num(); SpacecraftIndex++)
		{
			AFlareSpacecraft* ShipCandidate = PC->GetGame()->GetActiveSector()->GetSpacecrafts()[SpacecraftIndex];

			if (ShipCandidate->GetDamageSystem()->IsAlive())
			{
				// Owned ship
				if (ShipCandidate->GetCompany() == PC->GetCompany())
				{
					OwnedShipList->AddShip(ShipCandidate);
				}

				// other
				else
				{
					OtherShipList->AddShip(ShipCandidate);
				}
			}
		}
	}

	OwnedShipList->RefreshList();
	OtherShipList->RefreshList();
}

void SFlareDashboard::Exit()
{
	OwnedShipList->Reset();
	OtherShipList->Reset();
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

EVisibility SFlareDashboard::GetDockedVisibility() const
{
	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		AFlareSpacecraft* Ship = PC->GetShipPawn();
		if (Ship)
		{
			return (Ship->GetNavigationSystem()->IsDocked() ? EVisibility::Visible : EVisibility::Collapsed);
		}
	}

	return EVisibility::Collapsed;
}

void SFlareDashboard::OnInspectShip()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Ship);
}

void SFlareDashboard::OnConfigureShip()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_ShipConfig);
}

void SFlareDashboard::OnOrbit()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Orbit);
}

void SFlareDashboard::OnUndock()
{
	// Ask to undock, and close the menu
	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		AFlareSpacecraft* Ship = PC->GetShipPawn();
		if (Ship)
		{
			Ship->GetNavigationSystem()->Undock();
			MenuManager->CloseMenu();
		}
	}
}

void SFlareDashboard::OnExit()
{
	MenuManager->CloseMenu();
}


#undef LOCTEXT_NAMESPACE

