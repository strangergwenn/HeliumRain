
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
		.Padding(Theme.ContentPadding)
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
				
						// Sector transport info
						+ SVerticalBox::Slot()
						.Padding(Theme.ContentPadding)
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(this, &SFlareSectorMenu::GetSectorTransportInfo)
							.TextStyle(&Theme.TextFont)
							.WrapTextAt(Theme.ContentWidth)
						]

						// Travel here
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.SmallContentPadding)
						.HAlign(HAlign_Left)
						[
							SNew(SFlareButton)
							.Width(10)
							.Text(this, &SFlareSectorMenu::GetPlayerTravelText)
							.HelpText(LOCTEXT("TravelInfo", "Start travelling to this sector with the player ship or fleet"))
							.Icon(FFlareStyleSet::GetIcon("Travel"))
							.OnClicked(this, &SFlareSectorMenu::OnPlayerTravelHereClicked)
							.IsDisabled(this, &SFlareSectorMenu::IsPlayerTravelDisabled)
						]

						// Travel here
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.SmallContentPadding)
						.HAlign(HAlign_Left)
						[
							SNew(SFlareButton)
							.Width(10)
							.Text(this, &SFlareSectorMenu::GetTravelText)
							.HelpText(LOCTEXT("TravelInfo", "Start travelling to this sector with the selected ship or fleet"))
							.Icon(FFlareStyleSet::GetIcon("Travel"))
							.OnClicked(this, &SFlareSectorMenu::OnTravelHereClicked)
							.IsDisabled(this, &SFlareSectorMenu::IsTravelDisabled)
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

						// Show prices
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.SmallContentPadding)
						.HAlign(HAlign_Left)
						[
							SNew(SFlareButton)
							.Width(10)
							.Text(LOCTEXT("ResourcePrices", "Resource prices"))
							.HelpText(LOCTEXT("ResourcePricesInfo", "See the price and use of resources in this sector"))
							.Icon(FFlareStyleSet::GetIcon("Travel"))
							.OnClicked(this, &SFlareSectorMenu::OnResourcePrices)
							.IsDisabled(this, &SFlareSectorMenu::IsResourcePricesDisabled)
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

						// Build station button
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.SmallContentPadding)
						.HAlign(HAlign_Left)
						[
							SNew(SFlareButton)
							.Width(10)
							.Text(this, &SFlareSectorMenu::GetBuildStationText)
							.HelpText(LOCTEXT("BuildStationInfo", "Build a station in this sector"))
							.Icon(FFlareStyleSet::GetIcon("Travel"))
							.OnClicked(this, &SFlareSectorMenu::OnBuildStationClicked)
							.IsDisabled(this, &SFlareSectorMenu::IsBuildStationDisabled)
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

	OwnedShipList->RefreshList();
	OtherShipList->RefreshList();
	OwnedShipList->SetVisibility(EVisibility::Visible);
	OtherShipList->SetVisibility(EVisibility::Visible);
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

FText SFlareSectorMenu::GetPlayerTravelText() const
{
	UFlareFleet* PlayerFleet = MenuManager->GetPC()->GetPlayerFleet();

	if (PlayerFleet)
	{
		FText Reason;

		if (PlayerFleet->GetCurrentSector() == TargetSector)
		{
			return FText::Format(LOCTEXT("PlayerTravelAlreadyHereFormat", "{0} is already there"), PlayerFleet->GetFleetName());
		}
		else if (!PlayerFleet->CanTravel(Reason))
		{
			return Reason;
		}
		else
		{
			return FText::Format(LOCTEXT("PlayerTravelFormat", "Travel with player fleet ({0})"), PlayerFleet->GetFleetName());
		}
	}
	else
	{
		return LOCTEXT("PlayerTravelNoSelection", "Travel here (player fleet unavailable)");
	}
}

FText SFlareSectorMenu::GetTravelText() const
{
	UFlareFleet* SelectedFleet = MenuManager->GetPC()->GetSelectedFleet();

	if (SelectedFleet)
	{
		FText Reason;

		if (SelectedFleet->GetCurrentSector() == TargetSector)
		{
			return FText::Format(LOCTEXT("TravelAlreadyHereFormat", "{0} is already there"), SelectedFleet->GetFleetName());
		}
		else if (!SelectedFleet->CanTravel(Reason))
		{
			return Reason;
		}
		else
		{
			return FText::Format(LOCTEXT("TravelFormat", "Travel with selection ({0})"), SelectedFleet->GetFleetName());
		}
	}
	else
	{
		return LOCTEXT("TravelNoSelection", "Travel here (no fleet selected)");
	}
}

bool SFlareSectorMenu::IsPlayerTravelDisabled() const
{
	UFlareFleet* PlayerFleet = MenuManager->GetPC()->GetPlayerFleet();

	if (PlayerFleet && PlayerFleet->GetCurrentSector() != TargetSector && PlayerFleet->CanTravel())
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool SFlareSectorMenu::IsTravelDisabled() const
{
	UFlareFleet* SelectedFleet = MenuManager->GetPC()->GetSelectedFleet();

	if (SelectedFleet && SelectedFleet->GetCurrentSector() != TargetSector && SelectedFleet->CanTravel())
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
	if (TargetSector)
	{
		Result = FText::Format(LOCTEXT("SectorFormat", "Sector : {0} ({1})"),
			FText::FromString(TargetSector->GetSectorName().ToString()), //FString needed here
			TargetSector->GetSectorFriendlynessText(MenuManager->GetPC()->GetCompany()));
	}

	return Result;
}

FText SFlareSectorMenu::GetSectorDescription() const
{
	FText Result;

	if (TargetSector)
	{
		Result = TargetSector->GetSectorDescription();
	}

	return Result;
}

FText SFlareSectorMenu::GetSectorTransportInfo() const
{
	FText Result;

	if (TargetSector)
	{
		UFlareCompany* PlayerCompany = MenuManager->GetGame()->GetPC()->GetCompany();
		FText TransportInfoText = LOCTEXT("SectorDescriptionFormat",
			"Stations here require {0} cargo units per day (current capacity is {1}). The sector has a population of {2}.");

		Result = FText::Format(TransportInfoText,
			FText::AsNumber(TargetSector->GetTransportCapacityNeeds(PlayerCompany)),
			FText::AsNumber(TargetSector->GetTransportCapacity(PlayerCompany)),
			FText::AsNumber(TargetSector->GetPeople()->GetPopulation())
		);
	}

	return Result;
}

FText SFlareSectorMenu::GetSectorLocation() const
{
	FText Result;

	if (TargetSector)
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
	MenuManager->OpenMenu(EFlareMenu::MENU_ResourcePrices, TargetSector);
}

void SFlareSectorMenu::OnPlayerTravelHereClicked()
{
	UFlareFleet* PlayerFleet = MenuManager->GetGame()->GetPC()->GetPlayerFleet();
	if (PlayerFleet)
	{
		if (PlayerFleet->GetImmobilizedShipCount() == 0)
		{
			OnPlayerStartTravelConfirmed();
		}
		else
		{
			FText ShipText = LOCTEXT("PlayerConfirmTravelSingular", "{0} ship is too damaged to travel. Do you really want to leave it in this sector ?");
			FText ShipsText = LOCTEXT("PlayerConfirmTravelPlural", "{0} ships are too damaged to travel. Do you really want to leave them in this sector ?");

			FText ConfirmText = FText::Format(PlayerFleet->GetImmobilizedShipCount() != 1 ? ShipsText : ShipText,
				FText::AsNumber(PlayerFleet->GetImmobilizedShipCount()));

			MenuManager->Confirm(LOCTEXT("PlayerConfirmTravelTitle", "ABANDON SHIPS ?"),
				ConfirmText,
				FSimpleDelegate::CreateSP(this, &SFlareSectorMenu::OnPlayerStartTravelConfirmed));
		}
	}
}

void SFlareSectorMenu::OnTravelHereClicked()
{
	UFlareFleet* SelectedFleet = MenuManager->GetGame()->GetPC()->GetSelectedFleet();
	if (SelectedFleet)
	{
		if (SelectedFleet->GetImmobilizedShipCount() == 0)
		{
			OnStartTravelConfirmed();
		}
		else
		{
			FText ShipText = LOCTEXT("ConfirmTravelSingular", "{0} ship is too damaged to travel. Do you really want to leave it in this sector ?");
			FText ShipsText = LOCTEXT("ConfirmTravelPlural", "{0} ships are too damaged to travel. Do you really want to leave them in this sector ?");

			FText ConfirmText = FText::Format(SelectedFleet->GetImmobilizedShipCount() != 1 ? ShipsText : ShipText,
											  FText::AsNumber(SelectedFleet->GetImmobilizedShipCount()));

			MenuManager->Confirm(LOCTEXT("ConfirmTravelTitle", "ABANDON SHIPS ?"),
								 ConfirmText,
								 FSimpleDelegate::CreateSP(this, &SFlareSectorMenu::OnStartTravelConfirmed));
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

void SFlareSectorMenu::OnPlayerStartTravelConfirmed()
{
	UFlareFleet* PlayerFleet = MenuManager->GetGame()->GetPC()->GetPlayerFleet();
	if (PlayerFleet)
	{
		MenuManager->GetGame()->GetGameWorld()->StartTravel(PlayerFleet, TargetSector);
		MenuManager->Back();
	}
}

void SFlareSectorMenu::OnStartTravelConfirmed()
{
	UFlareFleet* SelectedFleet = MenuManager->GetGame()->GetPC()->GetSelectedFleet();
	if (SelectedFleet)
	{
		MenuManager->GetGame()->GetGameWorld()->StartTravel(SelectedFleet, TargetSector);
		MenuManager->Back();
	}
}

void SFlareSectorMenu::OnBuildStationClicked()
{
	MenuManager->OpenSpacecraftOrder(TargetSector, FOrderDelegate::CreateSP(this, &SFlareSectorMenu::OnBuildStationSelected));
}

void SFlareSectorMenu::OnBuildStationSelected(FFlareSpacecraftDescription* NewStationDescription)
{
	StationDescription = NewStationDescription;

	if (StationDescription)
	{
		// Can we build ?
		TArray<FText> Reasons;
		FString ResourcesString;
		bool StationBuildable = TargetSector->CanBuildStation(StationDescription, MenuManager->GetPC()->GetCompany(), Reasons);

		// Build it
		if (StationBuildable)
		{
			TargetSector->BuildStation(StationDescription, MenuManager->GetPC()->GetCompany());
			MenuManager->OpenMenu(EFlareMenu::MENU_Sector, TargetSector);
		}
	}
}

#undef LOCTEXT_NAMESPACE

