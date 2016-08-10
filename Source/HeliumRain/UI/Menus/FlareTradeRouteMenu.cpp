
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
	MaxSectorsInRoute = 4;

	OperationList.Add(EFlareTradeRouteOperation::Load);
	OperationNameList.Add(MakeShareable(new FString(TEXT("Load"))));
	OperationList.Add(EFlareTradeRouteOperation::Buy);
	OperationNameList.Add(MakeShareable(new FString(TEXT("Buy"))));
	OperationList.Add(EFlareTradeRouteOperation::LoadOrBuy);
	OperationNameList.Add(MakeShareable(new FString(TEXT("Load or buy"))));
	OperationList.Add(EFlareTradeRouteOperation::Unload);
	OperationNameList.Add(MakeShareable(new FString(TEXT("Unload"))));
	OperationList.Add(EFlareTradeRouteOperation::Sell);
	OperationNameList.Add(MakeShareable(new FString(TEXT("Sell"))));
	OperationList.Add(EFlareTradeRouteOperation::UnloadOrSell);
	OperationNameList.Add(MakeShareable(new FString(TEXT("Unload or sell"))));

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
			
			// Content block
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Left)
			[
				SNew(SScrollBox)
				.Style(&Theme.ScrollBoxStyle)
				.ScrollBarStyle(&Theme.ScrollBarStyle)

				+ SScrollBox::Slot()
				.HAlign(HAlign_Center)
				[
					SNew(SVerticalBox)

					// Buttons
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Center)
					[
						SNew(SBox)
						.WidthOverride(Theme.ContentWidth)
						.HAlign(HAlign_Fill)
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

							// Trade route name
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(SHorizontalBox)

								// Name field
								+ SHorizontalBox::Slot()
								.Padding(Theme.SmallContentPadding)
								[
									SAssignNew(EditRouteName, SEditableText)
									.Style(&Theme.TextInputStyle)
								]

								// Confirm
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(Theme.SmallContentPadding)
								[
									SNew(SFlareButton)
									.Width(5)
									.Text(LOCTEXT("Rename", "Rename"))
									.HelpText(LOCTEXT("ChangeNameInfo", "Rename this trade route"))
									.Icon(FFlareStyleSet::GetIcon("OK"))
									.OnClicked(this, &SFlareTradeRouteMenu::OnConfirmChangeRouteNameClicked)
								]
							]

							// Fleet list
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(SHorizontalBox)

								// List
								+ SHorizontalBox::Slot()
								.Padding(Theme.SmallContentPadding)
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
								.Padding(Theme.SmallContentPadding)
								.HAlign(HAlign_Right)
								[
									SNew(SFlareButton)
									.Width(5)
									.Text(LOCTEXT("AssignFleet", "Assign"))
									.HelpText(LOCTEXT("AssignFleetInfo", "Assign this fleet to the trade route"))
									.OnClicked(this, &SFlareTradeRouteMenu::OnAssignFleetClicked)
									.Visibility(this, &SFlareTradeRouteMenu::GetAssignFleetVisibility)
								]
							]

							// Fleet list (assigned)
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SAssignNew(TradeFleetList, SVerticalBox)
							]

							// Sector selection
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(SHorizontalBox)

								// List
								+ SHorizontalBox::Slot()
								.Padding(Theme.SmallContentPadding)
								[
									SAssignNew(SectorSelector, SComboBox<UFlareSimulatedSector*>)
									.OptionsSource(&SectorList)
									.OnGenerateWidget(this, &SFlareTradeRouteMenu::OnGenerateSectorComboLine)
									.OnSelectionChanged(this, &SFlareTradeRouteMenu::OnSectorComboLineSelectionChanged)
									.ComboBoxStyle(&Theme.ComboBoxStyle)
									.ForegroundColor(FLinearColor::White)
									[
										SNew(STextBlock)
										.Text(this, &SFlareTradeRouteMenu::OnGetCurrentSectorComboLine)
										.TextStyle(&Theme.TextFont)
									]
								]

								// Button
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(Theme.SmallContentPadding)
								[
									SNew(SFlareButton)
									.Width(5)
									.Text(this, &SFlareTradeRouteMenu::GetAddSectorText)
									.HelpText(LOCTEXT("AddSectorInfo", "Add this sector to the trade route"))
									.OnClicked(this, &SFlareTradeRouteMenu::OnAddSectorClicked)
									.IsDisabled(this, &SFlareTradeRouteMenu::IsAddSectorDisabled)
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


					// Operation edition box
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(Theme.ContentPadding)
					[
						SNew(SVerticalBox)
						.Visibility(this, &SFlareTradeRouteMenu::GetOperationDetailsVisibility)

						// Title operation
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.TitlePadding)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("ResourceSelection", "Change current resource"))
							.TextStyle(&Theme.SubTitleFont)
						]

						// Resource selection
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.SmallContentPadding)
						[
							SAssignNew(OperationSelector, SComboBox<TSharedPtr<FString>>)
							.OptionsSource(&OperationNameList)
							.OnGenerateWidget(this, &SFlareTradeRouteMenu::OnGenerateOperationComboLine)
							.OnSelectionChanged(this, &SFlareTradeRouteMenu::OnOperationComboLineSelectionChanged)
							.ComboBoxStyle(&Theme.ComboBoxStyle)
							.ForegroundColor(FLinearColor::White)
							[
								SNew(STextBlock)
								.Text(this, &SFlareTradeRouteMenu::OnGetCurrentOperationComboLine)
								.TextStyle(&Theme.TextFont)
							]
						]

						// Title resource
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.TitlePadding)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("OperationSelection", "Operation"))
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
							[
								SNew(STextBlock)
								.Text(this, &SFlareTradeRouteMenu::OnGetCurrentResourceComboLine)
								.TextStyle(&Theme.TextFont)
							]
						]




						// Delete Operation
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SFlareButton)
							.OnClicked(this, &SFlareTradeRouteMenu::OnDeleteOperationClicked)
							.Text(LOCTEXT("DeleteOperation", "Delete"))
						]

						// Move Operation up
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SFlareButton)
							.OnClicked(this, &SFlareTradeRouteMenu::OnOperationUpClicked)
							.Text(LOCTEXT("MoveUpOperation", "Up"))
						]

						// Move Operation down
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SFlareButton)
							.OnClicked(this, &SFlareTradeRouteMenu::OnOperationDownClicked)
							.Text(LOCTEXT("MoveDownOperation", "Down"))
						]
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
	SetVisibility(EVisibility::Collapsed);
}

void SFlareTradeRouteMenu::Enter(UFlareTradeRoute* TradeRoute)
{
	FLOG("SFlareTradeMenu::Enter");

	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	TargetTradeRoute = TradeRoute;
	SelectedOperation = NULL;
	EditRouteName->SetText(GetTradeRouteName());
	GenerateSectorList();
	GenerateFleetList();
}

void SFlareTradeRouteMenu::Exit()
{
	SetEnabled(false);
	TargetTradeRoute = NULL;
	SelectedOperation = NULL;
	TradeSectorList->ClearChildren();
	TradeFleetList->ClearChildren();

	SetVisibility(EVisibility::Collapsed);
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
		SectorSelector->RefreshOptions();

		// Iterate on the trade route
		TArray<FFlareTradeRouteSectorSave>& Sectors = TargetTradeRoute->GetSectors();
		for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
		{
			TSharedPtr<SVerticalBox> SectorOperationList;
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

					// Remove sector
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Top)
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Transparent(true)
						.Text(FText())
						.HelpText(LOCTEXT("RemoveSectorHelp", "Remove this sector from the trade route"))
						.Icon(FFlareStyleSet::GetIcon("Stop"))
						.OnClicked(this, &SFlareTradeRouteMenu::OnRemoveSectorClicked, Sector)
						.Width(1)
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

				// Operation list
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SVerticalBox)

					// Operation list
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					[
						SAssignNew(SectorOperationList, SVerticalBox)
					]

					// Add operation button
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					[
						SNew(SFlareButton)
						.OnClicked(this, &SFlareTradeRouteMenu::OnAddOperationClicked, Sector)
						.Text(LOCTEXT("SectorAddOperation", "Add operation"))
						.HelpText(LOCTEXT("UnloadHelp", "Add an operation for this sector"))
					]

				]
			];

			// Fill operation list
			for (int OperationIndex = 0; OperationIndex < SectorOrders->Operations.Num(); OperationIndex++)
			{
				FFlareTradeRouteSectorOperationSave* Operation = &SectorOrders->Operations[OperationIndex];
				FFlareResourceDescription* Resource = MenuManager->GetGame()->GetResourceCatalog()->Get(Operation->ResourceIdentifier);

				int32 OperationNameIndex;
				OperationNameIndex = OperationList.Find(Operation->Type);
				if(OperationNameIndex < 0)
				{
					OperationNameIndex = 0;
				}

				FText OperationName = FText::FromString(*OperationNameList[OperationNameIndex]);


				FText OperationResume =	FText::Format(LOCTEXT("OperationResume", "{0} - {1} {2}"), FText::AsNumber(OperationIndex), OperationName, Resource->Name);


/*

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
*/
				// TODO current operation progress
				// Add operation limits
				SectorOperationList->AddSlot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				[
					SNew(SHorizontalBox)

					// Operation name
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
							.Text(OperationResume)
						]
					]

				];

				SectorOperationList->AddSlot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				[
					SNew(SHorizontalBox)

					// Edit operation
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.OnClicked(this, &SFlareTradeRouteMenu::OnEditOperationClicked, Operation)
						.Text(LOCTEXT("EditOperation", "Edit"))
						.Visibility(this, &SFlareTradeRouteMenu::GetEditOperationVisibility, Operation)
					]

					// Skip operation
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.OnClicked(this, &SFlareTradeRouteMenu::OnEditOperationClicked, Operation)
						.Text(LOCTEXT("EditOperation", "Edit"))
						.Visibility(this, &SFlareTradeRouteMenu::GetSkipOperationVisibility, Operation)
					]
				];
			}

			/*for (int ResourceIndex = 0; ResourceIndex < SectorOrders->ResourcesToUnload.Num(); ResourceIndex++)
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
					UnloadLimits = FText(LOCTEXT("UnloadAll", "Unload all"));
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
			}*/
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
		FleetSelector->RefreshOptions();

		// Assigned fleets (text display)
		if(TargetTradeRoute->GetFleet())
		{
			UFlareFleet* Fleet = TargetTradeRoute->GetFleet();

			TradeFleetList->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			[
				SNew(SHorizontalBox)

				// Fleet info
				+ SHorizontalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(FText::Format(LOCTEXT("FleetNameFormat", "Assigned fleet : {0}"), Fleet->GetFleetName()))
				]

				// Unassign fleet
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Right)
				.Padding(Theme.SmallContentPadding)
				[
					SNew(SFlareButton)
					.Width(5)
					.OnClicked(this, &SFlareTradeRouteMenu::OnUnassignFleetClicked, Fleet)
					.Text(FText(LOCTEXT("Unassign", "Unassign")))
					.HelpText(FText(LOCTEXT("UnassignHelp", "Unassign this fleet and pick another one")))
					.Icon(FFlareStyleSet::GetIcon("Stop"))
				]
			];
		}

		// No fleets
		if (FleetList.Num() == 0 && TargetTradeRoute->GetFleet() == NULL)
		{
			TradeFleetList->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TextFont)
				.Text(LOCTEXT("NoFleet", "No fleet assigned or available, route disabled"))
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
	Content callbacks
----------------------------------------------------*/

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

	UFlareSimulatedPlanetarium* Planetarium = MenuManager->GetGame()->GetGameWorld()->GetPlanerarium();
	FFlareCelestialBody* CelestialBody = Planetarium->FindCelestialBody(Item->GetOrbitParameters()->CelestialBodyIdentifier);

	return SNew(STextBlock)
		.Text(FText::Format(LOCTEXT("SectorLineFormat", "{0} (Orbiting {1})"), Item->GetSectorName(), CelestialBody->Name))
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

bool SFlareTradeRouteMenu::IsAddSectorDisabled() const
{
	if (TargetTradeRoute)
	{
		if (TargetTradeRoute->GetSectors().Num() >= MaxSectorsInRoute)
		{
			return true;
		}
	}

	if (SectorList.Num() == 0)
	{
		return true;
	}

	return false;
}

TSharedRef<SWidget> SFlareTradeRouteMenu::OnGenerateResourceComboLine(UFlareResourceCatalogEntry* Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(STextBlock)
		.Text(Item->Data.Name)
		.TextStyle(&Theme.TextFont);
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

TSharedRef<SWidget> SFlareTradeRouteMenu::OnGenerateOperationComboLine(TSharedPtr<FString> Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(STextBlock)
		.Text(FText::FromString(*Item)) // FString needed here
		.TextStyle(&Theme.TextFont);
}

FText SFlareTradeRouteMenu::OnGetCurrentOperationComboLine() const
{
	TSharedPtr<FString> Item = OperationSelector->GetSelectedItem();
	return Item.IsValid() ? FText::FromString(*Item) : FText::FromString(*OperationNameList[0]);
}

EVisibility SFlareTradeRouteMenu::GetOperationDetailsVisibility() const
{
	EVisibility visibility = EVisibility::Collapsed;
	if (TargetTradeRoute)
	{
		if (SelectedOperation != NULL)
		{
			visibility = EVisibility::Visible;
		}
	}
	return visibility;
}


FText SFlareTradeRouteMenu::GetAddSectorText() const
{
	if (TargetTradeRoute)
	{
		return FText::Format(LOCTEXT("AddSectorFormat", "Add sector ({0} / {1})"),
			FText::AsNumber(TargetTradeRoute->GetSectors().Num()),
			FText::AsNumber(MaxSectorsInRoute));
	}

	return FText();
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

TSharedRef<SWidget> SFlareTradeRouteMenu::OnGenerateFleetComboLine(UFlareFleet* Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(STextBlock)
		.Text(Item->GetFleetName())
		.TextStyle(&Theme.TextFont);
}

FText SFlareTradeRouteMenu::OnGetCurrentFleetComboLine() const
{
	UFlareFleet* Item = FleetSelector->GetSelectedItem();
	return Item ? Item->GetFleetName() : LOCTEXT("SelectFleet", "Select a fleet");
}

EVisibility SFlareTradeRouteMenu::GetAssignFleetVisibility() const
{
	// Only one fleet !
	return FleetList.Num() > 0 && TargetTradeRoute && TargetTradeRoute->GetFleet() == NULL ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SFlareTradeRouteMenu::GetEditOperationVisibility(FFlareTradeRouteSectorOperationSave* Operation) const
{
	return SelectedOperation != Operation ? EVisibility::Visible :EVisibility::Hidden;
}

EVisibility SFlareTradeRouteMenu::GetSkipOperationVisibility(FFlareTradeRouteSectorOperationSave* Operation) const
{
	if(TargetTradeRoute)
	{
		return TargetTradeRoute->GetActiveOperation() == Operation ? EVisibility::Visible : EVisibility::Collapsed;
	}
	return EVisibility::Hidden;
}


/*----------------------------------------------------
	Action callbacks
----------------------------------------------------*/

void SFlareTradeRouteMenu::OnConfirmChangeRouteNameClicked()
{
	if (TargetTradeRoute)
	{
		TargetTradeRoute->SetTradeRouteName(EditRouteName->GetText());
	}
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

void SFlareTradeRouteMenu::OnResourceComboLineSelectionChanged(UFlareResourceCatalogEntry* Item, ESelectInfo::Type SelectInfo)
{
	if(SelectedOperation)
	{
		SelectedOperation->ResourceIdentifier = Item->Data.Identifier;
		GenerateSectorList();
	}
}

void SFlareTradeRouteMenu::OnOperationComboLineSelectionChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo)
{
	if(SelectedOperation)
	{
		int32 OperationIndex  = OperationNameList.Find(Item);


		if(OperationIndex == -1)
		{
			OperationIndex = 0;
		}
		EFlareTradeRouteOperation::Type OperationType = OperationList[OperationIndex];
		SelectedOperation->Type = OperationType;
		GenerateSectorList();
	}
}

void SFlareTradeRouteMenu::OnRemoveSectorClicked(UFlareSimulatedSector* Sector)
{
	if (TargetTradeRoute)
	{
		TargetTradeRoute->RemoveSector(Sector);
		GenerateSectorList();
	}
}

void SFlareTradeRouteMenu::OnAddOperationClicked(UFlareSimulatedSector* Sector)
{
	UFlareResourceCatalogEntry* Resource = ResourceSelector->GetSelectedItem();
	int32 OperationIndex  = OperationNameList.Find(OperationSelector->GetSelectedItem());


	if(OperationIndex == -1)
	{
		OperationIndex = 0;
	}
	EFlareTradeRouteOperation::Type OperationType = OperationList[OperationIndex];

	int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
	if (SectorIndex >= 0 && Resource)
	{
		TargetTradeRoute->AddSectorOperation(SectorIndex, OperationType, &Resource->Data);
		GenerateSectorList();
	}
}

void SFlareTradeRouteMenu::OnEditOperationClicked(FFlareTradeRouteSectorOperationSave* Operation)
{
	SelectedOperation = Operation;
}


void SFlareTradeRouteMenu::OnDeleteOperationClicked()
{
	if(SelectedOperation && TargetTradeRoute)
	{
		TargetTradeRoute->DeleteOperation(SelectedOperation);
		SelectedOperation = NULL;
		GenerateSectorList();
	}
}

void SFlareTradeRouteMenu::OnOperationUpClicked()
{
	if(SelectedOperation && TargetTradeRoute)
	{
		SelectedOperation = TargetTradeRoute->MoveOperationUp(SelectedOperation);
		GenerateSectorList();
	}
}

void SFlareTradeRouteMenu::OnOperationDownClicked()
{
	if(SelectedOperation && TargetTradeRoute)
	{
		SelectedOperation = TargetTradeRoute->MoveOperationDown(SelectedOperation);
		GenerateSectorList();
	}
}

void SFlareTradeRouteMenu::OnDecreaseLoadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource)
{
	int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
	FFlareTradeRouteSectorSave* PreviousOrder = TargetTradeRoute->GetSectorOrders(Sector);
	/*
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
	}*/
}

void SFlareTradeRouteMenu::OnIncreaseLoadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource)
{
	int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
	FFlareTradeRouteSectorSave* PreviousOrder = TargetTradeRoute->GetSectorOrders(Sector);
//TODO
	/*if (SectorIndex >= 0 && Resource && PreviousOrder)
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
	}*/
}

void SFlareTradeRouteMenu::OnClearLoadResourceClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource)
{
	int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
/*	if (SectorIndex >= 0 && Resource)
	{
		TargetTradeRoute->ClearSectorLoadOrder(SectorIndex, Resource);
		GenerateSectorList();
	}*/
}

void SFlareTradeRouteMenu::OnUnloadResourceClicked(UFlareSimulatedSector* Sector)
{
	UFlareResourceCatalogEntry* Resource = ResourceSelector->GetSelectedItem();
	int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);

	/*if (SectorIndex >= 0 && Resource)
	{
		TargetTradeRoute->SetSectorUnloadOrder(SectorIndex, &Resource->Data, 0);
		GenerateSectorList();
	}*/
}


void SFlareTradeRouteMenu::OnDecreaseUnloadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource)
{
	int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
	FFlareTradeRouteSectorSave* PreviousOrder = TargetTradeRoute->GetSectorOrders(Sector);
	/*
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
	}*/
}

void SFlareTradeRouteMenu::OnIncreaseUnloadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource)
{
	int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
	FFlareTradeRouteSectorSave* PreviousOrder = TargetTradeRoute->GetSectorOrders(Sector);
	/*
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
	}*/
}

void SFlareTradeRouteMenu::OnClearUnloadResourceClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource)
{
	int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
	/*if (SectorIndex >= 0 && Resource)
	{
		TargetTradeRoute->ClearSectorUnloadOrder(SectorIndex, Resource);
		GenerateSectorList();
	}*/
}

void SFlareTradeRouteMenu::OnFleetComboLineSelectionChanged(UFlareFleet* Item, ESelectInfo::Type SelectInfo)
{
}

void SFlareTradeRouteMenu::OnAssignFleetClicked()
{
	UFlareFleet* Item = FleetSelector->GetSelectedItem();
	if (Item)
	{
		TargetTradeRoute->AssignFleet(Item);
		GenerateFleetList();
	}
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

