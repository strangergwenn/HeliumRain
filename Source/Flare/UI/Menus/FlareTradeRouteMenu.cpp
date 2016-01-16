
#include "../../Flare.h"
#include "FlareTradeRouteMenu.h"

#include "../Components/FlareSectorButton.h"

#include "../../Game/FlareGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"

#define LOCTEXT_NAMESPACE "FlareTradeRouteMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareTradeRouteMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	AFlarePlayerController* PC = MenuManager->GetPC();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	TArray<UFlareResourceCatalogEntry*> ResourceList = PC->GetGame()->GetResourceCatalog()->Resources;

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
				SNew(SImage).Image(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Trade))
			]

			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TitleFont)
				.Text(LOCTEXT("TradeRouteInfo", "TRADE ROUTE"))
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
				.OnClicked(this, &SFlareTradeRouteMenu::OnBackClicked)
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

			// Title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.TitlePadding)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("TradeRouteControls", "Trade route controls"))
				.TextStyle(&Theme.SubTitleFont)
			]

			// Content block
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Left)
			[
				SNew(SScrollBox)
				.Style(&Theme.ScrollBoxStyle)
				.ScrollBarStyle(&Theme.ScrollBarStyle)

				+ SScrollBox::Slot()
				[
					SNew(SVerticalBox)

					// Buttons
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Left)
					[
						SNew(SBox)
						.WidthOverride(Theme.ContentWidth)
						.HAlign(HAlign_Fill)
						[
							SNew(SVerticalBox)

							// Trade route name
							+ SVerticalBox::Slot()
							.Padding(Theme.SmallContentPadding)
							.AutoHeight()
							[
								SNew(SHorizontalBox)

								// Name field
								+ SHorizontalBox::Slot()
								[
									SAssignNew(EditRouteName, SEditableText)
									.Style(&Theme.TextInputStyle)
								]

								// Confirm
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.HAlign(HAlign_Right)
								[
									SNew(SFlareButton)
									.Width(4)
									.Text(LOCTEXT("Rename", "Rename"))
									.HelpText(LOCTEXT("ChangeNameInfo", "Rename this trade route"))
									.Icon(FFlareStyleSet::GetIcon("OK"))
									.OnClicked(this, &SFlareTradeRouteMenu::OnConfirmChangeRouteNameClicked)
								]
							]

							// Fleet list
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.SmallContentPadding)
							[
								SNew(SHorizontalBox)

								// List
								+ SHorizontalBox::Slot()
								[
									SAssignNew(FleetSelector, SComboBox<UFlareFleet*>)
									.OptionsSource(&FleetList)
									.OnGenerateWidget(this, &SFlareTradeRouteMenu::OnGenerateFleetComboLine)
									.OnSelectionChanged(this, &SFlareTradeRouteMenu::OnFleetComboLineSelectionChanged)
									.ComboBoxStyle(&Theme.ComboBoxStyle)
									.ForegroundColor(FLinearColor::White)
									.Visibility(this, &SFlareTradeRouteMenu::GetAssignFleetVisibility)
									[
										SNew(STextBlock)
										.Text(this, &SFlareTradeRouteMenu::OnGetCurrentFleetComboLine)
										.TextStyle(&Theme.TextFont)
									]
								]

								// Name field
								+ SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SFlareButton)
									.Width(4)
									.Text(LOCTEXT("AssignFleet", "Assign"))
									.HelpText(LOCTEXT("AssignFleetInfo", "Assign this fleet to the trade route"))
									.OnClicked(this, &SFlareTradeRouteMenu::OnAssignFleetClicked)
									.Visibility(this, &SFlareTradeRouteMenu::GetAssignFleetVisibility)
								]

								// Trade route fleet list
								+ SHorizontalBox::Slot()
								.AutoWidth()
								[
									SAssignNew(TradeFleetList, SVerticalBox)
								]
							]

							// Sector selection
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.SmallContentPadding)
							[
								SNew(SHorizontalBox)

								// Button
								+ SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SFlareButton)
									.Width(4)
									.Text(LOCTEXT("AddSector", "Add sector"))
									.HelpText(LOCTEXT("AddSectorInfo", "Add this sector to the trade route"))
									.OnClicked(this, &SFlareTradeRouteMenu::OnAddSectorClicked)
									.Visibility(this, &SFlareTradeRouteMenu::GetAddSectorVisibility)
								]

								// List
								+ SHorizontalBox::Slot()
								[
									SAssignNew(SectorSelector, SComboBox<UFlareSimulatedSector*>)
									.OptionsSource(&SectorList)
									.OnGenerateWidget(this, &SFlareTradeRouteMenu::OnGenerateSectorComboLine)
									.OnSelectionChanged(this, &SFlareTradeRouteMenu::OnSectorComboLineSelectionChanged)
									.ComboBoxStyle(&Theme.ComboBoxStyle)
									.ForegroundColor(FLinearColor::White)
									.Visibility(this, &SFlareTradeRouteMenu::GetAddSectorVisibility)
									[
										SNew(STextBlock)
										.Text(this, &SFlareTradeRouteMenu::OnGetCurrentSectorComboLine)
										.TextStyle(&Theme.TextFont)
									]
								]
							]

							// Title
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.TitlePadding)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("ResourceSelection", "Current resource"))
								.TextStyle(&Theme.SubTitleFont)
							]

							// Resource selection
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.SmallContentPadding)
							[
								SAssignNew(ResourceSelector, SComboBox<UFlareResourceCatalogEntry*>)
								.OptionsSource(&PC->GetGame()->GetResourceCatalog()->Resources)
								.OnGenerateWidget(this, &SFlareTradeRouteMenu::OnGenerateResourceComboLine)
								.OnSelectionChanged(this, &SFlareTradeRouteMenu::OnResourceComboLineSelectionChanged)
								.ComboBoxStyle(&Theme.ComboBoxStyle)
								.ForegroundColor(FLinearColor::White)
								.Visibility(this, &SFlareTradeRouteMenu::GetResourceSelectorVisibility)
								[
									SNew(STextBlock)
									.Text(this, &SFlareTradeRouteMenu::OnGetCurrentResourceComboLine)
									.TextStyle(&Theme.TextFont)
								]
							]
						]
					]

					// Header
					+ SVerticalBox::Slot()
					.AutoHeight()
						.Padding(Theme.TitlePadding)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("TradeRouteDetails", "Trade route details"))
						.TextStyle(&Theme.SubTitleFont)
					]

					// Trade route sector list
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(Theme.ContentPadding)
					[
						SAssignNew(TradeSectorList, SHorizontalBox)
					]
				]
			]
		]
	];

	if (ResourceList.Num() > 0)
	{
		ResourceSelector->SetSelectedItem(ResourceList[0]);
	}
}


/*----------------------------------------------------
Interaction
----------------------------------------------------*/

void SFlareTradeRouteMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}

void SFlareTradeRouteMenu::Enter(UFlareTradeRoute* TradeRoute)
{
	FLOG("SFlareTradeMenu::Enter");

	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	TargetTradeRoute = TradeRoute;
	EditRouteName->SetText(GetTradeRouteName());
	GenerateSectorList();
	GenerateFleetList();
}

void SFlareTradeRouteMenu::Exit()
{
	SetEnabled(false);
	TargetTradeRoute = NULL;
	TradeSectorList->ClearChildren();
	TradeFleetList->ClearChildren();

	SetVisibility(EVisibility::Hidden);
}

void SFlareTradeRouteMenu::GenerateSectorList()
{
	SectorList.Empty();
	TradeSectorList->ClearChildren();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	if (TargetTradeRoute)
	{
		// Fetch all known sectors
		TArray<UFlareSimulatedSector*>& VisitedSectors = MenuManager->GetGame()->GetPC()->GetCompany()->GetVisitedSectors();
		for (int SectorIndex = 0; SectorIndex < VisitedSectors.Num(); SectorIndex++)
		{
			if (!TargetTradeRoute->IsVisiting(VisitedSectors[SectorIndex]))
			{
				SectorList.Add(VisitedSectors[SectorIndex]);
			}
		}

		// Iterate on the trade route
		TArray<FFlareTradeRouteSectorSave>& Sectors = TargetTradeRoute->GetSectors();
		for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
		{
			TSharedPtr<SVerticalBox> LoadResourceList;
			TSharedPtr<SVerticalBox> UnloadResourceList;
			FFlareTradeRouteSectorSave* SectorOrders = &Sectors[SectorIndex];
			UFlareSimulatedSector* Sector = MenuManager->GetGame()->GetGameWorld()->FindSector(SectorOrders->SectorIdentifier);

			TradeSectorList->AddSlot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			[
				SNew(SVerticalBox)

				// Sector info
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					// Arrow 1
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Top)
					.AutoWidth()
					[
						SNew(SImage)
						.Image(FFlareStyleSet::GetImage(SectorIndex > 0 ? "TradeRouteArrow2" : "TradeRouteArrow_Start"))
					]

					// Sector info
					+ SHorizontalBox::Slot()
					[
						SNew(SFlareSectorButton)
						.Sector(Sector)
						.PlayerCompany(MenuManager->GetGame()->GetPC()->GetCompany())
					]

					// Arrow 2
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Top)
					.AutoWidth()
					[
						SNew(SImage)
						.Image(FFlareStyleSet::GetImage(SectorIndex < Sectors.Num() - 1 ? "TradeRouteArrow" : "TradeRouteArrow_End"))
					]
				]

				// Load / unload buttons
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						[
							SNew(SFlareButton)
							.OnClicked(this, &SFlareTradeRouteMenu::OnLoadResourceClicked, Sector)
							.Text(this, &SFlareTradeRouteMenu::GetLoadText)
							.HelpText(LOCTEXT("LoadHelp", "Load the selected resource in this sector"))
							.Width(4)
						]

						+ SHorizontalBox::Slot()
						[
							SNew(SFlareButton)
							.OnClicked(this, &SFlareTradeRouteMenu::OnUnloadResourceClicked, Sector)
							.Text(this, &SFlareTradeRouteMenu::GetUnloadText)
							.HelpText(LOCTEXT("UnloadHelp", "Unload the selected resource in this sector"))
							.Width(4)
						]
					]

					// Resources to load
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					[
						SAssignNew(LoadResourceList, SVerticalBox)
					]

					// Resources to unload
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					[
						SAssignNew(UnloadResourceList, SVerticalBox)
					]
				]
			];

			// Fill resource lists
			for (int ResourceIndex = 0; ResourceIndex < SectorOrders->ResourcesToLoad.Num(); ResourceIndex++)
			{
				FFlareCargoSave* LoadedResource = &SectorOrders->ResourcesToLoad[ResourceIndex];
				FFlareResourceDescription* Resource = MenuManager->GetGame()->GetResourceCatalog()->Get(LoadedResource->ResourceIdentifier);

				FText LoadLimits;
				if (LoadedResource->Quantity > 0)
				{
					LoadLimits = FText::Format(LOCTEXT("LoadLimits", "Load, leave {0}"), FText::AsNumber(LoadedResource->Quantity));
				}
				else
				{
					LoadLimits = FText(LOCTEXT("NoLoadLimits", "Load all"));
				}

				// Add resource limits
				LoadResourceList->AddSlot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				[
					SNew(SHorizontalBox)

					// Resource name
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.ContentPadding)
					[
						SNew(SBox)
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(Resource->Acronym)
						]
					]

					// Limits
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.ContentPadding)
					[
						SNew(SBox)
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(LoadLimits)
						]
					]

					// Limit decrease
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.OnClicked(this, &SFlareTradeRouteMenu::OnDecreaseLoadLimitClicked, Sector, Resource)
						.Text(FText::FromString(TEXT("-")))
						.Transparent(true)
						.Width(1)
					]

					// Limit increase
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.OnClicked(this, &SFlareTradeRouteMenu::OnIncreaseLoadLimitClicked, Sector, Resource)
						.Text(FText::FromString(TEXT("+")))
						.Transparent(true)
						.Width(1)
					]

					// Clear resource load
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.OnClicked(this, &SFlareTradeRouteMenu::OnClearLoadResourceClicked, Sector, Resource)
						.Text(FText())
						.HelpText(LOCTEXT("ClearLoadResource", "Stop loading this resource"))
						.Icon(FFlareStyleSet::GetIcon("Stop"))
						.Transparent(true)
						.Width(1)
					]
				];
			}

			for (int ResourceIndex = 0; ResourceIndex < SectorOrders->ResourcesToUnload.Num(); ResourceIndex++)
			{
				FFlareCargoSave* UnloadedResource = &SectorOrders->ResourcesToUnload[ResourceIndex];
				FFlareResourceDescription* Resource = MenuManager->GetGame()->GetResourceCatalog()->Get(UnloadedResource->ResourceIdentifier);

				FText UnloadLimits;
				if (UnloadedResource->Quantity > 0)
				{
					UnloadLimits = FText::Format(LOCTEXT("UnloadLimits", "Unload, stop at {0}"), FText::AsNumber(UnloadedResource->Quantity));
				}
				else
				{
					UnloadLimits = FText(LOCTEXT("UnloadLimits", "Unload all"));
				}

				UnloadResourceList->AddSlot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				[
					SNew(SHorizontalBox)

					// Resource name
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.ContentPadding)
					[
						SNew(SBox)
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(Resource->Acronym)
						]
					]

					// Limits
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.ContentPadding)
					[
						SNew(SBox)
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(UnloadLimits)
						]
					]

					// Limit decrease
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.OnClicked(this, &SFlareTradeRouteMenu::OnDecreaseUnloadLimitClicked, Sector, Resource)
						.Text(FText::FromString(TEXT("-")))
						.Transparent(true)
						.Width(1)
					]

					// Limit increase
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.OnClicked(this, &SFlareTradeRouteMenu::OnIncreaseUnloadLimitClicked, Sector, Resource)
						.Text(FText::FromString(TEXT("+")))
						.Transparent(true)
						.Width(1)
					]

					// Clear resource load
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.OnClicked(this, &SFlareTradeRouteMenu::OnClearUnloadResourceClicked, Sector, Resource)
						.Text(FText())
						.HelpText(LOCTEXT("ClearUnloadResource", "Stop unloading this resource"))
						.Icon(FFlareStyleSet::GetIcon("Stop"))
						.Transparent(true)
						.Width(1)
					]
				];
			}
		}
	}

	if (SectorList.Num() > 0)
	{
		SectorSelector->SetSelectedItem(SectorList[0]);
	}
}

void SFlareTradeRouteMenu::GenerateFleetList()
{
	FleetList.Empty();
	TradeFleetList->ClearChildren();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	if (TargetTradeRoute)
	{
		// Available fleets (to combo box)
		TArray<UFlareFleet*>& Fleets = MenuManager->GetGame()->GetPC()->GetCompany()->GetCompanyFleets();
		for (int FleetIndex = 0; FleetIndex < Fleets.Num(); FleetIndex++)
		{
			if (!Fleets[FleetIndex]->GetCurrentTradeRoute())
			{
				FleetList.Add(Fleets[FleetIndex]);
			}
		}

		// Assigned fleets (text display)
		TArray<UFlareFleet*>& TradeFleets = TargetTradeRoute->GetFleets();
		for (int FleetIndex = 0; FleetIndex < TradeFleets.Num(); FleetIndex++)
		{
			UFlareFleet* Fleet = TradeFleets[FleetIndex];

			TradeFleetList->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			[
				SNew(SHorizontalBox)

				// Fleet info
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)
				[
					SNew(SBox)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Center)
					.WidthOverride(Theme.ContentWidth - 4 * Theme.ButtonWidth - 2 * Theme.SmallContentPadding.Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(FText::Format(LOCTEXT("FleetNameFormat", "Assigned fleet : {0}"), Fleet->GetFleetName()))
					]
				]

				// Unassign fleet
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)
				[
					SNew(SFlareButton)
					.OnClicked(this, &SFlareTradeRouteMenu::OnUnassignFleetClicked, Fleet)
					.Text(FText(LOCTEXT("Unassign", "Unassign")))
					.HelpText(FText(LOCTEXT("UnassignHelp", "Unassign this fleet and pick another one")))
					.Icon(FFlareStyleSet::GetIcon("Stop"))
					.Width(4)
				]
			];
		}

		// No fleets
		if (FleetList.Num() == 0 && TradeFleets.Num() == 0)
		{
			TradeFleetList->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TextFont)
				.Text(LOCTEXT("NoFleet", "No fleet assigned or available !"))
			];
		}
	}

	// Pre-select the first fleet
	if (FleetList.Num() > 0)
	{
		FleetSelector->SetSelectedItem(FleetList[0]);
	}
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareTradeRouteMenu::OnBackClicked()
{
	MenuManager->Back();
}

void SFlareTradeRouteMenu::OnConfirmChangeRouteNameClicked()
{
	if (TargetTradeRoute)
	{
		TargetTradeRoute->SetTradeRouteName(EditRouteName->GetText());
	}
}

FText SFlareTradeRouteMenu::GetTradeRouteName() const
{
	FText Result;

	if (TargetTradeRoute)
	{
		Result = TargetTradeRoute->GetTradeRouteName();
	}

	return Result;
}

TSharedRef<SWidget> SFlareTradeRouteMenu::OnGenerateSectorComboLine(UFlareSimulatedSector* Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(STextBlock)
		.Text(Item->GetSectorName())
		.TextStyle(&Theme.TextFont);
}

void SFlareTradeRouteMenu::OnSectorComboLineSelectionChanged(UFlareSimulatedSector* Item, ESelectInfo::Type SelectInfo)
{
}

FText SFlareTradeRouteMenu::OnGetCurrentSectorComboLine() const
{
	UFlareSimulatedSector* Item = SectorSelector->GetSelectedItem();
	return Item ? Item->GetSectorName() : LOCTEXT("SelectSector", "Select a sector");
}

void SFlareTradeRouteMenu::OnAddSectorClicked()
{
	UFlareSimulatedSector* Item = SectorSelector->GetSelectedItem();
	if (Item)
	{
		TargetTradeRoute->AddSector(Item);
		GenerateSectorList();
	}
}

EVisibility SFlareTradeRouteMenu::GetAddSectorVisibility() const
{
	return SectorList.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed;
}

TSharedRef<SWidget> SFlareTradeRouteMenu::OnGenerateResourceComboLine(UFlareResourceCatalogEntry* Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(STextBlock)
		.Text(Item->Data.Name)
		.TextStyle(&Theme.TextFont);
}

void SFlareTradeRouteMenu::OnResourceComboLineSelectionChanged(UFlareResourceCatalogEntry* Item, ESelectInfo::Type SelectInfo)
{

}

FText SFlareTradeRouteMenu::OnGetCurrentResourceComboLine() const
{
	UFlareResourceCatalogEntry* Item = ResourceSelector->GetSelectedItem();
	if (Item)
	{
		return FText::Format(LOCTEXT("ComboResourceLineFormat", "{0} ({1})"), Item->Data.Name, Item->Data.Acronym);
	}
	else
	{
		return LOCTEXT("SelectResource", "Select a resource");
	}
}

EVisibility SFlareTradeRouteMenu::GetResourceSelectorVisibility() const
{
	EVisibility visibility = EVisibility::Collapsed;
	if (TargetTradeRoute)
	{
		if (TargetTradeRoute->GetSectors().Num() > 0)
		{
			visibility = EVisibility::Visible;
		}
	}
	return visibility;
}

FText SFlareTradeRouteMenu::GetLoadText() const
{
	FText Result;
	UFlareResourceCatalogEntry* Resource = ResourceSelector->GetSelectedItem();

	if (TargetTradeRoute && Resource)
	{
		Result = FText::Format(LOCTEXT("LoadFormat", "Load {0}"), Resource->Data.Acronym);
	}
	else
	{
		Result = LOCTEXT("LoadImpossible", "Load");
	}

	return Result;
}

FText SFlareTradeRouteMenu::GetUnloadText() const
{
	FText Result;
	UFlareResourceCatalogEntry* Resource = ResourceSelector->GetSelectedItem();

	if (TargetTradeRoute && Resource)
	{
		Result = FText::Format(LOCTEXT("UnloadFormat", "Unload {0}"), Resource->Data.Acronym);
	}
	else
	{
		Result = LOCTEXT("UnloadImpossible", "Select");
	}

	return Result;
}

void SFlareTradeRouteMenu::OnLoadResourceClicked(UFlareSimulatedSector* Sector)
{
	UFlareResourceCatalogEntry* Resource = ResourceSelector->GetSelectedItem();
	int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
	if (SectorIndex >= 0 && Resource)
	{
		TargetTradeRoute->SetSectorLoadOrder(SectorIndex, &Resource->Data, 0);
		GenerateSectorList();
	}
}

void SFlareTradeRouteMenu::OnDecreaseLoadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource)
{
	int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
	FFlareTradeRouteSectorSave* PreviousOrder = TargetTradeRoute->GetSectorOrders(Sector);

	if (SectorIndex >= 0 && Resource && PreviousOrder)
	{
		uint32 PreviousLimit = 0;
		for (int ResourceIndex = 0; ResourceIndex < PreviousOrder->ResourcesToLoad.Num(); ResourceIndex++)
		{
			if (Resource->Identifier == PreviousOrder->ResourcesToLoad[ResourceIndex].ResourceIdentifier)
			{
				PreviousLimit = PreviousOrder->ResourcesToLoad[ResourceIndex].Quantity;
				break;
			}
		}

		TargetTradeRoute->SetSectorLoadOrder(SectorIndex, Resource, PreviousLimit + 1);
		GenerateSectorList();
	}
}

void SFlareTradeRouteMenu::OnIncreaseLoadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource)
{
	int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
	FFlareTradeRouteSectorSave* PreviousOrder = TargetTradeRoute->GetSectorOrders(Sector);

	if (SectorIndex >= 0 && Resource && PreviousOrder)
	{
		uint32 PreviousLimit = 0;
		for (int ResourceIndex = 0; ResourceIndex < PreviousOrder->ResourcesToLoad.Num(); ResourceIndex++)
		{
			if (Resource->Identifier == PreviousOrder->ResourcesToLoad[ResourceIndex].ResourceIdentifier)
			{
				PreviousLimit = PreviousOrder->ResourcesToLoad[ResourceIndex].Quantity;
				break;
			}
		}

		if (PreviousLimit == 0)
		{
			return;
		}

		TargetTradeRoute->SetSectorLoadOrder(SectorIndex, Resource, PreviousLimit - 1);
		GenerateSectorList();
	}
}

void SFlareTradeRouteMenu::OnClearLoadResourceClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource)
{
	int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
	if (SectorIndex >= 0 && Resource)
	{
		TargetTradeRoute->ClearSectorLoadOrder(SectorIndex, Resource);
		GenerateSectorList();
	}
}

void SFlareTradeRouteMenu::OnUnloadResourceClicked(UFlareSimulatedSector* Sector)
{
	UFlareResourceCatalogEntry* Resource = ResourceSelector->GetSelectedItem();
	int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);

	if (SectorIndex >= 0 && Resource)
	{
		TargetTradeRoute->SetSectorUnloadOrder(SectorIndex, &Resource->Data, 0);
		GenerateSectorList();
	}
}


void SFlareTradeRouteMenu::OnDecreaseUnloadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource)
{
	int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
	FFlareTradeRouteSectorSave* PreviousOrder = TargetTradeRoute->GetSectorOrders(Sector);

	if (SectorIndex >= 0 && Resource && PreviousOrder)
	{
		uint32 PreviousLimit = 0;
		for (int ResourceIndex = 0; ResourceIndex < PreviousOrder->ResourcesToUnload.Num(); ResourceIndex++)
		{
			if (Resource->Identifier == PreviousOrder->ResourcesToUnload[ResourceIndex].ResourceIdentifier)
			{
				PreviousLimit = PreviousOrder->ResourcesToUnload[ResourceIndex].Quantity;
				break;
			}
		}

		TargetTradeRoute->SetSectorUnloadOrder(SectorIndex, Resource, PreviousLimit + 1);
		GenerateSectorList();
	}
}

void SFlareTradeRouteMenu::OnIncreaseUnloadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource)
{
	int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
	FFlareTradeRouteSectorSave* PreviousOrder = TargetTradeRoute->GetSectorOrders(Sector);

	if (SectorIndex >= 0 && Resource && PreviousOrder)
	{
		uint32 PreviousLimit = 0;
		for (int ResourceIndex = 0; ResourceIndex < PreviousOrder->ResourcesToUnload.Num(); ResourceIndex++)
		{
			if (Resource->Identifier == PreviousOrder->ResourcesToUnload[ResourceIndex].ResourceIdentifier)
			{
				PreviousLimit = PreviousOrder->ResourcesToUnload[ResourceIndex].Quantity;
				break;
			}
		}

		if (PreviousLimit == 0)
		{
			return;
		}

		TargetTradeRoute->SetSectorUnloadOrder(SectorIndex, Resource, PreviousLimit - 1);
		GenerateSectorList();
	}
}

void SFlareTradeRouteMenu::OnClearUnloadResourceClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource)
{
	int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
	if (SectorIndex >= 0 && Resource)
	{
		TargetTradeRoute->ClearSectorUnloadOrder(SectorIndex, Resource);
		GenerateSectorList();
	}
}

TSharedRef<SWidget> SFlareTradeRouteMenu::OnGenerateFleetComboLine(UFlareFleet* Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(STextBlock)
		.Text(Item->GetFleetName())
		.TextStyle(&Theme.TextFont);
}

void SFlareTradeRouteMenu::OnFleetComboLineSelectionChanged(UFlareFleet* Item, ESelectInfo::Type SelectInfo)
{
}

FText SFlareTradeRouteMenu::OnGetCurrentFleetComboLine() const
{
	UFlareFleet* Item = FleetSelector->GetSelectedItem();
	return Item ? Item->GetFleetName() : LOCTEXT("SelectFleet", "Select a fleet");
}

void SFlareTradeRouteMenu::OnAssignFleetClicked()
{
	UFlareFleet* Item = FleetSelector->GetSelectedItem();
	if (Item)
	{
		TargetTradeRoute->AddFleet(Item);
		GenerateFleetList();
	}
}

EVisibility SFlareTradeRouteMenu::GetAssignFleetVisibility() const
{
	// Only one fleet !
	return FleetList.Num() > 0 && TargetTradeRoute && TargetTradeRoute->GetFleets().Num() == 0 ? EVisibility::Visible : EVisibility::Collapsed;
}


void SFlareTradeRouteMenu::OnUnassignFleetClicked(UFlareFleet* Fleet)
{
	if (TargetTradeRoute)
	{
		TargetTradeRoute->RemoveFleet(Fleet);
		GenerateFleetList();
	}
}

#undef LOCTEXT_NAMESPACE

