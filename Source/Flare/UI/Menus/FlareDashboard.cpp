
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
		SNew(SHorizontalBox)

		// UI
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SScrollBox)

			+ SScrollBox::Slot()
			[
				SNew(SHorizontalBox)

				// Company
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				[
					SAssignNew(CompanyBox, SVerticalBox)
				]

				// Station
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				[
					SAssignNew(StationBox, SVerticalBox)
				]

				// Universe
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				[
					SAssignNew(UniverseBox, SVerticalBox)
				]

				// Settings
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				[
					SAssignNew(SettingsBox, SVerticalBox)
				]
			]
		]

		// Dashboard button
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.AutoWidth()
		[
			SNew(SFlareRoundButton)
			.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Exit))
			.OnClicked(this, &SFlareDashboard::OnExit)
		]
	];

	// Company box
	CompanyBox->AddSlot()
	.HAlign(HAlign_Center)
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("CompanyMenuTitle", "COMPANY"))
		.TextStyle(&Theme.SubTitleFont)
	];
	CompanyBox->AddSlot()
	.AutoHeight()
	[
		SNew(SFlareRoundButton)
		.Text(LOCTEXT("InspectCompany", "Company"))
		.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Company))
		.OnClicked(this, &SFlareDashboard::OnInspectCompany)
	];
	CompanyBox->AddSlot()
	.AutoHeight()
	[
		SNew(SFlareRoundButton)
		.Text(LOCTEXT("InspectShip", "Ship"))
		.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Ship))
		.OnClicked(this, &SFlareDashboard::OnInspectShip)
	];

	// Station box
	StationBox->AddSlot()
	.HAlign(HAlign_Center)
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("StationMenuTitle", "STATION"))
		.TextStyle(&Theme.SubTitleFont)
	];
	StationBox->AddSlot()
	.AutoHeight()
	[
		SNew(SFlareRoundButton)
		.Text(LOCTEXT("InspectStation", "Station"))
		.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Station))
		.OnClicked(this, &SFlareDashboard::OnInspectStation)
	];
	StationBox->AddSlot()
	.AutoHeight()
	[
		SNew(SFlareRoundButton)
		.Text(LOCTEXT("UpgradeShip", "Upgrade ship"))
		.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_ShipConfig))
		.OnClicked(this, &SFlareDashboard::OnConfigureShip)
	];
	StationBox->AddSlot()
	.AutoHeight()
	[
		SNew(SFlareRoundButton)
		.Text(LOCTEXT("Undock", "Undock"))
		.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Undock))
		.OnClicked(this, &SFlareDashboard::OnUndock)
	];

	// Universe box
	UniverseBox->AddSlot()
	.HAlign(HAlign_Center)
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("UniverseMenuTitle", "UNIVERSE"))
		.TextStyle(&Theme.SubTitleFont)
	];
	UniverseBox->AddSlot()
	.AutoHeight()
	[
		SNew(SFlareRoundButton)
		.Text(LOCTEXT("Sector", "Sector map"))
		.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Sector))
		.OnClicked(this, &SFlareDashboard::OnOpenSector)
	];

	// Settings box
	SettingsBox->AddSlot()
	.HAlign(HAlign_Center)
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("SettingsMenuTitle", "SETTINGS"))
		.TextStyle(&Theme.SubTitleFont)
	];
	SettingsBox->AddSlot()
	.AutoHeight()
	[
		SNew(SFlareRoundButton)
		.Text(LOCTEXT("Quit", "Quit game"))
		.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Quit))
		.OnClicked(this, &SFlareDashboard::OnQuitGame)
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

	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		AFlareSpacecraft* Ship = PC->GetShipPawn();
		if (Ship)
		{
			StationBox->SetVisibility(Ship->GetNavigationSystem()->IsDocked() ? EVisibility::Visible : EVisibility::Collapsed);
		}
	}
}

void SFlareDashboard::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareDashboard::OnExit()
{
	MenuManager->CloseMenu();
}

void SFlareDashboard::OnInspectCompany()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Company);
}

void SFlareDashboard::OnInspectShip()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Ship);
}

void SFlareDashboard::OnInspectStation()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Station);
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
			Cast<AFlareMenuManager>(PC->GetHUD())->CloseMenu();
		}
	}
}

void SFlareDashboard::OnOpenSector()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Sector);
}

void SFlareDashboard::OnQuitGame()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Quit);
}

#undef LOCTEXT_NAMESPACE

