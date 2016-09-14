
#include "../../Flare.h"
#include "FlareShipList.h"

#include "../../Game/FlareGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"

#define LOCTEXT_NAMESPACE "FlareSectorList"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareShipList::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	UseCompactDisplay = InArgs._UseCompactDisplay;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	AFlarePlayerController* PC = MenuManager->GetPC();
	OnItemSelected = InArgs._OnItemSelected;
	
	// Build structure
	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Left)
	[
		SNew(SBox)
		.WidthOverride(Theme.ContentWidth)
		[
			SNew(SVerticalBox)

			// Section title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.TitlePadding)
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(InArgs._Title)
				.TextStyle(&FFlareStyleSet::GetDefaultTheme().SubTitleFont)
			]

			// Section title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.TitlePadding)
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Nothing", "No objects."))
				.TextStyle(&FFlareStyleSet::GetDefaultTheme().TextFont)
				.Visibility(this, &SFlareShipList::GetNoObjectsVisibility)
			]

			// Box
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.ContentPadding)
			.HAlign(HAlign_Fill)
			[
				SAssignNew(TargetList, SListView< TSharedPtr<FInterfaceContainer> >)
				.ListItemsSource(&TargetListData)
				.SelectionMode(ESelectionMode::Single)
				.OnGenerateRow(this, &SFlareShipList::GenerateTargetInfo)
				.OnSelectionChanged(this, &SFlareShipList::OnTargetSelected)
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareShipList::AddFleet(UFlareFleet* Fleet)
{
	TargetListData.AddUnique(FInterfaceContainer::New(Fleet));
}

void SFlareShipList::AddShip(UFlareSimulatedSpacecraft* Ship)
{
	TargetListData.AddUnique(FInterfaceContainer::New(Ship));
}

void SFlareShipList::RefreshList()
{
	struct FSortBySize
	{
		FORCEINLINE bool operator()(const TSharedPtr<FInterfaceContainer> PtrA, const TSharedPtr<FInterfaceContainer> PtrB) const
		{
			FCHECK(PtrA.IsValid());
			FCHECK(PtrB.IsValid());
			UFlareSimulatedSpacecraft* A = PtrA->ShipInterfacePtr;
			UFlareSimulatedSpacecraft* B = PtrB->ShipInterfacePtr;

			if (PtrA->FleetPtr)
			{
				if (PtrB->FleetPtr)
				{
					return (PtrA->FleetPtr->GetShips().Num() > PtrB->FleetPtr->GetShips().Num());
				}
				else
				{
					return true;
				}
			}
			else if (PtrB->FleetPtr)
			{
				return false;
			}

			if (A->IsStation())
			{
					return true;
			}
			else if (B->IsStation())
			{
				return false;
			}
			else
			{
				if (A->GetSize() > B->GetSize())
				{
					return true;
				}
				else if (A->GetSize() < B->GetSize())
				{
					return false;
				}
				else if (A->IsMilitary())
				{
					if (!B->IsMilitary())
					{
						return true;
					}
					else
					{
						return A->GetWeaponsSystem()->GetWeaponGroupCount() > B->GetWeaponsSystem()->GetWeaponGroupCount();
					}
				}
				else
				{
					return false;
				}
			}
			return false;
		}
	};

	ClearSelection();
	TargetListData.Sort(FSortBySize());
	TargetList->RequestListRefresh();

	SlatePrepass(FSlateApplicationBase::Get().GetApplicationScale());
}

void SFlareShipList::ClearSelection()
{
	TargetList->ClearSelection();

	if (PreviousSelection.IsValid())
	{
		TSharedRef<SFlareSpacecraftInfo> ShipInfoWidget = StaticCastSharedRef<SFlareSpacecraftInfo>(PreviousSelection->GetContainer()->GetContent());
		ShipInfoWidget->SetMinimized(true);
		PreviousSelection->SetSelected(false);
	}
}

void SFlareShipList::Reset()
{
	TargetListData.Empty();
	TargetList->ClearSelection();
	TargetList->RequestListRefresh();
	SelectedItem.Reset();
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

EVisibility SFlareShipList::GetNoObjectsVisibility() const
{
	return (TargetListData.Num() > 0 ? EVisibility::Collapsed : EVisibility::Visible);
}

TSharedRef<ITableRow> SFlareShipList::GenerateTargetInfo(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	AFlarePlayerController* PC = MenuManager->GetPC();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	int32 Width = 15;
	int32 Height = 1;

	// Ship
	if (Item->ShipInterfacePtr)
	{
		return SNew(SFlareListItem, OwnerTable)
			.Width(Width)
			.Height(Height)
			.Content()
			[
				SNew(SFlareSpacecraftInfo)
				.Player(PC)
				.Spacecraft(Item->ShipInterfacePtr)
				.OwnerWidget(this)
				.Minimized(true)
				.Visible(true)
			];
	}

	// Fleet
	else if (Item->FleetPtr)
	{
		TSharedPtr<SVerticalBox> Temp;
		int32 ShipCount = Item->FleetPtr->GetShips().Num();
		FText ShipText = ShipCount == 1 ? LOCTEXT("Ship", "ship") : LOCTEXT("Ships", "ships");
		FText FleetDescription = FText::Format(LOCTEXT("FleetDescriptionFormat", "{0} {1}"), FText::AsNumber(ShipCount), ShipText);
		
		// Single ship : just the ship
		if (Item->FleetPtr->GetShips().Num() == 1)
		{
			return SNew(SFlareListItem, OwnerTable)
				.Width(Width)
				.Height(Height)
				.Content()
				[
					SNew(SFlareSpacecraftInfo)
					.Player(PC)
					.Spacecraft(Item->FleetPtr->GetShips()[0])
					.OwnerWidget(this)
					.Minimized(true)
					.Visible(true)
				];
		}

		// Actual fleet
		else
		{
			return SNew(SFlareListItem, OwnerTable)
			.Width(Width)
			.Height(Height)
			.Content()
			[
				SNew(SHorizontalBox)

				// Ship name
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(10))
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Item->FleetPtr->GetFleetName())
					.TextStyle(&Theme.NameFont)
				]

				// Ship class
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(12))
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FleetDescription)
					.TextStyle(&Theme.TextFont)
				]
			];
		}
	}

	// Invalid item
	else
	{
		return SNew(SFlareListItem, OwnerTable)
			.Content()
			[
				SNew(STextBlock).Text(LOCTEXT("Invalid", "Invalid item"))
			];
	}
}

void SFlareShipList::OnTargetSelected(TSharedPtr<FInterfaceContainer> Item, ESelectInfo::Type SelectInfo)
{
	FLOG("SFlareShipList::OnTargetSelected");
	TSharedPtr<SFlareListItem> ItemWidget = StaticCastSharedPtr<SFlareListItem>(TargetList->WidgetFromItem(Item));
	SelectedItem = Item;

	bool ExpandShipWidgets = Item.IsValid() && Item->ShipInterfacePtr && !UseCompactDisplay;

	// Update selection
	if (PreviousSelection.IsValid())
	{
		TSharedRef<SFlareSpacecraftInfo> ShipInfoWidget = StaticCastSharedRef<SFlareSpacecraftInfo>(PreviousSelection->GetContainer()->GetContent());
		if (ExpandShipWidgets)
		{
			ShipInfoWidget->SetMinimized(true);
		}
		PreviousSelection->SetSelected(false);
	}
	if (ItemWidget.IsValid())
	{
		TSharedRef<SFlareSpacecraftInfo> ShipInfoWidget = StaticCastSharedRef<SFlareSpacecraftInfo>(ItemWidget->GetContainer()->GetContent());

		if (OnItemSelected.IsBound())
		{
			OnItemSelected.Execute(Item); 
		}
		if (ExpandShipWidgets)
		{
			ShipInfoWidget->SetMinimized(false);
		}

		ItemWidget->SetSelected(true);
		PreviousSelection = ItemWidget;
	}
}


#undef LOCTEXT_NAMESPACE

