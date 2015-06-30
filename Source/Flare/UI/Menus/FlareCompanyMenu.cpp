
#include "../../Flare.h"
#include "FlareStationMenu.h"
#include "../Widgets/FlarePartInfo.h"
#include "../../Game/FlareGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareCompanyMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareCompanyMenu::Construct(const FArguments& InArgs)
{
	// Data
	Company = NULL;
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	AFlarePlayerController* PC = MenuManager->GetPC();
	
	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SHorizontalBox)

		// UI container
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.AutoWidth()
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)

				// Object name
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SImage).Image(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Company))
					]

					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Title", "COMPANY"))
						.TextStyle(&Theme.TitleFont)
					]
				]

				// Company info
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(ActionMenu, SFlareTargetActions)
					.Player(PC)
					.MinimizedMode(true)
				]

				// Title
				+ SVerticalBox::Slot()
				.Padding(Theme.TitlePadding)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Colors", "COLORS"))
					.TextStyle(&Theme.SubTitleFont)
				]

				// Color picker
				+ SVerticalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoHeight()
				[
					SAssignNew(ColorBox, SFlareColorPanel).MenuManager(MenuManager)
				]

				// Object list
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(ShipList, SFlareShipList)
					.MenuManager(MenuManager)
					.Title(LOCTEXT("Property", "PROPERTY"))
				]
			]
		]

		// Dashboard button
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		[
			SNew(SFlareRoundButton)
			.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Exit))
			.OnClicked(this, &SFlareCompanyMenu::OnDashboardClicked)
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareCompanyMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);	
}

void SFlareCompanyMenu::Enter(UFlareCompany* Target)
{
	FLOG("SFlareCompanyMenu::Enter");
	SetEnabled(true);

	// Company data
	Company = Target;
	ActionMenu->SetCompany(Target);
	ActionMenu->Show();
	SetVisibility(EVisibility::Visible);

	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC && Target)
	{
		// Colors
		FFlarePlayerSave Data;
		PC->Save(Data);
		ColorBox->Setup(Data);

		// Menu
		const FFlareSpacecraftComponentDescription* PartDesc = PC->GetGame()->GetShipPartsCatalog()->Get("object-safe");
		PC->GetMenuPawn()->SetHorizontalOffset(100);
		PC->GetMenuPawn()->ShowPart(PartDesc);
		
		// Station list
		TArray<IFlareSpacecraftInterface*>& CompanyStations = Target->GetCompanyStations();
		for (int32 i = 0; i < CompanyStations.Num(); i++)
		{
			if (CompanyStations[i]->GetDamageSystem()->IsAlive())
			{
				ShipList->AddShip(CompanyStations[i]);
			}
		}

		// Ship list
		TArray<IFlareSpacecraftInterface*>& CompanyShips = Target->GetCompanyShips();
		for (int32 i = 0; i < CompanyShips.Num(); i++)
		{
			if (CompanyShips[i]->GetDamageSystem()->IsAlive())
			{
				ShipList->AddShip(CompanyShips[i]);
			}
		}
	}

	ShipList->RefreshList();
}

void SFlareCompanyMenu::Exit()
{
	SetEnabled(false);
	ShipList->Reset();
	Company = NULL;
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FString SFlareCompanyMenu::GetCompanyName() const
{
	if (Company)
	{
		return Company->GetCompanyName();
	}
	else
	{
		return FString("<undefined>");
	}
}

void SFlareCompanyMenu::OnDashboardClicked()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Dashboard);
}



#undef LOCTEXT_NAMESPACE
