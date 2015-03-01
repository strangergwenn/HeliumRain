
#include "../../Flare.h"
#include "FlareShipList.h"

#include "../../Game/FlareGame.h"
#include "../../Player/FlareHUD.h"
#include "../../Player/FlarePlayerController.h"

#include "../Components/FlareDashboardButton.h"

#define LOCTEXT_NAMESPACE "FlareSectorList"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareShipList::Construct(const FArguments& InArgs)
{
	// Data
	OwnerHUD = InArgs._OwnerHUD;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	
	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SVerticalBox)

		// Section title
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(10))
		[
			SNew(STextBlock)
			.Text(InArgs._Title)
			.TextStyle(FFlareStyleSet::Get(), "Flare.Title2")
		]

		// Box
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(10))
		[
			SAssignNew(TargetList, SListView< TSharedPtr<FInterfaceContainer> >)
			.ListItemsSource(&TargetListData)
			.SelectionMode(ESelectionMode::Single)
			.OnGenerateRow(this, &SFlareShipList::GenerateTargetInfo)
			.OnSelectionChanged(this, &SFlareShipList::OnTargetSelected)
		]

		// Section title
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(10))
		[
			SAssignNew(SelectedTargetText, STextBlock)
			.Text(LOCTEXT("SectorSelectedTitle", "SELECTED OBJECT"))
			.TextStyle(FFlareStyleSet::Get(), "Flare.Title2")
		]

		// Action box
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(10))
		[
			SAssignNew(ActionMenu, SFlareTargetActions)
			.Player(PC)
		]
	];

	// Defaults
	SelectedTargetText->SetVisibility(EVisibility::Collapsed);
	ActionMenu->Hide();
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareShipList::AddStation(IFlareStationInterface* StationCandidate)
{
	TargetListData.AddUnique(FInterfaceContainer::New(StationCandidate));
}

void SFlareShipList::AddShip(IFlareShipInterface* ShipCandidate)
{
	TargetListData.AddUnique(FInterfaceContainer::New(ShipCandidate));
}

void SFlareShipList::ClearList()
{
	TargetListData.Empty();
}

void SFlareShipList::RefreshList()
{
	TargetList->RequestListRefresh();
}

void SFlareShipList::Reset()
{
	ActionMenu->Hide();
	SelectedTargetText->SetVisibility(EVisibility::Collapsed);
	TargetListData.Empty();
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

TSharedRef<ITableRow> SFlareShipList::GenerateTargetInfo(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());

	if (Item->StationInterfacePtr)
	{
		return SNew(SFlareListItem, OwnerTable)
			.ButtonStyle(&FFlareStyleSet::Get(), "/Style/ShipInstanceButton")
			.Content()
			[
				SNew(SFlareShipInstanceInfo)
				.Player(PC)
				.Ship(NULL)
				.Station(Item->StationInterfacePtr)
			];
	}
	else if (Item->ShipInterfacePtr)
	{
		return SNew(SFlareListItem, OwnerTable)
			.ButtonStyle(&FFlareStyleSet::Get(), "/Style/ShipInstanceButton")
			.Content()
			[
				SNew(SFlareShipInstanceInfo)
				.Player(PC)
				.Ship(Item->ShipInterfacePtr)
				.Station(NULL)
			];
	}
	else
	{
		return SNew(SFlareListItem, OwnerTable)
			.Content()
			[
				SNew(STextBlock).Text(FText::FromString("Invalid item"))
			];
	}
}

void SFlareShipList::OnTargetSelected(TSharedPtr<FInterfaceContainer> Item, ESelectInfo::Type SelectInfo)
{
	FLOG("SFlareShipList::OnTargetSelected");
	TSharedPtr<SFlareListItem> ItemWidget = StaticCastSharedPtr<SFlareListItem>(TargetList->WidgetFromItem(Item));

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

	// Update the action menu
	if (Item.IsValid())
	{
		if (Item->StationInterfacePtr)
		{
			ActionMenu->SetStation(Item->StationInterfacePtr);
		}
		else
		{
			ActionMenu->SetShip(Item->ShipInterfacePtr);
		}
		ActionMenu->Show();
		SelectedTargetText->SetVisibility(EVisibility::Visible);
	}
	else
	{
		ActionMenu->Hide();
		SelectedTargetText->SetVisibility(EVisibility::Collapsed);
	}
}


#undef LOCTEXT_NAMESPACE

