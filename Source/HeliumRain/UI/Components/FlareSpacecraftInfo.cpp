
#include "../../Flare.h"
#include "FlareSpacecraftInfo.h"
#include "FlareCargoInfo.h"
#include "../../Economy/FlareCargoBay.h"
#include "../../Player/FlarePlayerController.h"

#define LOCTEXT_NAMESPACE "FlareSpacecraftInfo"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSpacecraftInfo::Construct(const FArguments& InArgs)
{
	// Data
	PC = InArgs._Player;
	NoInspect = InArgs._NoInspect;
	Minimized = InArgs._Minimized;
	AFlareGame* Game = InArgs._Player->GetGame();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	// Create the layout
	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Left)
	[
		SNew(SVerticalBox)

		// Header container
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.WidthOverride(Theme.ContentWidth)
			[
				SNew(SHorizontalBox)
			
				// Data block
				+ SHorizontalBox::Slot()
				[
					SNew(SVerticalBox)

					// Main line
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Class icon
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(FMargin(8))
						.VAlign(VAlign_Center)
						[
							SNew(SImage).Image(this, &SFlareSpacecraftInfo::GetClassIcon)
						]

						// Ship name
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(FMargin(10))
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(this, &SFlareSpacecraftInfo::GetName)
							.TextStyle(&Theme.NameFont)
						]

						// Ship class
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(FMargin(12))
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(this, &SFlareSpacecraftInfo::GetDescription)
							.TextStyle(&Theme.TextFont)
						]

						// Status
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Right)
						[
							SAssignNew(ShipStatus, SFlareShipStatus)
						]
					]

					// Company line
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Company flag
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(FMargin(8))
						[
							SAssignNew(CompanyFlag, SFlareCompanyFlag)
							.Player(InArgs._Player)
							.Visibility(this, &SFlareSpacecraftInfo::GetCompanyFlagVisibility)
						]

						// Company info
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(FMargin(8))
						[
							SNew(STextBlock)
							.Text(this, &SFlareSpacecraftInfo::GetSpacecraftInfo)
							.TextStyle(&Theme.TextFont)
							.Visibility(this, &SFlareSpacecraftInfo::GetSpacecraftInfoVisibility)
						]
					]

					// Cargo bay block
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(6))
					.HAlign(HAlign_Left)
					[
						SAssignNew(CargoBay, SHorizontalBox)
					]

					// Buttons : Generic
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(8, 8, 8, 0))
					[
						SNew(SHorizontalBox)

						// Inspect
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew(InspectButton, SFlareButton)
							.Text(LOCTEXT("Inspect", "INSPECT"))
							.HelpText(LOCTEXT("InspectInfo", "Take a closer look at this spacecraft"))
							.OnClicked(this, &SFlareSpacecraftInfo::OnInspect)
							.Width(4)
						]

						// Upgrade
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew(UpgradeButton, SFlareButton)
							.Text(LOCTEXT("Upgrade", "UPGRADE"))
							.HelpText(LOCTEXT("UpgradeInfo", "Upgrade this spacecraft"))
							.OnClicked(this, &SFlareSpacecraftInfo::OnUpgrade)
							.Width(4)
						]

						// Fly this ship
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew(FlyButton, SFlareButton)
							.Text(LOCTEXT("ShipFly", "FLY"))
							.HelpText(LOCTEXT("ShipFlyInfo", "Take command of this spacecraft"))
							.OnClicked(this, &SFlareSpacecraftInfo::OnFly)
							.Width(4)
						]
					]

					// Buttons : advanced
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(8, 0, 8, 8))
					[
						SNew(SHorizontalBox)

						// Select this ship
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew(SelectButton, SFlareButton)
							.Text(LOCTEXT("ShipSelect", "SELECT"))
							.HelpText(LOCTEXT("ShipSelectInfo", "Use this ship for orital navigation"))
							.OnClicked(this, &SFlareSpacecraftInfo::OnSelect)
							.Width(4)
						]

						// Trade
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew(TradeButton, SFlareButton)
							.Text(LOCTEXT("Trade", "TRADE"))
							.HelpText(LOCTEXT("TradeInfo", "Trade with this spacecraft"))
							.OnClicked(this, &SFlareSpacecraftInfo::OnTrade)
							.Width(4)
						]

						// Assign to sector
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew(AssignButton, SFlareButton)
							.Text(LOCTEXT("Assign", "ASSIGN HERE"))
							.HelpText(LOCTEXT("AssignInfo", "Assign this ship to it's current sector"))
							.OnClicked(this, &SFlareSpacecraftInfo::OnAssign)
							.Width(4)
						]

						// UnAssign to sector
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew(UnassignButton, SFlareButton)
							.Text(LOCTEXT("Unassign", "UNASSIGN"))
							.HelpText(LOCTEXT("UnassignInfo", "Unassign this ship from the sector"))
							.OnClicked(this, &SFlareSpacecraftInfo::OnUnassign)
							.Width(4)
						]
			
						// Dock here
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew(DockButton, SFlareButton)
							.Text(LOCTEXT("Dock", "DOCK HERE"))
							.HelpText(LOCTEXT("DockInfo", "Try to dock at this spacecraft"))
							.OnClicked(this, &SFlareSpacecraftInfo::OnDockAt)
							.Width(4)
						]

						// Undock
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew(UndockButton, SFlareButton)
							.Text(LOCTEXT("Undock", "UNDOCK"))
							.HelpText(LOCTEXT("UndockInfo", "Undock from this spacecraft and go back to flying the ship"))
							.OnClicked(this, &SFlareSpacecraftInfo::OnUndock)
							.Width(4)
						]
					]
				]

				// Icon
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Top)
				[
					SNew(SImage).Image(this, &SFlareSpacecraftInfo::GetIcon)
					.Visibility(InArgs._NoInspect ? EVisibility::Hidden : EVisibility::Visible)
				]
			]
		]
	];

	// Setup
	if (InArgs._Spacecraft)
	{
		SetSpacecraft(InArgs._Spacecraft);
	}
	if (InArgs._Visible && TargetSpacecraft)
	{
		Show();
	}
	else
	{
		Hide();
	}
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareSpacecraftInfo::SetSpacecraft(IFlareSpacecraftInterface* Target)
{
	TargetSpacecraft = Target;
	ShipStatus->SetTargetShip(Target);

	// Get the save data info to retrieve the class data
	if (Target && PC)
	{
		// Setup basic info
		CompanyFlag->SetCompany(Target->GetCompany());
		TargetName = FText::FromName(Target->GetImmatriculation());
		FFlareSpacecraftSave* SaveData = Target->Save();
		if (SaveData)
		{
			TargetSpacecraftDesc = PC->GetGame()->GetSpacecraftCatalog()->Get(SaveData->Identifier);
		}

		// Fill the cargo bay
		CargoBay->ClearChildren();
		if (Target->GetCompany()->GetPlayerHostility() == EFlareHostility::Owned)
		{
			for (uint32 CargoIndex = 0; CargoIndex < Target->GetCargoBay()->GetSlotCount() ; CargoIndex++)
			{
				CargoBay->AddSlot()
				[
					SNew(SFlareCargoInfo)
					.Spacecraft(Target)
					.CargoIndex(CargoIndex)
				];
			}
		}
	}
}

void SFlareSpacecraftInfo::SetNoInspect(bool NewState)
{
	NoInspect = NewState;
}

void SFlareSpacecraftInfo::SetMinimized(bool NewState)
{
	Minimized = NewState;

	if (GetVisibility() == EVisibility::Visible)
	{
		Show();
	}
}

void SFlareSpacecraftInfo::Show()
{
	SetVisibility(EVisibility::Visible);

	if (Minimized)
	{
		CargoBay->SetVisibility(EVisibility::Collapsed);
		InspectButton->SetVisibility(EVisibility::Collapsed);
		UpgradeButton->SetVisibility(EVisibility::Collapsed);
		TradeButton->SetVisibility(EVisibility::Collapsed);
		AssignButton->SetVisibility(EVisibility::Collapsed);
		UnassignButton->SetVisibility(EVisibility::Collapsed);
		FlyButton->SetVisibility(EVisibility::Collapsed);
		SelectButton->SetVisibility(EVisibility::Collapsed);
		DockButton->SetVisibility(EVisibility::Collapsed);
		UndockButton->SetVisibility(EVisibility::Collapsed);
	}
	else if (TargetSpacecraft)
	{
		// Useful data
		IFlareSpacecraftDockingSystemInterface* TargetDockingSystem = TargetSpacecraft->GetDockingSystem();
		bool Owned = TargetSpacecraft->GetCompany()->GetPlayerHostility() == EFlareHostility::Owned;
		bool OwnedAndNotSelf = Owned && TargetSpacecraft != PC->GetShipPawn();
		bool IsDocked = TargetDockingSystem->IsDockedShip(PC->GetShipPawn());
		bool CanDock = OwnedAndNotSelf && TargetDockingSystem->HasCompatibleDock(PC->GetShipPawn()) && !IsDocked;
		bool CanTrade = OwnedAndNotSelf && !TargetSpacecraft->IsStation() && TargetSpacecraft->GetDescription()->CargoBayCount > 0;
		bool CanAssign = OwnedAndNotSelf && !TargetSpacecraft->IsStation();
		bool CanUpgrade = Owned && !TargetSpacecraft->IsStation() && (IsDocked || Cast<AFlareSpacecraft>(TargetSpacecraft) == NULL);

		// Button states
		CargoBay->SetVisibility(CargoBay->NumSlots() > 0 ? EVisibility::Visible : EVisibility::Collapsed);
		InspectButton->SetVisibility(NoInspect ? EVisibility::Collapsed : EVisibility::Visible);
		UpgradeButton->SetVisibility(CanUpgrade ? EVisibility::Visible : EVisibility::Collapsed);
		TradeButton->SetVisibility(CanTrade ? EVisibility::Visible : EVisibility::Collapsed);
		DockButton->SetVisibility(CanDock ? EVisibility::Visible : EVisibility::Collapsed);
		UndockButton->SetVisibility(IsDocked ? EVisibility::Visible : EVisibility::Collapsed);

		// Flyable ships
		if (OwnedAndNotSelf && TargetSpacecraft->CanBeFlown())
		{
			FlyButton->SetVisibility(EVisibility::Visible);
			SelectButton->SetVisibility(EVisibility::Visible);
		}
		else
		{
			FlyButton->SetVisibility(EVisibility::Collapsed);
			SelectButton->SetVisibility(EVisibility::Collapsed);
		}

		if (CanAssign)
		{
			AssignButton->SetVisibility(TargetSpacecraft->IsAssignedToSector() ? EVisibility::Collapsed : EVisibility::Visible);
			UnassignButton->SetVisibility(TargetSpacecraft->IsAssignedToSector() ? EVisibility::Visible : EVisibility::Collapsed);
		}
		else
		{
			AssignButton->SetVisibility(EVisibility::Collapsed);
			UnassignButton->SetVisibility(EVisibility::Collapsed);
		}
	}
}

void SFlareSpacecraftInfo::Hide()
{
	TargetSpacecraft = NULL;
	TargetSpacecraftDesc = NULL;
	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareSpacecraftInfo::OnInspect()
{
	if (PC && TargetSpacecraft)
	{
		FLOGV("SFlareSpacecraftInfo::OnInspect : TargetSpacecraft=%p", TargetSpacecraft);
		UFlareSimulatedSpacecraft* SimulatedSpacecraft = Cast<UFlareSimulatedSpacecraft>(TargetSpacecraft);
		if(SimulatedSpacecraft)
		{
			FLOGV("SFlareSpacecraftInfo::OnInspect : Immatriculation=%s, Parent Sector=%p",
				*SimulatedSpacecraft->GetImmatriculation().ToString(), SimulatedSpacecraft->GetCurrentSector());
		}

		PC->GetMenuManager()->OpenMenuSpacecraft(EFlareMenu::MENU_Ship, TargetSpacecraft);
	}
}

void SFlareSpacecraftInfo::OnUpgrade()
{
	if (PC && TargetSpacecraft)
	{
		FLOGV("SFlareSpacecraftInfo::OnUpgrade : TargetSpacecraft=%p", TargetSpacecraft);
		PC->GetMenuManager()->OpenMenuSpacecraft(EFlareMenu::MENU_ShipConfig, TargetSpacecraft);
	}
}

void SFlareSpacecraftInfo::OnTrade()
{
	if (PC && TargetSpacecraft)
	{
		PC->GetMenuManager()->OpenMenuSpacecraft(EFlareMenu::MENU_Trade, TargetSpacecraft);
	}
}

void SFlareSpacecraftInfo::OnFly()
{
	if (PC && TargetSpacecraft && TargetSpacecraft->CanBeFlown())
	{
		// Check if a simulated spacecraft
		UFlareSimulatedSpacecraft* SimulatedSpacecraft = Cast<UFlareSimulatedSpacecraft>(TargetSpacecraft);

		if (SimulatedSpacecraft)
		{
			SimulatedSpacecraft->GetCurrentSector()->SetShipToFly(SimulatedSpacecraft);
			PC->GetGame()->ActivateSector(PC, SimulatedSpacecraft->GetCurrentSector());
		}
		else
		{
			PC->GetMenuManager()->OpenMenuSpacecraft(EFlareMenu::MENU_FlyShip, Cast<AFlareSpacecraft>(TargetSpacecraft));
		}
	}
}

void SFlareSpacecraftInfo::OnSelect()
{
	if (PC && TargetSpacecraft && TargetSpacecraft->CanBeFlown())
	{
		// Check if a simulated spacecraft
		UFlareSimulatedSpacecraft* SimulatedSpacecraft = Cast<UFlareSimulatedSpacecraft>(TargetSpacecraft);

		if (!SimulatedSpacecraft)
		{
			SimulatedSpacecraft = PC->GetGame()->GetGameWorld()->FindSpacecraft(TargetSpacecraft->GetImmatriculation());
		}

		PC->SelectFleet(SimulatedSpacecraft->GetCurrentFleet());
		PC->GetMenuManager()->OpenMenu(EFlareMenu::MENU_Orbit);
	}
}

void SFlareSpacecraftInfo::OnDockAt()
{
	if (PC && TargetSpacecraft && TargetSpacecraft->GetDockingSystem()->GetDockCount() > 0)
	{
		PC->GetShipPawn()->GetNavigationSystem()->DockAt(TargetSpacecraft);
		PC->GetMenuManager()->CloseMenu();
	}
}

void SFlareSpacecraftInfo::OnUndock()
{
	if (PC && TargetSpacecraft && TargetSpacecraft->GetDockingSystem()->GetDockCount() > 0)
	{
		PC->GetShipPawn()->GetNavigationSystem()->Undock();
		PC->GetMenuManager()->CloseMenu();
	}
}

void SFlareSpacecraftInfo::OnAssign()
{
	if (PC && TargetSpacecraft)
	{
		TargetSpacecraft->AssignToSector(true);
		Show();
	}
}

void SFlareSpacecraftInfo::OnUnassign()
{
	if (PC && TargetSpacecraft)
	{
		TargetSpacecraft->AssignToSector(false);
		Show();
	}
}


/*----------------------------------------------------
	Content
----------------------------------------------------*/

FText SFlareSpacecraftInfo::GetName() const
{
	return TargetName;
}

FText SFlareSpacecraftInfo::GetDescription() const
{
	// Common text
	FText DefaultText = LOCTEXT("Default", "UNKNOWN OBJECT");

	// Description builder
	if (TargetSpacecraftDesc)
	{
		return FText::Format(LOCTEXT("DescriptionFormat", "({0})"), TargetSpacecraftDesc->Name);
	}

	return DefaultText;
}

const FSlateBrush* SFlareSpacecraftInfo::GetIcon() const
{
	if (TargetSpacecraftDesc)
	{
		return &TargetSpacecraftDesc->MeshPreviewBrush;
	}
	return NULL;
}

const FSlateBrush* SFlareSpacecraftInfo::GetClassIcon() const
{
	if (TargetSpacecraftDesc)
	{
		return IFlareSpacecraftInterface::GetIcon(TargetSpacecraftDesc);
	}
	return NULL;
}

EVisibility SFlareSpacecraftInfo::GetCompanyFlagVisibility() const
{
	if (TargetSpacecraft)
	{
		UFlareCompany* TargetCompany = TargetSpacecraft->GetCompany();

		if (TargetCompany && PC && TargetCompany == PC->GetCompany())
		{
			// Not interesting after all
			return EVisibility::Collapsed;
		}
	}

	return EVisibility::Visible;
}

EVisibility SFlareSpacecraftInfo::GetSpacecraftInfoVisibility() const
{
	return (GetSpacecraftInfo().ToString().Len() > 0) ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SFlareSpacecraftInfo::GetSpacecraftInfo() const
{
	if (TargetSpacecraft)
	{
		UFlareCompany* TargetCompany = TargetSpacecraft->GetCompany();
		UFlareSimulatedSpacecraft* SimulatedSpacecraft = Cast<UFlareSimulatedSpacecraft>(TargetSpacecraft);

		// Our company
		if (TargetCompany && PC && TargetCompany == PC->GetCompany())
		{
			// Station : show production, if simulated
			if (TargetSpacecraft->IsStation())
			{
				if (SimulatedSpacecraft)
				{
					FText ProductionStatusText = FText();
					TArray<UFlareFactory*>& Factories = SimulatedSpacecraft->GetFactories();

					for (int FactoryIndex = 0; FactoryIndex < Factories.Num(); FactoryIndex++)
					{
						FText NewLineText = (FactoryIndex > 0) ? FText::FromString("\n") : FText();
						UFlareFactory* Factory = Factories[FactoryIndex];

						ProductionStatusText = FText::Format(LOCTEXT("ProductionStatusFormat", "{0}{1}{2} : {3}"),
							ProductionStatusText,
							NewLineText,
							Factory->GetDescription()->Name,
							Factory->GetFactoryStatus());
					}

					return ProductionStatusText;
				}
			}

			// Ship : show fleet, if > 1 ship
			else if (SimulatedSpacecraft)
			{
				UFlareFleet* Fleet = SimulatedSpacecraft->GetCurrentFleet();
				if (Fleet->GetShips().Num() > 1)
				{
					return FText::Format(LOCTEXT("FleetFormat", "In fleet \"{0}\""), Fleet->GetFleetName());
				}
			}
		}

		// Other company
		else
		{
			return FText::Format(LOCTEXT("OwnedByFormat", "Owned by {0}"), TargetCompany->GetShortInfoText());
		}
	}

	return FText();
}


#undef LOCTEXT_NAMESPACE
