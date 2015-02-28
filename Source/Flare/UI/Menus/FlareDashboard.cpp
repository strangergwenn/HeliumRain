
#include "../../Flare.h"
#include "FlareDashboard.h"
#include "../../Game/FlareGame.h"
#include "../../Player/FlareHUD.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareDashboard"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareDashboard::Construct(const FArguments& InArgs)
{
	// Data
	OwnerHUD = InArgs._OwnerHUD;
	TSharedPtr<SFlareButton> BackButton;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());

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
			.Padding(FMargin(10))
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
			SAssignNew(BackButton, SFlareButton)
			.ContainerStyle(FFlareStyleSet::Get(), "/Style/InvisibleContainerStyle")
			.ButtonStyle(FFlareStyleSet::Get(), "/Style/BackToDashboardButton")
			.OnClicked(this, &SFlareDashboard::OnExit)
		]
	];

	// Company box
	CompanyBox->AddSlot()
	.Padding(FMargin(10))
	.HAlign(HAlign_Center)
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("CompanyMenuTitle", "COMPANY"))
		.TextStyle(FFlareStyleSet::Get(), "Flare.Title2")
	];
	CompanyBox->AddSlot()
	.AutoHeight()
	[
		SNew(SFlareDashboardButton)
		.Text(LOCTEXT("InspectCompany", "Company"))
		.Icon(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Company))
		.OnClicked(this, &SFlareDashboard::OnInspectCompany)
	];
	CompanyBox->AddSlot()
	.AutoHeight()
	[
		SNew(SFlareDashboardButton)
		.Text(LOCTEXT("InspectShip", "Ship"))
		.Icon(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Ship))
		.OnClicked(this, &SFlareDashboard::OnInspectShip)
	];

	// Station box
	StationBox->AddSlot()
	.Padding(FMargin(10))
	.HAlign(HAlign_Center)
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("StationMenuTitle", "STATION"))
		.TextStyle(FFlareStyleSet::Get(), "Flare.Title2")
	];
	StationBox->AddSlot()
	.AutoHeight()
	[
		SNew(SFlareDashboardButton)
		.Text(LOCTEXT("InspectStation", "Station"))
		.Icon(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Station))
		.OnClicked(this, &SFlareDashboard::OnInspectStation)
	];
	StationBox->AddSlot()
	.AutoHeight()
	[
		SNew(SFlareDashboardButton)
		.Text(LOCTEXT("UpgradeShip", "Upgrade ship"))
		.Icon(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_ShipConfig))
		.OnClicked(this, &SFlareDashboard::OnConfigureShip)
	];
	StationBox->AddSlot()
	.AutoHeight()
	[
		SNew(SFlareDashboardButton)
		.Text(LOCTEXT("Undock", "Undock"))
		.Icon(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Undock))
		.OnClicked(this, &SFlareDashboard::OnUndock)
	];

	// Universe box
	UniverseBox->AddSlot()
	.Padding(FMargin(10))
	.HAlign(HAlign_Center)
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("UniverseMenuTitle", "UNIVERSE"))
		.TextStyle(FFlareStyleSet::Get(), "Flare.Title2")
	];
	UniverseBox->AddSlot()
	.AutoHeight()
	[
		SNew(SFlareDashboardButton)
		.Text(LOCTEXT("Sector", "Sector map"))
		.Icon(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Sector))
		.OnClicked(this, &SFlareDashboard::OnOpenSector)
	];
	UniverseBox->AddSlot()
	.AutoHeight()
	[
		SNew(SFlareDashboardButton)
		.Text(LOCTEXT("Orbit", "Orbit map"))
		.Icon(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Orbit))
		.OnClicked(this, &SFlareDashboard::OnOpenOrbit)
	];
	UniverseBox->AddSlot()
	.AutoHeight()
	[
		SNew(SFlareDashboardButton)
		.Text(LOCTEXT("Encyclopedia", "Encyclopedia"))
		.Icon(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Encyclopedia))
		.OnClicked(this, &SFlareDashboard::OnOpenEncyclopedia)
	];

	// Settings box
	SettingsBox->AddSlot()
	.Padding(FMargin(10))
	.HAlign(HAlign_Center)
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("SettingsMenuTitle", "SETTINGS"))
		.TextStyle(FFlareStyleSet::Get(), "Flare.Title2")
	];
	SettingsBox->AddSlot()
	.AutoHeight()
	[
		SNew(SFlareDashboardButton)
		.Text(LOCTEXT("Help", "Help"))
		.Icon(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Help))
		.OnClicked(this, &SFlareDashboard::OnOpenHelp)
	];
	SettingsBox->AddSlot()
	.AutoHeight()
	[
		SNew(SFlareDashboardButton)
		.Text(LOCTEXT("Settings", "Settings"))
		.Icon(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Settings))
		.OnClicked(this, &SFlareDashboard::OnOpenSettings)
	];
	SettingsBox->AddSlot()
	.AutoHeight()
	[
		SNew(SFlareDashboardButton)
		.Text(LOCTEXT("Quit", "Quit game"))
		.Icon(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Quit))
		.OnClicked(this, &SFlareDashboard::OnQuitGame)
	];

	// Dashboard close button
	BackButton->GetContainer()->SetContent(SNew(SImage).Image(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Exit)));
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareDashboard::Setup()
{
	SetVisibility(EVisibility::Hidden);
}

void SFlareDashboard::Enter()
{
	FLOG("SFlareDashboard::Enter");
	SetVisibility(EVisibility::Visible);

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC)
	{
		AFlareShip* Ship = PC->GetShipPawn();
		StationBox->SetVisibility(Ship->IsDocked() ? EVisibility::Visible : EVisibility::Collapsed);
		PC->GetMenuPawn()->UpdateBackgroundColor(0.15, 0.15);
	}
}

void SFlareDashboard::Exit()
{
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareDashboard::OnExit()
{
	OwnerHUD->CloseMenu();
}

void SFlareDashboard::OnInspectCompany()
{
	OwnerHUD->OpenMenu(EFlareMenu::MENU_Company);
}

void SFlareDashboard::OnInspectShip()
{
	OwnerHUD->OpenMenu(EFlareMenu::MENU_Ship);
}

void SFlareDashboard::OnInspectStation()
{
	OwnerHUD->OpenMenu(EFlareMenu::MENU_Station);
}

void SFlareDashboard::OnConfigureShip()
{
	OwnerHUD->OpenMenu(EFlareMenu::MENU_ShipConfig);
}

void SFlareDashboard::OnUndock()
{
	// Ask to undock, and close the menu
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC)
	{
		AFlareShip* Ship = PC->GetShipPawn();
		if (Ship)
		{
			Ship->Undock();
			Cast<AFlareHUD>(PC->GetHUD())->CloseMenu();
		}
	}
}

void SFlareDashboard::OnOpenSector()
{
	OwnerHUD->OpenMenu(EFlareMenu::MENU_Sector);
}

void SFlareDashboard::OnOpenOrbit()
{
	OwnerHUD->OpenMenu(EFlareMenu::MENU_Orbit);
}

void SFlareDashboard::OnOpenEncyclopedia()
{
	OwnerHUD->OpenMenu(EFlareMenu::MENU_Encyclopedia);
}

void SFlareDashboard::OnOpenHelp()
{
	OwnerHUD->OpenMenu(EFlareMenu::MENU_Help);
}

void SFlareDashboard::OnOpenSettings()
{
	OwnerHUD->OpenMenu(EFlareMenu::MENU_Settings);
}

void SFlareDashboard::OnQuitGame()
{
	OwnerHUD->OpenMenu(EFlareMenu::MENU_Quit);
}

#undef LOCTEXT_NAMESPACE

