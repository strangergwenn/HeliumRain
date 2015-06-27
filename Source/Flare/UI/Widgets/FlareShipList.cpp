
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
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	AFlarePlayerController* PC = MenuManager->GetPC();
	
	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SVerticalBox)

		// Section title
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.TitlePadding)
		[
			SNew(STextBlock)
			.Text(InArgs._Title)
			.TextStyle(&FFlareStyleSet::GetDefaultTheme().SubTitleFont)
		]

		// Box
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.ContentPadding)
		[
			SAssignNew(TargetList, SListView< TSharedPtr<FInterfaceContainer> >)
			.ListItemsSource(&TargetListData)
			.SelectionMode(ESelectionMode::Single)
			.OnGenerateRow(this, &SFlareShipList::GenerateTargetInfo)
			.OnSelectionChanged(this, &SFlareShipList::OnTargetSelected)
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareShipList::AddShip(IFlareSpacecraftInterface* ShipCandidate)
{
	TargetListData.AddUnique(FInterfaceContainer::New(ShipCandidate));
}

void SFlareShipList::RefreshList()
{
	TargetList->RequestListRefresh();
}

void SFlareShipList::Reset()
{
	TargetListData.Empty();
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

TSharedRef<ITableRow> SFlareShipList::GenerateTargetInfo(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	AFlarePlayerController* PC = MenuManager->GetPC();

	if (Item->ShipInterfacePtr)
	{
		return SNew(SFlareListItem, OwnerTable)
			.Width(5)
			.Height(1)
			.Content()
			[
				SNew(SFlareShipInstanceInfo)
				.Player(PC)
				.Ship(Item->ShipInterfacePtr)
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
		TSharedRef<SFlareShipInstanceInfo> ShipInfoWidget = StaticCastSharedRef<SFlareShipInstanceInfo>(PreviousSelection->GetContainer()->GetContent());
		ShipInfoWidget->SetActionsVisible(false);

		PreviousSelection->SetSelected(false);
	}
	if (ItemWidget.IsValid())
	{
		TSharedRef<SFlareShipInstanceInfo> ShipInfoWidget = StaticCastSharedRef<SFlareShipInstanceInfo>(ItemWidget->GetContainer()->GetContent());
		ShipInfoWidget->SetActionsVisible(true);

		ItemWidget->SetSelected(true);
		PreviousSelection = ItemWidget;
	}
}


#undef LOCTEXT_NAMESPACE

