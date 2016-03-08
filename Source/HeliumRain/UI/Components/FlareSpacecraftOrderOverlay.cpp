
#include "../../Flare.h"
#include "FlareFactoryInfo.h"
#include "FlareSpacecraftOrderOverlay.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Economy/FlareFactory.h"
#include "../../Game/FlareGame.h"

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

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Fill)
	[
		SNew(SBox)
		[
			SNew(SBorder)
			.HAlign(HAlign_Fill)
			.Padding(FMargin(0, 10))
			.BorderImage(&Theme.BackgroundBrush)
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
						.Text(LOCTEXT("Title", "Spacecraft order"))
						.TextStyle(&Theme.TitleFont)
						.Justification(ETextJustify::Center)
					]
	
					// List
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					[
						SAssignNew(SpacecraftSelector, SListView<TSharedPtr<FInterfaceContainer>>)
						.ListItemsSource(&SpacecraftList)
						.SelectionMode(ESelectionMode::Single)
						.OnGenerateRow(this, &SFlareSpacecraftOrderOverlay::OnGenerateSpacecraftLine)
						.OnSelectionChanged(this, &SFlareSpacecraftOrderOverlay::OnSpacecraftSelectionChanged)
					]
	
					// Buttons
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Right)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SFlareButton)
							.Text(LOCTEXT("Confirm", "Confirm"))
							.HelpText(LOCTEXT("ConfirmInfo", "Confirm the choice and star production"))
							.Icon(FFlareStyleSet::GetIcon("OK"))
							.OnClicked(this, &SFlareSpacecraftOrderOverlay::OnConfirmed)
							.Visibility(this, &SFlareSpacecraftOrderOverlay::GetConfirmVisibility)
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
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

void SFlareSpacecraftOrderOverlay::Open(UFlareFactory* Factory)
{
	SetVisibility(EVisibility::Visible);
	TargetFactory = Factory;

	// Init buildable Spacecraft list
	SpacecraftList.Empty();
	if (TargetFactory && TargetFactory->IsShipyard())
	{
		UFlareSpacecraftCatalogEntry* SelectedEntry = NULL;
		UFlareSpacecraftCatalog* SpacecraftCatalog = MenuManager->GetGame()->GetSpacecraftCatalog();

		for (int SpacecraftIndex = 0; SpacecraftIndex < SpacecraftCatalog->ShipCatalog.Num(); SpacecraftIndex++)
		{
			UFlareSpacecraftCatalogEntry* Entry = SpacecraftCatalog->ShipCatalog[SpacecraftIndex];
			FFlareSpacecraftDescription* Description = &Entry->Data;

			// Filter by Spacecraft size, and add
			bool LargeFactory = TargetFactory->IsLargeShipyard();
			bool LargeSpacecraft = Description->Size >= EFlarePartSize::L;
			if (LargeFactory == LargeSpacecraft)
			{
				SpacecraftList.AddUnique(FInterfaceContainer::New(&Entry->Data));
			}

			// Pre-selection
			if (Description->Identifier == TargetFactory->GetTargetShipClass())
			{
				SelectedEntry = Entry;
			}
		}
	}

	SpacecraftSelector->RequestListRefresh();
}

void SFlareSpacecraftOrderOverlay::Close()
{
	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Content callbacks
----------------------------------------------------*/

TSharedRef<ITableRow> SFlareSpacecraftOrderOverlay::OnGenerateSpacecraftLine(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FFlareSpacecraftDescription* Desc = Item->SpacecraftDescriptionPtr;
	const FFlareProductionData* CycleData = &TargetFactory->GetCycleDataForShipClass(Desc->Identifier);

	// Spacecraft type
	FText SpacecraftInfoText;
	if (Desc->OrbitalEngineCount == 0)
	{
		SpacecraftInfoText = LOCTEXT("Station", "(Station)");
	}
	else if (Desc->TurretSlots.Num())
	{
		SpacecraftInfoText = FText::Format(LOCTEXT("WeaponFormat", "(Military ship, {0} turrets)"), FText::AsNumber(Desc->TurretSlots.Num()));
	}
	else if (Desc->GunSlots.Num())
	{
		SpacecraftInfoText = FText::Format(LOCTEXT("WeaponFormat", "(Military ship, {0} gun slots)"), FText::AsNumber(Desc->GunSlots.Num()));
	}
	else
	{
		SpacecraftInfoText = LOCTEXT("NoWeapons", "(Unarmed ship)");
	}
	
	// Production cost
	FText ProductionCost;
	int ProductionTime = CycleData->ProductionTime;
	if (MenuManager->GetPC()->GetCompany() == TargetFactory->GetParent()->GetCompany())
	{
		ProductionCost = TargetFactory->GetFactoryCycleCost(CycleData);
	}
	else
	{
		ProductionTime += TargetFactory->GetRemainingProductionDuration();
		int32 CycleProductionCost = UFlareGameTools::ComputeShipPrice(Desc->Identifier, TargetFactory->GetParent()->GetCurrentSector());
		ProductionCost = FText::Format(LOCTEXT("ProductionCostFormat", "{0} credits"), FText::AsNumber(CycleProductionCost));
	}

	// Structure
	return SNew(SFlareListItem, OwnerTable)
	.Width(25)
	.Height(2)
	.Content()
	[
		SNew(SHorizontalBox)

		// Picture
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(Theme.SmallContentPadding)
		.VAlign(VAlign_Top)
		[
			SNew(SImage)
			.Image(&Desc->MeshPreviewBrush)
		]

		// Main infos
		+ SHorizontalBox::Slot()
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
				SNew(STextBlock)
				.Text(Desc->Description)
				.TextStyle(&Theme.TextFont)
				.WrapTextAt(Theme.ContentWidth)
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

		+ SHorizontalBox::Slot()
		.AutoWidth()
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
				.WrapTextAt(Theme.ContentWidth / 3)
				.TextStyle(&Theme.TextFont)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::Format(LOCTEXT("ProductionTimeFormat", "{0} days"), FText::AsNumber(ProductionTime)))
				.WrapTextAt(Theme.ContentWidth / 3)
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
		if (Desc && TargetFactory)
		{
			FLOGV("SFlareSpacecraftOrderOverlay::OnConfirmed : picked '%s'", *Desc->Identifier.ToString());

			if (TargetFactory->GetTargetShipCompany() == MenuManager->GetPC()->GetCompany()->GetIdentifier())
			{
				// Player ship building
				if (TargetFactory->GetTargetShipClass() != Desc->Identifier)
				{
					// Replace it
					TargetFactory->Stop();
					TargetFactory->OrderShip(MenuManager->GetPC()->GetCompany(), Desc->Identifier);
					TargetFactory->Start();
				}
			}
			else
			{
				TargetFactory->OrderShip(MenuManager->GetPC()->GetCompany(), Desc->Identifier);
				TargetFactory->Start();
			}
		}
	}

	Close();
}

void SFlareSpacecraftOrderOverlay::OnClose()
{
	Close();
}


#undef LOCTEXT_NAMESPACE
