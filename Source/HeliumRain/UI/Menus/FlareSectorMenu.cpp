
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
	
	StationList.Empty();

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
				SNew(SImage).Image(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Sector))
			]

			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TitleFont)
				.Text(LOCTEXT("SectorInfo", "SECTOR INFO"))
			]

			// Close
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("GoOrbit", "Orbital map"))
				.HelpText(LOCTEXT("GoOrbitInfo", "Exit the sector menu and go back to the orbital map"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Orbit, true))
				.OnClicked(this, &SFlareSectorMenu::OnBackClicked)
			]
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(200, 20))
		[
			SNew(SImage).Image(&Theme.SeparatorBrush)
		]

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
					.WidthOverride(Theme.ContentWidth - Theme.ContentPadding.Left - Theme.ContentPadding.Right)
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
							.WrapTextAt(Theme.ContentWidth - 2 * Theme.ContentPadding.Left - 2 * Theme.ContentPadding.Right)
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
							.Text(this, &SFlareSectorMenu::GetTravelText)
							.HelpText(LOCTEXT("TravelInfo", "Start travelling to this sector with the current ship or fleet"))
							.Icon(FFlareStyleSet::GetIcon("Travel"))
							.OnClicked(this, &SFlareSectorMenu::OnTravelHereClicked)
							.IsDisabled(this, &SFlareSectorMenu::IsTravelDisabled)
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
							.HelpText(LOCTEXT("TravelInfo", "Refuel all fleets in this sector so that they have the necessary fuel, ammo and resources to fight."))
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
							.HelpText(LOCTEXT("TravelInfo", "Repair all fleets in this sector."))
							.Icon(FFlareStyleSet::GetIcon("Repair"))
							.OnClicked(this, &SFlareSectorMenu::OnRepairClicked)
							.IsDisabled(this, &SFlareSectorMenu::IsRepairDisabled)
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

						// Build station text
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.TitlePadding)
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Top)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(this, &SFlareSectorMenu::GetBuildStationText)
							.Visibility(this, &SFlareSectorMenu::GetBuildStationVisibility)
						]

						// Station selection
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(StationSelector, SComboBox<UFlareSpacecraftCatalogEntry*>)
							.OptionsSource(&StationList)
							.OnGenerateWidget(this, &SFlareSectorMenu::OnGenerateStationComboLine)
							.OnSelectionChanged(this, &SFlareSectorMenu::OnStationComboLineSelectionChanged)
							.ComboBoxStyle(&Theme.ComboBoxStyle)
							.ForegroundColor(FLinearColor::White)
							[
								SNew(STextBlock)
								.Text(this, &SFlareSectorMenu::OnGetCurrentStationComboLine)
								.TextStyle(&Theme.TextFont)
							]
							.Visibility(this, &SFlareSectorMenu::GetBuildStationVisibility)
						]

						// Station cost text
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						.HAlign(HAlign_Left)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(this, &SFlareSectorMenu::OnGetStationCost)
							.WrapTextAt(10 * Theme.ButtonWidth)
							.Visibility(this, &SFlareSectorMenu::GetBuildStationVisibility)
						]

						// Build station button
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						.HAlign(HAlign_Left)
						[
							SNew(SFlareButton)
							.Width(10)
							.Text(LOCTEXT("BuildStationButton", "Build station"))
							.HelpText(LOCTEXT("BuildStationInfo", "Build a station"))
							.Icon(FFlareStyleSet::GetIcon("Travel"))
							.OnClicked(this, &SFlareSectorMenu::OnBuildStationClicked)
							.IsDisabled(this, &SFlareSectorMenu::IsBuildStationDisabled)
							.Visibility(this, &SFlareSectorMenu::GetBuildStationVisibility)
						]
					]
				]
			]
					
			// Content block
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Top)
			[
				SNew(SScrollBox)
				.Style(&Theme.ScrollBoxStyle)
				.ScrollBarStyle(&Theme.ScrollBarStyle)

				+ SScrollBox::Slot()
				[
					SNew(SHorizontalBox)

					// Owned
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Right)
					[
						SAssignNew(OwnedShipList, SFlareShipList)
						.MenuManager(MenuManager)
						.Title(LOCTEXT("MyListTitle", "OWNED SPACECRAFTS IN SECTOR"))
					]

					// Others
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					[
						SAssignNew(OtherShipList, SFlareShipList)
						.MenuManager(MenuManager)
						.Title(LOCTEXT("OtherListTitle", "OTHER SPACECRAFTS IN SECTOR"))
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

	TargetSector = Sector;
	AFlarePlayerController* PC = MenuManager->GetPC();

	StationList.Empty();

	// Known sector
	if (PC->GetCompany()->HasVisitedSector(TargetSector))
	{
		// Init buildable station list
		UFlareSpacecraftCatalog* SpacecraftCatalog = MenuManager->GetGame()->GetSpacecraftCatalog();
		for (int SpacecraftIndex = 0; SpacecraftIndex < SpacecraftCatalog->StationCatalog.Num(); SpacecraftIndex++)
		{
			UFlareSpacecraftCatalogEntry* Entry = SpacecraftCatalog->StationCatalog[SpacecraftIndex];
			FFlareSpacecraftDescription* Description = &Entry->Data;

			if(TargetSector->GetDescription()->IsIcy && Description->BuildConstraint.Contains(EFlareBuildConstraint::HideOnIce))
			{
				continue;
			}

			if (!TargetSector->GetDescription()->IsIcy && Description->BuildConstraint.Contains(EFlareBuildConstraint::HideOnNoIce))
			{
				continue;
			}

			StationList.Add(Entry);
		}
		
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
	UpdateStationCost();
}

void SFlareSectorMenu::Exit()
{
	SetEnabled(false);
	TargetSector = NULL;

	OwnedShipList->Reset();
	OtherShipList->Reset();
	StationSelector->ClearSelection();
	StationBuildable = false;

	SetVisibility(EVisibility::Collapsed);
}

void SFlareSectorMenu::UpdateStationCost()
{
	UFlareSpacecraftCatalogEntry* Item = StationSelector->GetSelectedItem();
	if (Item)
	{
		FString ResourcesString;
		FFlareSpacecraftDescription* StationDescription = &Item->Data;
		StationBuildable = TargetSector->CanBuildStation(StationDescription, MenuManager->GetPC()->GetCompany());

		// Add resources
		for (int ResourceIndex = 0; ResourceIndex < StationDescription->CycleCost.InputResources.Num(); ResourceIndex++)
		{
			FFlareFactoryResource* FactoryResource = &StationDescription->CycleCost.InputResources[ResourceIndex];
			ResourcesString += FString::Printf(TEXT(", %u %s"), FactoryResource->Quantity, *FactoryResource->Resource->Data.Name.ToString()); // FString needed here
		}

		// Constraints
		FString ConstraintString;
		FText ConstraintText;
		FText AsteroidText = LOCTEXT("AsteroidNeeded", "a free asteroid");
		FText SunText = LOCTEXT("SunNeeded", "good sun exposure");
		FText GeostationaryText = LOCTEXT("GeostationaryNeeded", "a geostationary orbit");
;
		if (StationDescription->BuildConstraint.Contains(EFlareBuildConstraint::FreeAsteroid))
		{
			ConstraintString += " " + AsteroidText.ToString();
		}
		if (StationDescription->BuildConstraint.Contains(EFlareBuildConstraint::SunExposure))
		{
			ConstraintString += " " + SunText.ToString();
		}
		if (StationDescription->BuildConstraint.Contains(EFlareBuildConstraint::GeostationaryOrbit))
		{
			ConstraintString += " " + GeostationaryText.ToString();
		}
		if (ConstraintString.Len() > 0)
		{
			ConstraintText = FText::Format(LOCTEXT("ConstraintStationFormat", "You also need{0}."), FText::FromString(ConstraintString));
		}

		// Final text
		FText CanBuildText = LOCTEXT("CanBuildStation", "You can build this station !");
		FText CannotBuildText = LOCTEXT("CannotBuildStation", "You can't build this station yet.");
		StationCost = FText::Format(LOCTEXT("StationCostFormat", "{0} It costs {1} credits{2}, and requires a cargo ship in this sector. {3}"),
			StationBuildable ? CanBuildText : CannotBuildText, FText::AsNumber(StationDescription->CycleCost.ProductionCost), FText::FromString(ResourcesString), ConstraintText);
	}
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FText SFlareSectorMenu::GetBuildStationText() const
{
	if (TargetSector)
	{
		return FText::Format(LOCTEXT("BuildStation", "BUILD A STATION ({0} / {1})"),
			FText::AsNumber(TargetSector->GetSectorStations().Num()),
				FText::AsNumber(TargetSector->GetMaxStationsInSector()));
	}
	else
	{
		return FText();
	}
}

EVisibility SFlareSectorMenu::GetBuildStationVisibility() const
{
	AFlarePlayerController* PC = MenuManager->GetPC();
	
	// Known sector
	if (PC && PC->GetCompany()->HasVisitedSector(TargetSector))
	{
		return EVisibility::Visible;
	}
	{
		return EVisibility::Collapsed;
	}
}

FText SFlareSectorMenu::GetTravelText() const
{
	FText FlyText = LOCTEXT("Travel", "Travel");
	UFlareFleet* SelectedFleet = MenuManager->GetPC()->GetSelectedFleet();

	if (SelectedFleet)
	{
		return FText::Format(LOCTEXT("TravelFormat", "{0} with selection ({1})"), FlyText, SelectedFleet->GetFleetName());
	}
	else
	{
		return FlyText;
	}
}

bool SFlareSectorMenu::IsTravelDisabled() const
{
	UFlareFleet* CurrentFleet = MenuManager->GetPC()->GetSelectedFleet();

	if (CurrentFleet && CurrentFleet->GetCurrentSector() != TargetSector && CurrentFleet->CanTravel())
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
			return LOCTEXT("NoFleetToRefuel", "No fleet here needs refuelling");
		}
		else
		{
			return LOCTEXT("CantRefuel", "Can't refuel here !");
		}
	}
	else
	{
		return FText::Format(LOCTEXT("RefuelOkayFormat", "Refuel all fleets ({0} fleet supplies)"),
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
			FText::FromString(TargetSector->GetSectorName().ToString().ToUpper()), //FString needed here
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
			"Stations require {0} cargo units per day (current capacity is {1}).");

		Result = FText::Format(TransportInfoText,
			FText::AsNumber(TargetSector->GetTransportCapacityNeeds(PlayerCompany)),
			FText::AsNumber(TargetSector->GetTransportCapacity(PlayerCompany))
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

TSharedRef<SWidget> SFlareSectorMenu::OnGenerateStationComboLine(UFlareSpacecraftCatalogEntry* Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(STextBlock)
		.Text(Item->Data.Name)
		.TextStyle(&Theme.TextFont);
}

FText SFlareSectorMenu::OnGetStationCost() const
{
	return StationCost;
}

bool SFlareSectorMenu::IsBuildStationDisabled() const
{
	return (!StationBuildable);
}

FText SFlareSectorMenu::OnGetCurrentStationComboLine() const
{
	UFlareSpacecraftCatalogEntry* Item = StationSelector->GetSelectedItem();
	return Item ? Item->Data.Name : LOCTEXT("Select", "Select a station");
}


/*----------------------------------------------------
	Action callbacks
----------------------------------------------------*/

void SFlareSectorMenu::OnBackClicked()
{
	MenuManager->Back();
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

void SFlareSectorMenu::OnStartTravelConfirmed()
{
	UFlareFleet* SelectedFleet = MenuManager->GetGame()->GetPC()->GetSelectedFleet();
	if (SelectedFleet)
	{
		MenuManager->GetGame()->GetGameWorld()->StartTravel(SelectedFleet, TargetSector);
		MenuManager->Back();
	}
}

void SFlareSectorMenu::OnStationComboLineSelectionChanged(UFlareSpacecraftCatalogEntry* Item, ESelectInfo::Type SelectInfo)
{
	UpdateStationCost();
}

void SFlareSectorMenu::OnBuildStationClicked()
{
	UFlareSpacecraftCatalogEntry* Item = StationSelector->GetSelectedItem();
	if (Item)
	{
		FFlareSpacecraftDescription* StationDescription = &Item->Data;
		TargetSector->BuildStation(StationDescription, MenuManager->GetPC()->GetCompany());
		MenuManager->OpenMenu(EFlareMenu::MENU_Sector, TargetSector);
	}
}

#undef LOCTEXT_NAMESPACE

