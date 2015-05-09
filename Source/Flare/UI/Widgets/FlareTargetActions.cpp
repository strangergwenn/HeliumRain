
#include "../../Flare.h"
#include "FlareTargetActions.h"

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
	const FFlareButtonStyle* ButtonStyle = &FFlareStyleSet::Get().GetWidgetStyle<FFlareButtonStyle>("/Style/ActionButton");

	// Container style
	const FFlareContainerStyle* ContainerStyle;
	if (InArgs._Translucent)
	{
		ContainerStyle = &FFlareStyleSet::Get().GetWidgetStyle<FFlareContainerStyle>("/Style/InvisibleContainerStyle");
	}
	else
	{
		ContainerStyle = &FFlareStyleSet::Get().GetWidgetStyle<FFlareContainerStyle>("/Style/DefaultContainerStyle");
	}

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SVerticalBox)

		// Header container
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.WidthOverride(ButtonStyle->Width)
			.HeightOverride(ButtonStyle->Height)
			[
				SNew(SBorder)
				.VAlign(VAlign_Center)
				.BorderImage(&ContainerStyle->BackgroundBrush)
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
								.TextStyle(FFlareStyleSet::Get(), "Flare.Title3")
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
								.TextStyle(FFlareStyleSet::Get(), "Flare.Text")
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
								.TextStyle(FFlareStyleSet::Get(), "Flare.Text")
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
		]

		// Company menu container
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(CompanyContainer, SHorizontalBox)

			// Inspect a company
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SFlareButton)
				.Text(LOCTEXT("CompanyInspect", "INSPECT"))
				.OnClicked(this, &SFlareTargetActions::OnInspect)
			]
		]
		
		// Station menu container
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(StationContainer, SHorizontalBox)

			// Inspect a station
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(StationInspectButton, SFlareButton)
				.Text(LOCTEXT("StationInspect", "INSPECT"))
				.OnClicked(this, &SFlareTargetActions::OnInspect)
			]

			// Dock at this station
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(DockButton, SFlareButton)
				.Text(LOCTEXT("StationDock", "DOCK HERE"))
				.OnClicked(this, &SFlareTargetActions::OnDockAt)
			]

			// Dock at this station
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(UndockButton, SFlareButton)
				.Text(LOCTEXT("StationUnDock", "UNDOCK"))
				.OnClicked(this, &SFlareTargetActions::OnUndock)
			]
		]

		// Ship menu container
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(ShipContainer, SHorizontalBox)

			// Inspect a ship
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(ShipInspectButton, SFlareButton)
				.Text(LOCTEXT("ShipInspect", "INSPECT"))
				.OnClicked(this, &SFlareTargetActions::OnInspect)
			]

			// Fly this ship
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(ShipFlyButton, SFlareButton)
				.Text(LOCTEXT("ShipFly", "FLY"))
				.OnClicked(this, &SFlareTargetActions::OnFly)
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
	TargetStationDesc = NULL;
	TargetStation = NULL;
	TargetShipDesc = NULL;
	TargetShip = NULL;
	ShipStatus->SetTargetShip(NULL);

	// Get the save data info to retrieve the class data
	if (Target && PC)
	{
		// Data
		CompanyFlag->SetCompany(Target);
		TargetName = Target->GetName();
	}
}

void SFlareTargetActions::SetStation(IFlareStationInterface* Target)
{
	TargetCompany = NULL;
	TargetStation = Target;
	TargetShipDesc = NULL;
	TargetShip = NULL;
	ShipStatus->SetTargetShip(NULL);

	// Get the save data info to retrieve the class data
	if (Target && PC)
	{
		// Data
		TargetCompany = Target->GetCompany();
		CompanyFlag->SetCompany(TargetCompany);
		TargetName = Target->_getUObject()->GetName();
		FFlareStationSave* SaveData = Target->Save();
		if (SaveData)
		{
			TargetStationDesc = PC->GetGame()->GetStationCatalog()->Get(SaveData->Identifier);
		}

		// Are we docked here
		bool Docked = false;
		if (PC->GetShipPawn()->IsDocked())
		{
			if (PC->GetShipPawn()->GetDockStation() == Target)
			{
				Docked = true;
			}
		}

		// Docking
		if (!MinimizedMode)
		{
			UndockButton->SetVisibility(Docked ? EVisibility::Visible : EVisibility::Collapsed);
			DockButton->SetVisibility(Docked ? EVisibility::Collapsed : EVisibility::Visible);
		}
	}
}

void SFlareTargetActions::SetShip(IFlareShipInterface* Target)
{
	TargetCompany = NULL;
	TargetShip = Target;
	TargetStation = NULL;
	TargetStationDesc = NULL;
	ShipStatus->SetTargetShip(Target);

	// Get the save data info to retrieve the class data
	if (Target && PC)
	{
		TargetCompany = Target->GetCompany();
		CompanyFlag->SetCompany(TargetCompany);
		TargetName = Target->_getUObject()->GetName();
		FFlareShipSave* SaveData = Target->Save();
		if (SaveData)
		{
			TargetShipDesc = PC->GetGame()->GetShipCatalog()->Get(SaveData->Identifier);
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
		CompanyContainer->SetVisibility(EVisibility::Collapsed);
		StationContainer->SetVisibility(EVisibility::Collapsed);
		ShipContainer->SetVisibility(EVisibility::Collapsed);
	}
	else
	{
		if (TargetStation)
		{
			CompanyContainer->SetVisibility(EVisibility::Collapsed);
			StationContainer->SetVisibility(EVisibility::Visible);
			ShipContainer->SetVisibility(EVisibility::Collapsed);
			StationInspectButton->SetVisibility(NoInspect ? EVisibility::Collapsed : EVisibility::Visible);
		}
		else if (TargetShip)
		{
			CompanyContainer->SetVisibility(EVisibility::Collapsed);
			StationContainer->SetVisibility(EVisibility::Collapsed);
			ShipContainer->SetVisibility(EVisibility::Visible);
			ShipInspectButton->SetVisibility(NoInspect ? EVisibility::Collapsed : EVisibility::Visible);

			if (TargetShip != PC->GetShipPawn() && TargetShip->GetCompany()->GetPlayerHostility() == EFlareHostility::Owned)
			{
				ShipFlyButton->SetVisibility(EVisibility::Visible);
			}
			else
			{
				ShipFlyButton->SetVisibility(EVisibility::Collapsed);
			}
		}
		else if (TargetCompany)
		{
			CompanyContainer->SetVisibility(EVisibility::Visible);
			StationContainer->SetVisibility(EVisibility::Collapsed);
			ShipContainer->SetVisibility(EVisibility::Collapsed);
		}
	}
}

void SFlareTargetActions::Hide()
{
	TargetCompany = NULL;
	TargetShip = NULL;
	TargetStation = NULL;
	TargetShipDesc = NULL;
	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareTargetActions::OnInspect()
{
	if (PC)
	{
		if (TargetStation)
		{
			Cast<AFlareHUD>(PC->GetHUD())->OpenMenu(EFlareMenu::MENU_Station, TargetStation);
		}
		else if (TargetShip)
		{
			Cast<AFlareHUD>(PC->GetHUD())->OpenMenu(EFlareMenu::MENU_Ship, TargetShip);
		}
		else if (TargetCompany)
		{
			Cast<AFlareHUD>(PC->GetHUD())->OpenMenu(EFlareMenu::MENU_Company, TargetCompany);
		}
	}
}

void SFlareTargetActions::OnFly()
{
	if (PC && TargetShip)
	{
		PC->FlyShip(Cast<AFlareShip>(TargetShip));
		Cast<AFlareHUD>(PC->GetHUD())->CloseMenu();
	}
}

void SFlareTargetActions::OnDockAt()
{
	if (PC && TargetStation)
	{
		PC->GetShipPawn()->DockAt(TargetStation);
		Cast<AFlareHUD>(PC->GetHUD())->CloseMenu();
	}
}

void SFlareTargetActions::OnUndock()
{
	if (PC && TargetStation)
	{
		PC->GetShipPawn()->Undock();
		Cast<AFlareHUD>(PC->GetHUD())->CloseMenu();
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
	if (TargetStationDesc)
	{
		return FText::FromString(TargetStationDesc->Name.ToString() + " " + ClassText.ToString());
	}
	else if (TargetShipDesc)
	{
		return FText::FromString(TargetShipDesc->Name.ToString() + " " + ClassText.ToString());
	}
	else if (TargetCompany)
	{
		return LOCTEXT("Company", "COMPANY");
	}
	return DefaultText;
}

const FSlateBrush* SFlareTargetActions::GetIcon() const
{
	if (TargetStationDesc)
	{
		return &TargetStationDesc->MeshPreviewBrush;
	}
	else if (TargetShipDesc)
	{
		return &TargetShipDesc->MeshPreviewBrush;
	}
	else if (TargetCompany)
	{
		return NULL;
	}
	return NULL;
}

const FSlateBrush* SFlareTargetActions::GetClassIcon() const
{
	if (TargetStationDesc)
	{
		return IFlareStationInterface::GetIcon(TargetStationDesc);
	}
	else if (TargetShipDesc)
	{
		return IFlareShipInterface::GetIcon(TargetShipDesc);
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
