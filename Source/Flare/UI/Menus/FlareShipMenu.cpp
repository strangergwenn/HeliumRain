
#include "../../Flare.h"
#include "FlareShipMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlarePlayerController.h"
#include "../Components/FlarePartInfo.h"


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
				.Text(this, &SFlareShipMenu::GetExitText)
				.HelpText(this, &SFlareShipMenu::GetExitInfoText)
				.Icon(this, &SFlareShipMenu::GetExitIcon)
				.OnClicked(this, &SFlareShipMenu::OnExit)
			]
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(200, 20))
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
							.HelpText(LOCTEXT("EngineInfo", "Inspect the current orbital engines"))
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
							.HelpText(LOCTEXT("RCSInfo", "Inspect the current RCS thrusters"))
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
				.HAlign(HAlign_Left)
				[
					SAssignNew(ShipList, SFlareShipList)
					.MenuManager(MenuManager)
					.Title(LOCTEXT("DockedShips", "DOCKED SHIPS"))
				]

				// Factory list
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Left)
				[
					SAssignNew(FactoryList, SVerticalBox)
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

	UpdateFactoryList();
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
			FString Prefix = (TargetSpacecraft->IsStation() ? LOCTEXT("Station", "STATION : ") : LOCTEXT("Ship", "SHIP : ")).ToString();
			ObjectName->SetText(FText::FromString(Prefix + TargetSpacecraft->GetImmatriculation().ToString()));
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
						.HelpText(LOCTEXT("WeaponInfo", "Inspect this weapon system"))
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
		FLOGV("SFlareShipMenu::UpdatePartList : looking for %s", *SelectItem->Name.ToString());
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

void SFlareShipMenu::UpdateFactoryList()
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FactoryList->ClearChildren();

	UFlareSimulatedSpacecraft* SimulatedSpacecraft = Cast<UFlareSimulatedSpacecraft>(TargetSpacecraft);

	if (SimulatedSpacecraft)
	{
		TArray<UFlareFactory*>& Factories = SimulatedSpacecraft->GetFactories();

		for (int FactoryIndex = 0; FactoryIndex < Factories.Num(); FactoryIndex++)
		{
			UFlareFactory* Factory = Factories[FactoryIndex];

			FString ProductionCostString = FString::Printf(TEXT("%d $"), Factory->GetDescription()->ProductionCost);

			for (int ResourceIndex = 0; ResourceIndex < Factory->GetDescription()->InputResources.Num(); ResourceIndex++)
			{
				const FFlareFactoryResource* FactoryResource = &Factory->GetDescription()->InputResources[ResourceIndex];
				ProductionCostString += FString::Printf(TEXT(", %u %s"), FactoryResource->Quantity, *FactoryResource->Resource->Data.Acronym.ToString());
			}

			FString ProductionOutputString;
			for (int ActionIndex = 0; ActionIndex < Factory->GetDescription()->OutputActions.Num(); ActionIndex++)
			{
				const FFlareFactoryAction* FactoryAction = &Factory->GetDescription()->OutputActions[ActionIndex];
				if (!ProductionOutputString.IsEmpty())
				{
					ProductionOutputString += FString(TEXT(", "));
				}

				switch(FactoryAction->Action)
				{
					case EFlareFactoryAction::CreateShip:
						ProductionOutputString += FString::Printf(TEXT("%u %s"), FactoryAction->Quantity, *MenuManager->GetGame()->GetSpacecraftCatalog()->Get(FactoryAction->Identifier)->Name.ToString());
						break;
					case EFlareFactoryAction::DiscoverSector:
					case EFlareFactoryAction::GainTechnology:
						// TODO
					default:
						FLOGV("Warning ! Not implemented factory action %d", (FactoryAction->Action+0));
				}
			}

			for (int ResourceIndex = 0; ResourceIndex < Factory->GetDescription()->OutputResources.Num(); ResourceIndex++)
			{
				const FFlareFactoryResource* FactoryResource = &Factory->GetDescription()->OutputResources[ResourceIndex];
				if (!ProductionOutputString.IsEmpty())
				{
					ProductionOutputString += FString(TEXT(", "));
				}
				ProductionOutputString += FString::Printf(TEXT("%u %s"), FactoryResource->Quantity, *FactoryResource->Resource->Data.Acronym.ToString());
			}

			FString ProductionStatusString;

			if (Factory->IsActive())
			{
				if (!Factory->IsNeedProduction())
				{
					ProductionStatusString = FString(TEXT("Production not needed."));
				}
				else if (Factory->HasCostReserved())
				{
					ProductionStatusString = FString::Printf(TEXT("In production, remaining duration: %s."), *UFlareGameTools::FormatTime(Factory->GetRemainingProductionDuration(), 2) );

					if (!Factory->HasOutputFreeSpace())
					{
						ProductionStatusString += FString(TEXT(" No enough output space."));
					}
				}
				else if(Factory->HasInputMoney() && Factory->HasInputResources())
				{
					ProductionStatusString = FString::Printf(TEXT("Production will start."));
				}
				else
				{
					ProductionStatusString = FString::Printf(TEXT("Production cannot start."));

					if (!Factory->HasInputMoney())
					{
						ProductionStatusString += FString(TEXT(" No enough money."));
					}

					if (!Factory->HasInputResources())
					{
						ProductionStatusString += FString(TEXT(" No enough resources."));
					}
				}
			}
			else if (Factory->IsPaused())
			{
				ProductionStatusString = FString::Printf(TEXT("Production paused, remaining duration: %s."), *UFlareGameTools::FormatTime(Factory->GetRemainingProductionDuration(), 2) );
			}
			else
			{
				ProductionStatusString = FString(TEXT("Production stopped."));
			}

			FString ProductionCycleStatusString;
			if(Factory->HasInfiniteCycle())
			{
				ProductionCycleStatusString = FString(TEXT("no production limits"));
			}
			else
			{
				ProductionCycleStatusString = FString::Printf(TEXT("produce %u cycles"), Factory->GetCycleCount());
			}

			FactoryList->AddSlot()
			[
				SNew(SVerticalBox)

				// Factory name
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.TitlePadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.SubTitleFont)
					.Text(FText::FromString(Factory->GetDescription()->Name.ToString().ToUpper()))
				]

				// Factory description
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(FText::FromString(Factory->GetDescription()->Description.ToString()))
				]

				// Factory production time
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(FText::FromString(FString::Printf(TEXT("Production time: %s"), *UFlareGameTools::FormatTime(Factory->GetDescription()->ProductionTime, 2))))
				]

				// Factory production cost
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(FText::FromString(FString::Printf(TEXT("Production cost: %s"), *ProductionCostString)))
				]

				// Factory production output
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(FText::FromString(FString::Printf(TEXT("Production output: %s"), *ProductionOutputString)))
				]

				// Factory production status
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(FText::FromString(FString::Printf(TEXT("Production status: %s"), *ProductionStatusString)))
				]

				// Factory control
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					// Factory start production button
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Visibility(this, &SFlareShipMenu::GetStartProductionButtonVisibility, Factory)
						.OnClicked(this, &SFlareShipMenu::OnStartProductionClicked, Factory)
						.Text(FText::FromString(TEXT("Start production")))
					]

					// Factory pause production button
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Visibility(this, &SFlareShipMenu::GetPauseProductionButtonVisibility, Factory)
						.OnClicked(this, &SFlareShipMenu::OnPauseProductionClicked, Factory)
						.Text(FText::FromString(TEXT("Pause production")))
					]

					// Factory stop production button
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Visibility(this, &SFlareShipMenu::GetStopProductionButtonVisibility, Factory)
						.OnClicked(this, &SFlareShipMenu::OnStopProductionClicked, Factory)
						.Text(FText::FromString(TEXT("Stop production")))
					]
				]

				// Factory production cycle status
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(FText::FromString(FString::Printf(TEXT("Production cycle status: %s"), *ProductionCycleStatusString)))
				]

				// Factory mode control
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					// Factory switch mode
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.OnClicked(this, &SFlareShipMenu::OnSwitchProductionModeClicked, Factory)
						.Text(FText::FromString(TEXT("Switch production mode")))
					]

					// Factory remove production cycle button
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Visibility(this, &SFlareShipMenu::GetRemoveProductionCycleButtonVisibility, Factory)
						.OnClicked(this, &SFlareShipMenu::OnRemoveProductionCycleClicked, Factory)
						.Text(FText::FromString(TEXT("-")))
					]

					// Factory add production cycle button
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Visibility(this, &SFlareShipMenu::GetAddProductionCycleButtonVisibility, Factory)
						.OnClicked(this, &SFlareShipMenu::OnAddProductionCycleClicked, Factory)
						.Text(FText::FromString(TEXT("+")))
					]
				]
			];
		}

	}

}

EVisibility SFlareShipMenu::GetStartProductionButtonVisibility(UFlareFactory* Factory) const
{
	return (!Factory->IsActive() ? EVisibility::Visible : EVisibility::Collapsed);
}

EVisibility SFlareShipMenu::GetPauseProductionButtonVisibility(UFlareFactory* Factory) const
{
	return (Factory->IsActive() && Factory->GetProductedDuration() > 0 ? EVisibility::Visible : EVisibility::Collapsed);
}

EVisibility SFlareShipMenu::GetStopProductionButtonVisibility(UFlareFactory* Factory) const
{
	return (Factory->IsActive() ? EVisibility::Visible : EVisibility::Collapsed);
}

EVisibility SFlareShipMenu::GetAddProductionCycleButtonVisibility(UFlareFactory* Factory) const
{
	return (!Factory->HasInfiniteCycle() ? EVisibility::Visible : EVisibility::Collapsed);
}

EVisibility SFlareShipMenu::GetRemoveProductionCycleButtonVisibility(UFlareFactory* Factory) const
{
	return (!Factory->HasInfiniteCycle() ? EVisibility::Visible : EVisibility::Collapsed);
}

void SFlareShipMenu::OnStartProductionClicked(UFlareFactory* Factory)
{
	Factory->Start();
	UpdateFactoryList();
}

void SFlareShipMenu::OnPauseProductionClicked(UFlareFactory* Factory)
{
	Factory->Pause();
	UpdateFactoryList();
}

void SFlareShipMenu::OnStopProductionClicked(UFlareFactory* Factory)
{
	Factory->Stop();
	UpdateFactoryList();
}

void SFlareShipMenu::OnSwitchProductionModeClicked(UFlareFactory* Factory)
{
	Factory->SetInfiniteCycle(!Factory->HasInfiniteCycle());
	UpdateFactoryList();
}

void SFlareShipMenu::OnAddProductionCycleClicked(UFlareFactory* Factory)
{
	Factory->SetCycleCount(Factory->GetCycleCount() + 1);

	UpdateFactoryList();
}

void SFlareShipMenu::OnRemoveProductionCycleClicked(UFlareFactory* Factory)
{
	if(Factory->GetCycleCount() > 0)
	{
		Factory->SetCycleCount(Factory->GetCycleCount() - 1);
		UpdateFactoryList();
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
		return  LOCTEXT("StationMenuTitle", "STATION");
	}
	else
	{
		return (CanEdit ? LOCTEXT("ShipConfigMenuTitle", "SHIP UPGRADE") : LOCTEXT("ShipMenuTitle", "SHIP"));
	}
}

FText SFlareShipMenu::GetExitText() const
{
	if (Cast<AFlareSpacecraft>(TargetSpacecraft))
	{
		return LOCTEXT("ExitTextDashboard", "Dashboard");
	}
	else
	{
		return LOCTEXT("ExitTextOrbit", "Orbital map");
	}
}

FText SFlareShipMenu::GetExitInfoText() const
{
	if (Cast<AFlareSpacecraft>(TargetSpacecraft))
	{
		return LOCTEXT("ExitInfoTextDashboard", "Go back to the dashboard");
	}
	else
	{
		return LOCTEXT("ExitInfoTextOrbit", "Go back to the orbital map");
	}
}

const FSlateBrush* SFlareShipMenu::GetExitIcon() const
{
	if (Cast<AFlareSpacecraft>(TargetSpacecraft))
	{
		return AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Dashboard, true);
	}
	else
	{
		return AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Orbit, true);
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

		Catalog->GetRCSList(PartListData, TargetSpacecraft->GetDescription()->Size);
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

		Catalog->GetEngineList(PartListData, TargetSpacecraft->GetDescription()->Size);
		FLOGV("SFlareShipMenu::ShowEngines : %d parts", PartListData.Num());
		UpdatePartList(PartDesc);
	}
}

void SFlareShipMenu::ShowWeapons(TSharedPtr<int32> WeaponIndex)
{
	PartListData.Empty();
	CurrentWeaponIndex = *WeaponIndex;

	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		int32 CurrentSearchIndex = 0;
		FFlareSpacecraftComponentDescription* PartDesc = NULL;
		UFlareSpacecraftComponentsCatalog* Catalog = PC->GetGame()->GetShipPartsCatalog();

		// Browse all the parts in the save until we find the right one
		for (int32 Index = 0; Index < TargetSpacecraftData->Components.Num(); Index++)
		{
			FFlareSpacecraftComponentDescription* Desc = Catalog->Get(TargetSpacecraftData->Components[Index].ComponentIdentifier);
			if (Desc && Desc->Type == EFlarePartType::Weapon)
			{
				if (CurrentSearchIndex == CurrentWeaponIndex)
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

		Catalog->GetWeaponList(PartListData, TargetSpacecraft->GetDescription()->Size);
		FLOGV("SFlareShipMenu::ShowWeapons : %d parts", PartListData.Num());
		UpdatePartList(PartDesc);
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

void SFlareShipMenu::OnExit()
{
	if (Cast<AFlareSpacecraft>(TargetSpacecraft))
	{
		MenuManager->OpenMenu(EFlareMenu::MENU_Dashboard);
	}
	else
	{
		MenuManager->OpenMenu(EFlareMenu::MENU_Orbit);
	}
}

#undef LOCTEXT_NAMESPACE
