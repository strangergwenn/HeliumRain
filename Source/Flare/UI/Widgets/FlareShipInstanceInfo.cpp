
#include "../../Flare.h"
#include "FlareShipInstanceInfo.h"
#include "../Widgets/FlareShipStatus.h"
#include "../../Game/FlareCompany.h"
#include "../../Ships/FlareShipInterface.h"
#include "../../Player/FlarePlayerController.h"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareShipInstanceInfo::Construct(const FArguments& InArgs)
{
	// Data
	Station = InArgs._Station;
	Ship = InArgs._Ship;
	StationData = NULL;
	ShipData = NULL;
	StationDescription = NULL;
	ShipDescription = NULL;
	UObject* Target = NULL;
	const FSlateBrush* Icon = NULL;
	UFlareCompany* Company = NULL;
	AFlareGame* Game = InArgs._Player->GetGame();

	// Station data
	if (Station)
	{
		Target = Station->_getUObject();
		StationData = Station->Save();
		StationDescription = InArgs._Player->GetGame()->GetStationCatalog()->Get(StationData->Identifier);
		Icon = IFlareStationInterface::GetIcon(StationDescription);
		Company = Station->GetCompany();
	}

	// Ship data
	else if (Ship)
	{
		Target = Ship->_getUObject();
		ShipData = Ship->Save();
		ShipDescription = InArgs._Player->GetGame()->GetShipCatalog()->Get(ShipData->Identifier);
		Icon = IFlareShipInterface::GetIcon(ShipDescription);
		Company = Ship->GetCompany();
	}

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Fill)
	[
		SNew(SVerticalBox)

		// Minimal implementation
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(ListContainer, SHorizontalBox)

			// Icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(8))
			.VAlign(VAlign_Center)
			[
				SNew(SImage).Image(Icon)
			]

			// Name
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(10))
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Target->GetName())
				.TextStyle(FFlareStyleSet::Get(), "Flare.Title3")
			]

			// Status
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.Padding(0, 0, 76, 0)
			[
				SNew(SFlareShipStatus)
				.Ship(Ship)
			]

			// Company flag
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(10))
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Right)
			[
				SAssignNew(CompanyFlag, SFlareCompanyFlag)
				.Player(InArgs._Player)
				.Company(Company)
			]
		]

		// Full target actions
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(ActionContainer, SFlareTargetActions)
			.Player(InArgs._Player)
			.Translucent(true)
		]
	];

	// Defaults
	SetActionsVisible(false);
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareShipInstanceInfo::SetActionsVisible(bool State)
{
	if (State)
	{
		ListContainer->SetVisibility(EVisibility::Collapsed);
		if (Station)
		{
			ActionContainer->SetStation(Station);
		}
		else if (Ship)
		{
			ActionContainer->SetShip(Ship);
		}
		ActionContainer->Show();
	}
	else
	{
		ListContainer->SetVisibility(EVisibility::Visible);
		ActionContainer->Hide();
	}
}
