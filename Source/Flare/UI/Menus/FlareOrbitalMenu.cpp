
#include "../../Flare.h"
#include "FlareOrbitalMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareOrbitalMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareOrbitalMenu::Construct(const FArguments& InArgs)
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
				SNew(SImage).Image(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Orbit))
			]

			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TitleFont)
				.Text(LOCTEXT("Orbital", "ORBITAL MAP"))
			]

			// Company
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("InspectCompany", "Company"))
				.HelpText(LOCTEXT("InspectCompanyInfo", "Inspect your company"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Company, true))
				.OnClicked(this, &SFlareOrbitalMenu::OnInspectCompany)
			]

			// Leaderboard
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("Leaderboard", "Leaderboard"))
				.HelpText(LOCTEXT("LeaderboardInfo", "Take a look at the companies"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Leaderboard, true))
				.OnClicked(this, &SFlareOrbitalMenu::OnOpenLeaderboard)
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
				.HelpText(LOCTEXT("SaveQuitInfo", "Save the game and go back to the main menu"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Main, true))
				.OnClicked(this, &SFlareOrbitalMenu::OnMainMenu)
			]
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(200, 20))
		[
			SNew(SImage).Image(&Theme.SeparatorBrush)
		]

		// Content
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.ContentPadding)
		[
			SAssignNew(SectorsBox, SHorizontalBox)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.ContentPadding)
		[
			SAssignNew(TravelsBox, SVerticalBox)
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareOrbitalMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}

void SFlareOrbitalMenu::Enter()
{
	FLOG("SFlareOrbitalMenu::Enter");
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	Game->DeactivateSector(MenuManager->GetPC());

	UpdateMap();
}

void SFlareOrbitalMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);

	SectorsBox->ClearChildren();
	TravelsBox->ClearChildren();
}

void SFlareOrbitalMenu::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	if (IsEnabled() && MenuManager.IsValid())
	{
		UFlareWorld* GameWorld = MenuManager->GetGame()->GetGameWorld();

		if (GameWorld && LastUpdateTime != GameWorld->GetTime())
		{
			UpdateTravels();
			LastUpdateTime = GameWorld->GetTime();
		}
	}
}

void SFlareOrbitalMenu::UpdateMap()
{
	SectorsBox->ClearChildren();

	// Add sectors slots
	for (int32 SectorIndex = 0; SectorIndex < MenuManager->GetPC()->GetCompany()->GetKnownSectors().Num(); SectorIndex++)
	{
		TSharedPtr<int32> IndexPtr(new int32(SectorIndex));

		UFlareSimulatedSector* Sector = MenuManager->GetPC()->GetCompany()->GetKnownSectors()[SectorIndex];

		FString	SectorTitle = Sector->GetSectorName();

		if(Sector->GetSectorShips().Num() > 0)
		{
			SectorTitle += "\n" + FString::FromInt(Sector->GetSectorShips().Num()) + " ship" +(Sector->GetSectorShips().Num() > 1 ? "s": "");
		}

		if(Sector->GetSectorStations().Num() > 0)
		{
			SectorTitle += "\n" + FString::FromInt(Sector->GetSectorStations().Num()) + " station" +(Sector->GetSectorStations().Num() > 1 ? "s": "");
		}

		SectorsBox->AddSlot()
		[
			SNew(SFlareRoundButton)
			.Text(FText::FromString(SectorTitle))
			.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_None, true))
			.OnClicked(this, &SFlareOrbitalMenu::OnOpenSector, IndexPtr)
		];
	}
}

void SFlareOrbitalMenu::UpdateTravels()
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	TravelsBox->ClearChildren();

	// Add travels slots
	for (int32 TravelIndex = 0; TravelIndex < MenuManager->GetGame()->GetGameWorld()->GetTravels().Num(); TravelIndex++)
	{
		UFlareTravel* Travel = MenuManager->GetGame()->GetGameWorld()->GetTravels()[TravelIndex];
		if(Travel->GetFleet()->GetFleetCompany() == MenuManager->GetPC()->GetCompany())
		{
			FString TravelText = FString::Printf(TEXT("Travel to %s: %d seconds remaining."), *Travel->GetDestinationSector()->GetSectorName(), Travel->GetRemainingTravelDuration());

			TravelsBox->AddSlot()
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TextFont)
				.Text(FText::FromString(TravelText))
			];

		}
	}
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareOrbitalMenu::OnInspectCompany()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Company);
}

void SFlareOrbitalMenu::OnOpenLeaderboard()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Leaderboard);
}

void SFlareOrbitalMenu::OnMainMenu()
{
	AFlarePlayerController* PC = MenuManager->GetPC();

	MenuManager->FlushNotifications();
	MenuManager->OpenMenu(EFlareMenu::MENU_Main);

	PC->GetGame()->SaveGame(PC);
	PC->GetGame()->UnloadGame();
}

void SFlareOrbitalMenu::OnOpenSector(TSharedPtr<int32> Index)
{
	UFlareSimulatedSector* Sector = MenuManager->GetPC()->GetCompany()->GetKnownSectors()[*Index];
	MenuManager->OpenMenu(EFlareMenu::MENU_Sector, Sector);
	/*AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{

		//Game->ActivateSector(PC, Sector);
	}*/
}


#undef LOCTEXT_NAMESPACE

