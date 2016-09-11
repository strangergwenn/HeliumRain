
#include "../../Flare.h"
#include "FlareOrbitalMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareGameTools.h"
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
		SNew(SVerticalBox)
		
		// Planetarium
		+ SVerticalBox::Slot()
		.Padding(Theme.ContentPadding)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SHorizontalBox)

			// Left column
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Top)
			[
				SNew(SVerticalBox)

				// Trade routes title
				+ SVerticalBox::Slot()
				.Padding(Theme.TitlePadding)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("WorldStatus", "World status"))
					.TextStyle(&Theme.SubTitleFont)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(this, &SFlareOrbitalMenu::GetDateText)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(SHorizontalBox)

					// Fast forward
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Left)
					.Padding(Theme.SmallContentPadding)
					[
						SNew(SFlareButton)
						.Width(3.5)
						.Text(FText::Format(LOCTEXT("FastForwardSingleFormat", "Skip day ({0})"),
							FText::FromString(AFlareMenuManager::GetKeyNameFromActionName("Simulate"))))
						.Icon(FFlareStyleSet::GetIcon("Load_Small"))
						.OnClicked(this, &SFlareOrbitalMenu::OnFastForwardClicked)
						.IsDisabled(this, &SFlareOrbitalMenu::IsFastForwardDisabled)
						.HelpText(LOCTEXT("FastForwardInfo", "Wait for one day - Travels, production, building will be accelerated"))
					]

					// Fast forward
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Left)
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

					// World economy
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Left)
					.Padding(Theme.SmallContentPadding)
					[
						SNew(SFlareButton)
						.Width(3.5)
						.Text(LOCTEXT("WorldEconomy", "World prices"))
						.HelpText(LOCTEXT("WorldEconomyInfo", "See global prices, usage and variation for all resources"))
						.Icon(FFlareStyleSet::GetIcon("Sector_Small"))
						.OnClicked(this, &SFlareOrbitalMenu::OnWorldEconomyClicked)
					]
				]
			
				// Nema
				+ SVerticalBox::Slot()
				[
					SAssignNew(NemaBox, SFlarePlanetaryBox)
				]
			]

			// Main column 
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SVerticalBox)
				
				// Top line
				+ SVerticalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SHorizontalBox)

					// Anka
					+ SHorizontalBox::Slot()
					[
						SAssignNew(AnkaBox, SFlarePlanetaryBox)
					]

					// Travels
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Top)
					.HAlign(HAlign_Right)
					.Padding(Theme.ContentPadding)
					[
						SNew(SVerticalBox)

						// Travel list title
						+ SVerticalBox::Slot()
						.Padding(Theme.TitlePadding)
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Travels", "Travels & Orders"))
							.TextStyle(&Theme.SubTitleFont)
						]

						// Travel list
						+ SVerticalBox::Slot()
						.Padding(Theme.TitlePadding)
						[
							SNew(SScrollBox)
							.Style(&Theme.ScrollBoxStyle)
							.ScrollBarStyle(&Theme.ScrollBarStyle)
							+ SScrollBox::Slot()
							[
								SNew(STextBlock)
								.TextStyle(&Theme.TextFont)
								.Text(this, &SFlareOrbitalMenu::GetTravelText)
								.WrapTextAt(0.8 * Theme.ContentWidth)
							]
						]
					]

					// Travels
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Top)
					.HAlign(HAlign_Right)
					.Padding(Theme.ContentPadding)
					[
						SNew(SVerticalBox)

						// Trade routes title
						+ SVerticalBox::Slot()
						.Padding(Theme.TitlePadding)
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Trade routes", "Trade routes"))
							.TextStyle(&Theme.SubTitleFont)
						]

						// New trade route button
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.SmallContentPadding)
						.HAlign(HAlign_Left)
						[
							SNew(SFlareButton)
							.Width(8)
							.Text(LOCTEXT("NewTradeRouteButton", "Add new trade route"))
							.HelpText(LOCTEXT("NewTradeRouteInfo", "Create a new trade route and edit it"))
							.Icon(FFlareStyleSet::GetIcon("New"))
							.OnClicked(this, &SFlareOrbitalMenu::OnNewTradeRouteClicked)
						]

						// Trade route list
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Left)
						[
							SNew(SScrollBox)
							.Style(&Theme.ScrollBoxStyle)
							.ScrollBarStyle(&Theme.ScrollBarStyle)
							+ SScrollBox::Slot()
							[
								SAssignNew(TradeRouteList, SVerticalBox)
							]
						]
					]
				]

				// Bottom line
				+ SVerticalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SHorizontalBox)

					// Asta
					+ SHorizontalBox::Slot()
					[
						SAssignNew(AstaBox, SFlarePlanetaryBox)
					]

					// Hela
					+ SHorizontalBox::Slot()
					[
						SAssignNew(HelaBox, SFlarePlanetaryBox)
					]

					// Adena
					+ SHorizontalBox::Slot()
					[
						SAssignNew(AdenaBox, SFlarePlanetaryBox)
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
	SetVisibility(EVisibility::Collapsed);
}

void SFlareOrbitalMenu::Enter()
{
	FLOG("SFlareOrbitalMenu::Enter");
	
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	StopFastForward();

	UpdateMap();

	UpdateTradeRouteList();

	Game->SaveGame(MenuManager->GetPC(), true);
}

void SFlareOrbitalMenu::Exit()
{
	FLOG("SFlareOrbitalMenu::Exit");
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);

	NemaBox->ClearChildren();
	AnkaBox->ClearChildren();
	AstaBox->ClearChildren();
	HelaBox->ClearChildren();
	AdenaBox->ClearChildren();

	TradeRouteList->ClearChildren();

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
	FLOG("Stop fast forward requested");
	FastForwardStopRequested = true;
}

void SFlareOrbitalMenu::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (IsEnabled() && MenuManager.IsValid())
	{
		// Check sector state changes
		for (int32 SectorIndex = 0; SectorIndex < MenuManager->GetPC()->GetCompany()->GetKnownSectors().Num(); SectorIndex++)
		{
			UFlareSimulatedSector* Sector = MenuManager->GetPC()->GetCompany()->GetKnownSectors()[SectorIndex];

			EFlareSectorBattleState::Type BattleState = Sector->GetSectorBattleState(MenuManager->GetPC()->GetCompany());
			if (LastSectorBattleState.Contains(Sector))
			{
				EFlareSectorBattleState::Type LastBattleState = LastSectorBattleState[Sector];

				// TODO more detail state and only for some transition
				if (LastBattleState == BattleState)
				{
					continue;
				}

				// Notify
				FFlareMenuParameterData Data;
				Data.Sector = Sector;
				MenuManager->GetPC()->Notify(LOCTEXT("BattleStateChange", "Battle update"),
					FText::Format(LOCTEXT("BattleStateChangeFormat", "The military status of {0} has changed !"), Sector->GetSectorName()),
					FName("battle-state-changed"),
					EFlareNotification::NT_Military,
					false,
					EFlareMenu::MENU_Sector,
					Data);

				LastSectorBattleState[Sector] = BattleState;
			}
			else
			{
				LastSectorBattleState.Add(Sector,BattleState);
			}
		}

		// Fast forward every FastForwardPeriod
		TimeSinceFastForward += InDeltaTime;
		if (FastForwardActive)
		{
			if (TimeSinceFastForward > FastForwardPeriod)
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
	UpdateMapForBody(NemaBox, &Game->GetSectorCatalog()->OrbitalBodies[0]);
	UpdateMapForBody(AnkaBox, &Game->GetSectorCatalog()->OrbitalBodies[1]);
	UpdateMapForBody(AstaBox, &Game->GetSectorCatalog()->OrbitalBodies[2]);
	UpdateMapForBody(HelaBox, &Game->GetSectorCatalog()->OrbitalBodies[3]);
	UpdateMapForBody(AdenaBox, &Game->GetSectorCatalog()->OrbitalBodies[4]);
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
	Map->SetRadius(Body->CelestialBodyRadiusOnMap);
	Map->ClearChildren();

	// Add the name
	Map->AddSlot()
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
		[
			SNew(SFlareSectorButton)
			.Sector(Sector)
			.PlayerCompany(MenuManager->GetPC()->GetCompany())
			.OnClicked(this, &SFlareOrbitalMenu::OnOpenSector, IndexPtr)
		];
	}
}
void SFlareOrbitalMenu::UpdateTradeRouteList()
{
	UFlareCompany* Company = MenuManager->GetPC()->GetCompany();
	if (Company)
	{
		TradeRouteList->ClearChildren();
		const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
		TArray<UFlareTradeRoute*>& TradeRoutes = Company->GetCompanyTradeRoutes();

		for (int RouteIndex = 0; RouteIndex < TradeRoutes.Num(); RouteIndex++)
		{
			UFlareTradeRoute* TradeRoute = TradeRoutes[RouteIndex];

			// Add line
			TradeRouteList->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.Padding(Theme.SmallContentPadding)
			[
				SNew(SHorizontalBox)

				// Inspect
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SFlareButton)
					.Width(8)
					.Text(TradeRoute->GetTradeRouteName())
					.HelpText(FText(LOCTEXT("InspectHelp", "Edit this trade route")))
					.OnClicked(this, &SFlareOrbitalMenu::OnInspectTradeRouteClicked, TradeRoute)
				]

				// Remove
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SFlareButton)
					.Transparent(true)
					.Text(FText())
					.HelpText(LOCTEXT("RemoveTradeRouteHelp", "Remove this trade route"))
					.Icon(FFlareStyleSet::GetIcon("Stop"))
					.OnClicked(this, &SFlareOrbitalMenu::OnDeleteTradeRoute, TradeRoute)
					.Width(1)
				]
			];
		}
	}
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

			EFlareSectorBattleState::Type BattleState = Sector->GetSectorBattleState(MenuManager->GetPC()->GetCompany());
			if (BattleState == EFlareSectorBattleState::Battle || BattleState == EFlareSectorBattleState::BattleNoRetreat)
			{
				BattleInProgress = true;
			}
			else if (BattleState == EFlareSectorBattleState::BattleLost)
			{
				BattleLostWithRetreat = true;
			}
			else if (BattleState == EFlareSectorBattleState::BattleLostNoRetreat)
			{
				BattleLostWithoutRetreat = true;
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

		for (int32 SectorIndex = 0; SectorIndex < MenuManager->GetPC()->GetCompany()->GetKnownSectors().Num(); SectorIndex++)
		{
			UFlareSimulatedSector* Sector = MenuManager->GetPC()->GetCompany()->GetKnownSectors()[SectorIndex];

			EFlareSectorBattleState::Type BattleState = Sector->GetSectorBattleState(MenuManager->GetPC()->GetCompany());
			if(BattleState == EFlareSectorBattleState::Battle || BattleState == EFlareSectorBattleState::BattleNoRetreat)
			{
				return true;
			}
		}
		
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
			return FText::Format(LOCTEXT("DateCreditsInfoFormat", "{0} - {1} credits"), DateText, FText::AsNumber(UFlareGameTools::DisplayMoney(Credits)));
		}
	}

	return FText();
}

inline static bool EventDurationComparator (const FFlareIncomingEvent& ip1, const FFlareIncomingEvent& ip2)
 {
	 return (ip1.RemainingDuration < ip2.RemainingDuration);
 }

FText SFlareOrbitalMenu::GetTravelText() const
{
	if (IsEnabled())
	{
		UFlareWorld* GameWorld = MenuManager->GetGame()->GetGameWorld();
		if (GameWorld)
		{
			TArray<FFlareIncomingEvent> IncomingEvents;
			
			// List travels
			for (int32 TravelIndex = 0; TravelIndex < GameWorld->GetTravels().Num(); TravelIndex++)
			{
				UFlareTravel* Travel = GameWorld->GetTravels()[TravelIndex];
				if (Travel->GetFleet()->GetFleetCompany() == MenuManager->GetPC()->GetCompany())
				{
					int64 RemainingDuration = Travel->GetRemainingTravelDuration();
					FText TravelText;

					// Trade route version
					if (Travel->GetFleet()->GetCurrentTradeRoute())
					{
						TravelText = FText::Format(LOCTEXT("TravelTextTradeRouteFormat", "\u2022 {0} ({1})\n    {2}"),
							Travel->GetFleet()->GetFleetName(),
							Travel->GetFleet()->GetCurrentTradeRoute()->GetTradeRouteName(),
							Travel->GetFleet()->GetStatusInfo());
					}
					else
					{
						TravelText = FText::Format(LOCTEXT("TravelTextManualFormat", "\u2022 {0}\n    {1}"),
							Travel->GetFleet()->GetFleetName(),
							Travel->GetFleet()->GetStatusInfo());
					}

					// Add data
					FFlareIncomingEvent TravelEvent;
					TravelEvent.Text = TravelText;
					TravelEvent.RemainingDuration = RemainingDuration;
					IncomingEvents.Add(TravelEvent);
				}
			}

			// List ship productions
			TArray<UFlareCompany*> Companies = GameWorld->GetCompanies();
			for (int32 CompanyIndex = 0; CompanyIndex < Companies.Num(); CompanyIndex++)
			{
				TArray<UFlareSimulatedSpacecraft*>& CompanyStations = Companies[CompanyIndex]->GetCompanyStations();
				for (int32 StationIndex = 0; StationIndex < CompanyStations.Num(); StationIndex++)
				{
					for (int32 FactoryIndex = 0; FactoryIndex < CompanyStations[StationIndex]->GetFactories().Num(); FactoryIndex++)
					{
						// Get shipyard if existing
						UFlareFactory* TargetFactory = CompanyStations[StationIndex]->GetFactories()[FactoryIndex];
						FName CompanyIdentifier = MenuManager->GetPC()->GetCompany()->GetIdentifier();
						if (!TargetFactory->IsShipyard())
						{
							continue;
						}
						
						// Queue ship order
						if (TargetFactory->GetOrderShipCompany() == CompanyIdentifier)
						{
							FFlareSpacecraftDescription* OrderDesc = MenuManager->GetGame()->GetSpacecraftCatalog()->Get(TargetFactory->GetOrderShipClass());
							int64 ProductionTime = TargetFactory->GetRemainingProductionDuration() + OrderDesc->CycleCost.ProductionTime;

							FText ProductionText = FText::Format(LOCTEXT("ShipWaitingProdTextFormat", "\u2022 {0} ordered ({1} left)"),
								OrderDesc->Name,
								FText::FromString(*UFlareGameTools::FormatDate(ProductionTime, 2))); // FString needed here

							FFlareIncomingEvent ProductionEvent;
							ProductionEvent.Text = ProductionText;
							ProductionEvent.RemainingDuration = ProductionTime;
							IncomingEvents.Add(ProductionEvent);
						}

						// Ship being built
						else if (TargetFactory->GetTargetShipCompany() == CompanyIdentifier)
						{
							int64 ProductionTime = TargetFactory->GetRemainingProductionDuration();

							FText ProductionText = FText::Format(LOCTEXT("ShipProductionTextFormat", "\u2022 {0} being built ({1} left)"),
								MenuManager->GetGame()->GetSpacecraftCatalog()->Get(TargetFactory->GetTargetShipClass())->Name,
								FText::FromString(*UFlareGameTools::FormatDate(ProductionTime, 2))); // FString needed here

							FFlareIncomingEvent ProductionEvent;
							ProductionEvent.Text = ProductionText;
							ProductionEvent.RemainingDuration = ProductionTime;
							IncomingEvents.Add(ProductionEvent);
						}
					}
				}
			}

			// Generate list
			FString Result;
			IncomingEvents.Sort(&EventDurationComparator);
			for (FFlareIncomingEvent& Event : IncomingEvents)
			{
				Result += Event.Text.ToString() + "\n";
			}
			if (Result.Len() == 0)
			{
				Result = LOCTEXT("NoTravel", "No travel.").ToString();
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
	bool CanGoAhead = MenuManager->GetPC()->ConfirmFastForward(FSimpleDelegate::CreateSP(this, &SFlareOrbitalMenu::OnFastForwardConfirmed, false));
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
		bool CanGoAhead = MenuManager->GetPC()->ConfirmFastForward(FSimpleDelegate::CreateSP(this, &SFlareOrbitalMenu::OnFastForwardConfirmed, true));
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

void SFlareOrbitalMenu::OnNewTradeRouteClicked()
{
	UFlareTradeRoute* TradeRoute = MenuManager->GetPC()->GetCompany()->CreateTradeRoute(LOCTEXT("UntitledRoute", "Untitled Route"));
	check(TradeRoute);

	FFlareMenuParameterData Data;
	Data.Route = TradeRoute;
	MenuManager->OpenMenu(EFlareMenu::MENU_TradeRoute, Data);
}

void SFlareOrbitalMenu::OnInspectTradeRouteClicked(UFlareTradeRoute* TradeRoute)
{
	FFlareMenuParameterData Data;
	Data.Route = TradeRoute;
	MenuManager->OpenMenu(EFlareMenu::MENU_TradeRoute, Data);
}

void SFlareOrbitalMenu::OnDeleteTradeRoute(UFlareTradeRoute* TradeRoute)
{
	check(TradeRoute);
	TradeRoute->Dissolve();
	UpdateTradeRouteList();
}

void SFlareOrbitalMenu::OnWorldEconomyClicked()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_WorldEconomy);
}


#undef LOCTEXT_NAMESPACE

