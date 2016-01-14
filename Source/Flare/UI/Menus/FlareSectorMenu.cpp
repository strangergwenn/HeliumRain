
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
	
	// Init station list
	UFlareSpacecraftCatalog* SpacecraftCatalog = MenuManager->GetGame()->GetSpacecraftCatalog();
	for (int SpacecraftIndex = 0; SpacecraftIndex < SpacecraftCatalog->StationCatalog.Num(); SpacecraftIndex++)
	{
		StationList.Add(SpacecraftCatalog->StationCatalog[SpacecraftIndex]);
	}

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
						.WrapTextAt(Theme.ContentWidth)
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
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Left)
					[
						SNew(SFlareButton)
						.Width(8)
						.Text(this, &SFlareSectorMenu::GetTravelText)
						.HelpText(LOCTEXT("TravelInfo", "Start travelling to this sector with the current ship or fleet"))
						.Icon(FFlareStyleSet::GetIcon("Travel"))
						.OnClicked(this, &SFlareSectorMenu::OnTravelHereClicked)
						.Visibility(this, &SFlareSectorMenu::GetTravelVisibility)
					]
				]
				
				// Station construction
				+ SHorizontalBox::Slot()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Right)
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
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(LOCTEXT("BuildStation", "BUILD A STATION"))
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
	SetVisibility(EVisibility::Hidden);
}

void SFlareSectorMenu::Enter(UFlareSimulatedSector* Sector)
{
	FLOG("SFlareSectorMenu::Enter");

	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	TargetSector = Sector;
	AFlarePlayerController* PC = MenuManager->GetPC();

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

	SetVisibility(EVisibility::Hidden);
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
		for (int ResourceIndex = 0; ResourceIndex < StationDescription->ResourcesCost.Num(); ResourceIndex++)
		{
			FFlareFactoryResource* FactoryResource = &StationDescription->ResourcesCost[ResourceIndex];
			ResourcesString += FString::Printf(TEXT(", %u %s"), FactoryResource->Quantity, *FactoryResource->Resource->Data.Name.ToString()); // FString needed here
		}

		// Final text
		FText CanBuildText = LOCTEXT("CanBuild", "You can build this station !");
		FText CannotBuildText = LOCTEXT("CannotBuild", "You can't build this station yet.");
		StationCost = FText::Format(LOCTEXT("StationCostFormat", "{0} It costs {1} credits{2} and requires an unnassigned cargo ship in this sector."),
			StationBuildable ? CanBuildText : CannotBuildText,
			FText::AsNumber(StationDescription->Cost),
			FText::FromString(ResourcesString));
	}
}

/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FText SFlareSectorMenu::GetTravelText() const
{
	FText FlyText = LOCTEXT("Travel", "Travel");
	UFlareFleet* SelectedFleet = MenuManager->GetPC()->GetSelectedFleet();

	if (SelectedFleet)
	{
		return FText::Format(LOCTEXT("TravelFormat", "{0} with {1}"), FlyText, SelectedFleet->GetName());
	}
	else
	{
		return FlyText;
	}
}

EVisibility SFlareSectorMenu::GetTravelVisibility() const
{
	UFlareFleet* CurrentFleet = MenuManager->GetPC()->GetSelectedFleet();

	if (CurrentFleet && CurrentFleet->GetCurrentSector() != TargetSector && !(CurrentFleet->IsTraveling() && !CurrentFleet->GetCurrentTravel()->CanChangeDestination()))
	{
		return EVisibility::Visible;
	}
	else
	{
		return EVisibility::Collapsed;
	}
}

FText SFlareSectorMenu::GetSectorName() const
{
	FText Result;

	if (TargetSector)
	{
		Result = FText::Format(LOCTEXT("SectorFormat", "SECTOR : {0} ({1})"),
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
		Result = FText::Format(LOCTEXT("SectorDescriptionFormat", "Capacity of assigned transports : {0} cargo units per day"),
			FText::AsNumber(TargetSector->GetTransportCapacity(MenuManager->GetGame()->GetPC()->GetCompany()))
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
			Result = FText::Format(LOCTEXT("SectorLocation",  "{0} - Altitude: {1} km - Phase: {2} \u00B0"), Body->Name, FText::AsNumber(TargetSector->GetOrbitParameters()->Altitude), FText::AsNumber(TargetSector->GetOrbitParameters()->Phase));
		}
	}

	return Result;
}

void SFlareSectorMenu::OnBackClicked()
{
	MenuManager->Back();
}

void SFlareSectorMenu::OnTravelHereClicked()
{
	UFlareFleet* SelectedFleet = MenuManager->GetGame()->GetPC()->GetSelectedFleet();
	if (SelectedFleet)
	{
		MenuManager->GetGame()->GetGameWorld()->StartTravel(SelectedFleet, TargetSector);
		MenuManager->Back();
	}
}

TSharedRef<SWidget> SFlareSectorMenu::OnGenerateStationComboLine(UFlareSpacecraftCatalogEntry* Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(STextBlock)
	.Text(Item->Data.Name)
	.TextStyle(&Theme.TextFont);
}

void SFlareSectorMenu::OnStationComboLineSelectionChanged(UFlareSpacecraftCatalogEntry* Item, ESelectInfo::Type SelectInfo)
{
	UpdateStationCost();
}

FText SFlareSectorMenu::OnGetStationCost() const
{
	return StationCost;
}

EVisibility SFlareSectorMenu::GetBuildStationVisibility() const
{
	return (StationBuildable ? EVisibility::Visible : EVisibility::Collapsed);
}

FText SFlareSectorMenu::OnGetCurrentStationComboLine() const
{
	UFlareSpacecraftCatalogEntry* Item = StationSelector->GetSelectedItem();
	return Item ? Item->Data.Name : LOCTEXT("Select", "Select a station");
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

