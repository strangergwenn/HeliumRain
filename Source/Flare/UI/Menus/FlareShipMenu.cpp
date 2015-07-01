
#include "../../Flare.h"
#include "FlareShipMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlarePlayerController.h"
#include "../Widgets/FlarePartInfo.h"


#define LOCTEXT_NAMESPACE "FlareShipMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareShipMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	AFlarePlayerController* PC = MenuManager->GetPC();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
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
				SNew(SImage).Image(this, &SFlareShipMenu::GetTitleIcon)
			]

			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TitleFont)
				.Text(this, &SFlareShipMenu::GetTitleText)
			]

			// Quit
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("Dashboard", "Dashboard"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Dashboard, true))
				.OnClicked(this, &SFlareShipMenu::OnDashboardClicked)
			]

			// Close
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("Close", "Close"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Exit, true))
				.OnClicked(this, &SFlareShipMenu::OnExit)
			]
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(200, 30))
		[
			SNew(SImage).Image(&Theme.SeparatorBrush)
		]
		
		// Content
		+ SVerticalBox::Slot()
		[
			SNew(SScrollBox)
			.Style(&Theme.ScrollBoxStyle)
			.ScrollBarStyle(&Theme.ScrollBarStyle)

			+ SScrollBox::Slot()
			.HAlign(HAlign_Left)
			[
				SNew(SVerticalBox)

				// Object name
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.TitlePadding)
				[
					SAssignNew(ObjectName, STextBlock)
					.TextStyle(&Theme.SubTitleFont)
				]

				// Action box
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SAssignNew(ObjectActionMenu, SFlareSpacecraftInfo)
					.Player(PC)
					.NoInspect(true)
				]

				// Object class
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.TitlePadding)
				[
					SAssignNew(ObjectClassName, STextBlock)
					.TextStyle(&Theme.SubTitleFont)
				]

				// Object description
				+ SVerticalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoHeight()
				[
					SAssignNew(ObjectDescription, STextBlock)
					.TextStyle(&Theme.TextFont)
					.WrapTextAt(Theme.ContentWidth)
				]

				// Ship part characteristics
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.HAlign(HAlign_Fill)
					.WidthOverride(Theme.ContentWidth)
					[
						SAssignNew(PartCharacteristicBox, SHorizontalBox)
					]
				]

				// Ship customization panel
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(ShipCustomizationBox, SVerticalBox)
			
					// Components
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Engines
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.TitleButtonPadding)
						[
							SNew(SFlareRoundButton)
							.OnClicked(this, &SFlareShipMenu::ShowEngines)
							.Icon(this, &SFlareShipMenu::GetEngineIcon)
							.Text(this, &SFlareShipMenu::GetEngineText)
							.InvertedBackground(true)
							.Visibility(this, &SFlareShipMenu::GetEngineVisibility)
						]

						// RCS
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.TitleButtonPadding)
						[
							SNew(SFlareRoundButton)
							.OnClicked(this, &SFlareShipMenu::ShowRCSs)
							.Icon(this, &SFlareShipMenu::GetRCSIcon)
							.Text(this, &SFlareShipMenu::GetRCSText)
							.InvertedBackground(true)
							.Visibility(this, &SFlareShipMenu::GetEngineVisibility)
						]

						// Weapons
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew(WeaponButtonBox, SHorizontalBox)
						]
					]
				]

				// Ship part customization panel
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(ShipPartCustomizationBox, SVerticalBox)

					// Section title
					+ SVerticalBox::Slot()
					.Padding(Theme.TitlePadding)
					.AutoHeight()
					[
						SAssignNew(ShipPartPickerTitle, STextBlock)
						.Text(LOCTEXT("ShipParts", "AVAILABLE COMPONENTS"))
						.TextStyle(&FFlareStyleSet::GetDefaultTheme().SubTitleFont)
					]

					// Ship part picker
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(PartList, SListView< TSharedPtr<FInterfaceContainer> >)
						.ListItemsSource(&PartListDataShared)
						.SelectionMode(ESelectionMode::Single)
						.OnGenerateRow(this, &SFlareShipMenu::GeneratePartInfo)
						.OnSelectionChanged(this, &SFlareShipMenu::OnPartPicked)
					]

					// Button box
					+ SVerticalBox::Slot()
					.Padding(Theme.ContentPadding)
					.AutoHeight()
					[
						SAssignNew(BuyConfirmation, SFlareConfirmationBox)
						.ConfirmText(LOCTEXT("Confirm", "CONFIRM TRANSACTION"))
						.CancelText(LOCTEXT("BackTopShip", "BACK TO SHIP"))
						.OnConfirmed(this, &SFlareShipMenu::OnPartConfirmed)
						.OnCancelled(this, &SFlareShipMenu::OnPartCancelled)
					]
				]

				// Object list
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(ShipList, SFlareShipList)
					.MenuManager(MenuManager)
					.Title(LOCTEXT("DockedShips", "DOCKED SHIPS"))
				]
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareShipMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);

	TargetSpacecraft = NULL;
	RCSDescription = NULL;
	EngineDescription = NULL;
}

void SFlareShipMenu::Enter(IFlareSpacecraftInterface* Target, bool IsEditable)
{
	FLOG("SFlareShipMenu::Enter");
	SetEnabled(true);

	CanEdit = IsEditable;
	TargetSpacecraft = Target;
	TargetSpacecraftData = Target->Save();
	LoadTargetSpacecraft();

	// Move the viewer to the right
	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		PC->GetMenuPawn()->SetCameraOffset(FVector2D(100, -30));
	}

	// Fill the docking data
	TArray<IFlareSpacecraftInterface*> DockedShips = Target->GetDockingSystem()->GetDockedShips();
	for (int32 i = 0; i < DockedShips.Num(); i++)
	{
		AFlareSpacecraft* Spacecraft = Cast<AFlareSpacecraft>(DockedShips[i]);

		if (Spacecraft)
		{
			FLOGV("SFlareShipMenu::Enter %s", *Spacecraft->GetName());
		}
		if (DockedShips[i]->GetDamageSystem()->IsAlive())
		{
			ShipList->AddShip(DockedShips[i]);
		}
	}
	ShipList->RefreshList();

	SetVisibility(EVisibility::Visible);
}

void SFlareShipMenu::Exit()
{
	ObjectActionMenu->Hide();
	PartListData.Empty();
	PartList->RequestListRefresh();
	ShipList->Reset();

	TargetSpacecraft = NULL;
	RCSDescription = NULL;
	EngineDescription = NULL;

	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}

void SFlareShipMenu::LoadTargetSpacecraft()
{
	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		// Make the right boxes visible
		if (!CanEdit)
		{
			ObjectActionMenu->SetSpacecraft(TargetSpacecraft);
			ObjectActionMenu->Show();
		}
		ObjectName->SetVisibility(EVisibility::Visible);
		ShipPartCustomizationBox->SetVisibility(EVisibility::Collapsed);
		PartCharacteristicBox->SetVisibility(EVisibility::Collapsed);
		ShipCustomizationBox->SetVisibility(EVisibility::Visible);

		// Get the description data
		UFlareSpacecraftComponentsCatalog* Catalog = PC->GetGame()->GetShipPartsCatalog();
		const FFlareSpacecraftDescription* ShipDesc = PC->GetGame()->GetSpacecraftCatalog()->Get(TargetSpacecraftData->Identifier);
		if (ShipDesc)
		{
			ObjectName->SetText(FText::FromString(TargetSpacecraft->GetImmatriculation()));
			ObjectClassName->SetText(FText::FromString(ShipDesc->Name.ToString()));
			ObjectDescription->SetText(ShipDesc->Description);
			PC->GetMenuPawn()->ShowShip(ShipDesc, TargetSpacecraftData);
		}

		// Reset weapon data
		int32 WeaponCount = 0;
		WeaponButtonBox->ClearChildren();
		WeaponDescriptions.Empty();
		
		// Setup component descriptions
		for (int32 i = 0; i < TargetSpacecraftData->Components.Num(); i++)
		{
			FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(TargetSpacecraftData->Components[i].ComponentIdentifier);

			if (ComponentDescription->Type == EFlarePartType::Weapon)
			{
				// Register the weapon data
				TSharedPtr<int32> IndexPtr(new int32(WeaponCount));
				WeaponDescriptions.Add(Catalog->Get(ComponentDescription->Identifier));
				WeaponCount++;

				// Add the button itself
				WeaponButtonBox->AddSlot()
					.AutoWidth()
					.Padding(FFlareStyleSet::GetDefaultTheme().TitleButtonPadding)
					.VAlign(VAlign_Top)
					[
						SNew(SFlareRoundButton)
						.OnClicked(this, &SFlareShipMenu::ShowWeapons, IndexPtr)
						.Icon(this, &SFlareShipMenu::GetWeaponIcon, IndexPtr)
						.Text(this, &SFlareShipMenu::GetWeaponText, IndexPtr)
						.InvertedBackground(true)
					];

			} 
			else if (ComponentDescription->Type == EFlarePartType::RCS)
			{
				RCSDescription = Catalog->Get(ComponentDescription->Identifier);
			}
			else if (ComponentDescription->Type == EFlarePartType::OrbitalEngine)
			{
				EngineDescription = Catalog->Get(ComponentDescription->Identifier);
			}
		}

		WeaponButtonBox->SetVisibility(WeaponCount > 0 ? EVisibility::Visible : EVisibility::Collapsed);
	}
}

void SFlareShipMenu::LoadPart(FName InternalName)
{
	// Spawn the part
	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		const FFlareSpacecraftComponentDescription* PartDesc = PC->GetGame()->GetShipPartsCatalog()->Get(InternalName);
		if (PartDesc)
		{
			// Show part
			ObjectClassName->SetText(PartDesc->Name);
			ObjectDescription->SetText(PartDesc->Description);
			PC->GetMenuPawn()->ShowPart(PartDesc);

			// Build info
			SFlarePartInfo::BuildInfoBlock(PartCharacteristicBox, PartDesc);
		}
	}

	// Make the right box visible
	ObjectActionMenu->Hide();
	ObjectName->SetVisibility(EVisibility::Collapsed);
	ObjectDescription->SetVisibility(EVisibility::Visible);
	ShipPartCustomizationBox->SetVisibility(EVisibility::Visible);
	PartCharacteristicBox->SetVisibility(EVisibility::Visible);
	ShipCustomizationBox->SetVisibility(EVisibility::Collapsed);
}

void SFlareShipMenu::UpdatePartList(FFlareSpacecraftComponentDescription* SelectItem)
{
	ShipPartPickerTitle->SetVisibility(CanEdit ? EVisibility::Visible : EVisibility::Collapsed);
	PartList->SetVisibility(CanEdit ? EVisibility::Visible : EVisibility::Collapsed);
	PartListDataShared.Empty();
	PartList->RequestListRefresh();

	if (CanEdit)
	{
		int32 Index = PartListData.Find(SelectItem);
		ShipPartIndex = Index;
		CurrentPartIndex = Index;
		CurrentEquippedPartIndex = Index;

		// Copy items
		for (int32 i = 0; i < PartListData.Num(); i++)
		{
			PartListDataShared.AddUnique(FInterfaceContainer::New(PartListData[i]));
		}

		// Update list
		PartList->RequestListRefresh();
		PartList->SetSelection(PartListDataShared[Index]);
	}

	LoadPart(SelectItem->Identifier);
}


/*----------------------------------------------------
	Content callbacks
----------------------------------------------------*/

const FSlateBrush* SFlareShipMenu::GetTitleIcon() const
{
	if (TargetSpacecraft && TargetSpacecraft->IsStation())
	{
		return AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Station);
	}
	else
	{
		return AFlareMenuManager::GetMenuIcon(CanEdit ? EFlareMenu::MENU_ShipConfig : EFlareMenu::MENU_Ship);
	}
}

FText SFlareShipMenu::GetTitleText() const
{
	if (TargetSpacecraft && TargetSpacecraft->IsStation())
	{
		return  LOCTEXT("StationMenuTitle", "STATION");
	}
	else
	{
		return (CanEdit ? LOCTEXT("ShipConfigMenuTitle", "SHIP UPGRADE") : LOCTEXT("ShipMenuTitle", "SHIP"));
	}
}

EVisibility SFlareShipMenu::GetEngineVisibility() const
{
	return (TargetSpacecraft && !TargetSpacecraft->IsStation() ? EVisibility::Visible : EVisibility::Collapsed);
}

const FSlateBrush* SFlareShipMenu::GetRCSIcon() const
{
	return (RCSDescription ? &RCSDescription->MeshPreviewBrush : NULL);
}

FText SFlareShipMenu::GetRCSText() const
{
	FText Result;

	if (RCSDescription)
	{
		Result = RCSDescription->Name;
	}

	return Result;
}

const FSlateBrush* SFlareShipMenu::GetEngineIcon() const
{
	return (EngineDescription ? &EngineDescription->MeshPreviewBrush : NULL);
}

FText SFlareShipMenu::GetEngineText() const
{
	FText Result;

	if (EngineDescription)
	{
		Result = EngineDescription->Name;
	}

	return Result;
}

const FSlateBrush* SFlareShipMenu::GetWeaponIcon(TSharedPtr<int32> Index) const
{
	if (*Index < WeaponDescriptions.Num())
	{
		FFlareSpacecraftComponentDescription* Desc = WeaponDescriptions[*Index];
		return (Desc ? &Desc->MeshPreviewBrush : NULL);
	}
	return NULL;
}

FText SFlareShipMenu::GetWeaponText(TSharedPtr<int32> Index) const
{
	FText Result;

	if (*Index < WeaponDescriptions.Num())
	{
		FFlareSpacecraftComponentDescription* Desc = WeaponDescriptions[*Index];
		if (Desc)
		{
			Result = Desc->Name;
		}
	}

	return Result;
}

TSharedRef<ITableRow> SFlareShipMenu::GeneratePartInfo(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedPtr<SFlarePartInfo> Temp;
	TSharedPtr<SFlareListItem> TempWidget;

	// Create the row
	TSharedRef<ITableRow> res = SAssignNew(TempWidget, SFlareListItem, OwnerTable)
		.Width(5)
		.Height(2)
		.Content()
		[
			SAssignNew(Temp, SFlarePartInfo)
			.Description(Item->PartDescription)
			.ShowOwnershipInfo(true)
		];

	// Update the selection to force select the first item
	int32 Index = PartListData.Find(Item->PartDescription);
	if (Index == CurrentEquippedPartIndex)
	{
		Temp->SetOwned(true);
		TSharedPtr<SFlareListItem> ItemWidget = StaticCastSharedPtr<SFlareListItem>(TempWidget);
		if (ItemWidget.IsValid())
		{
			ItemWidget->SetSelected(true);
			PreviousSelection = ItemWidget;
		}
	}

	return res;
}


/*----------------------------------------------------
	Action callbacks
----------------------------------------------------*/

void SFlareShipMenu::ShowRCSs()
{
	PartListData.Empty();

	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		AFlareSpacecraft* Ship = Cast<AFlareSpacecraft>(TargetSpacecraft);
		PC->GetGame()->GetShipPartsCatalog()->GetRCSList(PartListData, Ship->GetDescription()->Size);
		UpdatePartList(Ship->GetRCSDescription());
	}
}

void SFlareShipMenu::ShowEngines()
{
	PartListData.Empty();

	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		AFlareSpacecraft* Ship = Cast<AFlareSpacecraft>(TargetSpacecraft);
		PC->GetGame()->GetShipPartsCatalog()->GetEngineList(PartListData, Ship->GetDescription()->Size);
		UpdatePartList(Ship->GetOrbitalEngineDescription());
	}
}

void SFlareShipMenu::ShowWeapons(TSharedPtr<int32> WeaponIndex)
{
	PartListData.Empty();
	CurrentWeaponIndex = *WeaponIndex;

	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		AFlareSpacecraft* Ship = Cast<AFlareSpacecraft>(TargetSpacecraft);
		PC->GetGame()->GetShipPartsCatalog()->GetWeaponList(PartListData, Ship->GetDescription()->Size);
		UpdatePartList(Ship->GetWeaponsSystem()->GetWeaponDescription(CurrentWeaponIndex));
	}
}

void SFlareShipMenu::OnPartPicked(TSharedPtr<FInterfaceContainer> Item, ESelectInfo::Type SelectInfo)
{
	int32 Index = PartListData.Find(Item->PartDescription);
	AFlarePlayerController* PC = MenuManager->GetPC();

	if (PC && Item->PartDescription && Index != CurrentPartIndex)
	{
		AFlareMenuPawn* Viewer = PC->GetMenuPawn();

		// Load the part
		if (Viewer)
		{
			Viewer->SetSlideDirection(Index > CurrentPartIndex);
			LoadPart(Item->PartDescription->Identifier);
		}
		CurrentPartIndex = Index;

		// Show the confirmation dialog
		if (CurrentPartIndex != ShipPartIndex)
		{
			BuyConfirmation->Show(PartListData[CurrentEquippedPartIndex]->Cost - Item->PartDescription->Cost);
		}
		else
		{
			BuyConfirmation->Hide();
		}
	}

	// De-select old
	if (PreviousSelection.IsValid())
	{
		PreviousSelection->SetSelected(false);
	}

	// Re-select new
	TSharedPtr<SFlareListItem> ItemWidget = StaticCastSharedPtr<SFlareListItem>(PartList->WidgetFromItem(Item));
	if (ItemWidget.IsValid())
	{
		ItemWidget->SetSelected(true);
		PreviousSelection = ItemWidget;
	}
}

void SFlareShipMenu::OnPartConfirmed()
{
	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		CurrentEquippedPartIndex = CurrentPartIndex;

		// Edit the correct save data property
		FFlareSpacecraftComponentDescription* PartDesc = PartListData[CurrentPartIndex];
		UFlareSpacecraftComponentsCatalog* Catalog = PC->GetGame()->GetShipPartsCatalog();
		int32 WeaponCount = 0;
		for (int32 i = 0; i < TargetSpacecraftData->Components.Num(); i++)
		{
			bool UpdatePart = false;
			FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(TargetSpacecraftData->Components[i].ComponentIdentifier);

			if (ComponentDescription->Type == PartDesc->Type)
			{
				if (ComponentDescription->Type == EFlarePartType::Weapon)
				{
					if (WeaponCount == CurrentWeaponIndex)
					{
						UpdatePart = true;
					}
					WeaponCount ++;
				}
				else
				{
					UpdatePart = true;

				}
			}

			if (UpdatePart)
			{
				TargetSpacecraftData->Components[i].ComponentIdentifier = PartDesc->Identifier;
			}
		}
		
		// Update the world ship if it exists
		if (TargetSpacecraft)
		{
			TargetSpacecraft->Load(*TargetSpacecraftData);
		}

		// Get back to the ship config
		BuyConfirmation->Hide();
		LoadTargetSpacecraft();
	}
}

void SFlareShipMenu::OnPartCancelled()
{
	BuyConfirmation->Hide();
	LoadTargetSpacecraft();
}

void SFlareShipMenu::OnDashboardClicked()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Dashboard);
}

void SFlareShipMenu::OnExit()
{
	MenuManager->CloseMenu();
}

#undef LOCTEXT_NAMESPACE
