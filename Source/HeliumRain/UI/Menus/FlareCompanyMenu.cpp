
#include "FlareCompanyMenu.h"
#include "../../Flare.h"
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
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SHorizontalBox)

		// Content block
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.AutoWidth()
		[
			SNew(SScrollBox)
			.Style(&Theme.ScrollBoxStyle)
			.ScrollBarStyle(&Theme.ScrollBarStyle)

			+ SScrollBox::Slot()
			[
				SNew(SBox)
				.WidthOverride(Theme.ContentWidth)
				.HAlign(HAlign_Fill)
				[
					SNew(SVerticalBox)

					// Trade routes title
					+ SVerticalBox::Slot()
					.Padding(Theme.TitlePadding)
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Trade routes", "Trade routes"))
						.TextStyle(&Theme.SubTitleFont)
					]

					// New trade route button
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.SmallContentPadding)
					.HAlign(HAlign_Left)
					[
						SNew(SFlareButton)
						.Width(8)
						.Text(LOCTEXT("NewTradeRouteButton", "Add new trade route"))
						.HelpText(LOCTEXT("NewTradeRouteInfo", "Create a new trade route and edit it"))
						.Icon(FFlareStyleSet::GetIcon("New"))
						.OnClicked(this, &SFlareCompanyMenu::OnNewTradeRouteClicked)
					]

					// Trade route list
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Left)
					[
						SNew(SScrollBox)
						.Style(&Theme.ScrollBoxStyle)
						.ScrollBarStyle(&Theme.ScrollBarStyle)
						+ SScrollBox::Slot()
						[
							SAssignNew(TradeRouteList, SVerticalBox)
						]
					]

					// Object list
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Left)
					[
						SAssignNew(ShipList, SFlareList)
						.MenuManager(MenuManager)
						.Title(LOCTEXT("Property", "Property"))
					]
				]
			]
		]

		// Color box
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		[
			SNew(SVerticalBox)

			// Company info
			+ SVerticalBox::Slot()
			.Padding(Theme.ContentPadding)
			.AutoHeight()
			[
				SNew(SBox)
				.WidthOverride(0.8 * Theme.ContentWidth)
				[
					SAssignNew(CompanyInfo, SFlareCompanyInfo)
					.Player(PC)
				]
			]

			// Color picker
			+ SVerticalBox::Slot()
			.Padding(Theme.ContentPadding)
			.AutoHeight()
			[
				SAssignNew(ColorBox, SFlareColorPanel)
				.MenuManager(MenuManager)
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
	SetVisibility(EVisibility::Collapsed);
}

void SFlareCompanyMenu::Enter(UFlareCompany* Target)
{
	FLOG("SFlareCompanyMenu::Enter");
	SetEnabled(true);

	// Company data
	Company = Target;
	SetVisibility(EVisibility::Visible);
	CompanyInfo->SetCompany(Company);

	UpdateTradeRouteList();

	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC && Target)
	{
		// Colors
		FFlarePlayerSave Data;
		FFlareCompanyDescription Unused;
		PC->Save(Data, Unused);
		ColorBox->Setup(Data);

		// Menu
		PC->GetMenuPawn()->SetCameraOffset(FVector2D(100, -30));
		if (PC->GetPlayerShip())
		{
			PC->GetMenuPawn()->ShowShip(PC->GetPlayerShip());
		}
		else
		{
			const FFlareSpacecraftComponentDescription* PartDesc = PC->GetGame()->GetShipPartsCatalog()->Get("object-safe");
			PC->GetMenuPawn()->ShowPart(PartDesc);
		}
		
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
	ShipList->SetVisibility(EVisibility::Visible);
}

void SFlareCompanyMenu::Exit()
{
	SetEnabled(false);
	ShipList->Reset();
	ShipList->SetVisibility(EVisibility::Collapsed);

	TradeRouteList->ClearChildren();

	Company = NULL;
	SetVisibility(EVisibility::Collapsed);
}

void SFlareCompanyMenu::UpdateTradeRouteList()
{
	if (Company)
	{
		TradeRouteList->ClearChildren();
		const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
		TArray<UFlareTradeRoute*>& TradeRoutes = Company->GetCompanyTradeRoutes();

		for (int RouteIndex = 0; RouteIndex < TradeRoutes.Num(); RouteIndex++)
		{
			UFlareTradeRoute* TradeRoute = TradeRoutes[RouteIndex];

			FText TradeRouteName = FText::Format(LOCTEXT("TradeRouteNameFormat", "{0}{1}"),
				TradeRoute->GetTradeRouteName(),
				(TradeRoute->IsPaused() ? LOCTEXT("FleetTradeRoutePausedFormat", " (Paused)") : FText()));

			// Add line
			TradeRouteList->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.Padding(Theme.SmallContentPadding)
			[
				SNew(SHorizontalBox)

				// Inspect
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SFlareButton)
					.Width(7)
					.Text(TradeRouteName)
					.HelpText(FText(LOCTEXT("InspectHelp", "Edit this trade route")))
					.OnClicked(this, &SFlareCompanyMenu::OnInspectTradeRouteClicked, TradeRoute)
				]

			// Remove
			+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SFlareButton)
					.Transparent(true)
					.Text(FText())
					.HelpText(LOCTEXT("RemoveTradeRouteHelp", "Remove this trade route"))
					.Icon(FFlareStyleSet::GetIcon("Stop"))
					.OnClicked(this, &SFlareCompanyMenu::OnDeleteTradeRoute, TradeRoute)
					.Width(1)
				]
			];
		}
	}
}

void SFlareCompanyMenu::OnNewTradeRouteClicked()
{
	UFlareTradeRoute* TradeRoute = MenuManager->GetPC()->GetCompany()->CreateTradeRoute(LOCTEXT("UntitledRoute", "Untitled Route"));
	FCHECK(TradeRoute);

	FFlareMenuParameterData Data;
	Data.Route = TradeRoute;
	MenuManager->OpenMenu(EFlareMenu::MENU_TradeRoute, Data);
}

void SFlareCompanyMenu::OnInspectTradeRouteClicked(UFlareTradeRoute* TradeRoute)
{
	FFlareMenuParameterData Data;
	Data.Route = TradeRoute;
	MenuManager->OpenMenu(EFlareMenu::MENU_TradeRoute, Data);
}

void SFlareCompanyMenu::OnDeleteTradeRoute(UFlareTradeRoute* TradeRoute)
{
	FCHECK(TradeRoute);
	TradeRoute->Dissolve();
	UpdateTradeRouteList();
}


#undef LOCTEXT_NAMESPACE
