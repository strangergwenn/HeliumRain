
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
						.HelpText(LOCTEXT("FastForwardOneDayInfo", "Wait for one day - Travels, production, building will be accelerated"))
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
						.Text(LOCTEXT("WorldEconomy", "Economy"))
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

			// Central column 1
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
			
			// Central column 2
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SAssignNew(AstaBox, SFlarePlanetaryBox)
				]

				+ SVerticalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SAssignNew(AdenaBox, SFlarePlanetaryBox)
				]
			]
			
			// Right column 
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				.WidthOverride(Theme.ContentWidth / 2)
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
							SNew(SRichTextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(this, &SFlareOrbitalMenu::GetTravelText)
							.WrapTextAt(Theme.ContentWidth / 2)
							.DecoratorStyleSet(&FFlareStyleSet::Get())
						]
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
			return FText::Format(LOCTEXT("DateCreditsInfoFormat", "Company date : {0} - {1} credits"), DateText, FText::AsNumber(UFlareGameTools::DisplayMoney(Credits)));
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
			UFlareCompany* PlayerCompany = MenuManager->GetPC()->GetCompany();

			// List incoming threats first
			TMap<IncomingKey, IncomingValue> IncomingMap = GameWorld->GetIncomingPlayerEnemy();

			FText SingleShip = LOCTEXT("ShipSingle", "ship");
			FText MultipleShips = LOCTEXT("ShipPlural", "ships");
			
			auto GetShipsText = [&](int32 LightShipCount, int32 HeavyShipCount)
			{
				// Fighters
				FText LightShipText;
				if (LightShipCount > 0)
				{
					LightShipText = FText::Format(LOCTEXT("PlayerAttackedLightsFormat", "{0} light {1}"),
						FText::AsNumber(LightShipCount),
						(LightShipCount > 1) ? MultipleShips : SingleShip);
				}

				// Heavies
				FText HeavyShipText;
				if (HeavyShipCount > 0)
				{
					HeavyShipText = FText::Format(LOCTEXT("PlayerAttackedHeaviesFormat", "{0} heavy {1}"),
						FText::AsNumber(HeavyShipCount),
						(HeavyShipCount > 1) ? MultipleShips : SingleShip);

					if (LightShipCount > 0)
					{
						HeavyShipText = FText::FromString(", " + HeavyShipText.ToString());
					}
				}

				return FText::Format(LOCTEXT("PlayerAttackedSoonShipsFormat","{0}{1}"),
												LightShipText,
												HeavyShipText);
			};

			auto GetUnknownShipText = [&](int32 UnknownShipCount)
			{
				return FText::Format(LOCTEXT("PlayerAttackedUnknownFormat", "{0} {1}"),
				FText::AsNumber(UnknownShipCount),
				(UnknownShipCount > 1) ? MultipleShips : SingleShip);
			};

			for (auto Entry : IncomingMap)
			{
				UFlareCompany* Company = Entry.Key.Company;
				UFlareSimulatedSector* Sector = Entry.Key.DestinationSector;
				int32 EnemyValue = Entry.Value.CombatValue;
				int64 RemainingDuration = Entry.Key.RemainingDuration;


				if (RemainingDuration <=1 || PlayerCompany->IsTechnologyUnlocked("early-warning"))
				{

					FText TravelText;
					FText TravelPartText = FText::Format(LOCTEXT("ThreatTextTravelPart", "Traveling to {0} ({1} left)"),
							Sector->GetSectorName(),
							FText::FromString(*UFlareGameTools::FormatDate(RemainingDuration, 1)));



					if (PlayerCompany->IsTechnologyUnlocked("advanced-radar"))
					{

						TravelText = FText::Format(LOCTEXT("ThreatTextAdvancedFormat", "\u2022 <WarningText>{0} (Combat value of {1})</>"
																					   "\n    <WarningText>{3}</>"
																					   "\n    <WarningText>{2}</>"),
								Company->GetCompanyName(),
								EnemyValue,
								GetShipsText(Entry.Value.LightShipCount, Entry.Value.HeavyShipCount),
								TravelPartText);
					}
					else
					{
						TravelText = FText::Format(LOCTEXT("ThreatTextFormat", "\u2022 <WarningText>{0}</>"
																			   "\n    <WarningText>{2}</>"
																			   "\n    <WarningText>{1}</>"),

							Company->GetCompanyName(),
							GetUnknownShipText (Entry.Value.LightShipCount + Entry.Value.HeavyShipCount),
							TravelPartText);
					}

					// Add data
					FFlareIncomingEvent TravelEvent;
					TravelEvent.Text = TravelText;
					TravelEvent.RemainingDuration = RemainingDuration;
					IncomingEvents.Add(TravelEvent);

				}
			}

			// List travels
			for (UFlareTravel* Travel : GameWorld->GetTravels())
			{
				if (Travel->GetFleet()->GetFleetCompany() == PlayerCompany)
				{
					int64 RemainingDuration = Travel->GetRemainingTravelDuration();
					FText TravelText;

					// Trade route version
					if (Travel->GetFleet()->GetCurrentTradeRoute())
					{
						TravelText = FText::Format(LOCTEXT("TravelTextTradeRouteFormat", "\u2022 {0} ({1}{2})\n    {3}"),
							Travel->GetFleet()->GetFleetName(),
							Travel->GetFleet()->GetCurrentTradeRoute()->GetTradeRouteName(),
							(Travel->GetFleet()->GetCurrentTradeRoute()->IsPaused() ? LOCTEXT("FleetTradeRoutePausedFormat", " (paused)") : FText()),
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
							int64 ProductionTime = TargetFactory->GetRemainingProductionDuration() + TargetFactory->GetProductionTime(OrderDesc->CycleCost);

							FText ProductionText;
							if (TargetFactory->IsActive() && TargetFactory->IsNeedProduction() && !TargetFactory->HasCostReserved() && !TargetFactory->HasInputResources())
							{

								ProductionText = FText::Format(LOCTEXT("ShipNoResourcesProdTextFormat", "\u2022 {0} ordered (missing resources)"),
								OrderDesc->Name);
							}
							else
							{
								ProductionText = FText::Format(LOCTEXT("ShipWaitingProdTextFormat", "\u2022 {0} ordered ({1} left)"),
								OrderDesc->Name,
								FText::FromString(*UFlareGameTools::FormatDate(ProductionTime, 2))); // FString needed here
							}

							FFlareIncomingEvent ProductionEvent;
							ProductionEvent.Text = ProductionText;
							ProductionEvent.RemainingDuration = ProductionTime;
							IncomingEvents.Add(ProductionEvent);
						}

						// Ship being built
						else if (TargetFactory->GetTargetShipCompany() == CompanyIdentifier)
						{
							int64 ProductionTime = TargetFactory->GetRemainingProductionDuration();

							FText ProductionText;

							if (TargetFactory->IsActive() && TargetFactory->IsNeedProduction() && !TargetFactory->HasCostReserved() && !TargetFactory->HasInputResources())
							{

								ProductionText = FText::Format(LOCTEXT("ShipProductionNoResourcesProdTextFormat", "\u2022 {0}  being built (missing resources)"),
								MenuManager->GetGame()->GetSpacecraftCatalog()->Get(TargetFactory->GetTargetShipClass())->Name);
							}
							else
							{
								ProductionText = FText::Format(LOCTEXT("ShipProductionTextFormat", "\u2022 {0} being built ({1} left)"),
									MenuManager->GetGame()->GetSpacecraftCatalog()->Get(TargetFactory->GetTargetShipClass())->Name,
									FText::FromString(*UFlareGameTools::FormatDate(ProductionTime, 2))); // FString needed here
							}

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

void SFlareOrbitalMenu::OnWorldEconomyClicked()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_WorldEconomy);
}


#undef LOCTEXT_NAMESPACE

