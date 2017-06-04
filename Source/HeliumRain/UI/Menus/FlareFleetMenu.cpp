
#include "../../Flare.h"
#include "FlareFleetMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareCompany.h"
#include "../../Game/FlareFleet.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../Components/FlareRoundButton.h"

#include "SComplexGradient.h"


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
	FleetToEdit = NULL;
	ShipToRemove = NULL;

	// Gradient
	TArray<FLinearColor> HueGradientColors;
	for (int32 i = 0; i < 7; ++i)
	{
		HueGradientColors.Add(FLinearColor((i % 6) * 60.f, 1.f, 1.f).HSVToLinearRGB());
	}

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

			// Fleet list & tools
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			[
				SNew(SVerticalBox)
				
				// Fleet details
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.TitlePadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.SubTitleFont)
					.Text(LOCTEXT("ManageFleet", "Fleet details"))
					.Visibility(this, &SFlareFleetMenu::GetEditVisibility)
				]
		
				// Fleet tools
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(SHorizontalBox)

					// Name field
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Center)
					.Padding(Theme.SmallContentPadding)
					[
						SAssignNew(EditFleetName, SEditableText)
						.Style(&Theme.TextInputStyle)
						.Visibility(this, &SFlareFleetMenu::GetEditVisibility)
					]

					// Confirm name
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Right)
					.Padding(Theme.SmallContentPadding)
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Width(4)
						.Icon(FFlareStyleSet::GetIcon("OK"))
						.Text(LOCTEXT("Rename", "Rename"))
						.HelpText(this, &SFlareFleetMenu::GetRenameHintText)
						.OnClicked(this, &SFlareFleetMenu::OnRenameFleet)
						.IsDisabled(this, &SFlareFleetMenu::IsRenameDisabled)
						.Visibility(this, &SFlareFleetMenu::GetEditVisibility)
					]

					// Finish
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Right)
					.Padding(Theme.SmallContentPadding)
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Width(4)
						.Icon(FFlareStyleSet::GetIcon("Stop"))
						.Text(LOCTEXT("DoneEditing", "Back"))
						.HelpText(LOCTEXT("DoneEditingInfo", "Finish editing this fleet"))
						.OnClicked(this, &SFlareFleetMenu::OnEditFinished)
						.Visibility(this, &SFlareFleetMenu::GetEditVisibility)
					]
				]

				// Color box
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(SOverlay)

					+ SOverlay::Slot()
					.Padding(FMargin(4.0f, 0.0f))
					[
						SNew(SComplexGradient)
						.Visibility(this, &SFlareFleetMenu::GetEditVisibility)
						.GradientColors(HueGradientColors)
						.Orientation(Orient_Vertical)
					]

					+ SOverlay::Slot()
					[
						SNew(SSlider)
						.IndentHandle(false)
						.Orientation(Orient_Horizontal)
						.Visibility(this, &SFlareFleetMenu::GetEditVisibility)
						.SliderBarColor(FLinearColor::Transparent)
						.Value(this, &SFlareFleetMenu::GetColorSpinBoxValue)
						.OnValueChanged(this, &SFlareFleetMenu::OnColorSpinBoxValueChanged)
					]
				]

				// Fleet list
				+ SVerticalBox::Slot()
				[
					SNew(SScrollBox)
					.Style(&Theme.ScrollBoxStyle)
					.ScrollBarStyle(&Theme.ScrollBarStyle)

					+ SScrollBox::Slot()
					[
						SAssignNew(FleetList, SFlareList)
						.MenuManager(MenuManager)
						.OnItemSelected(this, &SFlareFleetMenu::OnFleetSelected)
					]
				]
			]

			// Fleet details
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			[
				SNew(SVerticalBox)
				
				// Fleet details
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.TitlePadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.SubTitleFont)
					.Text(LOCTEXT("ManageShips", "Fleet composition"))
					.Visibility(this, &SFlareFleetMenu::GetEditVisibility)
				]

				// Add & remove
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(SHorizontalBox)

					// Add
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.Padding(Theme.SmallContentPadding)
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Width(4)
						.Icon(FFlareStyleSet::GetIcon("MoveRight"))
						.Text(LOCTEXT("AddToFleet", "Merge fleet"))
						.HelpText(this, &SFlareFleetMenu::GetAddHintText)
						.IsDisabled(this, &SFlareFleetMenu::IsAddDisabled)
						.OnClicked(this, &SFlareFleetMenu::OnAddToFleet)
						.Visibility(this, &SFlareFleetMenu::GetEditVisibility)
					]

					// Add
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.Padding(Theme.SmallContentPadding)
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Width(4)
						.Icon(FFlareStyleSet::GetIcon("MoveLeft"))
						.Text(LOCTEXT("RemoveFromFleet", "Remove ship"))
						.HelpText(this, &SFlareFleetMenu::GetRemoveHintText)
						.IsDisabled(this, &SFlareFleetMenu::IsRemoveDisabled)
						.OnClicked(this, &SFlareFleetMenu::OnRemoveFromFleet)
						.Visibility(this, &SFlareFleetMenu::GetEditVisibility)
					]
				]

				// Ship list
				+ SVerticalBox::Slot()
				[
					SNew(SScrollBox)
					.Style(&Theme.ScrollBoxStyle)
					.ScrollBarStyle(&Theme.ScrollBarStyle)

					+ SScrollBox::Slot()
					[
						SAssignNew(ShipList, SFlareList)
						.MenuManager(MenuManager)
						.OnItemSelected(this, &SFlareFleetMenu::OnSpacecraftSelected)
						.UseCompactDisplay(true)
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
	FleetToEdit = TargetFleet;

	// We are in edit mode
	if (FleetToEdit)
	{
		FleetList->SetTitle(LOCTEXT("OtherFleetsListTitle", "Other fleets"));
		ShipList->SetUseCompactDisplay(true);
		EditFleetName->SetText(FleetToEdit->GetFleetName());
	}

	// We are in preview mode
	else
	{
		FleetList->SetTitle(LOCTEXT("AllFleetsListTitle", "Fleets"));
		ShipList->SetUseCompactDisplay(false);
	}

	UpdateShipList(FleetToEdit);
	UpdateFleetList();
}

void SFlareFleetMenu::Exit()
{
	SetEnabled(false);

	ShipList->Reset();
	FleetList->Reset();

	FleetToEdit = NULL;
	FleetToAdd = NULL;
	ShipToRemove = NULL;

	SetVisibility(EVisibility::Collapsed);
}

void SFlareFleetMenu::UpdateFleetList()
{
	FleetList->Reset();

	int32 FleetCount = MenuManager->GetPC()->GetCompany()->GetCompanyFleets().Num();
	FLOGV("SFlareFleetMenu::UpdateFleetList : found %d fleets", FleetCount);

	for (int32 FleetIndex = 0; FleetIndex < FleetCount; FleetIndex++)
	{
		UFlareFleet* Fleet = MenuManager->GetPC()->GetCompany()->GetCompanyFleets()[FleetIndex];
		if (Fleet && Fleet->GetShips().Num() && Fleet != FleetToEdit)
		{
			FleetList->AddFleet(Fleet);
		}
	}

	FleetList->RefreshList();
}

void SFlareFleetMenu::UpdateShipList(UFlareFleet* Fleet)
{
	ShipList->Reset();

	if (Fleet)
	{
		int32 ShipCount = Fleet->GetShips().Num();
		FLOGV("SFlareFleetMenu::UpdateShipList : found %d ships", ShipCount);

		for (int32 SpacecraftIndex = 0; SpacecraftIndex < ShipCount; SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* ShipCandidate = Fleet->GetShips()[SpacecraftIndex];
			if (ShipCandidate && ShipCandidate->GetDamageSystem()->IsAlive())
			{
				ShipList->AddShip(ShipCandidate);
			}
		}

		ShipList->SetTitle(Fleet->GetFleetName());
	}
	else
	{
		ShipList->SetTitle(LOCTEXT("NoFleetSelectedTitle", "No fleet selected"));
	}

	ShipList->RefreshList();
}


/*----------------------------------------------------
	Content callbacks
----------------------------------------------------*/

EVisibility SFlareFleetMenu::GetEditVisibility() const
{
	return FleetToEdit ? EVisibility::Visible : EVisibility::Collapsed;
}

bool SFlareFleetMenu::IsAddDisabled() const
{
	FText Unused;
	if (!FleetToAdd || !FleetToEdit || FleetToAdd == FleetToEdit || !FleetToEdit->CanMerge(FleetToAdd, Unused))
	{
		return true;
	}
	else
	{
		return false;
	}
}

FText SFlareFleetMenu::GetAddHintText() const
{
	if (FleetToAdd && FleetToEdit)
	{
		FText Reason;
		if (FleetToEdit->CanMerge(FleetToAdd, Reason))
		{
			return LOCTEXT("AddToFleetInfo", "Merge selected fleet with this one");
		}
		else
		{
			return Reason;
		}
	}
	else
	{
		return LOCTEXT("NoFleetToEditToAddToInfo", "No fleet selected for merging");
	}
}

bool SFlareFleetMenu::IsRemoveDisabled() const
{
	if (ShipToRemove == NULL
		|| ShipToRemove == MenuManager->GetPC()->GetPlayerShip()
		|| FleetToEdit == NULL
		|| FleetToEdit->GetShips().Num() <= 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

FText SFlareFleetMenu::GetRemoveHintText() const
{
	if (ShipToRemove)
	{
		if(ShipToRemove == MenuManager->GetPC()->GetPlayerShip())
		{
			return LOCTEXT("CantRemovePlayerShipFromFleetInfo", "Can't remove the ship you are currenly flying");
		}
		else if (FleetToEdit->GetShips().Num() > 1)
		{
			return LOCTEXT("RemoveFromFleetInfo", "Remove the selected ship from the fleet");
		}
		else
		{
			return LOCTEXT("CantRemoveFromFleetInfo", "Can't remove the only ship in the fleet");
		}
	}
	else
	{
		return LOCTEXT("NoFleetToEditToRemoveFromInfo", "No ship selected for removal");
	}
}

bool SFlareFleetMenu::IsRenameDisabled() const
{
	if (!FleetToEdit)
	{
		return true;
	}
	else
	{
		if (MenuManager->GetPC()->GetPlayerFleet() == FleetToEdit)
		{
			return true;
		}

		return false;
	}
}

FText SFlareFleetMenu::GetRenameHintText() const
{
	if (FleetToEdit)
	{
		if (MenuManager->GetPC()->GetPlayerFleet() == FleetToEdit)
		{
			return LOCTEXT("NoPlayeFleetRenameInfo", "Can't rename the player fleet");
		}
		else
		{
			return LOCTEXT("FleetRenameInfo", "Rename this fleet");
		}
	}
	else
	{
		return LOCTEXT("NoFleetToRenameInfo", "No fleet is selected for renaming");
	}
}

float SFlareFleetMenu::GetColorSpinBoxValue() const
{
	return FleetToEdit->GetFleetColor().LinearRGBToHSV().R / 360.0f;
}


/*----------------------------------------------------
	Action callbacks
----------------------------------------------------*/

void SFlareFleetMenu::OnSpacecraftSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer)
{
	UFlareSimulatedSpacecraft* Spacecraft = SpacecraftContainer->SpacecraftPtr;
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
		// Fleet editing
		if (FleetToEdit)
		{
			FleetToAdd = Fleet;
			FLOGV("SFlareFleetMenu::OnFleetSelected : fleet to add/edit '%s'", *Fleet->GetFleetName().ToString());
		}

		// Simple preview : list ships
		else
		{
			UpdateShipList(Fleet);
		}
	}
}

void SFlareFleetMenu::OnEditFinished()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Fleet);
}

void SFlareFleetMenu::OnAddToFleet()
{
	FCHECK(FleetToEdit);
	FCHECK(FleetToAdd);

	FLOGV("SFlareFleetMenu::OnAddToFleet : adding '%s'", *FleetToAdd->GetFleetName().ToString());
	FleetToEdit->Merge(FleetToAdd);

	UpdateShipList(FleetToEdit);
	UpdateFleetList();
	FleetToAdd = NULL;
	ShipToRemove = NULL;
}

void SFlareFleetMenu::OnRemoveFromFleet()
{
	FCHECK(FleetToEdit);
	FCHECK(ShipToRemove);

	FLOGV("SFlareFleetMenu::OnRemoveFromFleet : removing '%s'", *ShipToRemove->GetImmatriculation().ToString());
	FleetToEdit->RemoveShip(ShipToRemove);

	UpdateShipList(FleetToEdit);
	UpdateFleetList();
	FleetToAdd = NULL;
	ShipToRemove = NULL;
}

void SFlareFleetMenu::OnRenameFleet()
{
	FCHECK(FleetToEdit);

	FText NewText = EditFleetName->GetText();
	FLOGV("SFlareFleetMenu::OnRenameFleet : renaming as '%s'", *NewText.ToString());

	FleetToEdit->SetFleetName(NewText);
}

void SFlareFleetMenu::SFlareFleetMenu::OnColorSpinBoxValueChanged(float NewValue)
{
	FLinearColor Color = FLinearColor(360.0f * NewValue, 1.0f, 1.0f, 1.0f).HSVToLinearRGB();
	FleetToEdit->SetFleetColor(Color);
}

#undef LOCTEXT_NAMESPACE

