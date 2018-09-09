
#include "FlareTradeRouteInfo.h"
#include "../../Flare.h"
#include "../../Game/FlareGameTools.h"
#include "../../Game/FlareCompany.h"
#include "../../Game/FlareGameTools.h"
#include "../../Game/FlareTradeRoute.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Game/FlareGame.h"

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
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Fill)
	[
		SNew(SBox)
		.WidthOverride(0.6 * Theme.ContentWidth)
		.HAlign(HAlign_Fill)
		[
			SNew(SVerticalBox)

			// Trade routes title
			+ SVerticalBox::Slot()
			.Padding(Theme.TitlePadding)
			.AutoHeight()
			[
				UFlareUITypes::Header(LOCTEXT("Trade routes", "Trade routes"))
			]

			// New trade route button
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.ContentPadding)
			.HAlign(HAlign_Left)
			[
				SNew(SFlareButton)
				.Width(9)
				.Text(LOCTEXT("NewTradeRouteButton", "Add new trade route"))
				.HelpText(LOCTEXT("NewTradeRouteInfo", "Create a new trade route and edit it. You need an available fleet to create a new trade route."))
				.Icon(FFlareStyleSet::GetIcon("New"))
				.OnClicked(this, &SFlareTradeRouteInfo::OnNewTradeRouteClicked)
				.IsDisabled(this, &SFlareTradeRouteInfo::IsNewTradeRouteDisabled)
			]

			// Trade route list
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Left)
			.AutoHeight()
			[
				SAssignNew(TradeRouteList, SVerticalBox)
			]

			// Automated fleets title
			+ SVerticalBox::Slot()
			.Padding(Theme.TitlePadding)
			.AutoHeight()
			[
				UFlareUITypes::Header(LOCTEXT("AutomatedFleets", "Automated fleets"))
			]

			// Automated fleets list
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Left)
			.AutoHeight()
			[
				SAssignNew(AutomatedFleetList, SVerticalBox)
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareTradeRouteInfo::Update()
{
	Clear();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// List trade routes
	for (UFlareTradeRoute* TradeRoute : MenuManager->GetPC()->GetCompany()->GetCompanyTradeRoutes())
	{
		TradeRouteList->AddSlot()
		.AutoHeight()
		.HAlign(HAlign_Right)
		.Padding(Theme.ContentPadding)
		[
			SNew(SVerticalBox)

			// Buttons
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// Inspect
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SFlareButton)
					.Width(6)
					.Text(this, &SFlareTradeRouteInfo::GetTradeRouteName, TradeRoute)
					.HelpText(FText(LOCTEXT("InspectHelp", "Edit this trade route")))
					.OnClicked(this, &SFlareTradeRouteInfo::OnInspectTradeRouteClicked, TradeRoute)
				]

				// Pause
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SFlareButton)
					.Transparent(true)
					.Text(FText())
					.HelpText(LOCTEXT("PauseTradeRouteHelp", "Pause or restart this trade route"))
					.Icon(this, &SFlareTradeRouteInfo::GetTogglePauseTradeRouteIcon, TradeRoute)
					.OnClicked(this, &SFlareTradeRouteInfo::OnTogglePauseTradeRoute, TradeRoute)
					.Width(1)
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
			]

			// Infos
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.SmallContentPadding)
			[
				SNew(SRichTextBlock)
				.TextStyle(&Theme.TextFont)
				.Text(this, &SFlareTradeRouteInfo::GetDetailText, TradeRoute)
				.WrapTextAt(Theme.ContentWidth / 2)
				.DecoratorStyleSet(&FFlareStyleSet::Get())
			]
		];
	}

	// List auto-trading fleets
	int32 AutoTradingFleetCount = 0;
	for (UFlareFleet* Fleet : MenuManager->GetPC()->GetCompany()->GetCompanyFleets())
	{
		if (Fleet->IsAutoTrading())
		{
			AutoTradingFleetCount++;

			AutomatedFleetList->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.Padding(Theme.ContentPadding)
			[
				SNew(SVerticalBox)

				// Buttons
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					// Inspect
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Width(6)
						.Text(Fleet->GetFleetName())
						.HelpText(FText(LOCTEXT("InspectFleetHelp", "Inspect this fleet")))
						.OnClicked(this, &SFlareTradeRouteInfo::OnInspectFleetClicked, Fleet)
					]
				]

				// Infos
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.SmallContentPadding)
				[
					SNew(SRichTextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(this, &SFlareTradeRouteInfo::GetFleetDetailText, Fleet)
					.WrapTextAt(Theme.ContentWidth / 2)
					.DecoratorStyleSet(&FFlareStyleSet::Get())
				]
			];
		}
	}

	// Default text
	if (AutoTradingFleetCount == 0)
	{
		AutomatedFleetList->AddSlot()
		.AutoHeight()
		.HAlign(HAlign_Right)
		.Padding(Theme.ContentPadding)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TextFont)
			.Text(LOCTEXT("NoAutoFleet", "No fleet."))
		];
	}
}

void SFlareTradeRouteInfo::Clear()
{
	TradeRouteList->ClearChildren();
	AutomatedFleetList->ClearChildren();
}

FText SFlareTradeRouteInfo::GetDetailText(UFlareTradeRoute* TradeRoute) const
{
	FCHECK(TradeRoute);
	const FFlareTradeRouteSave* TradeRouteData = TradeRoute->Save();
	FCHECK(TradeRouteData);

	// Get data
	int32 TotalOperations = TradeRouteData->StatsOperationSuccessCount + TradeRouteData->StatsOperationFailCount;
	int32 SuccessPercentage = (TotalOperations > 0) ? (FMath::RoundToInt(100.0f * TradeRouteData->StatsOperationSuccessCount / float(TotalOperations))) : 0;
	int32 CreditsGain = (TradeRouteData->StatsDays > 0) ? (FMath::RoundToInt(0.01f * float(TradeRouteData->StatsMoneySell - TradeRouteData->StatsMoneyBuy) / float(TradeRouteData->StatsDays))) : 0;

	// Format result
	if (CreditsGain > 0)
	{
		return UFlareGameTools::AddLeadingSpace(FText::Format(LOCTEXT("TradeRouteDetailsGain", "<TradeText>{0} credits per day, {1}% OK</>"),
			FText::AsNumber(CreditsGain),
			FText::AsNumber(SuccessPercentage)),
			3);
	}
	else
	{
		return UFlareGameTools::AddLeadingSpace(FText::Format(LOCTEXT("TradeRouteDetailsLoss", "<WarningText>{0} credits per day, {1}% OK</>"),
			FText::AsNumber(CreditsGain),
			FText::AsNumber(SuccessPercentage)),
			3);
	}
}

bool SFlareTradeRouteInfo::IsNewTradeRouteDisabled() const
{
	int32 FleetCount = 0;
	TArray<UFlareFleet*>& Fleets = MenuManager->GetGame()->GetPC()->GetCompany()->GetCompanyFleets();

	for (int FleetIndex = 0; FleetIndex < Fleets.Num(); FleetIndex++)
	{
		if (!Fleets[FleetIndex]->GetCurrentTradeRoute() && Fleets[FleetIndex] != MenuManager->GetPC()->GetPlayerFleet())
		{
			FleetCount++;
		}
	}

	return (FleetCount == 0);
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
	MenuManager->Confirm(LOCTEXT("AreYouSure", "ARE YOU SURE ?"),
		LOCTEXT("ConfirmDeleteTR", "Do you really want to remove this trade route ?"),
		FSimpleDelegate::CreateSP(this, &SFlareTradeRouteInfo::OnDeleteTradeRouteConfirmed, TradeRoute));
}

void SFlareTradeRouteInfo::OnDeleteTradeRouteConfirmed(UFlareTradeRoute* TradeRoute)
{
	FCHECK(TradeRoute);
	TradeRoute->Dissolve();
	Update();
}

FText SFlareTradeRouteInfo::GetTradeRouteName(UFlareTradeRoute* TradeRoute) const
{
	return FText::Format(LOCTEXT("TradeRouteNameFormat", "{0}{1}"),
		TradeRoute->GetTradeRouteName(),
		(TradeRoute->IsPaused() ? UFlareGameTools::AddLeadingSpace(LOCTEXT("FleetTradeRoutePausedFormat", "(Paused)")) : FText()));
}

const FSlateBrush* SFlareTradeRouteInfo::GetTogglePauseTradeRouteIcon(UFlareTradeRoute* TradeRoute) const
{
	if (TradeRoute->IsPaused())
	{
		return FFlareStyleSet::GetIcon("Load");
	}
	else
	{
		return FFlareStyleSet::GetIcon("Pause");
	}
}

void SFlareTradeRouteInfo::OnTogglePauseTradeRoute(UFlareTradeRoute* TradeRoute)
{
	TradeRoute->SetPaused(!TradeRoute->IsPaused());
}

FText SFlareTradeRouteInfo::GetFleetDetailText(UFlareFleet* Fleet) const
{
	int32 CreditsGain = 0; // TODO #1047
	
	// Format result
	if (CreditsGain > 0)
	{
		return UFlareGameTools::AddLeadingSpace(FText::Format(LOCTEXT("FleetDetailsGain", "<TradeText>{0} credits per day</>"),
			FText::AsNumber(CreditsGain)),
			3);
	}
	else
	{
		return UFlareGameTools::AddLeadingSpace(FText::Format(LOCTEXT("FleetDetailsLoss", "<WarningText>{0} credits per day</>"),
			FText::AsNumber(CreditsGain)),
			3);
	}
}

void SFlareTradeRouteInfo::OnInspectFleetClicked(UFlareFleet* Fleet)
{
	FFlareMenuParameterData Data;
	Data.Fleet = Fleet;
	MenuManager->OpenMenu(EFlareMenu::MENU_Fleet, Data);
}


#undef LOCTEXT_NAMESPACE
