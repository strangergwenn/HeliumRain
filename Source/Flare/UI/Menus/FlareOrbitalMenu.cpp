
#include "../../Flare.h"
#include "FlareOrbitalMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareGameTools.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../Components/FlareSectorButton.h"


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
		SNew(SBorder)
		.BorderImage(FFlareStyleSet::GetImage("OrbitBackground"))
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
					.Text(LOCTEXT("Leaderboard", "Competition"))
					.HelpText(LOCTEXT("LeaderboardInfo", "Take a closer look at all the companies"))
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

			// Travels
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.ContentPadding)
			[
				SNew(SHorizontalBox)

				// list of current travels
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				[
					SAssignNew(TravelsBox, SVerticalBox)
				]

				// Fast forward
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				[
					SNew(SFlareButton)
					.Text(LOCTEXT("Fast forward", "Fast forward"))
					.HelpText(LOCTEXT("FastForwardInfo", "Fast forward to the next event (travel, construction...)"))
					.Icon(FFlareStyleSet::GetIcon("FastForward"))
					.OnClicked(this, &SFlareOrbitalMenu::OnFastForwardClicked)
				]
			]

			// Planetarium
			+ SVerticalBox::Slot()
			.Padding(Theme.ContentPadding)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				[
					SAssignNew(NemaBox, SFlarePlanetaryBox)
				]
				+ SHorizontalBox::Slot()
				[
					SAssignNew(AnkaBox, SFlarePlanetaryBox)
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					[
						SAssignNew(AstaBox, SFlarePlanetaryBox)
					]
					+ SVerticalBox::Slot()
					[
						SAssignNew(AriadneBox, SFlarePlanetaryBox)
					]
				]
			]
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
	MenuManager->UseDarkBackground();

	Game->DeactivateSector(MenuManager->GetPC());
	
	UpdateMap();
	UpdateTravels();
}

void SFlareOrbitalMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);

	NemaBox->ClearChildren();
	TravelsBox->ClearChildren();
}

void SFlareOrbitalMenu::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
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
	UpdateMapForBody(NemaBox, &Game->GetSectorCatalog()->OrbitalBodies[0]);
	UpdateMapForBody(AnkaBox, &Game->GetSectorCatalog()->OrbitalBodies[1]);
	UpdateMapForBody(AstaBox, &Game->GetSectorCatalog()->OrbitalBodies[2]);
	UpdateMapForBody(AriadneBox, &Game->GetSectorCatalog()->OrbitalBodies[3]);
}

void SFlareOrbitalMenu::UpdateMapForBody(TSharedPtr<SFlarePlanetaryBox> Map, const FFlareSectorCelestialBodyDescription* Body)
{
	// Setup the planetary map
	Map->SetPlanetImage(&Body->CelestialBodyPicture);
	Map->SetRadius(Body->CelestialBodyRadiusOnMap);
	Map->ClearChildren();

	// Add the name
	Map->AddSlot()
	[
		SNew(STextBlock)
		.TextStyle(&FFlareStyleSet::GetDefaultTheme().SubTitleFont)
		.Text(Body->CelestialBodyName)
	];

	// Add the sectors
	for (int32 SectorIndex = 0; SectorIndex < MenuManager->GetPC()->GetCompany()->GetKnownSectors().Num(); SectorIndex++)
	{
		TSharedPtr<int32> IndexPtr(new int32(SectorIndex));
		UFlareSimulatedSector* Sector = MenuManager->GetPC()->GetCompany()->GetKnownSectors()[SectorIndex];

		if (Sector->GetOrbitParameters()->CelestialBodyIdentifier == Body->CelestialBodyIdentifier)
		{
			Map->AddSlot()
			[
				SNew(SFlareSectorButton)
				.Sector(Sector)
				.PlayerCompany(MenuManager->GetPC()->GetCompany())
				.OnClicked(this, &SFlareOrbitalMenu::OnOpenSector, IndexPtr)
			];
		}
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
		if (Travel->GetFleet()->GetFleetCompany() == MenuManager->GetPC()->GetCompany())
		{
			FString TravelText = FString::Printf(TEXT("Travel to %s: %s remaining."), *Travel->GetDestinationSector()->GetSectorName(), *UFlareGameTools::FormatTime(Travel->GetRemainingTravelDuration(), 1));

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
}

void SFlareOrbitalMenu::OnFastForwardClicked()
{
	MenuManager->GetGame()->GetGameWorld()->FastForward();

	UpdateMap();
	UpdateTravels();
}

FVector2D SFlareOrbitalMenu::GetWidgetPosition(int32 Index) const
{
	return FVector2D(1920, 1080) / 2;
}

FVector2D SFlareOrbitalMenu::GetWidgetSize(int32 Index) const
{
	int WidgetSize = 200;
	FVector2D BaseSize(WidgetSize, WidgetSize);
	return BaseSize;
}

#undef LOCTEXT_NAMESPACE

