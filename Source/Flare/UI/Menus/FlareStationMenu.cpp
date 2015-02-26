
#include "../../Flare.h"
#include "FlareStationMenu.h"
#include "../Widgets/FlarePartInfo.h"
#include "../../Game/FlareGame.h"
#include "../../Player/FlareHUD.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareStationMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareStationMenu::Construct(const FArguments& InArgs)
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
							SNew(SImage).Image(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Station))
						]

						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Title", "STATION OVERVIEW"))
							.TextStyle(FFlareStyleSet::Get(), "Flare.Title1")
						]
					]

					// Section title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(10))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("DockedShips", "DOCKED SHIPS"))
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
						.OnGenerateRow(this, &SFlareStationMenu::GenerateTargetInfo)
						.OnSelectionChanged(this, &SFlareStationMenu::OnTargetSelected)
					]

					// Section title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(10))
					[
						SAssignNew(SelectedTargetText, STextBlock)
						.Text(LOCTEXT("SelectedShip", "SELECTED SHIP"))
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
			.OnClicked(this, &SFlareStationMenu::OnDashboardClicked)
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareStationMenu::Setup()
{
	SetVisibility(EVisibility::Hidden);	
}

void SFlareStationMenu::Enter(IFlareStationInterface* Target)
{
	FLOG("SFlareStationMenu::Enter");

	CurrentStationTarget = Target;
	SetVisibility(EVisibility::Visible);

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC && Target)
	{
		PC->GetMenuPawn()->SetHorizontalOffset(100);
		PC->GetMenuPawn()->UpdateBackgroundColor(0.1, 0.8);

		// Load station data
		FFlareStationSave* Data = Target->Save();
		if (Data)
		{
			FFlareStationDescription* Desc = PC->GetGame()->GetStationCatalog()->Get(Data->Identifier);
			if (Desc)
			{
				PC->GetMenuPawn()->ShowStation(Desc, Data);
			}
		}

		// Fill the data
		TArray<IFlareShipInterface*> DockedShips = Target->GetDockedShips();
		for (int32 i = 0; i < DockedShips.Num(); i++)
		{
			TargetListData.AddUnique(FInterfaceContainer::New(DockedShips[0]));
		}
	}

	TargetList->RequestListRefresh();
}

void SFlareStationMenu::Exit()
{
	ActionMenu->Hide();
	SelectedTargetText->SetVisibility(EVisibility::Collapsed);

	TargetListData.Empty();
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

TSharedRef<ITableRow> SFlareStationMenu::GenerateTargetInfo(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());

	if (Item->ShipInterfacePtr)
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

void SFlareStationMenu::OnTargetSelected(TSharedPtr<FInterfaceContainer> Item, ESelectInfo::Type SelectInfo)
{
	FLOG("SFlareStationMenu::OnTargetSelected");
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
		if (Item->ShipInterfacePtr)
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

void SFlareStationMenu::OnDashboardClicked()
{
	OwnerHUD->OpenMenu(EFlareMenu::MENU_Dashboard);
}

#undef LOCTEXT_NAMESPACE
