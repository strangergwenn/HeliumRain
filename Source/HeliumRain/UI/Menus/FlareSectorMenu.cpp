
#include "../../Flare.h"
#include "FlareSectorMenu.h"

#include "../../Game/FlareGame.h"
#include "../../Game/FlareSectorHelper.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Game/FlareScenarioTools.h"

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

						// Combat value
						+ SVerticalBox::Slot()
						.AutoHeight()
						.VAlign(VAlign_Center)
						.Padding(Theme.ContentPadding)
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SImage)
								.Image(FFlareStyleSet::GetIcon("CombatValue"))
								.Visibility(this, &SFlareSectorMenu::GetCombatValueVisibility)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.SmallContentPadding)
							[
								SNew(STextBlock)
								.Text(this, &SFlareSectorMenu::GetOwnCombatValue)
								.TextStyle(&Theme.TextFont)
								.Visibility(this, &SFlareSectorMenu::GetCombatValueVisibility)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("SectorOwnCombatValue", " owned combat value here, "))
								.TextStyle(&Theme.TextFont)
								.Visibility(this, &SFlareSectorMenu::GetCombatValueVisibility)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SImage)
								.Image(FFlareStyleSet::GetIcon("CombatValue"))
								.Visibility(this, &SFlareSectorMenu::GetCombatValueVisibility)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.SmallContentPadding)
							[
								SNew(STextBlock)
								.Text(this, &SFlareSectorMenu::GetHostileCombatValue)
								.TextStyle(&Theme.TextFont)
								.Visibility(this, &SFlareSectorMenu::GetCombatValueVisibility)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("SectorHostileCombatValue", " hostile, "))
								.TextStyle(&Theme.TextFont)
								.Visibility(this, &SFlareSectorMenu::GetCombatValueVisibility)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SImage)
								.Image(FFlareStyleSet::GetIcon("CombatValue"))
								.Visibility(this, &SFlareSectorMenu::GetCombatValueVisibility)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.SmallContentPadding)
							[
								SNew(STextBlock)
								.Text(this, &SFlareSectorMenu::GetFullCombatValue)
								.TextStyle(&Theme.TextFont)
								.Visibility(this, &SFlareSectorMenu::GetCombatValueVisibility)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("SectorFullCombatValue", " total"))
								.TextStyle(&Theme.TextFont)
								.Visibility(this, &SFlareSectorMenu::GetCombatValueVisibility)
							]
						]

						// Tools line 1
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Left)
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SHorizontalBox)

							// Fleet list
							+ SHorizontalBox::Slot()
							[
								SNew(SBox)
								.WidthOverride(10 * Theme.ButtonWidth)
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
					.WidthOverride(Theme.ContentWidth)
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

							// Show prices
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SFlareButton)
								.Width(6)
								.Text(LOCTEXT("ResourcePrices", "Local prices"))
								.HelpText(LOCTEXT("ResourcePricesInfo", "See the price and use of resources in this sector"))
								.Icon(FFlareStyleSet::GetIcon("Travel"))
								.OnClicked(this, &SFlareSectorMenu::OnResourcePrices)
								.IsDisabled(this, &SFlareSectorMenu::IsResourcePricesDisabled)
							]

							// Build station button
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SFlareButton)
								.Width(6)
								.Text(this, &SFlareSectorMenu::GetBuildStationText)
								.HelpText(LOCTEXT("BuildStationInfo", "Build a station in this sector"))
								.Icon(FFlareStyleSet::GetIcon("Travel"))
								.OnClicked(this, &SFlareSectorMenu::OnBuildStationClicked)
								.IsDisabled(this, &SFlareSectorMenu::IsBuildStationDisabled)
							]
						]

						// Refill fleets
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Left)
						[
							SNew(SFlareButton)
							.Width(12)
							.Text(this, &SFlareSectorMenu::GetRefillText)
							.HelpText(LOCTEXT("RefillInfo", "Refill all fleets in this sector so that they have the necessary fuel, ammo and resources to fight."))
							.Icon(FFlareStyleSet::GetIcon("Tank"))
							.OnClicked(this, &SFlareSectorMenu::OnRefillClicked)
							.IsDisabled(this, &SFlareSectorMenu::IsRefillDisabled)
						]

						// Repair fleets
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Left)
						[
							SNew(SFlareButton)
							.Width(12)
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
						SAssignNew(OwnedShipList, SFlareList)
						.MenuManager(MenuManager)
						.Title(LOCTEXT("OwnedSpacecraftsSector", "Owned spacecrafts in sector"))
					]

					+ SScrollBox::Slot()
					[
						SAssignNew(OwnedReserveShipList, SFlareList)
						.MenuManager(MenuManager)
						.Title(LOCTEXT("OwnedSpacecraftsReserve", "Owned spacecrafts in reserve"))
						.Visibility(this, &SFlareSectorMenu::GetOwnedReserveVisibility)
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
						SAssignNew(OtherShipList, SFlareList)
						.MenuManager(MenuManager)
						.Title(LOCTEXT("OtherSpacecraftsSector", "Other spacecrafts in sector"))
					]

					+ SScrollBox::Slot()
					[
						SAssignNew(OtherReserveShipList, SFlareList)
						.MenuManager(MenuManager)
						.Title(LOCTEXT("OtherSpacecraftsReserve", "Other spacecrafts in reserve"))
						.Visibility(this, &SFlareSectorMenu::GetOtherReserveVisibility)
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

	StationDescription = NULL;
	TargetSector = Sector;

	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	AFlarePlayerController* PC = MenuManager->GetPC();

	// Known sector
	if (PC->GetCompany()->HasVisitedSector(TargetSector))
	{
		// Add stations
		for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorStations().Num(); SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* StationCandidate = Sector->GetSectorStations()[SpacecraftIndex];

			if (StationCandidate && StationCandidate->GetDamageSystem()->IsAlive())
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
				if (ShipCandidate->IsReserve())
				{
					if (ShipCandidate->GetCompany() == PC->GetCompany())
					{
						OwnedReserveShipList->AddShip(ShipCandidate);
					}
					else
					{
						OtherReserveShipList->AddShip(ShipCandidate);
					}
				}
				else
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
	}

	// Unknown sector
	else
	{
	}

	// List setup
	OwnedShipList->RefreshList();
	OtherShipList->RefreshList();
	OwnedReserveShipList->RefreshList();
	OtherReserveShipList->RefreshList();
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
	OwnedReserveShipList->Reset();
	OtherReserveShipList->Reset();
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
			int64 TravelDuration = UFlareTravel::ComputeTravelDuration(MenuManager->GetGame()->GetGameWorld(), SelectedFleet->GetCurrentSector(), TargetSector);

			FText TravelWord;

			if(SelectedFleet->GetCurrentSector()->GetSectorBattleState(SelectedFleet->GetFleetCompany()).HasDanger)
			{
				TravelWord = LOCTEXT("EscapeWord", "Escape");
			}
			else
			{
				TravelWord = LOCTEXT("TravelWord", "Travel");
			}

			if(TravelDuration == 1)
			{
				return FText::Format(LOCTEXT("ShortTravelFormat", "{0} (1 day)"), TravelWord);
			}
			else
			{
				return FText::Format(LOCTEXT("TravelFormat", "{0} ({1} days)"), TravelWord, FText::AsNumber(TravelDuration));
			}
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

	if (SelectedFleet == NULL)
	{
		return true;
	}
	else if (!SelectedFleet->CanTravel() || SelectedFleet->GetCurrentSector() == TargetSector)
	{
		return true;
	}
	else
	{
		return false;
	}
}

FText SFlareSectorMenu::GetRefillText() const
{
	if (!TargetSector)
	{
		return FText();
	}

	int32 AvailableFS;
	int32 OwnedFS;
	int32 AffordableFS;
	int32 NeededFS;
	int32 TotalNeededFS;

	SectorHelper::GetRefillFleetSupplyNeeds(TargetSector, MenuManager->GetPC()->GetCompany(), NeededFS, TotalNeededFS);
	SectorHelper::GetAvailableFleetSupplyCount(TargetSector, MenuManager->GetPC()->GetCompany(), OwnedFS, AvailableFS, AffordableFS);

	if (IsRefillDisabled())
	{
		if (NeededFS > 0)
		{
			// Refill needed
			if(TargetSector->IsInDangerousBattle(MenuManager->GetPC()->GetCompany()))
			{
				return LOCTEXT("CantRefillBattle", "Can't refill here : battle in progress!");
			}
			else if (AvailableFS == 0) {
				return LOCTEXT("CantRefillNoFS", "Can't refill here : no fleet supply available !");
			}
			else
			{
				return LOCTEXT("CantRefillNoMoney", "Can't refill here : not enough money !");
			}
		}
		else if (TotalNeededFS > 0)
		{
			// Refill in progress
			return LOCTEXT("RefillInProgress", "Refill in progress...");
		}
		else
		{
			// No refill needed
			return LOCTEXT("NoFleetToRefill", "No fleet here needs refilling");
		}
	}
	else
	{
		int32 UsedFs = FMath::Min (AffordableFS, NeededFS);
		int32 UsedOwnedFs  = FMath::Min (OwnedFS, UsedFs);
		int32 UsedNotOwnedFs  = UsedFs - UsedOwnedFs;
		FFlareResourceDescription* FleetSupply = TargetSector->GetGame()->GetScenarioTools()->FleetSupply;

		int64 UsedNotOwnedFsCost = UsedNotOwnedFs * TargetSector->GetResourcePrice(FleetSupply, EFlareResourcePriceContext::MaintenanceConsumption);


		FText OwnedCostText;
		FText CostSeparatorText;
		FText NotOwnedCostText;

		if (UsedOwnedFs > 0)
		{
			OwnedCostText = FText::Format(LOCTEXT("RefillOwnedCostFormat", "{0} fleet supplies"), FText::AsNumber(UsedOwnedFs));
		}

		if (UsedNotOwnedFsCost > 0)
		{
			NotOwnedCostText = FText::Format(LOCTEXT("RefillNotOwnedCostFormat", "{0} credits"), FText::AsNumber(UFlareGameTools::DisplayMoney(UsedNotOwnedFsCost)));
		}

		if (UsedOwnedFs > 0 && UsedNotOwnedFsCost > 0)
		{
			CostSeparatorText = LOCTEXT("CostSeparatorText", " + ");
		}

		FText CostText = FText::Format(LOCTEXT("RefillCostFormat", "{0}{1}{2}"), OwnedCostText, CostSeparatorText, NotOwnedCostText);

		return FText::Format(LOCTEXT("RefillOkayFormat", "Refill all fleets ({0})"),
			CostText);
	}
}

FText SFlareSectorMenu::GetRepairText() const
{
	if (!TargetSector)
	{
		return FText();
	}

	int32 AvailableFS;
	int32 OwnedFS;
	int32 AffordableFS;
	int32 NeededFS;
	int32 TotalNeededFS;

	SectorHelper::GetRepairFleetSupplyNeeds(TargetSector, MenuManager->GetPC()->GetCompany(), NeededFS, TotalNeededFS);
	SectorHelper::GetAvailableFleetSupplyCount(TargetSector, MenuManager->GetPC()->GetCompany(), OwnedFS, AvailableFS, AffordableFS);

	if (IsRepairDisabled())
	{
		if (NeededFS > 0)
		{
			// Repair needed
			if(TargetSector->IsInDangerousBattle(MenuManager->GetPC()->GetCompany()))
			{
				return LOCTEXT("CantRepairBattle", "Can't repair here : battle in progress!");
			}
			else if (AvailableFS == 0) {
				return LOCTEXT("CantRepairNoFS", "Can't repair here : no fleet supply available !");
			}
			else
			{
				return LOCTEXT("CantRepairNoMoney", "Can't repair here : not enough money !");
			}
		}
		else if (TotalNeededFS > 0)
		{
			// Repair in progress
			return LOCTEXT("RepairInProgress", "Repair in progress...");
		}
		else
		{
			// No repair needed
			return LOCTEXT("NoFleetToRepair", "No fleet here needs repairing");
		}
	}
	else
	{
		int32 UsedFs = FMath::Min (AffordableFS, NeededFS);
		int32 UsedOwnedFs  = FMath::Min (OwnedFS, UsedFs);
		int32 UsedNotOwnedFs  = UsedFs - UsedOwnedFs;
		FFlareResourceDescription* FleetSupply = TargetSector->GetGame()->GetScenarioTools()->FleetSupply;

		int64 UsedNotOwnedFsCost = UsedNotOwnedFs * TargetSector->GetResourcePrice(FleetSupply, EFlareResourcePriceContext::MaintenanceConsumption);


		FText OwnedCostText;
		FText CostSeparatorText;
		FText NotOwnedCostText;

		if (UsedOwnedFs > 0)
		{
			OwnedCostText = FText::Format(LOCTEXT("RepairOwnedCostFormat", "{0} fleet supplies"), FText::AsNumber(UsedOwnedFs));
		}

		if (UsedNotOwnedFsCost > 0)
		{
			NotOwnedCostText = FText::Format(LOCTEXT("RepairNotOwnedCostFormat", "{0} credits"), FText::AsNumber(UFlareGameTools::DisplayMoney(UsedNotOwnedFsCost)));
		}

		if (UsedOwnedFs > 0 && UsedNotOwnedFsCost > 0)
		{
			CostSeparatorText = LOCTEXT("CostSeparatorText", " + ");
		}

		FText CostText = FText::Format(LOCTEXT("RepairCostFormat", "{0}{1}{2}"), OwnedCostText, CostSeparatorText, NotOwnedCostText);

		return FText::Format(LOCTEXT("RepairOkayFormat", "Repair all fleets ({0})"),
			CostText);
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

bool SFlareSectorMenu::IsRefillDisabled() const
{
	if (!TargetSector || TargetSector->IsInDangerousBattle(MenuManager->GetPC()->GetCompany()))
	{
		return true;
	}

	int32 NeededFS;
	int32 TotalNeededFS;

	SectorHelper::GetRefillFleetSupplyNeeds(TargetSector, MenuManager->GetPC()->GetCompany(), NeededFS, TotalNeededFS);
	if (NeededFS > 0)
	{
		// Refill needed

		int32 AvailableFS;
		int32 OwnedFS;
		int32 AffordableFS;

		SectorHelper::GetAvailableFleetSupplyCount(TargetSector, MenuManager->GetPC()->GetCompany(), OwnedFS, AvailableFS, AffordableFS);

		if (AffordableFS == 0) {
			return true;
		}

		// There is somme affordable FS, can refill
		return false;
	}
	else
	{
		// No refill needed
		return true;
	}
}

bool SFlareSectorMenu::IsRepairDisabled() const
{
	if (!TargetSector || TargetSector->IsInDangerousBattle(MenuManager->GetPC()->GetCompany()))
	{
		return true;
	}

	int32 NeededFS;
	int32 TotalNeededFS;

	SectorHelper::GetRepairFleetSupplyNeeds(TargetSector, MenuManager->GetPC()->GetCompany(), NeededFS, TotalNeededFS);
	if (NeededFS > 0)
	{
		// Repair needed

		int32 AvailableFS;
		int32 OwnedFS;
		int32 AffordableFS;

		SectorHelper::GetAvailableFleetSupplyCount(TargetSector, MenuManager->GetPC()->GetCompany(), OwnedFS, AvailableFS, AffordableFS);

		if (AffordableFS == 0) {
			return true;
		}

		// There is somme affordable FS, can repair
		return false;
	}
	else
	{
		// No repair needed
		return true;
	}
}

EVisibility SFlareSectorMenu::GetOwnedReserveVisibility() const
{
	return (IsEnabled() && OwnedReserveShipList->GetItemCount() > 0) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SFlareSectorMenu::GetOtherReserveVisibility() const
{
	return (IsEnabled() && OtherReserveShipList->GetItemCount() > 0) ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SFlareSectorMenu::GetSectorName() const
{
	FText Result;
	if (IsEnabled() && TargetSector)
	{
		if (MenuManager->GetPC()->GetPlayerFleet() && MenuManager->GetPC()->GetPlayerFleet()->IsTraveling() && TargetSector == MenuManager->GetPC()->GetPlayerFleet()->GetCurrentSector())
		{
			Result = MenuManager->GetPC()->GetPlayerFleet()->GetStatusInfo();
		}
		else
		{
			Result = FText::Format(LOCTEXT("CurrentSectorFormat", "Sector : {0} ({1})"),
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
				AttributeString += LOCTEXT("Dusty", "Dusty").ToString();
			}
			else
			{
				int32 LightRatio = 100 * TargetSector->GetGame()->GetGameWorld()->GetPlanerarium()->GetLightRatio(Body, TargetSector->GetOrbitParameters()->Altitude);
				LightRatioString = FText::Format(LOCTEXT("SectorLightRatioFormat", "{0}%"), FText::AsNumber(LightRatio));
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

EVisibility SFlareSectorMenu::GetCombatValueVisibility() const
{
	return (MenuManager->GetPC()->GetCompany()->HasVisitedSector(TargetSector) ? EVisibility::Visible : EVisibility::Collapsed);
}

FText SFlareSectorMenu::GetOwnCombatValue() const
{
	FText Result;

	if (IsEnabled() && TargetSector)
	{
		UFlareCompany* PlayerCompany = MenuManager->GetPC()->GetCompany();
		int32 CompanyPoints = PlayerCompany->GetCompanyValue(TargetSector, false).ArmyCombatPoints;
		return FText::AsNumber(CompanyPoints);
	}

	return Result;
}

FText SFlareSectorMenu::GetFullCombatValue() const
{
	FText Result;

	if (IsEnabled() && TargetSector)
	{
		int32 AllPoints = SectorHelper::GetArmyCombatPoints(TargetSector);
		return FText::AsNumber(AllPoints);
	}

	return Result;
}

FText SFlareSectorMenu::GetHostileCombatValue() const
{
	FText Result;

	if (IsEnabled() && TargetSector)
	{
		UFlareCompany* PlayerCompany = MenuManager->GetPC()->GetCompany();
		int32 HostilePoints = SectorHelper::GetHostileArmyCombatPoints(TargetSector, PlayerCompany);
		return FText::AsNumber(HostilePoints);
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
		bool Escape = SelectedFleet->GetCurrentSector()->GetSectorBattleState(SelectedFleet->GetFleetCompany()).HasDanger
				&& (SelectedFleet != MenuManager->GetPC()->GetPlayerFleet()  || SelectedFleet->GetShipCount() > 1);
		bool Abandon = SelectedFleet->GetImmobilizedShipCount() != 0;

		if (!Abandon && !Escape)
		{
			OnStartTravelConfirmed(SelectedFleet);
		}

		else
		{
			FText TitleText;
			FText ConfirmText;

			FText SingleShip = LOCTEXT("ShipIsSingle", "ship is");
			FText MultipleShips = LOCTEXT("ShipArePlural", "ships are");
			FText TooDamagedTravelText = LOCTEXT("TooDamagedToTravel", "too damaged to travel and will be left behind");

			// We can escape
			if (Escape)
			{
				TitleText = LOCTEXT("ConfirmTravelEscapeTitle", "ESCAPE ?");
				FText EscapeWarningText = LOCTEXT("ConfirmTravelEscapeWarningText", "Ships can be intercepted while escaping, are you sure ?");

				if (Abandon)
				{
					ConfirmText = FText::Format(LOCTEXT("ConfirmTravelEscapeFormat", "{0} {1} {2} {3}."),
						EscapeWarningText,
						FText::AsNumber(SelectedFleet->GetImmobilizedShipCount()),
						(SelectedFleet->GetImmobilizedShipCount() > 1) ? MultipleShips : SingleShip,
						TooDamagedTravelText);
				}
				else
				{
					ConfirmText = EscapeWarningText;
				}
			}

			// We have to abandon
			else
			{
				TitleText = LOCTEXT("ConfirmTravelAbandonTitle", "ABANDON SHIPS ?");

				ConfirmText = FText::Format(LOCTEXT("ConfirmTravelAbandonFormat", "{0} {1} {2}."),
					FText::AsNumber(SelectedFleet->GetImmobilizedShipCount()),
					(SelectedFleet->GetImmobilizedShipCount() > 1) ? MultipleShips : SingleShip,
					TooDamagedTravelText);
			}

			// Open the confirmation
			MenuManager->Confirm(TitleText,
								 ConfirmText,
								 FSimpleDelegate::CreateSP(this, &SFlareSectorMenu::OnStartTravelConfirmed, SelectedFleet));
		}
	}
}

void SFlareSectorMenu::OnRefillClicked()
{
	SectorHelper::RefillFleets(TargetSector, MenuManager->GetPC()->GetCompany());
}

void SFlareSectorMenu::OnRepairClicked()
{
	SectorHelper::RepairFleets(TargetSector, MenuManager->GetPC()->GetCompany());
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

