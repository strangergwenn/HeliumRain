
#include "../../Flare.h"
#include "FlareSectorMenu.h"

#include "../../Game/FlareGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"

#define LOCTEXT_NAMESPACE "FlareSectorMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSectorMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	AFlarePlayerController* PC = MenuManager->GetPC();
	
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
				SNew(SImage).Image(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Sector))
			]

			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TitleFont)
				.Text(LOCTEXT("Sector", "SECTOR INFO"))
			]

			// Close
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("GoOrbit", "Orbital map"))
				.HelpText(LOCTEXT("GoOrbitInfo", "Exit the sector menu and go back to the orbital map"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Orbit, true))
				.OnClicked(this, &SFlareSectorMenu::OnBackClicked)
			]
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(200, 20))
		[
			SNew(SImage).Image(&Theme.SeparatorBrush)
		]

		// UI container
		+ SVerticalBox::Slot()
		.Padding(Theme.ContentPadding)
		.HAlign(HAlign_Center)
		[
			SNew(SVerticalBox)

			// Sector name
			+ SVerticalBox::Slot()
			.Padding(Theme.TitlePadding)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(this, &SFlareSectorMenu::GetSectorName)
				.TextStyle(&Theme.SubTitleFont)
			]

			// Sector description
			+ SVerticalBox::Slot()
			.Padding(Theme.ContentPadding)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(this, &SFlareSectorMenu::GetSectorDescription)
				.TextStyle(&Theme.TextFont)
				.WrapTextAt(Theme.ContentWidth)
			]
				
			// Travel here
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.ContentPadding)
			.HAlign(HAlign_Left)
			[
				SNew(SFlareButton)
				.Text(LOCTEXT("Travel", "Travel"))
				.HelpText(LOCTEXT("TravelInfo", "Start travelling to this sector with the current ship or fleet"))
				.Icon(FFlareStyleSet::GetIcon("Travel"))
				.OnClicked(this, &SFlareSectorMenu::OnTravelHereClicked)
				.Visibility(this, &SFlareSectorMenu::GetTravelVisibility)
			]

			// Content block
			+ SVerticalBox::Slot()
			[
				SNew(SScrollBox)
				.Style(&Theme.ScrollBoxStyle)
				.ScrollBarStyle(&Theme.ScrollBarStyle)

				+ SScrollBox::Slot()
				[
					SNew(SHorizontalBox)

					// Owned
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Right)
					[
						SAssignNew(OwnedShipList, SFlareShipList)
						.MenuManager(MenuManager)
						.Title(LOCTEXT("MyListTitle", "OWNED SPACECRAFTS IN SECTOR"))
					]

					// Others
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					[
						SAssignNew(OtherShipList, SFlareShipList)
						.MenuManager(MenuManager)
						.Title(LOCTEXT("OtherListTitle", "OTHER SPACECRAFTS IN SECTOR"))
					]
				]
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareSectorMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}

void SFlareSectorMenu::Enter(UFlareSimulatedSector* Sector)
{
	FLOG("SFlareSectorMenu::Enter");

	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	TargetSector = Sector;
	AFlarePlayerController* PC = MenuManager->GetPC();

	// Add stations
	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorStations().Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* StationCandidate = Sector->GetSectorStations()[SpacecraftIndex];

		if (StationCandidate)
		{
			if (StationCandidate->GetCompany() == PC->GetCompany())
			{
				OwnedShipList->AddShip(StationCandidate);
			}
			else
			{
				OtherShipList->AddShip(StationCandidate);
			}
		}
	}

	// Add ships
	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorShips().Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* ShipCandidate = Sector->GetSectorShips()[SpacecraftIndex];

		if (ShipCandidate && ShipCandidate->GetDamageSystem()->IsAlive())
		{
			if (ShipCandidate->GetCompany() == PC->GetCompany())
			{
				OwnedShipList->AddShip(ShipCandidate);
			}
			else
			{
				OtherShipList->AddShip(ShipCandidate);
			}
		}
	}

	OwnedShipList->RefreshList();
	OtherShipList->RefreshList();
}

void SFlareSectorMenu::Exit()
{
	SetEnabled(false);
	TargetSector = NULL;
	OwnedShipList->Reset();
	OtherShipList->Reset();
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FText SFlareSectorMenu::GetSectorName() const
{
	FText Result = LOCTEXT("Sector", "SECTOR : ");

	if (TargetSector)
	{
		Result = FText::FromString(Result.ToString() + TargetSector->GetSectorName().ToString());
	}

	return Result;
}

FText SFlareSectorMenu::GetSectorDescription() const
{
	FText Result;

	if (TargetSector)
	{
		Result = TargetSector->GetSectorDescription();
	}

	return Result;
}

EVisibility SFlareSectorMenu::GetTravelVisibility() const
{
	UFlareFleet* CurrentFleet = MenuManager->GetPC()->GetSelectedFleet();

	if (CurrentFleet && CurrentFleet->GetCurrentSector() != TargetSector)
	{
		return EVisibility::Visible;
	}
	else
	{
		return EVisibility::Collapsed;
	}
}

void SFlareSectorMenu::OnBackClicked()
{
	MenuManager->Back();
}

void SFlareSectorMenu::OnTravelHereClicked()
{
	UFlareFleet* SelectedFleet = MenuManager->GetGame()->GetPC()->GetSelectedFleet();
	if(SelectedFleet)
	{
		MenuManager->GetGame()->GetGameWorld()->StartTravel(SelectedFleet, TargetSector);
		MenuManager->Back();
	}
}

#undef LOCTEXT_NAMESPACE

