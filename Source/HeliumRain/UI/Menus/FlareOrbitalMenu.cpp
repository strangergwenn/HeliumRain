
#include "HeliumRain/UI/Menus/FlareOrbitalMenu.h"
#include "../../Flare.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareGameTools.h"
#include "../../Economy/FlareFactory.h"
#include "../../Data/FlareOrbitalMap.h"
#include "../../Data/FlareSpacecraftCatalog.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Spacecrafts/FlareSpacecraft.h"
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

	// FF setup
	FastForwardPeriod = 0.5f;
	FastForwardStopRequested = false;

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SOverlay)

		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetImage("Sun"))
		]

		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
		
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.HAlign(HAlign_Left)
				.AutoWidth()
				[
					SNew(SVerticalBox)

					// World status
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Top)
					.Padding(Theme.ContentPadding)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareOrbitalMenu::GetDateText)
					]
				]

				// Display modes
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				.Padding(Theme.SmallContentPadding)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Width(3)
						.Text(LOCTEXT("DisplayModeFleets", "Fleets"))
						.HelpText(LOCTEXT("DisplayModeFleetsInfo", "Display fleets on the map"))
						.OnClicked(this, &SFlareOrbitalMenu::SetDisplayMode, EFlareOrbitalMode::Fleets)
						.IsDisabled(this, &SFlareOrbitalMenu::IsCurrentDisplayMode, EFlareOrbitalMode::Fleets)
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Width(3)
						.Text(LOCTEXT("DisplayModeStations", "Stations"))
						.HelpText(LOCTEXT("DisplayModeStationsInfo", "Display stations on the map"))
						.OnClicked(this, &SFlareOrbitalMenu::SetDisplayMode, EFlareOrbitalMode::Stations)
						.IsDisabled(this, &SFlareOrbitalMenu::IsCurrentDisplayMode, EFlareOrbitalMode::Stations)
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Width(3)
						.Text(LOCTEXT("DisplayModeShips", "Ships"))
						.HelpText(LOCTEXT("DisplayModeShipsInfo", "Display ships on the map"))
						.OnClicked(this, &SFlareOrbitalMenu::SetDisplayMode, EFlareOrbitalMode::Ships)
						.IsDisabled(this, &SFlareOrbitalMenu::IsCurrentDisplayMode, EFlareOrbitalMode::Ships)
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Width(3)
						.Text(LOCTEXT("DisplayModeBattles", "Battles"))
						.HelpText(LOCTEXT("DisplayModeBattlesInfo", "Display battles on the map"))
						.OnClicked(this, &SFlareOrbitalMenu::SetDisplayMode, EFlareOrbitalMode::Battles)
						.IsDisabled(this, &SFlareOrbitalMenu::IsCurrentDisplayMode, EFlareOrbitalMode::Battles)
					]
				]

				// Skip day
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Top)
				.Padding(Theme.SmallContentPadding)
				[
					SNew(SFlareButton)
					.Width(3.5)
					.Text(FText::Format(LOCTEXT("FastForwardSingleFormat", "Skip day ({0})"),
						FText::FromString(AFlareMenuManager::GetKeyNameFromActionName("Simulate"))))
					.Icon(FFlareStyleSet::GetIcon("Load_Small"))
					.OnClicked(this, &SFlareOrbitalMenu::OnFastForwardClicked)
					.IsDisabled(this, &SFlareOrbitalMenu::IsFastForwardDisabled)
					.HelpText(LOCTEXT("FastForwardOneDayInfo", "Wait for one day - Travels, production, building will be accelerated"))
				]

				// Fast forward
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Top)
				.Padding(Theme.SmallContentPadding)
				[
					SAssignNew(FastForwardAuto, SFlareButton)
					.Width(4.5)
					.Toggle(true)
					.Text(this, &SFlareOrbitalMenu::GetFastForwardText)
					.Icon(this, &SFlareOrbitalMenu::GetFastForwardIcon)
					.OnClicked(this, &SFlareOrbitalMenu::OnFastForwardAutomaticClicked)
					.IsDisabled(this, &SFlareOrbitalMenu::IsFastForwardDisabled)
					.HelpText(LOCTEXT("FastForwardInfo", "Wait for the next event - Travels, production, building will be accelerated"))
				]
			]
		
			// Planetarium body
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SHorizontalBox)
			
				// Action column 
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SScrollBox)
					.Style(&Theme.ScrollBoxStyle)
					.ScrollBarStyle(&Theme.ScrollBarStyle)

					// Travel title
					+ SScrollBox::Slot()
					.Padding(Theme.ContentPadding)
					[
						UFlareUITypes::Header(LOCTEXT("TravelsTitle", "Events"))
					]

					// Travel list
					+ SScrollBox::Slot()
					.Padding(Theme.ContentPadding)
					[
						SNew(SRichTextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareOrbitalMenu::GetTravelText)
						.WrapTextAt(0.5 * Theme.ContentWidth)
						.DecoratorStyleSet(&FFlareStyleSet::Get())
					]

					// Trade route list
					+ SScrollBox::Slot()
					.HAlign(HAlign_Left)
					[
						SAssignNew(TradeRouteInfo, SFlareTradeRouteInfo)
						.MenuManager(MenuManager)
					]
				]

				// Moons 1
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SAssignNew(AnkaBox, SFlarePlanetaryBox)
					]

					+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SAssignNew(HelaBox, SFlarePlanetaryBox)
					]
				]

				// Nema
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Top)
				[
					SAssignNew(NemaBox, SFlarePlanetaryBox)
				]
			
				// Moons 2
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Top)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Top)
					.Padding(FMargin(0, 100, 0, 50))
					[
						SAssignNew(AstaBox, SFlarePlanetaryBox)
					]

					+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Top)
					[
						SAssignNew(AdenaBox, SFlarePlanetaryBox)
					]

					+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
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
	SetVisibility(EVisibility::Collapsed);
}

void SFlareOrbitalMenu::Enter()
{
	FLOG("SFlareOrbitalMenu::Enter");
	
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	DisplayMode = EFlareOrbitalMode::Fleets;

	// Update stuff
	StopFastForward();
	UpdateMap();
	TradeRouteInfo->Update();

	Game->SaveGame(MenuManager->GetPC(), true);
}

void SFlareOrbitalMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);

	NemaBox->ClearChildren();
	AnkaBox->ClearChildren();
	AstaBox->ClearChildren();
	HelaBox->ClearChildren();
	AdenaBox->ClearChildren();

	TradeRouteInfo->Clear();
	StopFastForward();
}

void SFlareOrbitalMenu::StopFastForward()
{
	TimeSinceFastForward = 0;
	FastForwardStopRequested = false;
	FastForwardAuto->SetActive(false);

	if (FastForwardActive)
	{
		FLOG("Stop fast forward");
		FastForwardActive = false;
		Game->SaveGame(MenuManager->GetPC(), true);
		Game->ActivateCurrentSector();
	}
}

void SFlareOrbitalMenu::RequestStopFastForward()
{
	FastForwardStopRequested = true;
}

void SFlareOrbitalMenu::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (IsEnabled() && MenuManager.IsValid())
	{
		for(UFlareSimulatedSector* Sector : MenuManager->GetPC()->GetCompany()->GetKnownSectors())
		{
			MenuManager->GetPC()->CheckSectorStateChanges(Sector);
		}

		// Fast forward every FastForwardPeriod
		TimeSinceFastForward += InDeltaTime;
		if (FastForwardActive)
		{
			if (!FastForwardStopRequested && (TimeSinceFastForward > FastForwardPeriod || UFlareGameTools::FastFastForward))
			{
				MenuManager->GetGame()->GetGameWorld()->FastForward();
				TimeSinceFastForward = 0;
			}

			// Stop request
			if (FastForwardStopRequested)
			{
				StopFastForward();
			}
		}
	}
}

void SFlareOrbitalMenu::UpdateMap()
{
	TArray<FFlareSectorCelestialBodyDescription>& OrbitalBodies = Game->GetOrbitalBodies()->OrbitalBodies;

	UpdateMapForBody(NemaBox,  &OrbitalBodies[0]);
	UpdateMapForBody(AnkaBox,  &OrbitalBodies[1]);
	UpdateMapForBody(AstaBox,  &OrbitalBodies[2]);
	UpdateMapForBody(HelaBox,  &OrbitalBodies[3]);
	UpdateMapForBody(AdenaBox, &OrbitalBodies[4]);
}

struct FSortByAltitudeAndPhase
{
	inline bool operator()(UFlareSimulatedSector& SectorA, UFlareSimulatedSector& SectorB) const
	{
		if (SectorA.GetOrbitParameters()->Altitude == SectorB.GetOrbitParameters()->Altitude)
		{
			// Compare phase
			if(SectorA.GetOrbitParameters()->Phase == SectorB.GetOrbitParameters()->Phase)
			{
				FLOGV("WARNING: %s and %s are at the same phase", *SectorA.GetSectorName().ToString(), *SectorB.GetSectorName().ToString())
			}

			return SectorA.GetOrbitParameters()->Phase < SectorB.GetOrbitParameters()->Phase;
		}
		else
		{
			return (SectorA.GetOrbitParameters()->Altitude < SectorB.GetOrbitParameters()->Altitude);
		}
	}
};

void SFlareOrbitalMenu::UpdateMapForBody(TSharedPtr<SFlarePlanetaryBox> Map, const FFlareSectorCelestialBodyDescription* Body)
{
	// Setup the planetary map
	Map->SetPlanetImage(&Body->CelestialBodyPicture);
	Map->SetRadius(Body->CelestialBodyRadiusOnMap, 110);
	Map->ClearChildren();

	// Find highest altitude
	int32 MaxAltitude = 0;
	for (UFlareSimulatedSector* Sector : MenuManager->GetGame()->GetGameWorld()->GetSectors())
	{
		if (Sector->GetOrbitParameters()->Altitude > MaxAltitude
		 && Sector->GetOrbitParameters()->CelestialBodyIdentifier == Body->CelestialBodyIdentifier)
		{
			MaxAltitude = Sector->GetOrbitParameters()->Altitude;
		}
	}

	// Add the name
	Map->AddSlot()
	.Altitude(MaxAltitude)
	.Phase(0)
	[
		SNew(STextBlock)
		.TextStyle(&FFlareStyleSet::GetDefaultTheme().SubTitleFont)
		.Text(Body->CelestialBodyName)
	];

	// Get sectors
	TArray<UFlareSimulatedSector*> KnownSectors = MenuManager->GetPC()->GetCompany()->GetKnownSectors();
	KnownSectors = KnownSectors.FilterByPredicate(
		[&](UFlareSimulatedSector* Sector)
		{
			return Sector->GetOrbitParameters()->CelestialBodyIdentifier == Body->CelestialBodyIdentifier;
		});
	KnownSectors.Sort(FSortByAltitudeAndPhase());

	// Add the sectors
	for (int32 SectorIndex = 0; SectorIndex < KnownSectors.Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = KnownSectors[SectorIndex];
		TSharedPtr<int32> IndexPtr(new int32(MenuManager->GetPC()->GetCompany()->GetKnownSectors().Find(Sector)));

		Map->AddSlot()
		.Altitude(Sector->GetOrbitParameters()->Altitude)
		.Phase(Sector->GetOrbitParameters()->Phase)
		[
			SNew(SFlareSectorButton)
			.Sector(Sector)
			.PlayerCompany(MenuManager->GetPC()->GetCompany())
			.OnClicked(this, &SFlareOrbitalMenu::OnOpenSector, IndexPtr)
		];
	}
}

EFlareOrbitalMode::Type SFlareOrbitalMenu::GetDisplayMode() const
{
	return DisplayMode;
}

bool SFlareOrbitalMenu::IsCurrentDisplayMode(EFlareOrbitalMode::Type Mode) const
{
	return (Mode == GetDisplayMode());
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FText SFlareOrbitalMenu::GetFastForwardText() const
{
	if (!IsEnabled())
	{
		return FText();
	}

	if (!FastForwardAuto->IsActive())
	{
		bool BattleInProgress = false;
		bool BattleLostWithRetreat = false;
		bool BattleLostWithoutRetreat = false;

		for (int32 SectorIndex = 0; SectorIndex < MenuManager->GetPC()->GetCompany()->GetKnownSectors().Num(); SectorIndex++)
		{
			UFlareSimulatedSector* Sector = MenuManager->GetPC()->GetCompany()->GetKnownSectors()[SectorIndex];

			FFlareSectorBattleState BattleState = Sector->GetSectorBattleState(MenuManager->GetPC()->GetCompany());
			if (BattleState.InBattle)
			{
				if(BattleState.InFight)
				{
					BattleInProgress = true;
				}
				else if (!BattleState.BattleWon)
				{
					if(BattleState.RetreatPossible)
					{
						BattleLostWithRetreat = true;
					}
					else
					{
						BattleLostWithoutRetreat = true;
					}
				}
			}
		}

		if (BattleInProgress)
		{
			return LOCTEXT("NoFastForwardBattleText", "Battle in progress");
		}
		else if (BattleLostWithRetreat)
		{
			return LOCTEXT("FastForwardBattleLostWithRetreatText", "Fast forward (!)");
		}
		else if (BattleLostWithoutRetreat)
		{
			return LOCTEXT("FastForwardBattleLostWithoutRetreatText", "Fast forward (!)");
		}
		else
		{
			return LOCTEXT("FastForwardText", "Fast forward");
		}
	}
	else
	{
		return LOCTEXT("FastForwardingText", "Fast forwarding...");
	}
}

const FSlateBrush* SFlareOrbitalMenu::GetFastForwardIcon() const
{
	if (FastForwardAuto->IsActive())
	{
		return FFlareStyleSet::GetIcon("Stop");
	}
	else
	{
		return FFlareStyleSet::GetIcon("FastForward");
	}
}

bool SFlareOrbitalMenu::IsFastForwardDisabled() const
{
	if (IsEnabled())
	{
		UFlareWorld* GameWorld = MenuManager->GetGame()->GetGameWorld();
		
		if (GameWorld && (GameWorld->GetTravels().Num() > 0 || true)) // Not true if there is pending todo event
		{
			return false;
		}
	}

	return true;
}

FText SFlareOrbitalMenu::GetDateText() const
{
	if (IsEnabled())
	{
		UFlareWorld* GameWorld = MenuManager->GetGame()->GetGameWorld();
		UFlareCompany* PlayerCompany = MenuManager->GetPC()->GetCompany();

		if (GameWorld && PlayerCompany)
		{
			int64 Credits = PlayerCompany->GetMoney();
			FText DateText = UFlareGameTools::GetDisplayDate(GameWorld->GetDate());
			return FText::Format(LOCTEXT("DateCreditsInfoFormat", "Date : {0} - {1} credits"), DateText, FText::AsNumber(UFlareGameTools::DisplayMoney(Credits)));
		}
	}

	return FText();
}

FText SFlareOrbitalMenu::GetTravelText() const
{
	if (IsEnabled())
	{
		UFlareWorld* GameWorld = MenuManager->GetGame()->GetGameWorld();
		if (GameWorld)
		{
			TArray<FFlareIncomingEvent> IncomingEvents = GameWorld->GetIncomingEvents();

			// Generate list
			FString Result;
			for (FFlareIncomingEvent& Event : IncomingEvents)
			{
				Result += Event.Text.ToString() + "\n";
			}
			if (Result.Len() == 0)
			{
				Result = LOCTEXT("NoTravel", "No event.").ToString();
			}

			return FText::FromString(Result);
		}
	}

	return FText();
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


/*----------------------------------------------------
	Action callbacks
----------------------------------------------------*/

void SFlareOrbitalMenu::SetDisplayMode(EFlareOrbitalMode::Type Mode)
{
	DisplayMode = Mode;
}

void SFlareOrbitalMenu::OnOpenSector(TSharedPtr<int32> Index)
{
	UFlareSimulatedSector* Sector = MenuManager->GetPC()->GetCompany()->GetKnownSectors()[*Index];
	FFlareMenuParameterData Data;
	Data.Sector = Sector;
	MenuManager->OpenMenu(EFlareMenu::MENU_Sector, Data);
}

void SFlareOrbitalMenu::OnFastForwardClicked()
{
	// Confirm and go on
	bool CanGoAhead = MenuManager->GetPC()->ConfirmFastForward(FSimpleDelegate::CreateSP(this, &SFlareOrbitalMenu::OnFastForwardConfirmed, false), FSimpleDelegate(), false);
	if (CanGoAhead)
	{
		OnFastForwardConfirmed(false);
	}
}

void SFlareOrbitalMenu::OnFastForwardAutomaticClicked()
{
	if (FastForwardAuto->IsActive())
	{
		// Avoid too fast double fast forward
		if (!FastForwardActive && TimeSinceFastForward < FastForwardPeriod)
		{
			FastForwardAuto->SetActive(false);
			return;
		}

		// Confirm and go on
		bool CanGoAhead = MenuManager->GetPC()->ConfirmFastForward(FSimpleDelegate::CreateSP(this, &SFlareOrbitalMenu::OnFastForwardConfirmed, true), FSimpleDelegate::CreateSP(this, &SFlareOrbitalMenu::OnFastForwardCanceled), true);
		if (CanGoAhead)
		{
			OnFastForwardConfirmed(true);
		}
	}
	else
	{
		StopFastForward();
	}
}

void SFlareOrbitalMenu::OnFastForwardConfirmed(bool Automatic)
{
	FLOGV("Start fast forward, automatic = %d", Automatic);

	if (Automatic)
	{
		// Mark FF
		FastForwardActive = true;
		FastForwardStopRequested = false;

		// Prepare for FF
		Game->SaveGame(MenuManager->GetPC(), true);
		Game->DeactivateSector();
	}
	else
	{
		MenuManager->GetPC()->SimulateConfirmed();
	}
}

void SFlareOrbitalMenu::OnFastForwardCanceled()
{
	FastForwardAuto->SetActive(false);
}


#undef LOCTEXT_NAMESPACE

