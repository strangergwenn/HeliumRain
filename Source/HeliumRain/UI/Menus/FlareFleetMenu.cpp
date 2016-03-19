
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

				// Fleet list
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				[
					SNew(SBox)
					.HAlign(HAlign_Fill)
					.WidthOverride(Theme.ContentWidth)
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Right)
						[
							SNew(SVerticalBox)

							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.SmallContentPadding)
							[
								SNew(SFlareButton)
								.Width(8)
								.Icon(FFlareStyleSet::GetIcon("OK"))
								.Text(LOCTEXT("SelectFleet", "Select fleet"))
								.HelpText(LOCTEXT("SelectFleetInfo", "Select"))
								.IsDisabled(this, &SFlareFleetMenu::IsSelectDisabled)
								.OnClicked(this, &SFlareFleetMenu::OnSelectFleet)
							]

							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.SmallContentPadding)
							[
								SNew(SFlareButton)
								.Width(8)
								.Icon(FFlareStyleSet::GetIcon("MoveRight"))
								.Text(LOCTEXT("AddToFleet", "Merge with selected"))
								.HelpText(LOCTEXT("AddToFleetInfo", "Merge this fleet or ship with the current fleet"))
								.IsDisabled(this, &SFlareFleetMenu::IsAddDisabled)
								.OnClicked(this, &SFlareFleetMenu::OnAddToFleet)
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(FleetList, SFlareShipList)
							.MenuManager(MenuManager)
							.Title(LOCTEXT("Unassigned", "Unassigned fleets & ships"))
							.OnItemSelected(this, &SFlareFleetMenu::OnFleetSelected)
							.UseCompactDisplay(true)
						]
					]
				]

				// Ship list
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				[
					SNew(SBox)
					.HAlign(HAlign_Right)
					.WidthOverride(Theme.ContentWidth)
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Left)
						[
							SNew(SVerticalBox)
							
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.SmallContentPadding)
							[
								SNew(SHorizontalBox)

								// Name field
								+ SHorizontalBox::Slot()
								[
									SAssignNew(EditFleetName, SEditableText)
									.Style(&Theme.TextInputStyle)
								]

								// Confirm
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.HAlign(HAlign_Right)
								[
									SNew(SFlareButton)
									.Width(3)
									.Icon(FFlareStyleSet::GetIcon("OK"))
									.Text(LOCTEXT("Rename", "Rename"))
									.HelpText(LOCTEXT("ChangeNameInfo", "Rename"))
									.OnClicked(this, &SFlareFleetMenu::OnRenameFleet)
									.IsDisabled(this, &SFlareFleetMenu::IsRenameDisabled)
								]
							]

							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.SmallContentPadding)
							[
								SNew(SFlareButton)
								.Width(8)
								.Icon(FFlareStyleSet::GetIcon("MoveLeft"))
								.Text(LOCTEXT("RemoveFromFleet", "Remove from selected"))
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
							.Title(LOCTEXT("CurrentFleet", "Selected fleet"))
							.OnItemSelected(this, &SFlareFleetMenu::OnSpacecraftSelected)
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
	
	FleetToAdd = NULL;
	ShipToRemove = NULL;
	SelectedFleet = NULL;

	ShipList->Reset();
	UpdateFleetList();
	EditFleetName->SetText(LOCTEXT("NoFleet", "No fleet selected"));
}

void SFlareFleetMenu::Exit()
{
	SetEnabled(false);

	ShipList->Reset();
	FleetList->Reset();

	FleetToAdd = NULL;
	ShipToRemove = NULL;
	SelectedFleet = NULL;

	SetVisibility(EVisibility::Collapsed);
}

void SFlareFleetMenu::UpdateFleetList()
{
	AFlarePlayerController* PC = MenuManager->GetPC();

	FleetList->Reset();

	int32 FleetCount = PC->GetCompany()->GetCompanyFleets().Num();
	FLOGV("SFlareFleetMenu::UpdateFleetList : found %d fleets", FleetCount);

	for (int32 FleetIndex = 0; FleetIndex < FleetCount; FleetIndex++)
	{
		UFlareFleet* Fleet = PC->GetCompany()->GetCompanyFleets()[FleetIndex];
		if (Fleet && Fleet->GetShips().Num() && !Fleet->GetShips()[0]->IsAssignedToSector() && Fleet != SelectedFleet)
		{
			FleetList->AddFleet(Fleet);
		}
	}

	FleetList->RefreshList();
}

void SFlareFleetMenu::UpdateShipList()
{
	ShipList->Reset();

	int32 ShipCount = SelectedFleet->GetShips().Num();
	FLOGV("SFlareFleetMenu::UpdateShipList : found %d ships", ShipCount);

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < ShipCount; SpacecraftIndex++)
	{
		IFlareSpacecraftInterface* ShipCandidate = SelectedFleet->GetShips()[SpacecraftIndex];
		if (ShipCandidate && ShipCandidate->GetDamageSystem()->IsAlive())
		{
			ShipList->AddShip(ShipCandidate);
		}
	}
	
	ShipList->RefreshList();
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
	if (!FleetToAdd || !SelectedFleet || FleetToAdd == SelectedFleet || !SelectedFleet->CanMerge(FleetToAdd))
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

bool SFlareFleetMenu::IsRenameDisabled() const
{
	if (!SelectedFleet)
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
	check(FleetToAdd);

	SelectedFleet = FleetToAdd;
	EditFleetName->SetText(SelectedFleet->GetFleetName());
	FLOGV("SFlareFleetMenu::OnSelectFleet : selected '%s'", *SelectedFleet->GetFleetName().ToString());

	UpdateShipList();
	FleetToAdd = NULL;
	ShipToRemove = NULL;
}

void SFlareFleetMenu::OnAddToFleet()
{
	check(SelectedFleet);
	check(FleetToAdd);

	FLOGV("SFlareFleetMenu::OnAddToFleet : adding '%s'", *FleetToAdd->GetFleetName().ToString());
	SelectedFleet->Merge(FleetToAdd);

	UpdateShipList();
	UpdateFleetList();
	FleetToAdd = NULL;
	ShipToRemove = NULL;
}

void SFlareFleetMenu::OnRemoveFromFleet()
{
	check(SelectedFleet);
	check(ShipToRemove);

	FLOGV("SFlareFleetMenu::OnRemoveFromFleet : removing '%s'", *ShipToRemove->GetImmatriculation().ToString());
	SelectedFleet->RemoveShip(Cast<UFlareSimulatedSpacecraft>(ShipToRemove));

	UpdateShipList();
	UpdateFleetList();
	FleetToAdd = NULL;
	ShipToRemove = NULL;
}

void SFlareFleetMenu::OnRenameFleet()
{
	check(SelectedFleet);

	FText NewText = EditFleetName->GetText();
	FLOGV("SFlareFleetMenu::OnRenameFleet : renaming as '%s'", *NewText.ToString());

	SelectedFleet->SetFleetName(NewText);
}


#undef LOCTEXT_NAMESPACE

