
#include "FlareSpacecraftOrderOverlay.h"
#include "../../Flare.h"

#include "FlareFactoryInfo.h"
#include "../Menus/FlareShipMenu.h"

#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Economy/FlareFactory.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareGameTools.h"

#include "SBackgroundBlur.h"

#define LOCTEXT_NAMESPACE "FlareSpacecraftOrderOverlay"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSpacecraftOrderOverlay::Construct(const FArguments& InArgs)
{
	// Data
	SpacecraftList.Empty();
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	TargetComplex = NULL;
	TargetShipyard = NULL;
	TargetSector = NULL;

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Fill)
	[
		SNew(SBox)
		[
			SNew(SBackgroundBlur)
			.BlurRadius(Theme.BlurRadius)
			.BlurStrength(Theme.BlurStrength)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(FMargin(0))
			[
				SNew(SBorder)
				.HAlign(HAlign_Center)
				.Padding(Theme.ContentPadding)
				.BorderImage(&Theme.BackgroundBrush)
				[
					SNew(SVerticalBox)

					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					[
						SNew(STextBlock)
						.Text(this, &SFlareSpacecraftOrderOverlay::GetWindowTitle)
						.TextStyle(&Theme.TitleFont)
						.Justification(ETextJustify::Center)
					]
	
					// List
					+ SVerticalBox::Slot()
					[
						SNew(SScrollBox)
						.Style(&Theme.ScrollBoxStyle)
						.ScrollBarStyle(&Theme.ScrollBarStyle)

						+ SScrollBox::Slot()
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(SpacecraftSelector, SListView<TSharedPtr<FInterfaceContainer>>)
							.ListItemsSource(&SpacecraftList)
							.SelectionMode(ESelectionMode::Single)
							.OnGenerateRow(this, &SFlareSpacecraftOrderOverlay::OnGenerateSpacecraftLine)
							.OnSelectionChanged(this, &SFlareSpacecraftOrderOverlay::OnSpacecraftSelectionChanged)
						]
					]

					// Help text
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Right)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareSpacecraftOrderOverlay::GetWalletText)
					]
	
					// Buttons
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Top)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(ConfirmText, STextBlock)
							.TextStyle(&Theme.TextFont)
							.WrapTextAt(Theme.ContentWidth)
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Top)
						[
							SAssignNew(ConfirmButon, SFlareButton)
							.Text(LOCTEXT("Confirm", "Confirm"))
							.HelpText(LOCTEXT("ConfirmInfo", "Confirm the choice and start production"))
							.Icon(FFlareStyleSet::GetIcon("OK"))
							.OnClicked(this, &SFlareSpacecraftOrderOverlay::OnConfirmed)
							.Visibility(this, &SFlareSpacecraftOrderOverlay::GetConfirmVisibility)
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Top)
						[
							SNew(SFlareButton)
							.Text(LOCTEXT("Cancel", "Cancel"))
							.HelpText(LOCTEXT("CancelInfo", "Go back without saving changes"))
							.Icon(FFlareStyleSet::GetIcon("Delete"))
							.OnClicked(this, &SFlareSpacecraftOrderOverlay::OnClose)
						]
					]
				]
			]
		]
	];

	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareSpacecraftOrderOverlay::Open(UFlareSimulatedSpacecraft* Complex, FName ConnectorName, FOrderDelegate ConfirmationCallback)
{
	SetVisibility(EVisibility::Visible);
	TargetSector = Complex->GetCurrentSector();
	TargetComplex = Complex;
	OnConfirmedCB = ConfirmationCallback;
	bool IsSlotSpecial = ConnectorName.ToString().Contains(TEXT("special"));

	// Init station list
	SpacecraftList.Empty();
	if (TargetSector)
	{
		UFlareSpacecraftCatalog* SpacecraftCatalog = MenuManager->GetGame()->GetSpacecraftCatalog();

		for (int SpacecraftIndex = 0; SpacecraftIndex < SpacecraftCatalog->StationCatalog.Num(); SpacecraftIndex++)
		{
			// Candidate have to be not a substation, compatible with complex, not restricted / built on special, and available in tech
			FFlareSpacecraftDescription* Description = &SpacecraftCatalog->StationCatalog[SpacecraftIndex]->Data;
			if (!Description->IsSubstation
			 && Description->CanBeBuiltInComplex
			 && (!Description->IsRestrictedInComplex || IsSlotSpecial)
			 && MenuManager->GetPC()->GetCompany()->IsTechnologyUnlockedStation(Description))
			{
				UFlareSpacecraftCatalogEntry* Entry = SpacecraftCatalog->StationCatalog[SpacecraftIndex];
				SpacecraftList.AddUnique(FInterfaceContainer::New(&Entry->Data));
			}
		}
	}

	SpacecraftSelector->RequestListRefresh();
	ConfirmText->SetText(FText());
}

void SFlareSpacecraftOrderOverlay::Open(UFlareSimulatedSpacecraft* Shipyard, bool IsHeavy)
{
	SetVisibility(EVisibility::Visible);
	TargetShipyard = Shipyard;

	// Init ship list
	SpacecraftList.Empty();
	if (TargetShipyard && TargetShipyard->IsShipyard())
	{
		UFlareSpacecraftCatalogEntry* SelectedEntry = NULL;
		UFlareSpacecraftCatalog* SpacecraftCatalog = MenuManager->GetGame()->GetSpacecraftCatalog();

		for (int SpacecraftIndex = 0; SpacecraftIndex < SpacecraftCatalog->ShipCatalog.Num(); SpacecraftIndex++)
		{
			UFlareSpacecraftCatalogEntry* Entry = SpacecraftCatalog->ShipCatalog[SpacecraftIndex];
			FFlareSpacecraftDescription* Description = &Entry->Data;

			if (!Description->IsSubstation)
			{
				// Filter by spacecraft size and add
				bool LargeSpacecraft = Description->Size >= EFlarePartSize::L;
				if (IsHeavy == LargeSpacecraft)
				{
					SpacecraftList.AddUnique(FInterfaceContainer::New(&Entry->Data));
				}
			}
		}
	}

	SpacecraftSelector->RequestListRefresh();
	ConfirmText->SetText(FText());
}

void SFlareSpacecraftOrderOverlay::Open(UFlareSimulatedSector* Sector, FOrderDelegate ConfirmationCallback)
{
	SetVisibility(EVisibility::Visible);
	TargetSector = Sector;
	OnConfirmedCB = ConfirmationCallback;

	// Init station list
	SpacecraftList.Empty();
	if (TargetSector)
	{
		UFlareSpacecraftCatalog* SpacecraftCatalog = MenuManager->GetGame()->GetSpacecraftCatalog();

		for (int SpacecraftIndex = 0; SpacecraftIndex < SpacecraftCatalog->StationCatalog.Num(); SpacecraftIndex++)
		{
			FFlareSpacecraftDescription* Description = &SpacecraftCatalog->StationCatalog[SpacecraftIndex]->Data;
			if (!Description->IsSubstation && MenuManager->GetPC()->GetCompany()->IsTechnologyUnlockedStation(Description))
			{
				UFlareSpacecraftCatalogEntry* Entry = SpacecraftCatalog->StationCatalog[SpacecraftIndex];
				SpacecraftList.AddUnique(FInterfaceContainer::New(&Entry->Data));
			}
		}
	}

	SpacecraftSelector->RequestListRefresh();
	ConfirmText->SetText(FText());
}

bool SFlareSpacecraftOrderOverlay::IsOpen() const
{
	return (GetVisibility() == EVisibility::Visible);
}

void SFlareSpacecraftOrderOverlay::Close()
{
	SetVisibility(EVisibility::Collapsed);

	SpacecraftList.Empty();
	SpacecraftSelector->RequestListRefresh();
	SpacecraftSelector->ClearSelection();

	ConfirmText->SetText(FText());
	TargetComplex = NULL;
	TargetShipyard = NULL;
	TargetSector = NULL;
}

void SFlareSpacecraftOrderOverlay::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (SpacecraftSelector->GetNumItemsSelected() > 0)
	{
		FFlareSpacecraftDescription* Desc = SpacecraftSelector->GetSelectedItems()[0]->SpacecraftDescriptionPtr;
		UFlareCompany* PlayerCompany = MenuManager->GetPC()->GetCompany();
		ConfirmText->SetText(FText());
		bool CanBuild = false;

		if (Desc)
		{
			// Shipyard mode
			if (TargetShipyard)
			{
				uint32 ShipPrice;

				// Get price
				if (TargetShipyard->GetCompany() != PlayerCompany)
				{
					ShipPrice = UFlareGameTools::ComputeSpacecraftPrice(Desc->Identifier, TargetShipyard->GetCurrentSector(), true);
				}
				else
				{
					ShipPrice = Desc->CycleCost.ProductionCost;
				}
				CanBuild = (PlayerCompany->GetMoney() >= ShipPrice);

				// Show reason
				if (!CanBuild)
				{
					ConfirmText->SetText(FText::Format(LOCTEXT("CannotBuildShip", "Not enough credits ({0} / {1})"),
						FText::AsNumber(UFlareGameTools::DisplayMoney(PlayerCompany->GetMoney())),
						FText::AsNumber(UFlareGameTools::DisplayMoney(ShipPrice))));
				}
			}

			// Sector mode
			else
			{
				TArray<FText> Reasons;
				CanBuild = TargetSector->CanBuildStation(Desc, PlayerCompany, Reasons);

				// Show reason
				if (!CanBuild)
				{
					FString CantBuildReasons;
					for (int32 Index = 0; Index < Reasons.Num(); Index++)
					{
						if (Index)
						{
							CantBuildReasons += FString("\n");
						}
						CantBuildReasons += Reasons[Index].ToString();
					}
					ConfirmText->SetText(FText::FromString(CantBuildReasons));
				}
			}
		}

		ConfirmButon->SetDisabled(!CanBuild);
	}
}


/*----------------------------------------------------
	Content callbacks
----------------------------------------------------*/

FText SFlareSpacecraftOrderOverlay::GetWindowTitle() const
{
	if (TargetComplex)
	{
		return LOCTEXT("BuildComplexStationTitle", "Build complex station");
	}
	else if (TargetShipyard)
	{
		return LOCTEXT("SpacecraftOrderTitle", "Order spacecraft");
	}
	else if (TargetSector)
	{
		return LOCTEXT("BuildStationTitle", "Build station");
	}
	else
	{
		return FText();
	}
}

FText SFlareSpacecraftOrderOverlay::GetWalletText() const
{
	if (MenuManager->GetPC())
	{
		return FText::Format(LOCTEXT("CompanyCurrentWallet", "You have {0} credits available."),
			FText::AsNumber(MenuManager->GetPC()->GetCompany()->GetMoney() / 100));
	}

	return FText();
}

TSharedRef<ITableRow> SFlareSpacecraftOrderOverlay::OnGenerateSpacecraftLine(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	// Setup
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FFlareSpacecraftDescription* Desc = Item->SpacecraftDescriptionPtr;
	FText SpacecraftInfoText;
	FText ProductionCost;
	int ProductionTime;

	// Factory
	if (TargetShipyard)
	{
		FCHECK(TargetShipyard);
		ProductionTime = TargetShipyard->GetShipProductionTime(Desc->Identifier);

		// Station
		if (Desc->OrbitalEngineCount == 0)
		{
			SpacecraftInfoText = FText::Format(LOCTEXT("FactoryStationFormat", "(Station, {0} factories)"),
				FText::AsNumber(Desc->Factories.Num()));
		}

		// Ship
		else if (Desc->TurretSlots.Num())
		{
			SpacecraftInfoText = FText::Format(LOCTEXT("FactoryWeaponTurretFormat", "(Military ship, {0} turrets)"),
				FText::AsNumber(Desc->TurretSlots.Num()));
		}
		else if (Desc->GunSlots.Num())
		{
			SpacecraftInfoText = FText::Format(LOCTEXT("FactoryWeaponGunFormat", "(Military ship, {0} gun slots)"),
				FText::AsNumber(Desc->GunSlots.Num()));
		}
		else
		{
			SpacecraftInfoText = FText::Format(LOCTEXT("FactoryTraderFormat", "(Trading ship, {0}x{1} cargo units)"),
				FText::AsNumber(Desc->CargoBayCount),
				FText::AsNumber(Desc->CargoBayCapacity));
		}

		ProductionTime += TargetShipyard->GetEstimatedQueueAndProductionDuration(Desc->Identifier, -1);
		// Production cost
		if (MenuManager->GetPC()->GetCompany() == TargetShipyard->GetCompany())
		{
			ProductionCost = FText::Format(LOCTEXT("FactoryProductionResourcesFormat", "\u2022 {0}"), TargetShipyard->GetShipCost(Desc->Identifier));
		}
		else
		{
			int32 CycleProductionCost = UFlareGameTools::ComputeSpacecraftPrice(Desc->Identifier, TargetShipyard->GetCurrentSector(), true);
			ProductionCost = FText::Format(LOCTEXT("FactoryProductionCostFormat", "\u2022 {0} credits"), FText::AsNumber(UFlareGameTools::DisplayMoney(CycleProductionCost)));
		}
	}

	// Sector
	else
	{
		FCHECK(TargetSector);
		SpacecraftInfoText = FText::Format(LOCTEXT("StationInfoFormat", "(Station, {0} factories)"), FText::AsNumber(Desc->Factories.Num()));
		ProductionTime = Desc->CycleCost.ProductionTime;

		// Add resources
		FString ResourcesString;
		for (int ResourceIndex = 0; ResourceIndex < Desc->CycleCost.InputResources.Num(); ResourceIndex++)
		{
			FFlareFactoryResource* FactoryResource = &Desc->CycleCost.InputResources[ResourceIndex];
			if (ResourcesString.Len())
			{
				ResourcesString += ", ";
			}
			ResourcesString += FString::Printf(TEXT("%u %s"), FactoryResource->Quantity, *FactoryResource->Resource->Data.Name.ToString()); // FString needed here
		}

		// Constraints
		FString ConstraintString;
		if (Desc->BuildConstraint.Contains(EFlareBuildConstraint::FreeAsteroid))
		{
			if (ConstraintString.Len())
			{
				ConstraintString += ", ";
			}
			ConstraintString += LOCTEXT("AsteroidNeeded", "a free asteroid").ToString();
		}
		if (Desc->BuildConstraint.Contains(EFlareBuildConstraint::SunExposure))
		{
			if (ConstraintString.Len())
			{
				ConstraintString += ", ";
			}
			ConstraintString += LOCTEXT("SunNeeded", "good sun exposure").ToString();
		}
		if (Desc->BuildConstraint.Contains(EFlareBuildConstraint::GeostationaryOrbit))
		{
			if (ConstraintString.Len())
			{
				ConstraintString += ", ";
			}
			ConstraintString += LOCTEXT("GeostationaryNeeded", "a geostationary orbit").ToString();
		}
		if (Desc->BuildConstraint.Contains(EFlareBuildConstraint::HideOnIce))
		{
			if (ConstraintString.Len())
			{
				ConstraintString += ", ";
			}
			ConstraintString += LOCTEXT("NonIcyNeeded", "a non-icy sector").ToString();
		}
		if (Desc->BuildConstraint.Contains(EFlareBuildConstraint::HideOnNoIce))
		{
			if (ConstraintString.Len())
			{
				ConstraintString += ", ";
			}
			ConstraintString += LOCTEXT("IcyNeeded", "an icy sector").ToString();
		}
		if (ConstraintString.Len())
		{
			ConstraintString = LOCTEXT("ConstructioNRequirement", "\n\u2022 Requires").ToString() + " " + ConstraintString;
		}

		int32 CompanyStationCountInSector = 0;
		for(UFlareSimulatedSpacecraft* Station : TargetSector->GetSectorStations())
		{
			if(Station->GetCompany() == MenuManager->GetPC()->GetCompany())
			{
				++CompanyStationCountInSector;
			}
		}

		// Final text
		ProductionCost = FText::Format(LOCTEXT("StationCostFormat", "\u2022 Costs {0} credits ({1} stations in this sector) {3}\n\u2022 Completion requires {2}"),
			FText::AsNumber(UFlareGameTools::DisplayMoney(TargetSector->GetStationConstructionFee(Desc->CycleCost.ProductionCost, MenuManager->GetPC()->GetCompany()))),
			FText::AsNumber(CompanyStationCountInSector),
			FText::FromString(ResourcesString),
			FText::FromString(ConstraintString));
	}

	// Structure
	return SNew(SFlareListItem, OwnerTable)
	.Width(32)
	.Height(2)
	.Content()
	[
		SNew(SHorizontalBox)

		// Picture
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(Theme.ContentPadding)
		.VAlign(VAlign_Top)
		[
			SNew(SImage)
			.Image(&Desc->MeshPreviewBrush)
		]

		// Main infos
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(Theme.ContentPadding)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.SmallContentPadding)
			[
				SNew(STextBlock)
				.Text(Desc->Name)
				.TextStyle(&Theme.NameFont)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.SmallContentPadding)
			[
				SNew(SBox)
				.WidthOverride(Theme.ContentWidth)
				[
					SNew(STextBlock)
					.Text(Desc->Description)
					.TextStyle(&Theme.TextFont)
					.WrapTextAt(Theme.ContentWidth)
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.SmallContentPadding)
			[
				SNew(STextBlock)
				.Text(SpacecraftInfoText)
				.TextStyle(&Theme.TextFont)
			]
		]

		// Costs
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(Theme.ContentPadding)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ProductionCost", "Production cost & duration"))
				.TextStyle(&Theme.NameFont)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(ProductionCost)
				.WrapTextAt(0.65 * Theme.ContentWidth)
				.TextStyle(&Theme.TextFont)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::Format(LOCTEXT("ProductionTimeFormat", "\u2022 {0} days"), FText::AsNumber(ProductionTime)))
				.WrapTextAt(0.65 * Theme.ContentWidth)
				.TextStyle(&Theme.TextFont)
			]
		]
	];
}

void SFlareSpacecraftOrderOverlay::OnSpacecraftSelectionChanged(TSharedPtr<FInterfaceContainer> Item, ESelectInfo::Type SelectInfo)
{
	FLOG("SFlareSpacecraftOrderOverlay::OnSpacecraftSelectionChanged");
	TSharedPtr<SFlareListItem> ItemWidget = StaticCastSharedPtr<SFlareListItem>(SpacecraftSelector->WidgetFromItem(Item));
	SelectedItem = Item;

	// Update selection
	if (PreviousSelection.IsValid())
	{
		PreviousSelection->SetSelected(false);
	}
	if (ItemWidget.IsValid())
	{
		ItemWidget->SetSelected(true);
		PreviousSelection = ItemWidget;
	}
}

EVisibility SFlareSpacecraftOrderOverlay::GetConfirmVisibility() const
{
	if (SpacecraftSelector->GetNumItemsSelected() > 0)
	{
		return EVisibility::Visible;
	}
	else
	{
		return EVisibility::Hidden;
	}
}


/*----------------------------------------------------
	Action callbacks
----------------------------------------------------*/

void SFlareSpacecraftOrderOverlay::OnConfirmed()
{
	// Apply
	if (SpacecraftSelector->GetNumItemsSelected() > 0)
	{
		FFlareSpacecraftDescription* Desc = SpacecraftSelector->GetSelectedItems()[0]->SpacecraftDescriptionPtr;
		if (Desc)
		{
			FLOGV("SFlareSpacecraftOrderOverlay::OnConfirmed : picked '%s'", *Desc->Identifier.ToString());

			// Factory
			if (TargetShipyard)
			{
				TargetShipyard->ShipyardOrderShip(MenuManager->GetPC()->GetCompany(), Desc->Identifier);

				if (MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Station || MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Ship)
				{
					MenuManager->GetShipMenu()->UpdateShipyard();
				}
			}

			// Sector
			else
			{
				OnConfirmedCB.ExecuteIfBound(Desc);
			}
		}

		MenuManager->GetPC()->ClientPlaySound(MenuManager->GetPC()->GetSoundManager()->InfoSound);
	}

	Close();
}

void SFlareSpacecraftOrderOverlay::OnClose()
{
	MenuManager->GetPC()->ClientPlaySound(MenuManager->GetPC()->GetSoundManager()->NegativeClickSound);
	Close();
}


#undef LOCTEXT_NAMESPACE
