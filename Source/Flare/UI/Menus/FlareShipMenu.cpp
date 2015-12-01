
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

				// Factory list
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Left)
				[
					SAssignNew(FactoryList, SVerticalBox)
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
			FText Prefix = TargetSpacecraft->IsStation() ? LOCTEXT("Station", "STATION") : LOCTEXT("Ship", "SHIP");
			FText Immatriculation = FText::FromString(TargetSpacecraft->GetImmatriculation().ToString());
			ObjectName->SetText(FText::Format(LOCTEXT("ObjectNameFormat", "{0} : {1}"), Prefix, Immatriculation));

			ObjectClassName->SetText(ShipDesc->Name);
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
	UFlareSimulatedSpacecraft* SimulatedSpacecraft = Cast<UFlareSimulatedSpacecraft>(TargetSpacecraft);
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FactoryList->ClearChildren();

	if (SimulatedSpacecraft)
	{
		TArray<UFlareFactory*>& Factories = SimulatedSpacecraft->GetFactories();
		FText CommaTextReference = LOCTEXT("Comma", ",");

		// Iterate on all factories
		for (int FactoryIndex = 0; FactoryIndex < Factories.Num(); FactoryIndex++)
		{
			FText ProductionCostText;
			FText ProductionOutputText;
			FText ProductionStatusText;
			UFlareFactory* Factory = Factories[FactoryIndex];
			check(Factory);
			
			// Cycle cost in credits
			uint32 CycleCost = Factory->GetDescription()->ProductionCost;
			if (CycleCost > 0)
			{
				ProductionCostText = FText::Format(LOCTEXT("ProductionCostFormat", "{0} credits"), FText::AsNumber(CycleCost));
			}

			// Cycle cost in resources
			for (int ResourceIndex = 0; ResourceIndex < Factory->GetDescription()->InputResources.Num(); ResourceIndex++)
			{
				FText CommaText = ProductionCostText.IsEmpty() ? FText() : CommaTextReference;
				const FFlareFactoryResource* FactoryResource = &Factory->GetDescription()->InputResources[ResourceIndex];
				check(FactoryResource);

				ProductionCostText = FText::Format(LOCTEXT("ProductionResourcesFormat", "{0}{1} {2} {3}"),
					ProductionCostText, CommaText, FText::AsNumber(FactoryResource->Quantity), FactoryResource->Resource->Data.Acronym);
			}

			// Cycle output in factory actions
			for (int ActionIndex = 0; ActionIndex < Factory->GetDescription()->OutputActions.Num(); ActionIndex++)
			{
				FText CommaText = ProductionOutputText.IsEmpty() ? FText() : CommaTextReference;
				const FFlareFactoryAction* FactoryAction = &Factory->GetDescription()->OutputActions[ActionIndex];
				check(FactoryAction);
				
				switch (FactoryAction->Action)
				{
					// Ship production
					case EFlareFactoryAction::CreateShip:
						ProductionOutputText = FText::Format(LOCTEXT("ProductionActionsFormat", "{0}{1} {2} {3}"), 
							ProductionOutputText, CommaText, FText::AsNumber(FactoryAction->Quantity),
							MenuManager->GetGame()->GetSpacecraftCatalog()->Get(FactoryAction->Identifier)->Name);
						break;

					// TODO
					case EFlareFactoryAction::DiscoverSector:
					case EFlareFactoryAction::GainTechnology:
					default:
						FLOGV("SFlareShipMenu::UpdateFactoryList : Unimplemented factory action %d", (FactoryAction->Action+0));
				}
			}

			// Cycle output in resources
			for (int ResourceIndex = 0; ResourceIndex < Factory->GetDescription()->OutputResources.Num(); ResourceIndex++)
			{
				FText CommaText = ProductionOutputText.IsEmpty() ? FText() : CommaTextReference;
				const FFlareFactoryResource* FactoryResource = &Factory->GetDescription()->OutputResources[ResourceIndex];
				check(FactoryResource);

				ProductionOutputText = FText::Format(LOCTEXT("ProductionOutputFormat", "{0}{1} {2} {3}"),
					ProductionOutputText, CommaText, FText::AsNumber(FactoryResource->Quantity), FactoryResource->Resource->Data.Acronym);
			}

			// Factory status
			if (Factory->IsActive())
			{
				if (!Factory->IsNeedProduction())
				{
					ProductionStatusText = LOCTEXT("ProductionNotNeeded", "Production not needed");
				}
				else if (Factory->HasCostReserved())
				{
					ProductionStatusText = FText::Format(LOCTEXT("ProductionInProgressFormat", "Producing ({0}{1})"),
						FText::FromString(*UFlareGameTools::FormatTime(Factory->GetRemainingProductionDuration(), 2)), // FString needed here
						Factory->HasOutputFreeSpace() ? FText() : LOCTEXT("ProductionNoSpace", ", not enough space"));
				}
				else if (Factory->HasInputMoney() && Factory->HasInputResources())
				{
					ProductionStatusText = LOCTEXT("ProductionWillStart", "Starting");
				}
				else
				{
					if (!Factory->HasInputMoney())
					{
						ProductionStatusText = LOCTEXT("ProductionNotEnoughMoney", "Waiting for credits");
					}

					if (!Factory->HasInputResources())
					{
						ProductionStatusText = LOCTEXT("ProductionNotEnoughResources", "Waiting for resources");
					}
				}
			}
			else if (Factory->IsPaused())
			{
				ProductionStatusText = FText::Format(LOCTEXT("ProductionPaused", "Paused ({0} to completion)"),
					FText::FromString(*UFlareGameTools::FormatTime(Factory->GetRemainingProductionDuration(), 2))); // FString needed here
			}
			else
			{
				ProductionStatusText = LOCTEXT("ProductionStopped", "Stopped");
			}

			// Production cycle limiter
			bool ProductionLimitEnabled = !Factory->HasInfiniteCycle();
			FText ProductionCycleStatusText;
			if (ProductionLimitEnabled)
			{
				ProductionCycleStatusText = FText::Format(LOCTEXT("ProductionLimitFormat", "Limited to {0} cycles (clear)"),
					FText::AsNumber(Factory->GetCycleCount()));
			}
			else
			{
				ProductionCycleStatusText = LOCTEXT("ProductionLimitEnable", "Limit production cycles");
			}

			// Add structure
			TSharedPtr<SVerticalBox> LimitList;
			FactoryList->AddSlot()
			[
				SNew(SVerticalBox)

				// Factory name
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.TitlePadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.NameFont)
					.Text(FText::Format(LOCTEXT("FactoryNameFormat", "{0} ({1})"), FText::FromString(Factory->GetDescription()->Name.ToString().ToUpper()), ProductionStatusText))
				]

				// Factory production status
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(SHorizontalBox)

					// Factory start production button
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Top)
					[
						SNew(SFlareButton)
						.Visibility(this, &SFlareShipMenu::GetStartProductionVisibility, Factory)
						.OnClicked(this, &SFlareShipMenu::OnStartProduction, Factory)
						.Text(FText())
						.HelpText(LOCTEXT("StartProduction", "Start production"))
						.Icon(FFlareStyleSet::GetIcon("Load"))
						.Transparent(true)
						.Width(1)
					]

					// Factory stop production button
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Top)
					[
						SNew(SFlareButton)
						.Visibility(this, &SFlareShipMenu::GetStopProductionVisibility, Factory)
						.OnClicked(this, &SFlareShipMenu::OnStopProduction, Factory)
						.Text(FText())
						.HelpText(LOCTEXT("StopProduction", "Stop production"))
						.Icon(FFlareStyleSet::GetIcon("Stop"))
						.Transparent(true)
						.Width(1)
					]

					// Factory status
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)

						// Progress
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SProgressBar)
							.Percent(0.42)
							.Style(&Theme.ProgressBarStyle)
						]

						// Factory production cycle description
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.Padding(Theme.SmallContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(FText::Format(LOCTEXT("FactoryCycleInfoFormat", "{0} \u2192 {1} (in {2})"),
								ProductionCostText, ProductionOutputText,
								FText::FromString(*UFlareGameTools::FormatTime(Factory->GetDescription()->ProductionTime, 2)))) // FString needed here
						]

						// Factory limits
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(LimitList, SVerticalBox)

							//// Factory mode control
							//+ SVerticalBox::Slot()
							//.AutoHeight()
							//.Padding(Theme.SmallContentPadding)
							//[
							//	SNew(SHorizontalBox)

							//	// Factory switch mode
							//	+ SHorizontalBox::Slot()
							//	.AutoWidth()
							//	[
							//		SNew(SFlareButton)
							//		.OnClicked(this, &SFlareShipMenu::OnSwitchProductionCyclesLimit, Factory)
							//		.Text(ProductionCycleStatusText)
							//		.Width(ProductionLimitEnabled ? 6 : 8)
							//	]

							//	// Factory remove production cycle button
							//	+ SHorizontalBox::Slot()
							//	.AutoWidth()
							//	[
							//		SNew(SFlareButton)
							//		.Visibility(this, &SFlareShipMenu::GetProductionCyclesLimitVisibility, Factory)
							//		.OnClicked(this, &SFlareShipMenu::OnDecreaseProductionCycles, Factory)
							//		.Text(FText::FromString(TEXT("-")))
							//		.Width(1)
							//	]

							//	// Factory add production cycle button
							//	+ SHorizontalBox::Slot()
							//	.AutoWidth()
							//	[
							//		SNew(SFlareButton)
							//		.Visibility(this, &SFlareShipMenu::GetProductionCyclesLimitVisibility, Factory)
							//		.OnClicked(this, &SFlareShipMenu::OnIncreaseProductionCycles, Factory)
							//		.Text(FText::FromString(TEXT("+")))
							//		.Width(1)
							//	]
							//]
						]
					]
				]
			];

			// Iterate all output resources
			for (int ResourceIndex = 0; ResourceIndex < Factory->GetDescription()->OutputResources.Num(); ResourceIndex++)
			{
				const FFlareFactoryResource* FactoryResource = &Factory->GetDescription()->OutputResources[ResourceIndex];
				FFlareResourceDescription* Resource = &FactoryResource->Resource->Data;
				check(Resource);
				
				// Production resource limiter
				bool ResourceLimitEnabled = Factory->HasOutputLimit(Resource);
				FText ProductionCycleStatusText;
				if (ResourceLimitEnabled)
				{
					ProductionCycleStatusText = FText::Format(LOCTEXT("ResourceLimitFormat", "Limited to {0} {1} (clear)"),
						FText::AsNumber(Factory->GetOutputLimit(Resource) * SimulatedSpacecraft->GetDescription()->CargoBayCapacity),
						Resource->Acronym);
				}
				else
				{
					ProductionCycleStatusText = FText::Format(LOCTEXT("ResourceLimitEnableFormat", "Limit {1} output"),
						FText::AsNumber(Factory->GetOutputLimit(Resource)), Resource->Acronym);
				}

				// Add a new limiter slot
				LimitList->AddSlot()
				.Padding(Theme.SmallContentPadding)
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					// Limit toggle
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.OnClicked(this, &SFlareShipMenu::OnSwitchOutputLimit, Factory, Resource)
						.Text(ProductionCycleStatusText)
						.Width(ResourceLimitEnabled ? 6 : 8)
					]

					// Limit decrease
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Visibility(this, &SFlareShipMenu::GetOutputLimitVisibility, Factory, Resource)
						.OnClicked(this, &SFlareShipMenu::OnDecreaseOutputLimit, Factory, Resource)
						.Text(FText::FromString(TEXT("-")))
						.Width(1)
					]

					// Limit increase
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Visibility(this, &SFlareShipMenu::GetOutputLimitVisibility, Factory, Resource)
						.OnClicked(this, &SFlareShipMenu::OnIncreaseOutputLimit, Factory, Resource)
						.Text(FText::FromString(TEXT("+")))
						.Width(1)
					]
				];
			}
		}

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

EVisibility SFlareShipMenu::GetStartProductionVisibility(UFlareFactory* Factory) const
{
	return (!Factory->IsActive() ? EVisibility::Visible : EVisibility::Collapsed);
}

EVisibility SFlareShipMenu::GetStopProductionVisibility(UFlareFactory* Factory) const
{
	return (Factory->IsActive() ? EVisibility::Visible : EVisibility::Collapsed);
}

EVisibility SFlareShipMenu::GetProductionCyclesLimitVisibility(UFlareFactory* Factory) const
{
	return (!Factory->HasInfiniteCycle() ? EVisibility::Visible : EVisibility::Collapsed);
}

EVisibility SFlareShipMenu::GetOutputLimitVisibility(UFlareFactory* Factory, FFlareResourceDescription* Resource) const
{
	return (Factory->HasOutputLimit(Resource) ? EVisibility::Visible : EVisibility::Collapsed);
}

void SFlareShipMenu::OnStartProduction(UFlareFactory* Factory)
{
	Factory->Start();
	UpdateFactoryList();
}

void SFlareShipMenu::OnStopProduction(UFlareFactory* Factory)
{
	Factory->Stop();
	UpdateFactoryList();
}

void SFlareShipMenu::OnSwitchProductionCyclesLimit(UFlareFactory* Factory)
{
	Factory->SetInfiniteCycle(!Factory->HasInfiniteCycle());
	UpdateFactoryList();
}

void SFlareShipMenu::OnIncreaseProductionCycles(UFlareFactory* Factory)
{
	Factory->SetCycleCount(Factory->GetCycleCount() + 1);
	UpdateFactoryList();
}

void SFlareShipMenu::OnDecreaseProductionCycles(UFlareFactory* Factory)
{
	if (Factory->GetCycleCount() > 1)
	{
		Factory->SetCycleCount(Factory->GetCycleCount() - 1);
		UpdateFactoryList();
	}
}

void SFlareShipMenu::OnSwitchOutputLimit(UFlareFactory* Factory, FFlareResourceDescription* Resource)
{
	if (Factory->HasOutputLimit(Resource))
	{
		Factory->ClearOutputLimit(Resource);
	}
	else
	{
		Factory->SetOutputLimit(Resource, 1);
	}
	UpdateFactoryList();
}

void SFlareShipMenu::OnDecreaseOutputLimit(UFlareFactory* Factory, FFlareResourceDescription* Resource)
{
	if (Factory->GetOutputLimit(Resource) > 1)
	{
		Factory->SetOutputLimit(Resource, Factory->GetOutputLimit(Resource) - 1);
		UpdateFactoryList();
	}
}

void SFlareShipMenu::OnIncreaseOutputLimit(UFlareFactory* Factory, FFlareResourceDescription* Resource)
{
	Factory->SetOutputLimit(Resource, Factory->GetOutputLimit(Resource) + 1);
	UpdateFactoryList();
}

#undef LOCTEXT_NAMESPACE
