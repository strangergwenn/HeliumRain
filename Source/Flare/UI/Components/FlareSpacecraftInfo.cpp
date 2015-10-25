
#include "../../Flare.h"
#include "FlareSpacecraftInfo.h"
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
						]

						// Company name
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(FMargin(8))
						[
							SNew(STextBlock)
							.Text(this, &SFlareSpacecraftInfo::GetCompanyName)
							.TextStyle(&Theme.TextFont)
						]
					]
				]

				// Icon
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Right)
				[
					SNew(SImage).Image(this, &SFlareSpacecraftInfo::GetIcon)
					.Visibility(InArgs._NoInspect ? EVisibility::Hidden : EVisibility::Visible)
				]
			]
		]

		// General purpose container
		+ SVerticalBox::Slot()
		.AutoHeight()
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
		CompanyFlag->SetCompany(Target->GetCompany());
		TargetName = Target->GetImmatriculation().ToString();
		FFlareSpacecraftSave* SaveData = Target->Save();
		if (SaveData)
		{
			TargetSpacecraftDesc = PC->GetGame()->GetSpacecraftCatalog()->Get(SaveData->Identifier);
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
		InspectButton->SetVisibility(EVisibility::Collapsed);
		FlyButton->SetVisibility(EVisibility::Collapsed);
		DockButton->SetVisibility(EVisibility::Collapsed);
		UndockButton->SetVisibility(EVisibility::Collapsed);
	}
	else if (TargetSpacecraft)
	{
		// Useful data
		IFlareSpacecraftDockingSystemInterface* TargetDockingSystem = TargetSpacecraft->GetDockingSystem();
		bool OwnedAndNotSelf = TargetSpacecraft != PC->GetShipPawn() && TargetSpacecraft->GetCompany()->GetPlayerHostility() == EFlareHostility::Owned;
		bool IsDocked = TargetDockingSystem->IsDockedShip(PC->GetShipPawn());
		bool CanDock = OwnedAndNotSelf && TargetDockingSystem->HasCompatibleDock(PC->GetShipPawn()) && !IsDocked;

		// Button states
		InspectButton->SetVisibility(NoInspect ? EVisibility::Collapsed : EVisibility::Visible);
		DockButton->SetVisibility(CanDock ? EVisibility::Visible : EVisibility::Collapsed);
		UndockButton->SetVisibility(IsDocked ? EVisibility::Visible : EVisibility::Collapsed);

		// Flyable ships
		if (OwnedAndNotSelf && TargetSpacecraft->CanBeFlown())
		{
			FlyButton->SetVisibility(EVisibility::Visible);
		}
		else
		{
			FlyButton->SetVisibility(EVisibility::Collapsed);
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
		PC->GetMenuManager()->OpenMenu(EFlareMenu::MENU_Ship, TargetSpacecraft);
	}
}

void SFlareSpacecraftInfo::OnFly()
{
	if (PC && TargetSpacecraft && !TargetSpacecraft->IsStation())
	{
		// Check if a simulated spacecraft
		UFlareSimulatedSpacecraft* SimulatedSpacecraft = Cast<UFlareSimulatedSpacecraft>(TargetSpacecraft);

		if(SimulatedSpacecraft)
		{
			SimulatedSpacecraft->GetCurrentSector()->SetShipToFly(SimulatedSpacecraft);
			PC->GetGame()->ActivateSector(PC, SimulatedSpacecraft->GetCurrentSector());
		}
		else
		{
			PC->GetMenuManager()->OpenMenu(EFlareMenu::MENU_FlyShip, Cast<AFlareSpacecraft>(TargetSpacecraft));
		}
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


/*----------------------------------------------------
	Content
----------------------------------------------------*/

FText SFlareSpacecraftInfo::GetName() const
{
	return FText::FromString(TargetName);
}

FText SFlareSpacecraftInfo::GetDescription() const
{
	// Common text
	FText DefaultText = LOCTEXT("Default", "UNKNOWN OBJECT");

	// Description builder
	if (TargetSpacecraftDesc)
	{
		return FText::FromString("(" + TargetSpacecraftDesc->Name.ToString() + ")");
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

FText SFlareSpacecraftInfo::GetCompanyName() const
{
	if (TargetSpacecraft)
	{
		UFlareCompany* TargetCompany = TargetSpacecraft->GetCompany();

		if (TargetCompany)
		{
			return FText::FromString(LOCTEXT("OwnedBy", "Owned by ").ToString() + TargetCompany->GetShortInfoText().ToString());
		}
	}

	return FText();
}


#undef LOCTEXT_NAMESPACE
