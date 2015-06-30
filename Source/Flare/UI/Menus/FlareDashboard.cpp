
#include "../../Flare.h"
#include "FlareDashboard.h"
#include "../../Game/FlareGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlarePlayerController.h"


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
				SNew(SImage)
				.Image(FFlareStyleSet::GetIcon("Sector"))
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
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("InspectShip", "Ship"))
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
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_ShipConfig, true))
				.OnClicked(this, &SFlareDashboard::OnConfigureShip)
				.Visibility(this, &SFlareDashboard::GetDockedVisibility)
			]

			// Station
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("InspectStation", "Station"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Station, true))
				.OnClicked(this, &SFlareDashboard::OnInspectStation)
				.Visibility(this, &SFlareDashboard::GetDockedVisibility)
			]

			// Undock
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("Undock", "Undock"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Undock, true))
				.OnClicked(this, &SFlareDashboard::OnUndock)
				.Visibility(this, &SFlareDashboard::GetDockedVisibility)
			]

			// Company
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("InspectCompany", "Company"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Company, true))
				.OnClicked(this, &SFlareDashboard::OnInspectCompany)
			]

			// Quit
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("SaveQuit", "Save and quit"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Main, true))
				.OnClicked(this, &SFlareDashboard::OnMainMenu)
			]
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(200, 40))
		[
			SNew(SImage).Image(&Theme.SeparatorBrush)
		]

		// Object list
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SScrollBox)
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
		AFlareSpacecraft* PlayerShip = PC->GetShipPawn();
		for (TActorIterator<AActor> ActorItr(PC->GetWorld()); ActorItr; ++ActorItr)
		{
			AFlareSpacecraft* ShipCandidate = Cast<AFlareSpacecraft>(*ActorItr);
			if (ShipCandidate && ShipCandidate->GetDamageSystem()->IsAlive())
			{
				// Owned ship
				if (PlayerShip && ShipCandidate->GetCompany() == PlayerShip->GetCompany())
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

void SFlareDashboard::OnExit()
{
	MenuManager->CloseMenu();
}

void SFlareDashboard::OnInspectShip()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Ship);
}

void SFlareDashboard::OnInspectCompany()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Company);
}

void SFlareDashboard::OnInspectStation()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Station);
}

void SFlareDashboard::OnMainMenu()
{
	AFlarePlayerController* PC = MenuManager->GetPC();

	PC->GetGame()->SaveWorld(PC);
	PC->GetGame()->DeleteWorld();

	MenuManager->FlushNotifications();
	MenuManager->OpenMenu(EFlareMenu::MENU_Main);
}

void SFlareDashboard::OnConfigureShip()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_ShipConfig);
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


#undef LOCTEXT_NAMESPACE

