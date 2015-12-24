
#include "../../Flare.h"
#include "FlareCompanyMenu.h"
#include "../Components/FlarePartInfo.h"
#include "../Components/FlareCompanyInfo.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareCompany.h"
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
				SNew(SImage).Image(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Company))
			]

			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TitleFont)
				.Text(LOCTEXT("Title", "COMPANY"))
			]

			// Orbit
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("GoOrbit", "Orbital map"))
				.HelpText(LOCTEXT("GoOrbitInfo", "Exit the company menu and go back to the orbital map"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Orbit, true))
				.OnClicked(this, &SFlareCompanyMenu::OnOrbit)
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
		[
			SNew(SScrollBox)
			.Style(&Theme.ScrollBoxStyle)
			.ScrollBarStyle(&Theme.ScrollBarStyle)

			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				
				// Company name
				+ SVerticalBox::Slot()
				.Padding(Theme.TitlePadding)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(this, &SFlareCompanyMenu::GetCompanyName)
					.TextStyle(&Theme.SubTitleFont)
				]

				// Company info
				+ SVerticalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoHeight()
				[
					SAssignNew(CompanyInfo, SFlareCompanyInfo)
					.Player(PC)
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
					SAssignNew(ColorBox, SFlareColorPanel)
					.MenuManager(MenuManager)
				]

				// Object list
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Left)
				[
					SAssignNew(ShipList, SFlareShipList)
					.MenuManager(MenuManager)
					.Title(LOCTEXT("Property", "PROPERTY"))
				]

				// Trade routes Title
				+ SVerticalBox::Slot()
				.Padding(Theme.TitlePadding)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Trade routes", "TRADE ROUTES"))
					.TextStyle(&Theme.SubTitleFont)
                    .Visibility(this, &SFlareCompanyMenu::GetTradeRouteVisibility)
				]

                // New trade route button
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Left)
				[
					SNew(SFlareButton)
					.Width(8)
					.Text(LOCTEXT("NewTradeRouteButton", "New trade route"))
					.HelpText(LOCTEXT("NewTradeRouteInfo", "Create a new trade route"))
					.Icon(FFlareStyleSet::GetIcon("TradeRoute"))
					.OnClicked(this, &SFlareCompanyMenu::OnNewTradeRouteClicked)
                    .Visibility(this, &SFlareCompanyMenu::GetTradeRouteVisibility)
				]

				// Trade route list
				+ SVerticalBox::Slot()
				.AutoHeight()
                .HAlign(HAlign_Left)
				[
					SAssignNew(TradeRouteList, SVerticalBox)
				]
			]
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
	SetVisibility(EVisibility::Visible);
	CompanyInfo->SetCompany(Company);

	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC && Target)
	{
		// Colors
		FFlarePlayerSave Data;
		FFlareCompanyDescription Unused;
		PC->Save(Data, Unused);
		ColorBox->Setup(Data);

		// Menu
		const FFlareSpacecraftComponentDescription* PartDesc = PC->GetGame()->GetShipPartsCatalog()->Get("object-safe");
		PC->GetMenuPawn()->SetCameraOffset(FVector2D(100, -30));
		PC->GetMenuPawn()->ShowPart(PartDesc);
		
		// Station list
		TArray<UFlareSimulatedSpacecraft*>& CompanyStations = Target->GetCompanyStations();
		for (int32 i = 0; i < CompanyStations.Num(); i++)
		{
			if (CompanyStations[i]->GetDamageSystem()->IsAlive())
			{
				ShipList->AddShip(CompanyStations[i]);
			}
		}

		// Ship list
		TArray<UFlareSimulatedSpacecraft*>& CompanyShips = Target->GetCompanyShips();
		for (int32 i = 0; i < CompanyShips.Num(); i++)
		{
			if (CompanyShips[i]->GetDamageSystem()->IsAlive())
			{
				ShipList->AddShip(CompanyShips[i]);
			}
		}
	}

	ShipList->RefreshList();
	UpdateTradeRouteList();
}

void SFlareCompanyMenu::Exit()
{
	SetEnabled(false);
	ShipList->Reset();
	TradeRouteList->ClearChildren();

	Company = NULL;
	SetVisibility(EVisibility::Hidden);
}

void SFlareCompanyMenu::UpdateTradeRouteList()
{
	if (Company)
	{
		const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();


		TArray<UFlareTradeRoute*>& TradeRoutes = Company->GetCompanyTradeRoutes();
		for (int RouteIndex = 0; RouteIndex < TradeRoutes.Num(); RouteIndex++)
		{
			UFlareTradeRoute* TradeRoute = TradeRoutes[RouteIndex];
			// TODO Trade route info
			TradeRouteList->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			[
				SNew(SHorizontalBox)

				// Route info
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(TradeRoute->GetTradeRouteName())
					]
				]

				// Inspect
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SFlareButton)
					.OnClicked(this, &SFlareCompanyMenu::OnInspectTradeRouteClicked, TradeRoute)
					.Text(FText(LOCTEXT("Inspect", "Inspect")))
				]

			];
		}
	}
}

/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FText SFlareCompanyMenu::GetCompanyName() const
{
	FText Result;

	if (Company)
	{
		Result = FText::Format(LOCTEXT("Company", "COMPANY : {0}"), Company->GetCompanyName());
	}

	return Result;
}

void SFlareCompanyMenu::OnOrbit()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Orbit);
}

void SFlareCompanyMenu::OnNewTradeRouteClicked()
{
	UFlareTradeRoute* TradeRoute = Company->CreateTradeRoute(LOCTEXT("UntitledRoute", "Untitled Route"));

	MenuManager->OpenMenu(EFlareMenu::MENU_TradeRoute, TradeRoute);
}

void SFlareCompanyMenu::OnInspectTradeRouteClicked(UFlareTradeRoute* TradeRoute)
{
	MenuManager->OpenMenu(EFlareMenu::MENU_TradeRoute, TradeRoute);
}

EVisibility SFlareCompanyMenu::GetTradeRouteVisibility() const
{
    if (Company)
    {
        return MenuManager->GetPC()->GetCompany()->GetVisitedSectors().Num() > 2 ? EVisibility::Visible : EVisibility::Collapsed;
    }
    else
    {
        return EVisibility::Collapsed;
    }
}



#undef LOCTEXT_NAMESPACE
