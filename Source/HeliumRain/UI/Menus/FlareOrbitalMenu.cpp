
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
					
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(SHorizontalBox)

					// Fast forward
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Left)
					.Padding(Theme.ContentPadding)
					[
						SAssignNew(FastForward, SFlareButton)
						.Width(4)
						.Toggle(true)
						.Text(this, &SFlareOrbitalMenu::GetFastForwardText)
						.Icon(this, &SFlareOrbitalMenu::GetFastForwardIcon)
						.OnClicked(this, &SFlareOrbitalMenu::OnFastForwardClicked)
						.IsDisabled(this, &SFlareOrbitalMenu::IsFastForwardDisabled)
						.HelpText(LOCTEXT("FastForwardInfo", "Wait and see - Travels, production, building will be accelerated."))
					]
					
					// Date
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareOrbitalMenu::GetDateText)
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
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareOrbitalMenu::GetTravelText)
						.WrapTextAt(0.8 * Theme.ContentWidth)
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

	StopFastForward();
}

void SFlareOrbitalMenu::StopFastForward()
{
	TimeSinceFastForward = 0;
	FastForwardStopRequested = false;
	FastForward->SetActive(false);

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

			if(FastForwardStopRequested)
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


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FText SFlareOrbitalMenu::GetFastForwardText() const
{
	if (!IsEnabled())
	{
		return FText();
	}

	if (!FastForward->IsActive())
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
	if (FastForward->IsActive())
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
			// TODO ALPHA : show the button during station/ship constructions as well
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
					FText TravelText = FText::Format(LOCTEXT("TravelTextFormat", "{0} : {1}"),
						Travel->GetFleet()->GetFleetName(),
						Travel->GetFleet()->GetStatusInfo());
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

							FText ProductionText = FText::Format(LOCTEXT("ShipWaitingProdTextFormat", "A {0} ordered to {1} ({2} left)"),
								OrderDesc->Name,
								Companies[CompanyIndex]->GetCompanyName(),
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

							FText ProductionText = FText::Format(LOCTEXT("ShipProductionTextFormat", "A {0} is being built by {1} ({2} left)"),
								MenuManager->GetGame()->GetSpacecraftCatalog()->Get(TargetFactory->GetTargetShipClass())->Name,
								Companies[CompanyIndex]->GetCompanyName(),
								FText::FromString(*UFlareGameTools::FormatDate(ProductionTime, 2))); // FString needed here

							FFlareIncomingEvent ProductionEvent;
							ProductionEvent.Text = ProductionText;
							ProductionEvent.RemainingDuration = ProductionTime;
							IncomingEvents.Add(ProductionEvent);
						}
					}
				}
			}
			// TODO merge with world event system

			IncomingEvents.Sort(&EventDurationComparator);

			FString Result = "\n";
			for(FFlareIncomingEvent& Event : IncomingEvents)
			{
				Result += Event.Text.ToString() + "\n";
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
	if (FastForward->IsActive())
	{
		if(!FastForwardActive && TimeSinceFastForward < FastForwardPeriod)
		{
			// Avoid too fast double fast forward
			FastForward->SetActive(false);
			return;
		}

		bool BattleInProgress = false;
		bool BattleLostWithRetreat = false;
		bool BattleLostWithoutRetreat = false;

		for (int32 SectorIndex = 0; SectorIndex < MenuManager->GetPC()->GetCompany()->GetKnownSectors().Num(); SectorIndex++)
		{
			UFlareSimulatedSector* Sector = MenuManager->GetPC()->GetCompany()->GetKnownSectors()[SectorIndex];

			EFlareSectorBattleState::Type BattleState = Sector->GetSectorBattleState(MenuManager->GetPC()->GetCompany());
			if(BattleState == EFlareSectorBattleState::Battle || BattleState == EFlareSectorBattleState::BattleNoRetreat)
			{
				BattleInProgress = true;
			}
			else if(BattleState == EFlareSectorBattleState::BattleLost)
			{
				BattleLostWithRetreat = true;
			}
			else if(BattleState == EFlareSectorBattleState::BattleLostNoRetreat)
			{
				BattleLostWithoutRetreat = true;
			}
		}

		if (BattleInProgress)
		{
			return;
		}
		else if (BattleLostWithoutRetreat)
		{
			MenuManager->Confirm(LOCTEXT("ConfirmBattleLostWithoutRetreatTitle", "SACRIFICE SHIPS ?"),
								 LOCTEXT("ConfirmBattleLostWithoutRetreatText", "Some of the ships engaged in a battle cannot retreat and will be lost."),
								 FSimpleDelegate::CreateSP(this, &SFlareOrbitalMenu::OnFastForwardConfirmed));

		}
		else if (BattleLostWithRetreat)
		{
			MenuManager->Confirm(LOCTEXT("ConfirmBattleLostWithRetreatTitle", "SACRIFICE SHIPS ?"),
								 LOCTEXT("ConfirmBattleLostWithRetreatText", "Some of the ships engaged in a battle can still retreat ! They will be lost."),
								 FSimpleDelegate::CreateSP(this, &SFlareOrbitalMenu::OnFastForwardConfirmed));
		}
		else
		{
			OnFastForwardConfirmed();
		}
	}
	else
	{
		StopFastForward();
	}
}


void SFlareOrbitalMenu::OnFastForwardConfirmed()
{
	FLOG("Start fast forward");
	FastForwardActive = true;
	FastForwardStopRequested = false;
	Game->SaveGame(MenuManager->GetPC(), true);
	Game->DeactivateSector();
}

#undef LOCTEXT_NAMESPACE

