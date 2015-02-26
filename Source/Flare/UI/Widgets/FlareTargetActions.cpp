
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
	const FFlareButtonStyle* ButtonStyle = &FFlareStyleSet::Get().GetWidgetStyle<FFlareButtonStyle>("/Style/PartButton");
	const FFlareContainerStyle* ContainerStyle = &FFlareStyleSet::Get().GetWidgetStyle<FFlareContainerStyle>("/Style/DefaultContainerStyle");

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
					.Padding(FMargin(10))
					[
						SNew(SVerticalBox)

						// Ship name
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(FMargin(0, 0, 0, 10))
						[
							SNew(STextBlock)
							.Text(this, &SFlareTargetActions::GetName)
							.TextStyle(FFlareStyleSet::Get(), "Flare.Title2")
						]

						// Class line
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)

							// Class icon
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SImage).Image(this, &SFlareTargetActions::GetClassIcon)
							]

							// Ship class
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(FMargin(10))
							[
								SNew(STextBlock)
								.Text(this, &SFlareTargetActions::GetClassName)
								.TextStyle(FFlareStyleSet::Get(), "Flare.Title3")
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
		
		// Station menu container
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(StationContainer, SHorizontalBox)

			// Inspect a station
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SFlareButton)
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
				SNew(SFlareButton)
				.Text(LOCTEXT("ShipInspect", "INSPECT"))
				.OnClicked(this, &SFlareTargetActions::OnInspect)
			]

			// Fly this ship
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SFlareButton)
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

void SFlareTargetActions::SetStation(IFlareStationInterface* Target)
{
	TargetStation = Target;
	TargetShipDesc = NULL;
	TargetShip = NULL;

	// Get the save data info to retrieve the class data
	if (Target && PC)
	{
		// Data
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
		UndockButton->SetVisibility(Docked ? EVisibility::Visible : EVisibility::Collapsed);
		DockButton->SetVisibility(Docked ? EVisibility::Collapsed : EVisibility::Visible);
	}
}

void SFlareTargetActions::SetShip(IFlareShipInterface* Target)
{
	TargetShip = Target;
	TargetStation = NULL;
	TargetStationDesc = NULL;

	// Get the save data info to retrieve the class data
	if (Target && PC)
	{
		TargetName = Target->_getUObject()->GetName();
		FFlareShipSave* SaveData = Target->Save();
		if (SaveData)
		{
			TargetShipDesc = PC->GetGame()->GetShipCatalog()->Get(SaveData->Identifier);
		}
	}
}

void SFlareTargetActions::Show()
{
	SetVisibility(EVisibility::Visible);
	if (TargetStation)
	{
		StationContainer->SetVisibility(EVisibility::Visible);
		ShipContainer->SetVisibility(EVisibility::Collapsed);
	}
	else if (TargetShip)
	{
		StationContainer->SetVisibility(EVisibility::Collapsed);
		ShipContainer->SetVisibility(EVisibility::Visible);
	}
}

void SFlareTargetActions::Hide()
{
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

FString SFlareTargetActions::GetName() const
{
	return TargetName;
}

FString SFlareTargetActions::GetClassName() const
{
	if (TargetStationDesc)
	{
		return "STATION - " + TargetStationDesc->Name.ToString();
	}
	else if (TargetShipDesc)
	{
		return "SHIP - " + TargetShipDesc->Name.ToString();
	}
	return "";
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
	return NULL;
}

#undef LOCTEXT_NAMESPACE
