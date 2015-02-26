
#include "../../Flare.h"
#include "FlareSectorMenu.h"

#include "../../Game/FlareGame.h"
#include "../../Player/FlareHUD.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlarePlayerController.h"

#include "../Components/FlareDashboardButton.h"

#define LOCTEXT_NAMESPACE "FlareSectorMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSectorMenu::Construct(const FArguments& InArgs)
{
	// Data
	OwnerHUD = InArgs._OwnerHUD;
	TSharedPtr<SFlareButton> BackButton;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	const FFlareContainerStyle* DefaultContainerStyle = &FFlareStyleSet::Get().GetWidgetStyle<FFlareContainerStyle>("/Style/DefaultContainerStyle");
	
	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SHorizontalBox)

		// UI container
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		[
			SNew(SBorder)
			.BorderImage(&DefaultContainerStyle->BackgroundBrush)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SNew(SVerticalBox)

					// Object name
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(10))
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SImage).Image(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Sector))
						]

						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Sector", "SECTOR MAP"))
							.TextStyle(FFlareStyleSet::Get(), "Flare.Title1")
						]
					]

					// Section title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(10))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("SectorTargetListTitle", "OBJECTS IN SECTOR"))
						.TextStyle(FFlareStyleSet::Get(), "Flare.Title2")
					]

					// Station box
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(10))
					[
						SAssignNew(TargetList, SListView< TSharedPtr<FInterfaceContainer> >)
						.ListItemsSource(&TargetListData)
						.SelectionMode(ESelectionMode::Single)
						.OnGenerateRow(this, &SFlareSectorMenu::GenerateTargetInfo)
						.OnSelectionChanged(this, &SFlareSectorMenu::OnTargetSelected)
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
				]
			]
		]

		// Dashboard button
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		[
			SAssignNew(BackButton, SFlareButton)
			.ContainerStyle(FFlareStyleSet::Get(), "/Style/InvisibleContainerStyle")
			.ButtonStyle(FFlareStyleSet::Get(), "/Style/BackToDashboardButton")
			.OnClicked(this, &SFlareSectorMenu::OnDashboardClicked)
		]
	];

	// SectorMenu close button
	BackButton->GetContainer()->SetContent(SNew(SImage).Image(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Exit)));
	SelectedTargetText->SetVisibility(EVisibility::Collapsed);
	ActionMenu->Hide();
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareSectorMenu::Setup()
{
	SetVisibility(EVisibility::Hidden);
}

void SFlareSectorMenu::Enter()
{
	FLOG("SFlareSectorMenu::Enter");
	SetVisibility(EVisibility::Visible);

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC)
	{
		PC->GetMenuPawn()->UpdateBackgroundColor(0.15, 0.15);

		for (TActorIterator<AActor> ActorItr(PC->GetWorld()); ActorItr; ++ActorItr)
		{
			// Station
			AFlareStation* StationCandidate = Cast<AFlareStation>(*ActorItr);
			if (StationCandidate)
			{
				TargetListData.AddUnique(FInterfaceContainer::New(StationCandidate));
			}

			// Ship
			AFlareShip* ShipCandidate = Cast<AFlareShip>(*ActorItr);
			if (ShipCandidate)
			{
				TargetListData.AddUnique(FInterfaceContainer::New(ShipCandidate));
			}
		}
	}

	TargetList->RequestListRefresh();
}

void SFlareSectorMenu::Exit()
{
	ActionMenu->Hide();
	SelectedTargetText->SetVisibility(EVisibility::Collapsed);

	TargetListData.Empty();
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

TSharedRef<ITableRow> SFlareSectorMenu::GenerateTargetInfo(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable)
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

void SFlareSectorMenu::OnTargetSelected(TSharedPtr<FInterfaceContainer> Item, ESelectInfo::Type SelectInfo)
{
	FLOG("SFlareSectorMenu::OnTargetSelected");
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

void SFlareSectorMenu::OnDashboardClicked()
{
	OwnerHUD->OpenMenu(EFlareMenu::MENU_Dashboard);
}

#undef LOCTEXT_NAMESPACE

