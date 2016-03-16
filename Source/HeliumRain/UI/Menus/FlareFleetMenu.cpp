
#include "../../Flare.h"
#include "FlareFleetMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareCompany.h"
#include "../../Game/FlareFleet.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../Components/FlareRoundButton.h"

#define LOCTEXT_NAMESPACE "FlareFleetMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareFleetMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Setup
	FleetToAdd = NULL;
	SelectedFleet = NULL;
	ShipToRemove = NULL;

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
				SNew(SImage).Image(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Fleet))
			]

			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Fleets", "FLEETS"))
				.TextStyle(&Theme.TitleFont)
			]

			// Close
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("Back", "Back"))
				.HelpText(LOCTEXT("BackInfo", "Go to the previous menu"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Exit, true))
				.OnClicked(this, &SFlareFleetMenu::OnBackClicked)
			]
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(200, 20))
		[
			SNew(SImage).Image(&Theme.SeparatorBrush)
		]

		// Content block
		+ SVerticalBox::Slot()
		.Padding(Theme.ContentPadding)
		.HAlign(HAlign_Center)
		[
			SNew(SScrollBox)
			.Style(&Theme.ScrollBoxStyle)
			.ScrollBarStyle(&Theme.ScrollBarStyle)

			+ SScrollBox::Slot()
			[
				SNew(SHorizontalBox)

				// Ship list
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				[
					SNew(SBox)
					.HAlign(HAlign_Left)
					.WidthOverride(Theme.ContentWidth)
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SFlareButton)
								.Width(8)
								.Text(LOCTEXT("RemoveFromFleet", "Remove from fleet"))
								.HelpText(LOCTEXT("RemoveFromFleetInfo", "Remove this ship from the fleet"))
								.IsDisabled(this, &SFlareFleetMenu::IsRemoveDisabled)
								.OnClicked(this, &SFlareFleetMenu::OnRemoveFromFleet)
							]
						]
									
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(ShipList, SFlareShipList)
							.MenuManager(MenuManager)
							.Title(LOCTEXT("CurrentFleet", "Current fleet"))
							.OnItemSelected(this, &SFlareFleetMenu::OnSpacecraftSelected)
							.UseCompactDisplay(true)
						]
					]
				]

				// Fleet list
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				[
					SNew(SBox)
					.HAlign(HAlign_Fill)
					.WidthOverride(Theme.ContentWidth)
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SFlareButton)
								.Width(8)
								.Text(LOCTEXT("AddToFleet", "Merge with current fleet"))
								.HelpText(LOCTEXT("AddToFleetInfo", "Merge this fleet or ship with the current fleet"))
								.IsDisabled(this, &SFlareFleetMenu::IsAddDisabled)
								.OnClicked(this, &SFlareFleetMenu::OnAddToFleet)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SFlareButton)
								.Width(8)
								.Text(LOCTEXT("SelectFleet", "Select fleet"))
								.HelpText(LOCTEXT("SelectFleetInfo", "Select this fleet"))
								.IsDisabled(this, &SFlareFleetMenu::IsSelectDisabled)
								.OnClicked(this, &SFlareFleetMenu::OnSelectFleet)
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(FleetList, SFlareShipList)
							.MenuManager(MenuManager)
							.Title(LOCTEXT("Unassigned", "Available fleets & ships"))
							.OnItemSelected(this, &SFlareFleetMenu::OnFleetSelected)
							.UseCompactDisplay(true)
						]
					]
				]
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareFleetMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareFleetMenu::Enter(UFlareFleet* TargetFleet)
{
	FLOGV("SFlareFleetMenu::Enter : TargetFleet=%p", TargetFleet);

	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	UpdateFleetList();
}

void SFlareFleetMenu::Exit()
{
	SetEnabled(false);

	ShipList->Reset();
	FleetList->Reset();
	SetVisibility(EVisibility::Collapsed);
}

void SFlareFleetMenu::UpdateFleetList()
{
	AFlarePlayerController* PC = MenuManager->GetPC();

	FleetList->Reset();

	for (int32 FleetIndex = 0; FleetIndex < PC->GetCompany()->GetCompanyFleets().Num(); FleetIndex++)
	{
		UFlareFleet* Fleet = PC->GetCompany()->GetCompanyFleets()[FleetIndex];
		if (Fleet)
		{
			FleetList->AddFleet(Fleet);
		}
	}

	FleetList->RefreshList();
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareFleetMenu::OnBackClicked()
{
	MenuManager->Back();
}

void SFlareFleetMenu::OnSpacecraftSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer)
{
	IFlareSpacecraftInterface* Spacecraft = SpacecraftContainer->ShipInterfacePtr;
	if (Spacecraft)
	{
		ShipToRemove = Spacecraft;
	}
}

void SFlareFleetMenu::OnFleetSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer)
{
	UFlareFleet* Fleet = SpacecraftContainer->FleetPtr;
	if (Fleet)
	{
		FleetToAdd = Fleet;
	}
}

bool SFlareFleetMenu::IsSelectDisabled() const
{
	return FleetToAdd && FleetToAdd != SelectedFleet ? false : true;
}

bool SFlareFleetMenu::IsAddDisabled() const
{
	if (!FleetToAdd || !SelectedFleet || FleetToAdd == SelectedFleet)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SFlareFleetMenu::IsRemoveDisabled() const
{
	if (!ShipToRemove || !SelectedFleet || SelectedFleet->GetShips().Num() <= 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void SFlareFleetMenu::OnSelectFleet()
{
	ShipList->Reset();

	if (FleetToAdd)
	{
		for (int32 SpacecraftIndex = 0; SpacecraftIndex < FleetToAdd->GetShips().Num(); SpacecraftIndex++)
		{
			IFlareSpacecraftInterface* ShipCandidate = FleetToAdd->GetShips()[SpacecraftIndex];
			if (ShipCandidate && ShipCandidate->GetDamageSystem()->IsAlive())
			{
				ShipList->AddShip(ShipCandidate);
			}
		}

		SelectedFleet = FleetToAdd;
		FleetToAdd = NULL;
	}

	ShipList->RefreshList();
}

void SFlareFleetMenu::OnAddToFleet()
{
	check(SelectedFleet);
	check(FleetToAdd);

	SelectedFleet->Merge(FleetToAdd);

	UpdateFleetList();
	FleetToAdd = NULL;
}

void SFlareFleetMenu::OnRemoveFromFleet()
{
	check(SelectedFleet);
	check(ShipToRemove);

	SelectedFleet->RemoveShip(Cast<UFlareSimulatedSpacecraft>(ShipToRemove));

	UpdateFleetList();
	ShipToRemove = NULL;
}


#undef LOCTEXT_NAMESPACE

