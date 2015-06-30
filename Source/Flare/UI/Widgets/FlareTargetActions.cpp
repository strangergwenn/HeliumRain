
#include "../../Flare.h"
#include "FlareTargetActions.h"
#include "../../Player/FlarePlayerController.h"

#define LOCTEXT_NAMESPACE "FlareTargetActions"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareTargetActions::Construct(const FArguments& InArgs)
{
	// Data
	PC = InArgs._Player;
	NoInspect = InArgs._NoInspect;
	MinimizedMode = InArgs._MinimizedMode;
	AFlareGame* Game = InArgs._Player->GetGame();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	// Create the layout
	ChildSlot
	.VAlign(VAlign_Fill)
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
							SNew(SImage).Image(this, &SFlareTargetActions::GetClassIcon)
						]

						// Ship name
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(FMargin(10))
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(this, &SFlareTargetActions::GetName)
							.TextStyle(&Theme.NameFont)
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
							.Text(this, &SFlareTargetActions::GetCompanyName)
							.TextStyle(&Theme.TextFont)
						]
					]

					// Ship info line
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
							
						// Ship class
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(FMargin(8))
						[
							SNew(STextBlock)
							.Text(this, &SFlareTargetActions::GetDescription)
							.TextStyle(&Theme.TextFont)
						]
					]
				]

				// Icon
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Right)
				[
					SNew(SImage).Image(this, &SFlareTargetActions::GetIcon)
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
				.OnClicked(this, &SFlareTargetActions::OnInspect)
			]

			// Fly this ship
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(FlyButton, SFlareButton)
				.Text(LOCTEXT("ShipFly", "FLY"))
				.OnClicked(this, &SFlareTargetActions::OnFly)
			]
			
			// Dock here
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(DockButton, SFlareButton)
				.Text(LOCTEXT("Dock", "DOCK HERE"))
				.OnClicked(this, &SFlareTargetActions::OnDockAt)
			]

			// Undock
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(UndockButton, SFlareButton)
				.Text(LOCTEXT("Undock", "UNDOCK"))
				.OnClicked(this, &SFlareTargetActions::OnUndock)
			]
		]
	];

	Hide();
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareTargetActions::SetCompany(UFlareCompany* Target)
{
	TargetCompany = Target;
	TargetSpacecraft = NULL;
	TargetSpacecraftDesc = NULL;
	ShipStatus->SetTargetShip(NULL);

	// Get the save data info to retrieve the class data
	if (Target && PC)
	{
		// Data
		CompanyFlag->SetCompany(Target);
		TargetName = Target->GetName();
	}
}

void SFlareTargetActions::SetSpacecraft(IFlareSpacecraftInterface* Target)
{
	TargetCompany = NULL;
	TargetSpacecraft = Target;
	ShipStatus->SetTargetShip(Target);

	// Get the save data info to retrieve the class data
	if (Target && PC)
	{
		TargetCompany = Target->GetCompany();
		CompanyFlag->SetCompany(TargetCompany);
		TargetName = Target->_getUObject()->GetName();
		FFlareSpacecraftSave* SaveData = Target->Save();
		if (SaveData)
		{
			TargetSpacecraftDesc = PC->GetGame()->GetSpacecraftCatalog()->Get(SaveData->Identifier);
		}
	}
}

void SFlareTargetActions::SetNoInspect(bool NewState)
{
	NoInspect = NewState;
}

void SFlareTargetActions::SetMinimized(bool NewState)
{
	MinimizedMode = NewState;
}

void SFlareTargetActions::Show()
{
	SetVisibility(EVisibility::Visible);

	if (MinimizedMode)
	{
		InspectButton->SetVisibility(EVisibility::Collapsed);
		FlyButton->SetVisibility(EVisibility::Collapsed);
		DockButton->SetVisibility(EVisibility::Collapsed);
		UndockButton->SetVisibility(EVisibility::Collapsed);
	}
	else
	{
		if (TargetSpacecraft)
		{
			// Useful data
			UFlareSpacecraftDockingSystem* TargetDockingSystem = TargetSpacecraft->GetDockingSystem();
			bool OwnedAndNotSelf = TargetSpacecraft != PC->GetShipPawn() && TargetSpacecraft->GetCompany()->GetPlayerHostility() == EFlareHostility::Owned;
			bool IsDocked = TargetDockingSystem->IsDockedShip(PC->GetShipPawn());
			bool CanDock = OwnedAndNotSelf && TargetDockingSystem->HasCompatibleDock(PC->GetShipPawn()) && !IsDocked;

			// Button states
			InspectButton->SetVisibility(NoInspect ? EVisibility::Collapsed : EVisibility::Visible);
			DockButton->SetVisibility(CanDock ? EVisibility::Visible : EVisibility::Collapsed);
			UndockButton->SetVisibility(IsDocked ? EVisibility::Visible : EVisibility::Collapsed);

			// Flyable ships
			if (OwnedAndNotSelf && !TargetSpacecraft->IsStation())
			{
				FlyButton->SetVisibility(EVisibility::Visible);
			}
			else
			{
				FlyButton->SetVisibility(EVisibility::Collapsed);
			}
		}

		// Company
		else if (TargetCompany)
		{
			InspectButton->SetVisibility(EVisibility::Collapsed);
			FlyButton->SetVisibility(EVisibility::Collapsed);
			DockButton->SetVisibility(EVisibility::Collapsed);
			UndockButton->SetVisibility(EVisibility::Collapsed);
		}
	}
}

void SFlareTargetActions::Hide()
{
	TargetCompany = NULL;
	TargetSpacecraft = NULL;
	TargetSpacecraftDesc = NULL;
	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareTargetActions::OnInspect()
{
	if (PC)
	{
		if (TargetSpacecraft)
		{
			PC->GetMenuManager()->OpenMenu(TargetSpacecraft->IsStation() ? EFlareMenu::MENU_Station : EFlareMenu::MENU_Ship, TargetSpacecraft);
		}
		else if (TargetCompany)
		{
			PC->GetMenuManager()->OpenMenu(EFlareMenu::MENU_Company, TargetCompany);
		}
	}
}

void SFlareTargetActions::OnFly()
{
	if (PC && TargetSpacecraft && !TargetSpacecraft->IsStation())
	{
		PC->GetMenuManager()->OpenMenu(EFlareMenu::MENU_FlyShip, Cast<AFlareSpacecraft>(TargetSpacecraft));
	}
}

void SFlareTargetActions::OnDockAt()
{
	if (PC && TargetSpacecraft && TargetSpacecraft->GetDockingSystem()->GetDockCount() > 0)
	{
		PC->GetShipPawn()->GetNavigationSystem()->DockAt(TargetSpacecraft);
		PC->GetMenuManager()->CloseMenu();
	}
}

void SFlareTargetActions::OnUndock()
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

FText SFlareTargetActions::GetName() const
{
	return FText::FromString(TargetName);
}

FText SFlareTargetActions::GetDescription() const
{
	// Common text
	FText ClassText = LOCTEXT("Class", "CLASS");
	FText DefaultText = LOCTEXT("Default", "UNKNOWN OBJECT");

	// Description builder
	if (TargetSpacecraftDesc)
	{
		return FText::FromString(TargetSpacecraftDesc->Name.ToString() + " " + ClassText.ToString());
	}
	else if (TargetCompany)
	{
		return LOCTEXT("Company", "COMPANY");
	}
	return DefaultText;
}

const FSlateBrush* SFlareTargetActions::GetIcon() const
{
	if (TargetSpacecraftDesc)
	{
		return &TargetSpacecraftDesc->MeshPreviewBrush;
	}
	else if (TargetCompany)
	{
		return NULL;
	}
	return NULL;
}

const FSlateBrush* SFlareTargetActions::GetClassIcon() const
{
	if (TargetSpacecraftDesc)
	{
		return IFlareSpacecraftInterface::GetIcon(TargetSpacecraftDesc);
	}
	else if (TargetCompany)
	{
		return FFlareStyleSet::GetIcon("CompanySmall");
	}
	return NULL;
}

FText SFlareTargetActions::GetCompanyName() const
{
	if (TargetCompany)
	{
		// Static text
		FText ShipText = LOCTEXT("Ship", "ship");
		FText ShipsText = LOCTEXT("Ships", "ships");
		FText StationText = LOCTEXT("Station", "station");
		FText StationsText = LOCTEXT("Stations", "stations");
		FText MoneyText = LOCTEXT("Money", "credits");

		// Dynamic data
		int32 ShipCount = TargetCompany->GetCompanyShips().Num();
		int32 StationCount = TargetCompany->GetCompanyStations().Num();
		FString ShipDescriptionString = FString::FromInt(ShipCount) + " " + (ShipCount > 1 ? ShipsText : ShipText).ToString();
		FString StationDescriptionString = FString::FromInt(StationCount) + " " + (StationCount > 1 ? StationsText : StationText).ToString();
		return FText::FromString((TargetCompany->GetCompanyName() + " (" + StationDescriptionString + ", " + ShipDescriptionString + ")"));
	}
	else
	{
		return LOCTEXT("Abandoned", "ABANDONED OBJECT");
	}
}


#undef LOCTEXT_NAMESPACE
