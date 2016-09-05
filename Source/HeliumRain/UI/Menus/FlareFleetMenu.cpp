
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
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SVerticalBox)

		// Content block
		+ SVerticalBox::Slot()
		.Padding(Theme.ContentPadding)
		.HAlign(HAlign_Center)
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
							.Text(this, &SFlareFleetMenu::GetSelectText)
							.HelpText(LOCTEXT("SelectFleetInfo", "Select this fleet to rename it or change its composition"))
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
							.Text(this, &SFlareFleetMenu::GetAddText)
							.HelpText(LOCTEXT("AddToFleetInfo", "Merge this fleet or ship with the current fleet"))
							.IsDisabled(this, &SFlareFleetMenu::IsAddDisabled)
							.OnClicked(this, &SFlareFleetMenu::OnAddToFleet)
							.Visibility(this, &SFlareFleetMenu::GetEditVisibility)
						]
					]

					+ SVerticalBox::Slot()
					[
						SNew(SScrollBox)
						.Style(&Theme.ScrollBoxStyle)
						.ScrollBarStyle(&Theme.ScrollBarStyle)

						+ SScrollBox::Slot()
						[
							SAssignNew(FleetList, SFlareShipList)
							.MenuManager(MenuManager)
							.Title(LOCTEXT("Unassigned", "Unassigned fleets & ships"))
							.OnItemSelected(this, &SFlareFleetMenu::OnFleetSelected)
							.UseCompactDisplay(true)
						]
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

							// Confirm
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SFlareButton)
								.Width(4)
								.Icon(FFlareStyleSet::GetIcon("OK"))
								.Text(LOCTEXT("Rename", "Rename fleet"))
								.HelpText(LOCTEXT("ChangeNameInfo", "Rename the selected fleet"))
								.OnClicked(this, &SFlareFleetMenu::OnRenameFleet)
								.IsDisabled(this, &SFlareFleetMenu::IsRenameDisabled)
								.Visibility(this, &SFlareFleetMenu::GetEditVisibility)
							]

							// Name field
							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Fill)
							.Padding(Theme.SmallContentPadding)
							[
								SAssignNew(EditFleetName, SEditableText)
								.Style(&Theme.TextInputStyle)
								.Visibility(this, &SFlareFleetMenu::GetEditVisibility)
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.SmallContentPadding)
						.HAlign(HAlign_Left)
						[
							SNew(SFlareButton)
							.Width(8)
							.Icon(FFlareStyleSet::GetIcon("MoveLeft"))
							.Text(this, &SFlareFleetMenu::GetRemoveText)
							.HelpText(LOCTEXT("RemoveFromFleetInfo", "Remove this ship from the fleet"))
							.IsDisabled(this, &SFlareFleetMenu::IsRemoveDisabled)
							.OnClicked(this, &SFlareFleetMenu::OnRemoveFromFleet)
							.Visibility(this, &SFlareFleetMenu::GetEditVisibility)
						]
					]
									
					+ SVerticalBox::Slot()
					[
						SNew(SScrollBox)
						.Style(&Theme.ScrollBoxStyle)
						.ScrollBarStyle(&Theme.ScrollBarStyle)

						+ SScrollBox::Slot()
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
		if (Fleet && Fleet->GetShips().Num() && Fleet != SelectedFleet)
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
		UFlareSimulatedSpacecraft* ShipCandidate = SelectedFleet->GetShips()[SpacecraftIndex];
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
	UFlareSimulatedSpacecraft* Spacecraft = SpacecraftContainer->ShipInterfacePtr;
	if (Spacecraft)
	{
		ShipToRemove = Spacecraft;
		FLOGV("SFlareFleetMenu::OnSpacecraftSelected : ship to remove '%s'", *Spacecraft->GetImmatriculation().ToString());
	}
}

void SFlareFleetMenu::OnFleetSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer)
{
	UFlareFleet* Fleet = SpacecraftContainer->FleetPtr;
	if (Fleet)
	{
		FleetToAdd = Fleet;
		FLOGV("SFlareFleetMenu::OnFleetSelected : fleet to add/edit '%s'", *Fleet->GetFleetName().ToString());
	}
}

EVisibility SFlareFleetMenu::GetEditVisibility() const
{
	return SelectedFleet ? EVisibility::Visible : EVisibility::Hidden;
}

FText SFlareFleetMenu::GetSelectText() const
{
	if (!FleetToAdd || FleetToAdd == SelectedFleet)
	{
		return LOCTEXT("NoSelectedFleet", "Edit selection");
	}
	else if (FleetToAdd->IsTraveling())
	{
		return LOCTEXT("CantEditTravelFleet", "Can't edit travelling fleets");
	}
	else if (FleetToAdd->GetCurrentSector()->IsPlayerBattleInProgress())
	{
		return LOCTEXT("CantEditBattleFleet", "Can't edit fleets during battles");
	}
	else
	{
		return LOCTEXT("SelectFleet", "Edit selection");
	}
}

FText SFlareFleetMenu::GetAddText() const
{
	if (FleetToAdd && SelectedFleet)
	{
		FText Reason;
		if (SelectedFleet->CanMerge(FleetToAdd, Reason))
		{
			return LOCTEXT("AddToFleet", "Add selection to fleet");
		}
		else
		{
			return Reason;
		}
	}

	return LOCTEXT("NoSelectedFleetToAddTo", "Add selection to fleet");
}

FText SFlareFleetMenu::GetRemoveText() const
{
	if (ShipToRemove)
	{
		if (SelectedFleet->GetShips().Num() > 1)
		{
			return LOCTEXT("RemoveFromFleet", "Remove selected ship");
		}
		else
		{
			return LOCTEXT("CantRemoveFromFleet", "Can't remove the only ship");
		}
	}

	return LOCTEXT("NoSelectedFleetToRemoveFrom", "Remove selected ship");
}

bool SFlareFleetMenu::IsSelectDisabled() const
{
	return (!FleetToAdd
	 || FleetToAdd == SelectedFleet
	 || FleetToAdd->IsTraveling()
	 || FleetToAdd->GetCurrentSector()->IsPlayerBattleInProgress());
}

bool SFlareFleetMenu::IsAddDisabled() const
{
	FText Unused;
	if (!FleetToAdd || !SelectedFleet || FleetToAdd == SelectedFleet || !SelectedFleet->CanMerge(FleetToAdd, Unused))
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
	UpdateFleetList();
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
	SelectedFleet->RemoveShip(ShipToRemove);

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

