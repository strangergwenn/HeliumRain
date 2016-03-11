
#include "../../Flare.h"
#include "FlareFleetMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../Components/FlareRoundButton.h"
#include "../Components/FlareCargoInfo.h"
#include "../../Economy/FlareCargoBay.h"

#define LOCTEXT_NAMESPACE "FlareFleetMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareFleetMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
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
				SNew(SImage).Image(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Fleet))
			]

			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Fleets", "FLEETS"))
				.TextStyle(&Theme.TitleFont)
			]

			// Close
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("Back", "Back"))
				.HelpText(LOCTEXT("BackInfo", "Go to the previous menu"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Exit, true))
				.OnClicked(this, &SFlareFleetMenu::OnBackClicked)
			]
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(200, 20))
		[
			SNew(SImage).Image(&Theme.SeparatorBrush)
		]

		// Content block
		+ SVerticalBox::Slot()
		.Padding(Theme.ContentPadding)
		.HAlign(HAlign_Center)
		[
			SNew(SScrollBox)
			.Style(&Theme.ScrollBoxStyle)
			.ScrollBarStyle(&Theme.ScrollBarStyle)

			+ SScrollBox::Slot()
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				[
					SNew(SBox)
					.HAlign(HAlign_Left)
					.WidthOverride(Theme.ContentWidth)
				]

				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				[
					SNew(SBox)
					.HAlign(HAlign_Fill)
					.WidthOverride(Theme.ContentWidth)
					[
						SNew(SVerticalBox)

						
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(ShipList, SFlareShipList)
							.MenuManager(MenuManager)
							.Title(LOCTEXT("SelectSpacecraft", "SELECT A SPECACRAFT TO TRADE WITH"))
							.OnItemSelected(this, &SFlareFleetMenu::OnSpacecraftSelected)
						]
					]
				]
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareFleetMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
	ShipList->SetVisibility(EVisibility::Collapsed);
}

void SFlareFleetMenu::Enter(UFlareFleet* TargetFleet)
{
	FLOGV("SFlareFleetMenu::Enter : TargetFleet=%p", TargetFleet);

	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	AFlarePlayerController* PC = MenuManager->GetPC();

	// Add ships
	for (int32 SpacecraftIndex = 0; SpacecraftIndex < PC->GetCompany()->GetCompanyShips().Num(); SpacecraftIndex++)
	{
		IFlareSpacecraftInterface* ShipCandidate = PC->GetCompany()->GetCompanyShips()[SpacecraftIndex];
		if (ShipCandidate && ShipCandidate->GetDamageSystem()->IsAlive())
		{
			ShipList->AddShip(ShipCandidate);
		}
	}

	// Setup widgets
	ShipList->RefreshList();
	ShipList->SetVisibility(EVisibility::Visible);
}

void SFlareFleetMenu::Exit()
{
	SetEnabled(false);

	ShipList->Reset();
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareFleetMenu::OnBackClicked()
{
	MenuManager->Back();
}

void SFlareFleetMenu::OnSpacecraftSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer)
{
	IFlareSpacecraftInterface* Spacecraft = SpacecraftContainer->ShipInterfacePtr;

	if (Spacecraft)
	{
	}
}


#undef LOCTEXT_NAMESPACE

