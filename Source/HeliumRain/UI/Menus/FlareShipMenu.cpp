
#include "FlareShipMenu.h"

#include "../../Flare.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareSector.h"
#include "../../Game/FlareGameTools.h"

#include "../../Data/FlareSpacecraftCatalog.h"
#include "../../Data/FlareResourceCatalog.h"

#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlarePlayerController.h"

#include "../Components/FlareRoundButton.h"
#include "../Components/FlarePartInfo.h"
#include "../Components/FlareFactoryInfo.h"


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
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SVerticalBox)
		
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
					.OwnerWidget(this)
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

				// Factory list
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SAssignNew(FactoryList, SVerticalBox)
				]

				// Factory box
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SAssignNew(UpgradeBox, SVerticalBox)
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
			
					// Edit info
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(LOCTEXT("EditInfoText", "Click on a part to exchange it. Please return only loaded, functional parts. Combat damage will void the warranty."))
						.Visibility(this, &SFlareShipMenu::GetEditVisibility)
					]

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
							.HelpText(LOCTEXT("EngineInfo", "Inspect the current orbital engines"))
							.InvertedBackground(true)
							.Visibility(this, &SFlareShipMenu::GetEngineVisibility)
							.HighlightColor(this, &SFlareShipMenu::GetEnginesHealthColor)
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
							.HelpText(LOCTEXT("RCSInfo", "Inspect the current attitude control thrusters (RCS)"))
							.InvertedBackground(true)
							.Visibility(this, &SFlareShipMenu::GetEngineVisibility)
							.HighlightColor(this, &SFlareShipMenu::GetRCSHealthColor)
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
						.Text(LOCTEXT("ShipParts", "Available components"))
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

					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.TitlePadding)
					[
						SAssignNew(UpgradeTitle, STextBlock)
						.TextStyle(&Theme.SubTitleFont)
						.Text(LOCTEXT("TransactionTitle", "Upgrade component"))
					]

					// Can't upgrade
					+ SVerticalBox::Slot()
					.Padding(Theme.ContentPadding)
					.AutoHeight()
					[
						SAssignNew(CantUpgradeReason, STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(LOCTEXT("CantUpgradeDamaged", "This system has been damaged and can't be exchanged. Please repair your ships through the Sector menu."))
					]

					// Button box
					+ SVerticalBox::Slot()
					.Padding(Theme.ContentPadding)
					.AutoHeight()
					[
						SAssignNew(BuyConfirmation, SFlareConfirmationBox)
						.ConfirmText(LOCTEXT("Confirm", "Upgrade component"))
						.CancelText(LOCTEXT("BackTopShip", "Back to ship"))
						.OnConfirmed(this, &SFlareShipMenu::OnPartConfirmed)
						.OnCancelled(this, &SFlareShipMenu::OnPartCancelled)
						.UpgradeBehavior(true)
						.PC(PC)
					]
				]

				// Object list
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Left)
				[
					SAssignNew(ShipList, SFlareList)
					.MenuManager(MenuManager)
					.Title(LOCTEXT("DockedShips", "Docked ships"))
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
	SetVisibility(EVisibility::Collapsed);

	TargetSpacecraft = NULL;
	RCSDescription = NULL;
	EngineDescription = NULL;
}

void SFlareShipMenu::Enter(UFlareSimulatedSpacecraft* Target, bool IsEditable)
{
	// Info
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	FLOGV("SFlareShipMenu::Enter : %s, CanEdit=%d", *Target->GetImmatriculation().ToString(), IsEditable);

	// Load data
	CanEdit = IsEditable;
	TargetSpacecraft = Target;
	TargetSpacecraftData = Target->Save();
	LoadTargetSpacecraft();
	UpdateFactoryList();
	UpdateUpgradeBox();

	// Move the viewer to the right
	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		PC->GetMenuPawn()->SetCameraOffset(FVector2D(100, -30));
	}

	// Is the docking list visible ?
	UFlareSpacecraftDockingSystem* DockSystem = NULL;
	if (TargetSpacecraft->IsActive())
	{
		DockSystem = TargetSpacecraft->GetActive()->GetDockingSystem();
	}
	
	// Fill the docking list if it is visible
	if (TargetSpacecraft->IsStation() && DockSystem && DockSystem->GetDockCount() > 0)
	{
		ShipList->SetVisibility(EVisibility::Visible);

		TArray<AFlareSpacecraft*> DockedShips = DockSystem->GetDockedShips();
		for (int32 i = 0; i < DockedShips.Num(); i++)
		{
			AFlareSpacecraft* Spacecraft = DockedShips[i];

			if (Spacecraft)
			{
				FLOGV("SFlareShipMenu::Enter : Found docked ship %s", *Spacecraft->GetName());
			}
			if (DockedShips[i]->GetParent()->GetDamageSystem()->IsAlive())
			{
				ShipList->AddShip(DockedShips[i]->GetParent());
			}
		}

		ShipList->RefreshList();
	}
	else
	{
		ShipList->SetVisibility(EVisibility::Collapsed);
	}
}

void SFlareShipMenu::Exit()
{
	ObjectActionMenu->Hide();
	PartListData.Empty();
	PartList->RequestListRefresh();
	ShipList->Reset();
	ShipList->SetVisibility(EVisibility::Collapsed);

	FactoryList->ClearChildren();
	UpgradeBox->ClearChildren();

	TargetSpacecraft = NULL;
	RCSDescription = NULL;
	EngineDescription = NULL;

	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
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
		ShipList->SetVisibility(CanEdit ? EVisibility::Collapsed : EVisibility::Visible);

		// Get the description data
		UFlareSpacecraftComponentsCatalog* Catalog = PC->GetGame()->GetShipPartsCatalog();
		const FFlareSpacecraftDescription* ShipDesc = PC->GetGame()->GetSpacecraftCatalog()->Get(TargetSpacecraftData->Identifier);
		if (ShipDesc)
		{
			// Name
			FText Prefix = TargetSpacecraft->IsStation() ? LOCTEXT("Station", "Station") : LOCTEXT("Ship", "Ship");
			FText Immatriculation = UFlareGameTools::DisplaySpacecraftName(TargetSpacecraft);
			ObjectName->SetText(FText::Format(LOCTEXT("ObjectNameFormat", "{0} : {1}"), Prefix, Immatriculation));
			ObjectClassName->SetText(ShipDesc->Name);

			// Description
			FText SpacecraftDescription = ShipDesc->Description;
			if (TargetSpacecraft->IsStation())
			{
				if (TargetSpacecraft->HasCapability(EFlareSpacecraftCapability::Consumer))
				{
					SpacecraftDescription = FText::Format(LOCTEXT("ObjectDescriptionConsumerFormat", "\u2022 This station can buy consumer resources !\n{0}"), SpacecraftDescription);
				}
				if (TargetSpacecraft->HasCapability(EFlareSpacecraftCapability::Upgrade))
				{
					SpacecraftDescription = FText::Format(LOCTEXT("ObjectDescriptionUpgradeFormat", "\u2022 You can upgrade your ship at this station !\n{0}"), SpacecraftDescription);
				}
			}
			ObjectDescription->SetText(SpacecraftDescription);
			
			// Show the ship if it's not a substation
			if (!ShipDesc->IsSubstation)
			{
				PC->GetMenuPawn()->ShowShip(TargetSpacecraft);
			}
		}

		// Setup weapon descriptions
		WeaponDescriptions.Empty();
		for (int32 GroupIndex = 0; GroupIndex < ShipDesc->WeaponGroups.Num(); GroupIndex++)
		{
			FName SlotName = UFlareSimulatedSpacecraftWeaponsSystem::GetSlotIdentifierFromWeaponGroupIndex(ShipDesc, GroupIndex);

			for (int32 i = 0; i < TargetSpacecraftData->Components.Num(); i++)
			{
				FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(TargetSpacecraftData->Components[i].ComponentIdentifier);
				if (ComponentDescription->Type == EFlarePartType::Weapon && TargetSpacecraftData->Components[i].ShipSlotIdentifier == SlotName)
				{
					FCHECK(ComponentDescription);
					WeaponDescriptions.Add(ComponentDescription);
					break;
				}
			}
		}

		// Setup engine descriptions
		for (int32 i = 0; i < TargetSpacecraftData->Components.Num(); i++)
		{
			FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(TargetSpacecraftData->Components[i].ComponentIdentifier);
			if (ComponentDescription->Type == EFlarePartType::RCS)
			{
				RCSDescription = Catalog->Get(ComponentDescription->Identifier);
			}
			else if (ComponentDescription->Type == EFlarePartType::OrbitalEngine)
			{
				EngineDescription = Catalog->Get(ComponentDescription->Identifier);
			}
		}
		
		// Add a button for each weapon group
		WeaponButtonBox->ClearChildren();
		for (int32 i = 0; i < WeaponDescriptions.Num(); i++)
		{
			TSharedPtr<int32> IndexPtr(new int32(i));

			WeaponButtonBox->AddSlot()
				.AutoWidth()
				.Padding(FFlareStyleSet::GetDefaultTheme().TitleButtonPadding)
				.VAlign(VAlign_Top)
				[
					SNew(SFlareRoundButton)
					.OnClicked(this, &SFlareShipMenu::ShowWeapons, IndexPtr)
					.Icon(this, &SFlareShipMenu::GetWeaponIcon, IndexPtr)
					.Text(this, &SFlareShipMenu::GetWeaponText, IndexPtr)
					.HelpText(LOCTEXT("WeaponInfo", "Inspect this weapon group"))
					.InvertedBackground(true)
					.HighlightColor(this, &SFlareShipMenu::GetWeaponHealthColor)
				];
		}
		WeaponButtonBox->SetVisibility(WeaponDescriptions.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed);
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
	ShipList->SetVisibility(EVisibility::Collapsed);
	ObjectName->SetVisibility(EVisibility::Collapsed);
	ObjectDescription->SetVisibility(EVisibility::Visible);
	ShipPartCustomizationBox->SetVisibility(EVisibility::Visible);
	PartCharacteristicBox->SetVisibility(EVisibility::Visible);
	ShipCustomizationBox->SetVisibility(EVisibility::Collapsed);
	CantUpgradeReason->SetVisibility(EVisibility::Collapsed);

	// Boxes depending on edit mode
	UpgradeTitle->SetVisibility(CanEdit ? EVisibility::Visible : EVisibility::Collapsed);
	if (!CanEdit)
	{
		BuyConfirmation->Hide();
	}
}

void SFlareShipMenu::UpdatePartList(FFlareSpacecraftComponentDescription* SelectItem)
{
	ShipPartPickerTitle->SetVisibility(CanEdit ? EVisibility::Visible : EVisibility::Collapsed);
	PartList->SetVisibility(CanEdit ? EVisibility::Visible : EVisibility::Collapsed);
	PartListDataShared.Empty();
	PartList->RequestListRefresh();

	if (CanEdit)
	{
		FLOGV("SFlareShipMenu::UpdatePartList : looking for %s", *SelectItem->Name.ToString());

		int32 Index = INDEX_NONE;

		// Copy items
		for (FFlareSpacecraftComponentDescription* Part : PartListData)
		{
			PartListDataShared.AddUnique(FInterfaceContainer::New(Part));
			if (Part == SelectItem)
			{
				Index = PartListDataShared.Num() - 1;
			}
		}

		ShipPartIndex = Index;
		CurrentPartIndex = Index;
		CurrentEquippedPartIndex = Index;

		// Update list
		PartList->RequestListRefresh();
		PartList->SetSelection(PartListDataShared[Index]);
	}

	LoadPart(SelectItem->Identifier);
}

void SFlareShipMenu::UpdateFactoryList()
{
	// Iterate on all factories
	if (TargetSpacecraft)
	{
		TArray<UFlareFactory*>& Factories = TargetSpacecraft->GetFactories();
		for (int FactoryIndex = 0; FactoryIndex < Factories.Num(); FactoryIndex++)
		{
			FactoryList->AddSlot()
			[
				SNew(SFlareFactoryInfo)
				.Factory(Factories[FactoryIndex])
				.MenuManager(MenuManager)
				.Visibility(this, &SFlareShipMenu::GetFactoryControlsVisibility)
			];
		}
	}
}

void SFlareShipMenu::UpdateUpgradeBox()
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	UpgradeBox->ClearChildren();

	if (!TargetSpacecraft
	 || !TargetSpacecraft->IsStation()
	 || TargetSpacecraft->GetCompany() != MenuManager->GetPC()->GetCompany())
	{
		return;
	}

	// Upgrade title
	UpgradeBox->AddSlot()
	.AutoHeight()
	.Padding(Theme.TitlePadding)
	[
		SNew(STextBlock)
		.TextStyle(&Theme.SubTitleFont)
		.Text(FText::Format(LOCTEXT("UpgradeTitleFormat", "Upgrade ({0}/{1})"),
			FText::AsNumber(TargetSpacecraft->GetLevel()),
			FText::AsNumber(TargetSpacecraft->GetDescription()->MaxLevel)))
	];

	// Max level
	if (TargetSpacecraft->GetLevel() >= TargetSpacecraft->GetDescription()->MaxLevel)
	{
		UpgradeBox->AddSlot()
		.AutoHeight()
		.Padding(Theme.TitlePadding)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TextFont)
			.Text(LOCTEXT("MaxLevelInfo", "This station has reached the maximum level."))
		];
	}
	else
	{
		UpgradeBox->AddSlot()
		.AutoHeight()
		.Padding(Theme.TitlePadding)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TextFont)
			.Text(LOCTEXT("CurrentLevelInfo", "Levels act as a multiplier to all station characteristics - a level 2 station acts like two level 1 stations."))
		];

		// Add resources
		FString ResourcesString;
		for (int ResourceIndex = 0; ResourceIndex < TargetSpacecraft->GetDescription()->CycleCost.InputResources.Num(); ResourceIndex++)
		{
			FFlareFactoryResource* FactoryResource = &TargetSpacecraft->GetDescription()->CycleCost.InputResources[ResourceIndex];
			ResourcesString += FString::Printf(TEXT(", %u %s"), FactoryResource->Quantity, *FactoryResource->Resource->Data.Name.ToString()); // FString needed here
		}

		// Final text
		FText ProductionCost = FText::Format(LOCTEXT("UpgradeCostFormat", "Upgrade to level {0} ({1} credits{2})"),
			FText::AsNumber(TargetSpacecraft->GetLevel() + 1),
			FText::AsNumber(UFlareGameTools::DisplayMoney(TargetSpacecraft->GetStationUpgradeFee())),
			FText::FromString(ResourcesString));

		// TODO increase stock inc cargo bay with level
		
		// Upgrade button
		UpgradeBox->AddSlot()
		.AutoHeight()
		.Padding(Theme.TitlePadding)
		.HAlign(HAlign_Left)
		[
			SNew(SFlareButton)
			.Width(12)
			.Text(ProductionCost)
			.Icon(FFlareStyleSet::GetIcon("Travel"))
			.OnClicked(this, &SFlareShipMenu::OnUpgradeStationClicked)
			.IsDisabled(this, &SFlareShipMenu::IsUpgradeStationDisabled)
		];
	}
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
		return  LOCTEXT("StationMenuTitle", "Station");
	}
	else
	{
		return (CanEdit ? LOCTEXT("ShipConfigMenuTitle", "Ship upgrade") : LOCTEXT("ShipMenuTitle", "Ship"));
	}
}

EVisibility SFlareShipMenu::GetEngineVisibility() const
{
	return (TargetSpacecraft && !TargetSpacecraft->IsStation() ? EVisibility::Visible : EVisibility::Collapsed);
}

EVisibility SFlareShipMenu::GetFactoryControlsVisibility() const
{
	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC && TargetSpacecraft && TargetSpacecraft->GetCompany()->GetWarState(PC->GetCompany()) !=  EFlareHostility::Hostile)
	{
		return EVisibility::Visible;
	}
	else
	{
		return EVisibility::Collapsed;
	}
}

EVisibility SFlareShipMenu::GetEditVisibility() const
{
	return (CanEdit ? EVisibility::Visible : EVisibility::Collapsed);
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

FSlateColor SFlareShipMenu::GetRCSHealthColor() const
{
	float ComponentHealth = 1;

	if (TargetSpacecraft)
	{
		ComponentHealth = TargetSpacecraft->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_RCS);
	}

	return FFlareStyleSet::GetHealthColor(ComponentHealth, false);
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

FSlateColor SFlareShipMenu::GetEnginesHealthColor() const
{
	float ComponentHealth = 1;

	if (TargetSpacecraft)
	{
		ComponentHealth = TargetSpacecraft->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Propulsion);
	}

	return FFlareStyleSet::GetHealthColor(ComponentHealth, false);
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

FText SFlareShipMenu::GetWeaponText(TSharedPtr<int32> GroupIndex) const
{
	FText Result;
	FText Comment;

	if (*GroupIndex < WeaponDescriptions.Num())
	{
		FFlareSpacecraftComponentDescription* Desc = WeaponDescriptions[*GroupIndex];
		if (Desc)
		{
			Result = Desc->Name;
		}
	}

	// Get group name
	if (TargetSpacecraft)
	{
		const FFlareSpacecraftDescription* ShipDesc = TargetSpacecraft->GetDescription();

		if (ShipDesc)
		{
			if (ShipDesc->Size == EFlarePartSize::L)
			{
				FCHECK(*GroupIndex >= 0 && *GroupIndex < ShipDesc->WeaponGroups.Num());
				Comment = ShipDesc->WeaponGroups[*GroupIndex].GroupName;
			}
			else
			{
				FCHECK(*GroupIndex >= 0 && *GroupIndex < ShipDesc->WeaponGroups.Num());
				Comment = ShipDesc->WeaponGroups[*GroupIndex].GroupName;
			}
		}
	}

	return FText::Format(LOCTEXT("WeaponTextFormat", "{0}\n({1})"), Result, Comment);
}

FSlateColor SFlareShipMenu::GetWeaponHealthColor() const
{
	float ComponentHealth = 1;

	if (TargetSpacecraft)
	{
		ComponentHealth = TargetSpacecraft->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon);
	}

	return FFlareStyleSet::GetHealthColor(ComponentHealth, false);
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

void SFlareShipMenu::OnUpgradeStationClicked()
{
	if (TargetSpacecraft)
	{
		UFlareSimulatedSector* Sector = TargetSpacecraft->GetCurrentSector();
		if (Sector)
		{
			Sector->UpgradeStation(TargetSpacecraft);
			FFlareMenuParameterData Data;
			Data.Spacecraft = TargetSpacecraft;
			MenuManager->OpenMenu(EFlareMenu::MENU_Ship, Data);
		}
	}
}

bool SFlareShipMenu::IsUpgradeStationDisabled() const
{
	AFlarePlayerController* PC = MenuManager->GetPC();

	if (TargetSpacecraft)
	{
		UFlareSimulatedSector* Sector = TargetSpacecraft->GetCurrentSector();
		if (Sector)
		{
			TArray<FText> Reasons;
			return !Sector->CanUpgradeStation(TargetSpacecraft, Reasons);
		}
	}

	return true;
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
		FFlareSpacecraftComponentDescription* PartDesc = NULL;
		UFlareSpacecraftComponentsCatalog* Catalog = PC->GetGame()->GetShipPartsCatalog();

		// Browse all the parts in the save until we find the right one
		for (int32 Index = 0; Index < TargetSpacecraftData->Components.Num(); Index++)
		{
			FFlareSpacecraftComponentDescription* Desc = Catalog->Get(TargetSpacecraftData->Components[Index].ComponentIdentifier);
			if (Desc && Desc->Type == EFlarePartType::RCS)
			{
				PartDesc = Desc;
				break;
			}
		}

		Catalog->GetRCSList(PartListData, TargetSpacecraft->GetDescription()->Size, MenuManager->GetPC()->GetCompany());
		FLOGV("SFlareShipMenu::ShowRCSs : %d parts", PartListData.Num());
		UpdatePartList(PartDesc);
	}
}

void SFlareShipMenu::ShowEngines()
{
	PartListData.Empty();

	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		FFlareSpacecraftComponentDescription* PartDesc = NULL;
		UFlareSpacecraftComponentsCatalog* Catalog = PC->GetGame()->GetShipPartsCatalog();
		
		// Browse all the parts in the save until we find the right one
		for (int32 Index = 0; Index < TargetSpacecraftData->Components.Num(); Index++)
		{
			FFlareSpacecraftComponentDescription* Desc = Catalog->Get(TargetSpacecraftData->Components[Index].ComponentIdentifier);
			if (Desc && Desc->Type == EFlarePartType::OrbitalEngine)
			{
				PartDesc = Desc;
				break;
			}
		}

		Catalog->GetEngineList(PartListData, TargetSpacecraft->GetDescription()->Size, MenuManager->GetPC()->GetCompany());
		FLOGV("SFlareShipMenu::ShowEngines : %d parts", PartListData.Num());
		UpdatePartList(PartDesc);
	}
}

void SFlareShipMenu::ShowWeapons(TSharedPtr<int32> WeaponGroupIndex)
{
	AFlarePlayerController* PC = MenuManager->GetPC();
	FCHECK(PC);

	UFlareSpacecraftComponentsCatalog* Catalog = PC->GetGame()->GetShipPartsCatalog();
	FCHECK(Catalog);

	FFlareSpacecraftDescription* ShipDesc = TargetSpacecraft->GetDescription();
	FCHECK(ShipDesc);

	// Setup data
	PartListData.Empty();
	int32 CurrentSearchIndex = 0;
	CurrentWeaponGroupIndex = *WeaponGroupIndex;
	FFlareSpacecraftComponentDescription* PartDesc = NULL;

	// Browse all the parts in the save until we find the right one
	for (int32 Index = 0; Index < TargetSpacecraftData->Components.Num(); Index++)
	{
		FFlareSpacecraftComponentDescription* Desc = Catalog->Get(TargetSpacecraftData->Components[Index].ComponentIdentifier);
		if (Desc && Desc->Type == EFlarePartType::Weapon)
		{
			FName TargetSlotName = UFlareSimulatedSpacecraftWeaponsSystem::GetSlotIdentifierFromWeaponGroupIndex(ShipDesc, CurrentWeaponGroupIndex);

			if (TargetSpacecraftData->Components[Index].ShipSlotIdentifier == TargetSlotName)
			{
				PartDesc = Desc;
				break;
			}
			else
			{
				CurrentSearchIndex++;
			}
		}
	}

	Catalog->GetWeaponList(PartListData, TargetSpacecraft->GetDescription()->Size, MenuManager->GetPC()->GetCompany());
	FLOGV("SFlareShipMenu::ShowWeapons : %d parts", PartListData.Num());
	UpdatePartList(PartDesc);
}

void SFlareShipMenu::OnPartPicked(TSharedPtr<FInterfaceContainer> Item, ESelectInfo::Type SelectInfo)
{
	int32 Index = PartListData.Find(Item->PartDescription);
	AFlarePlayerController* PC = MenuManager->GetPC();

	if (PC && Item->PartDescription && Index != CurrentPartIndex)
	{
		AFlareMenuPawn* Viewer = PC->GetMenuPawn();

		// Ensure this part can be changed
		bool CanBeChanged = TargetSpacecraft->CanUpgrade(Item->PartDescription->Type);

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
			if (CanBeChanged)
			{
				int64 TransactionCost = TargetSpacecraft->GetUpgradeCost(Item->PartDescription, PartListData[CurrentEquippedPartIndex]);
				BuyConfirmation->Show(TransactionCost, PC->GetCompany());
				CantUpgradeReason->SetVisibility(EVisibility::Collapsed);
			}
			else
			{
				BuyConfirmation->Hide();
				if (CanEdit)
				{
					CantUpgradeReason->SetVisibility(EVisibility::Visible);
				}
				else
				{
					CantUpgradeReason->SetVisibility(EVisibility::Collapsed);
				}
			}
		}
		else
		{
			BuyConfirmation->Hide();
			CantUpgradeReason->SetVisibility(EVisibility::Collapsed);
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
	FCHECK(PC);
	UFlareSpacecraftComponentsCatalog* Catalog = PC->GetGame()->GetShipPartsCatalog();

	// Edit the correct save data property
	FFlareSpacecraftComponentDescription* NewPartDesc = PartListData[CurrentPartIndex];
	CurrentEquippedPartIndex = CurrentPartIndex;
		
	// Upgrade the ship
	if (TargetSpacecraft)
	{
		TargetSpacecraft->UpgradePart(NewPartDesc, CurrentWeaponGroupIndex);
	}

	// Get back to the ship config
	BuyConfirmation->Hide();
	LoadTargetSpacecraft();
	MenuManager->GetPC()->ClientPlaySound(MenuManager->GetPC()->GetSoundManager()->InfoSound);
}

void SFlareShipMenu::OnPartCancelled()
{
	BuyConfirmation->Hide();
	LoadTargetSpacecraft();
	MenuManager->GetPC()->ClientPlaySound(MenuManager->GetPC()->GetSoundManager()->NegativeClickSound);
}

#undef LOCTEXT_NAMESPACE
