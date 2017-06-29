
#include "FlareTradeRouteInfo.h"
#include "../../Flare.h"
#include "../../Game/FlareCompany.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareTradeRouteInfo"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareTradeRouteInfo::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
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
				.OnClicked(this, &SFlareTradeRouteInfo::OnNewTradeRouteClicked)
			]

			// Trade route list
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Left)
			.AutoHeight()
			[
				SNew(SScrollBox)
				.Style(&Theme.ScrollBoxStyle)
				.ScrollBarStyle(&Theme.ScrollBarStyle)
				+ SScrollBox::Slot()
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

void SFlareTradeRouteInfo::UpdateTradeRouteList()
{
	Clear();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	TArray<UFlareTradeRoute*>& TradeRoutes = MenuManager->GetPC()->GetCompany()->GetCompanyTradeRoutes();

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
				.OnClicked(this, &SFlareTradeRouteInfo::OnInspectTradeRouteClicked, TradeRoute)
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
				.OnClicked(this, &SFlareTradeRouteInfo::OnDeleteTradeRoute, TradeRoute)
				.Width(1)
			]
		];
	}
}

void SFlareTradeRouteInfo::Clear()
{
	TradeRouteList->ClearChildren();
}

void SFlareTradeRouteInfo::OnNewTradeRouteClicked()
{
	UFlareTradeRoute* TradeRoute = MenuManager->GetPC()->GetCompany()->CreateTradeRoute(LOCTEXT("UntitledRoute", "Untitled Route"));
	FCHECK(TradeRoute);

	FFlareMenuParameterData Data;
	Data.Route = TradeRoute;
	MenuManager->OpenMenu(EFlareMenu::MENU_TradeRoute, Data);
}

void SFlareTradeRouteInfo::OnInspectTradeRouteClicked(UFlareTradeRoute* TradeRoute)
{
	FFlareMenuParameterData Data;
	Data.Route = TradeRoute;
	MenuManager->OpenMenu(EFlareMenu::MENU_TradeRoute, Data);
}

void SFlareTradeRouteInfo::OnDeleteTradeRoute(UFlareTradeRoute* TradeRoute)
{
	FCHECK(TradeRoute);
	TradeRoute->Dissolve();
	UpdateTradeRouteList();
}


#undef LOCTEXT_NAMESPACE
