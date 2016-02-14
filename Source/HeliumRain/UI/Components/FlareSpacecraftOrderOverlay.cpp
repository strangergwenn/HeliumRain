
#include "../../Flare.h"
#include "FlareFactoryInfo.h"
#include "FlareSpacecraftOrderOverlay.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Economy/FlareFactory.h"

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

void SFlareSpacecraftOrderOverlay::Open(SFlareFactoryInfo* FactoryMenu)
{
	SetVisibility(EVisibility::Visible);
	TargetFactory = FactoryMenu->GetFactory();

	// Init buildable Spacecraft list
	SpacecraftList.Empty();
	if (TargetFactory && TargetFactory->HasCreateShipAction())
	{
		UFlareSpacecraftCatalogEntry* SelectedEntry = NULL;
		UFlareSpacecraftCatalog* SpacecraftCatalog = MenuManager->GetGame()->GetSpacecraftCatalog();

		for (int SpacecraftIndex = 0; SpacecraftIndex < SpacecraftCatalog->ShipCatalog.Num(); SpacecraftIndex++)
		{
			UFlareSpacecraftCatalogEntry* Entry = SpacecraftCatalog->ShipCatalog[SpacecraftIndex];
			FFlareSpacecraftDescription* Description = &Entry->Data;

			// Filter by Spacecraft size, and add
			bool LargeFactory = TargetFactory->GetDescription()->Identifier.ToString().Contains("large");
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

	// Strings
	FText WeaponText;
	if (Desc->TurretSlots.Num())
	{
		WeaponText = FText::Format(LOCTEXT("WeaponFormat", "({0} turrets)"), FText::AsNumber(Desc->TurretSlots.Num()));
	}
	else if (Desc->GunSlots.Num())
	{
		WeaponText = FText::Format(LOCTEXT("WeaponFormat", "({0} gun slots)"), FText::AsNumber(Desc->GunSlots.Num()));
	}
	else
	{
		WeaponText = LOCTEXT("NoWeapons", "(Unarmed)");
	}

	// Structure
	return SNew(SFlareListItem, OwnerTable)
	.Width(20)
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
				.Text(WeaponText)
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
				.Text(LOCTEXT("ProductionCost", "Production cost"))
				.TextStyle(&Theme.NameFont)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(TargetFactory->GetFactoryCycleCost(&TargetFactory->GetCycleDataForShipClass(Desc->Identifier)))
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
			TargetFactory->SetTargetShipClass(Desc->Identifier);
			TargetFactory->Start();
		}
	}

	Close();
}

void SFlareSpacecraftOrderOverlay::OnClose()
{
	Close();
}


#undef LOCTEXT_NAMESPACE
