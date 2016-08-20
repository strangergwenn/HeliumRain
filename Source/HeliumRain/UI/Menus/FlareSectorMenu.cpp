
#include "../../Flare.h"
#include "FlareSectorMenu.h"

#include "../../Game/FlareGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"

#define LOCTEXT_NAMESPACE "FlareSectorMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSectorMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	AFlarePlayerController* PC = MenuManager->GetPC();

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SVerticalBox)

		// UI container
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Center)
		[
			SNew(SVerticalBox)

			// Header
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				
				// Info
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SNew(SBox)
					.WidthOverride(Theme.ContentWidth)
					[
						SNew(SVerticalBox)

						// Sector name
						+ SVerticalBox::Slot()
						.Padding(Theme.TitlePadding)
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(this, &SFlareSectorMenu::GetSectorName)
							.TextStyle(&Theme.SubTitleFont)
						]

						// Sector description
						+ SVerticalBox::Slot()
						.Padding(Theme.ContentPadding)
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(this, &SFlareSectorMenu::GetSectorDescription)
							.TextStyle(&Theme.TextFont)
							.WrapTextAt(Theme.ContentWidth - Theme.ContentPadding.Left - Theme.ContentPadding.Right)
						]
				
						// Sector location
						+ SVerticalBox::Slot()
						.Padding(Theme.ContentPadding)
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(this, &SFlareSectorMenu::GetSectorLocation)
							.TextStyle(&Theme.TextFont)
							.WrapTextAt(Theme.ContentWidth)
						]

						// Travel here
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Fill)
						[
							SNew(SHorizontalBox)

							// Fleet list
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.SmallContentPadding)
							[
								SNew(SBox)
								.WidthOverride(8 * Theme.ButtonWidth)
								[
									SAssignNew(FleetSelector, SComboBox<UFlareFleet*>)
									.OptionsSource(&FleetList)
									.OnGenerateWidget(this, &SFlareSectorMenu::OnGenerateFleetComboLine)
									.OnSelectionChanged(this, &SFlareSectorMenu::OnFleetComboLineSelectionChanged)
									.ComboBoxStyle(&Theme.ComboBoxStyle)
									.ForegroundColor(FLinearColor::White)
									[
										SNew(STextBlock)
										.Text(this, &SFlareSectorMenu::OnGetCurrentFleetComboLine)
										.TextStyle(&Theme.TextFont)
									]
								]
							]

							// Button
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.SmallContentPadding)
							.HAlign(HAlign_Right)
							[
								SNew(SFlareButton)
								.Width(6)
								.Text(this, &SFlareSectorMenu::GetTravelText)
								.HelpText(LOCTEXT("TravelInfo", "Start travelling to this sector with the selected ship or fleet"))
								.Icon(FFlareStyleSet::GetIcon("Travel"))
								.OnClicked(this, &SFlareSectorMenu::OnTravelHereClicked)
								.IsDisabled(this, &SFlareSectorMenu::IsTravelDisabled)
							]
						]
					]
				]
				
				// Station construction
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SNew(SBox)
					.WidthOverride(10 * Theme.ButtonWidth + Theme.ContentPadding.Left + Theme.ContentPadding.Right)
					[
						SNew(SVerticalBox)

						// Helper title
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.TitlePadding)
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Top)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(LOCTEXT("Actions", "Sector tools"))
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Left)
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.Padding(Theme.SmallContentPadding)
							.AutoWidth()
							[
								// Show prices
								SNew(SFlareButton)
								.Width(4.9)
								.Text(LOCTEXT("ResourcePrices", "Local prices"))
								.HelpText(LOCTEXT("ResourcePricesInfo", "See the price and use of resources in this sector"))
								.Icon(FFlareStyleSet::GetIcon("Travel"))
								.OnClicked(this, &SFlareSectorMenu::OnResourcePrices)
								.IsDisabled(this, &SFlareSectorMenu::IsResourcePricesDisabled)
							]

							// Build station button
							+ SHorizontalBox::Slot()
							.Padding(Theme.SmallContentPadding)
							.AutoWidth()
							[
								SNew(SFlareButton)
								.Width(5)
								.Text(this, &SFlareSectorMenu::GetBuildStationText)
								.HelpText(LOCTEXT("BuildStationInfo", "Build a station in this sector"))
								.Icon(FFlareStyleSet::GetIcon("Travel"))
								.OnClicked(this, &SFlareSectorMenu::OnBuildStationClicked)
								.IsDisabled(this, &SFlareSectorMenu::IsBuildStationDisabled)
							]
						]

						// Refuel fleets
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Left)
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SFlareButton)
							.Width(10)
							.Text(this, &SFlareSectorMenu::GetRefuelText)
							.HelpText(LOCTEXT("RefillInfo", "Refill all fleets in this sector so that they have the necessary fuel, ammo and resources to fight."))
							.Icon(FFlareStyleSet::GetIcon("Tank"))
							.OnClicked(this, &SFlareSectorMenu::OnRefuelClicked)
							.IsDisabled(this, &SFlareSectorMenu::IsRefuelDisabled)
						]

						// Repair fleets
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Left)
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SFlareButton)
							.Width(10)
							.Text(this, &SFlareSectorMenu::GetRepairText)
							.HelpText(LOCTEXT("RepairInfo", "Repair all fleets in this sector."))
							.Icon(FFlareStyleSet::GetIcon("Repair"))
							.OnClicked(this, &SFlareSectorMenu::OnRepairClicked)
							.IsDisabled(this, &SFlareSectorMenu::IsRepairDisabled)
						]
					]
				]
			]
					
			// Content block
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Top)
			[
				SNew(SHorizontalBox)

				// Owned
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				[
					SNew(SScrollBox)
					.Style(&Theme.ScrollBoxStyle)
					.ScrollBarStyle(&Theme.ScrollBarStyle)

					+ SScrollBox::Slot()
					[
						SAssignNew(OwnedShipList, SFlareShipList)
						.MenuManager(MenuManager)
						.Title(LOCTEXT("MyListTitle", "Owned spacecrafts in sector"))
					]
				]

				// Others
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				[
					SNew(SScrollBox)
					.Style(&Theme.ScrollBoxStyle)
					.ScrollBarStyle(&Theme.ScrollBarStyle)

					+ SScrollBox::Slot()
					[
						SAssignNew(OtherShipList, SFlareShipList)
						.MenuManager(MenuManager)
						.Title(LOCTEXT("OtherListTitle", "Other spacecrafts in sector"))
					]
				]
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareSectorMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareSectorMenu::Enter(UFlareSimulatedSector* Sector)
{
	FLOG("SFlareSectorMenu::Enter");

	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	StationDescription = NULL;
	AFlarePlayerController* PC = MenuManager->GetPC();
	TargetSector = Sector;
	
	// Known sector
	if (PC->GetCompany()->HasVisitedSector(TargetSector))
	{		
		// Add stations
		for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorStations().Num(); SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* StationCandidate = Sector->GetSectorStations()[SpacecraftIndex];

			if (StationCandidate)
			{
				if (StationCandidate->GetCompany() == PC->GetCompany())
				{
					OwnedShipList->AddShip(StationCandidate);
				}
				else
				{
					OtherShipList->AddShip(StationCandidate);
				}
			}
		}

		// Add ships
		for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorShips().Num(); SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* ShipCandidate = Sector->GetSectorShips()[SpacecraftIndex];

			if (ShipCandidate && ShipCandidate->GetDamageSystem()->IsAlive())
			{
				if (ShipCandidate->GetCompany() == PC->GetCompany())
				{
					OwnedShipList->AddShip(ShipCandidate);
				}
				else
				{
					OtherShipList->AddShip(ShipCandidate);
				}
			}
		}
	}

	// Unknown sector
	else
	{
	}

	// List setup
	OwnedShipList->RefreshList();
	OtherShipList->RefreshList();
	OwnedShipList->SetVisibility(EVisibility::Visible);
	OtherShipList->SetVisibility(EVisibility::Visible);

	// Fleet list
	FleetList.Empty();
	int32 FleetCount = PC->GetCompany()->GetCompanyFleets().Num();
	for (int32 FleetIndex = 0; FleetIndex < FleetCount; FleetIndex++)
	{
		UFlareFleet* Fleet = PC->GetCompany()->GetCompanyFleets()[FleetIndex];
		if (Fleet && Fleet->GetShips().Num())
		{
			FleetList.Add(Fleet);
		}
	}
	FleetSelector->RefreshOptions();
	FleetSelector->SetSelectedItem(PC->GetPlayerFleet());
}

void SFlareSectorMenu::Exit()
{
	SetEnabled(false);
	TargetSector = NULL;

	OwnedShipList->Reset();
	OtherShipList->Reset();
	OwnedShipList->SetVisibility(EVisibility::Collapsed);
	OtherShipList->SetVisibility(EVisibility::Collapsed);
	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FText SFlareSectorMenu::GetBuildStationText() const
{
	AFlarePlayerController* PC = MenuManager->GetPC();

	if (TargetSector)
	{
		if (PC && PC->GetCompany()->HasVisitedSector(TargetSector))
		{
			return FText::Format(LOCTEXT("BuildStationFormat", "Build station ({0} / {1})"),
				FText::AsNumber(TargetSector->GetSectorStations().Num()),
				FText::AsNumber(TargetSector->GetMaxStationsInSector()));
		}
		else
		{
			return LOCTEXT("BuildStation", "Build station (?)");
		}
	}
	else
	{
		return FText();
	}
}

bool SFlareSectorMenu::IsBuildStationDisabled() const
{
	AFlarePlayerController* PC = MenuManager->GetPC();

	if (TargetSector
		&& PC
		&& PC->GetCompany()->HasVisitedSector(TargetSector)
		&& TargetSector->GetSectorStations().Num() < TargetSector->GetMaxStationsInSector())
	{
		return false;
	}
	else
	{
		return true;
	}
}

TSharedRef<SWidget> SFlareSectorMenu::OnGenerateFleetComboLine(UFlareFleet* Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FText Name;

	if (MenuManager->GetPC()->GetPlayerFleet() == Item)
	{
		Name = FText::Format(LOCTEXT("PlayerFleetFormat", "{0} (Your fleet)"), Item->GetFleetName());
	}
	else
	{
		Name = Item->GetFleetName();
	}

	return SNew(STextBlock)
		.Text(Name)
		.TextStyle(&Theme.TextFont);
}

FText SFlareSectorMenu::OnGetCurrentFleetComboLine() const
{
	UFlareFleet* SelectedFleet = FleetSelector->GetSelectedItem();
	if (SelectedFleet)
	{
		if (MenuManager->GetPC()->GetPlayerFleet() == SelectedFleet)
		{
			return FText::Format(LOCTEXT("PlayerFleetFormat", "{0} (Your fleet)"), SelectedFleet->GetFleetName());
		}
		else
		{
			return SelectedFleet->GetFleetName();
		}
	}
	else
	{
		return LOCTEXT("SelectFleet", "Select a fleet");
	}
}

FText SFlareSectorMenu::GetTravelText() const
{
	UFlareFleet* SelectedFleet = FleetSelector->GetSelectedItem();

	if (SelectedFleet)
	{
		FText Reason;

		if (SelectedFleet->GetCurrentSector() == TargetSector)
		{
			return LOCTEXT("TravelAlreadyHereFormat", "Already there");
		}
		else if (!SelectedFleet->CanTravel(Reason))
		{
			return Reason;
		}
		else
		{
			return LOCTEXT("TravelFormat", "Travel");
		}
	}
	else
	{
		return LOCTEXT("TravelNoSelection", "No fleet selected");
	}
}

bool SFlareSectorMenu::IsTravelDisabled() const
{
	UFlareFleet* SelectedFleet = FleetSelector->GetSelectedItem();

	if (MenuManager->GetPC()->GetPlayerFleet() && MenuManager->GetPC()->GetPlayerFleet()->IsTraveling())
	{
		return true;
	}
	else if (SelectedFleet && SelectedFleet->GetCurrentSector() != TargetSector && SelectedFleet->CanTravel())
	{
		return false;
	}
	else
	{
		return true;
	}
}

FText SFlareSectorMenu::GetRefuelText() const
{
	if (IsRefuelDisabled())
	{
		if (true) // TODO FRED(#155) : utiliser les API de ravitaillement
		{
			return LOCTEXT("NoFleetToRefuel", "No fleet here needs refilling");
		}
		else
		{
			return LOCTEXT("CantRefuel", "Can't refill here !");
		}
	}
	else
	{
		return FText::Format(LOCTEXT("RefuelOkayFormat", "Refill all fleets ({0} fleet supplies)"),
			FText::AsNumber(42)); // TODO FRED (#155) : utiliser les API de ravitaillement
	}
}

FText SFlareSectorMenu::GetRepairText() const
{
	if (IsRepairDisabled())
	{
		if (true) // TODO FRED(#155) : utiliser les API de réparation
		{
			return LOCTEXT("NoFleetToRepair", "No fleet here needs repairing");
		}
		else
		{
			return LOCTEXT("CantRepair", "Can't repair here !");
		}
	}
	else
	{
		return FText::Format(LOCTEXT("RepairOkayFormat", "Repair all fleets ({0} fleet supplies)"),
			FText::AsNumber(42)); // TODO FRED (#155) : utiliser les API de réparation
	}
}

bool SFlareSectorMenu::IsResourcePricesDisabled() const
{
	if (TargetSector)
	{
		return !MenuManager->GetPC()->GetCompany()->HasVisitedSector(TargetSector);
	}
	return false;
}

bool SFlareSectorMenu::IsRefuelDisabled() const
{
	return true; // TODO FRED (#155) : utiliser les API de ravitaillement
}

bool SFlareSectorMenu::IsRepairDisabled() const
{
	return true; // TODO FRED (#155) : utiliser les API de réparation
}

FText SFlareSectorMenu::GetSectorName() const
{
	FText Result;
	if (IsEnabled() && TargetSector)
	{
		if (MenuManager->GetPC()->GetPlayerFleet() && MenuManager->GetPC()->GetPlayerFleet()->IsTraveling())
		{
			Result = MenuManager->GetPC()->GetPlayerFleet()->GetStatusInfo();
		}
		else
		{
			Result = FText::Format(LOCTEXT("CurrentSectorFormat", "Current sector : {0} ({1})"),
				TargetSector->GetSectorName(),
				TargetSector->GetSectorFriendlynessText(MenuManager->GetPC()->GetCompany()));
		}
	}

	return Result;
}

FText SFlareSectorMenu::GetSectorDescription() const
{
	FText Result;

	if (IsEnabled() && TargetSector)
	{
		Result = TargetSector->GetSectorDescription();
	}

	return Result;
}


FText SFlareSectorMenu::GetSectorLocation() const
{
	FText Result;

	if (IsEnabled() && TargetSector)
	{
		FFlareCelestialBody* Body = TargetSector->GetGame()->GetGameWorld()->GetPlanerarium()->FindCelestialBody(TargetSector->GetOrbitParameters()->CelestialBodyIdentifier);

		if (Body)
		{
			FText LightRatioString;
			FString AttributeString;

			// Light ratio
			if (TargetSector->GetDescription()->IsSolarPoor)
			{
				LightRatioString = LOCTEXT("SectorLightRatioFoggy", "0%");
				AttributeString += LOCTEXT("Foggy", "Foggy").ToString();
			}
			else
			{
				int32 LightRatio = 100 * TargetSector->GetGame()->GetGameWorld()->GetPlanerarium()->GetLightRatio(Body, TargetSector->GetOrbitParameters()->Altitude);
				LightRatioString = FText::Format(LOCTEXT("SectorLightRatioFormat", "{0}%"), FText::AsNumber(LightRatio));
			}

			// Peaceful
			if (TargetSector->GetDescription()->IsPeaceful)
			{
				if (AttributeString.Len())
				{
					AttributeString += ", ";
				}
				AttributeString += LOCTEXT("Peaceful", "Peaceful").ToString();
			}

			// Icy
			if (TargetSector->GetDescription()->IsIcy)
			{
				if (AttributeString.Len())
				{
					AttributeString += ", ";
				}
				AttributeString += LOCTEXT("Icy", "Icy").ToString();
			}

			// Geostationary
			if (TargetSector->GetDescription()->IsGeostationary)
			{
				if (AttributeString.Len())
				{
					AttributeString += ", ";
				}
				AttributeString += LOCTEXT("Geostationary", "Geostationary").ToString();
			}

			// Spacer
			if (AttributeString.Len())
			{
				AttributeString = "- " + AttributeString;
			}

			// Result
			Result = FText::Format(LOCTEXT("SectorLocation",  "Orbiting {0} - Altitude: {1} km - {2} light {3}"),
				Body->Name,
				FText::AsNumber(TargetSector->GetOrbitParameters()->Altitude),
				LightRatioString,
				FText::FromString(AttributeString));
		}
	}

	return Result;
}


/*----------------------------------------------------
	Action callbacks
----------------------------------------------------*/

void SFlareSectorMenu::OnResourcePrices()
{
	FFlareMenuParameterData Data;
	Data.Sector = TargetSector;
	MenuManager->OpenMenu(EFlareMenu::MENU_ResourcePrices, Data);
}

void SFlareSectorMenu::OnFleetComboLineSelectionChanged(UFlareFleet* Item, ESelectInfo::Type SelectInfo)
{
}

void SFlareSectorMenu::OnTravelHereClicked()
{
	UFlareFleet* SelectedFleet = FleetSelector->GetSelectedItem();
	if (SelectedFleet)
	{
		if (SelectedFleet->GetImmobilizedShipCount() == 0)
		{
			OnStartTravelConfirmed(SelectedFleet);
		}
		else
		{
			FText ShipText = LOCTEXT("ConfirmTravelSingular", "{0} ship is too damaged to travel. Do you really want to leave it in this sector ?");
			FText ShipsText = LOCTEXT("ConfirmTravelPlural", "{0} ships are too damaged to travel. Do you really want to leave them in this sector ?");

			FText ConfirmText = FText::Format(SelectedFleet->GetImmobilizedShipCount() != 1 ? ShipsText : ShipText,
											  FText::AsNumber(SelectedFleet->GetImmobilizedShipCount()));

			MenuManager->Confirm(LOCTEXT("ConfirmTravelTitle", "ABANDON SHIPS ?"),
								 ConfirmText,
								 FSimpleDelegate::CreateSP(this, &SFlareSectorMenu::OnStartTravelConfirmed, SelectedFleet));
		}
	}
}

void SFlareSectorMenu::OnRefuelClicked()
{
	// TODO FRED (#155) : utiliser les API de ravitaillement
}

void SFlareSectorMenu::OnRepairClicked()
{
	// TODO FRED (#155) : utiliser les API de réparation
}

void SFlareSectorMenu::OnStartTravelConfirmed(UFlareFleet* SelectedFleet)
{	
	if (SelectedFleet)
	{
		UFlareTravel* Travel = MenuManager->GetGame()->GetGameWorld()->StartTravel(SelectedFleet, TargetSector);
		if (Travel)
		{
			FFlareMenuParameterData Data;
			Data.Travel = Travel;
			MenuManager->OpenMenu(EFlareMenu::MENU_Travel, Data);
		}
	}
}

void SFlareSectorMenu::OnBuildStationClicked()
{
	FFlareMenuParameterData Data;
	Data.Sector = TargetSector;
	MenuManager->OpenSpacecraftOrder(Data, FOrderDelegate::CreateSP(this, &SFlareSectorMenu::OnBuildStationSelected));
}

void SFlareSectorMenu::OnBuildStationSelected(FFlareSpacecraftDescription* NewStationDescription)
{
	StationDescription = NewStationDescription;

	if (StationDescription)
	{
		// Can we build ?
		TArray<FText> Reasons;
		FString ResourcesString;
		UFlareFleet* PlayerFleet = MenuManager->GetPC()->GetPlayerFleet();
		bool StationBuildable = TargetSector->CanBuildStation(StationDescription, MenuManager->GetPC()->GetCompany(), Reasons);

		// Build it
		if (StationBuildable)
		{
			UFlareSimulatedSpacecraft* NewStation = TargetSector->BuildStation(StationDescription, MenuManager->GetPC()->GetCompany());

			// Same sector
			if (PlayerFleet && PlayerFleet->GetCurrentSector() == TargetSector)
			{
				FFlareMenuParameterData MenuParameters;
				MenuParameters.Spacecraft = PlayerFleet->GetShips()[0];
				MenuParameters.Sector = TargetSector;
				MenuManager->OpenMenu(EFlareMenu::MENU_ReloadSector, MenuParameters);
			}

			// Other sector
			else
			{
				FFlareMenuParameterData MenuParameters;
				MenuParameters.Sector = TargetSector;
				MenuManager->OpenMenu(EFlareMenu::MENU_Sector, MenuParameters);
			}

			// Notify
			FFlareMenuParameterData NotificationParameters;
			NotificationParameters.Spacecraft = NewStation;
			MenuManager->GetPC()->Notify(
				LOCTEXT("StationBuilt", "Station built"),
				LOCTEXT("StationBuiltInfo", "Your new station has been built."),
				"station-built",
				EFlareNotification::NT_Economy,
				false,
				EFlareMenu::MENU_Ship,
				NotificationParameters);
		}
	}
}

#undef LOCTEXT_NAMESPACE

