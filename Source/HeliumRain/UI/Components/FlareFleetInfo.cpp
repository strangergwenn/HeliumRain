
#include "FlareFleetInfo.h"
#include "../../Flare.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareGameTools.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"

#define LOCTEXT_NAMESPACE "FlareSpacecraftInfo"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareFleetInfo::Construct(const FArguments& InArgs)
{
	// Data
	PC = InArgs._Player;
	OwnerWidget = InArgs._OwnerWidget->AsShared();
	Minimized = InArgs._Minimized;
	AFlareGame* Game = InArgs._Player->GetGame();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	// Create the layout
	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Left)
	[
		SNew(SBox)
		.WidthOverride(Theme.ContentWidth)
		.Padding(Theme.SmallContentPadding)
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

					// Fleet name
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					.VAlign(VAlign_Center)
					[
						SAssignNew(FleetName, STextBlock)
						.Text(this, &SFlareFleetInfo::GetName)
						.TextStyle(&Theme.NameFont)
						.ColorAndOpacity(this, &SFlareFleetInfo::GetTextColor)
					]

					// Fleet composition
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(this, &SFlareFleetInfo::GetComposition)
						.TextStyle(&Theme.TextFont)
					]

					// Combat value icon
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Center)
					.Padding(FMargin(5, 0, 0, 0))
					[
						SNew(SImage)
						.Image(FFlareStyleSet::GetIcon("CombatValue"))
					]

					// Combat value
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(this, &SFlareFleetInfo::GetCombatValue)
						.TextStyle(&Theme.TextFont)
					]
				]

				// Company line
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.SmallContentPadding)
				[
					SNew(SHorizontalBox)

					// Company flag
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SAssignNew(CompanyFlag, SFlareCompanyFlag)
						.Player(InArgs._Player)
						.Visibility(this, &SFlareFleetInfo::GetCompanyFlagVisibility)
					]

					// Fleet info
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					[
						SNew(STextBlock)
						.Text(this, &SFlareFleetInfo::GetDescription)
						.TextStyle(&Theme.TextFont)
					]
				]

				// Buttons 
				+ SVerticalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					// Inspect
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SAssignNew(InspectButton, SFlareButton)
						.Text(LOCTEXT("Edit", "EDIT"))
						.HelpText(this, &SFlareFleetInfo::GetInspectHintText)
						.IsDisabled(this, &SFlareFleetInfo::IsInspectDisabled)
						.OnClicked(this, &SFlareFleetInfo::OnInspect)
						.Width(4)
					]

					// Inspect trade route
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SAssignNew(TradeRouteButton, SFlareButton)
						.Text(LOCTEXT("TradeRoute", "TRADE ROUTE"))
						.HelpText(this, &SFlareFleetInfo::GetInspectTradeRouteHintText)
						.IsDisabled(this, &SFlareFleetInfo::IsInspectTradeRouteDisabled)
						.OnClicked(this, &SFlareFleetInfo::OnOpenTradeRoute)
						.Width(6)
					]

					// Inspect trade route
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SAssignNew(AutoTradeButton, SFlareButton)
						.Text(LOCTEXT("AutoTrade", "AUTO-TRADE"))
						.HelpText(this, &SFlareFleetInfo::GetAutoTradeHintText)
						.IsDisabled(this, &SFlareFleetInfo::IsAutoTradeDisabled)
						.OnClicked(this, &SFlareFleetInfo::OnToggleAutoTrade)
						.Toggle(true)
						.Width(6)
					]
				]
			]
		]
	];

	// Setup
	if (InArgs._Fleet)
	{
		SetFleet(InArgs._Fleet);
	}
	Hide();
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareFleetInfo::SetFleet(UFlareFleet* Fleet)
{
	TargetFleet = Fleet;

	if (TargetFleet && PC)
	{
		CompanyFlag->SetCompany(TargetFleet->GetFleetCompany());
		TargetName = TargetFleet->GetFleetName();
	}

	// Text font
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	const FTextBlockStyle* TextFont = &Theme.NameFont;
	if (TargetFleet && TargetFleet == PC->GetPlayerShip()->GetCurrentFleet())
	{
		TextFont = &Theme.NameFontBold;
	}
	FleetName->SetTextStyle(TextFont);
}

void SFlareFleetInfo::SetMinimized(bool NewState)
{
	Minimized = NewState;

	if (GetVisibility() == EVisibility::Visible)
	{
		Show();
	}
}

void SFlareFleetInfo::Show()
{
	SetVisibility(EVisibility::Visible);

	if (Minimized)
	{
		InspectButton->SetVisibility(EVisibility::Collapsed);
		TradeRouteButton->SetVisibility(EVisibility::Collapsed);
		AutoTradeButton->SetVisibility(EVisibility::Collapsed);
	}
	else if (TargetFleet && TargetFleet->IsValidLowLevel())
	{
		InspectButton->SetVisibility(EVisibility::Visible);
		TradeRouteButton->SetVisibility(EVisibility::Visible);
		AutoTradeButton->SetVisibility(EVisibility::Visible);
		AutoTradeButton->SetActive(TargetFleet->IsAutoTrading());
	}
}

void SFlareFleetInfo::Hide()
{
	TargetFleet = NULL;
	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareFleetInfo::OnInspect()
{
	if (PC && TargetFleet)
	{
		FLOGV("SFlareFleetInfo::OnInspect : TargetFleet=%p", TargetFleet);
		FFlareMenuParameterData Data;
		Data.Fleet = TargetFleet;
		PC->GetMenuManager()->OpenMenu(EFlareMenu::MENU_Fleet, Data);
	}
}

void SFlareFleetInfo::OnOpenTradeRoute()
{
	if (PC && TargetFleet && TargetFleet->GetCurrentTradeRoute())
	{
		FLOGV("SFlareFleetInfo::OnOpenTradeRoute : TargetFleet=%p", TargetFleet);
		FFlareMenuParameterData Data;
		Data.Route = TargetFleet->GetCurrentTradeRoute();
		PC->GetMenuManager()->OpenMenu(EFlareMenu::MENU_TradeRoute, Data);
	}
}

void SFlareFleetInfo::OnToggleAutoTrade()
{
	if (PC && TargetFleet)
	{
		FLOGV("SFlareFleetInfo::OnToggleAutoTrade : TargetFleet=%p", TargetFleet);
		TargetFleet->SetAutoTrading(!TargetFleet->IsAutoTrading());
		AutoTradeButton->SetActive(TargetFleet->IsAutoTrading());
	}
}


/*----------------------------------------------------
	Content
----------------------------------------------------*/

FText SFlareFleetInfo::GetName() const
{
	return TargetName;
}

FSlateColor SFlareFleetInfo::GetTextColor() const
{
	FLinearColor Result;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	if (TargetFleet && TargetFleet->IsValidLowLevel())
	{
		if (TargetFleet->GetFleetCompany()->GetWarState(PC->GetCompany()) == EFlareHostility::Owned)
		{
			return TargetFleet->GetFleetColor();
		}
		else if (TargetFleet->GetFleetCompany()->GetWarState(PC->GetCompany()) == EFlareHostility::Hostile)
		{
			return Theme.EnemyColor;
		}
		else
		{
			return Theme.NeutralColor;
		}
	}

	return Result;
}

FText SFlareFleetInfo::GetInspectHintText() const
{
	if (TargetFleet->IsTraveling())
	{
		return LOCTEXT("CantEditTravelFleet", "Can't edit travelling fleets");
	}
	else if (TargetFleet->GetCurrentSector()->IsPlayerBattleInProgress())
	{
		return LOCTEXT("CantEditBattleFleet", "Can't edit fleets during battles");
	}
	else
	{
		return LOCTEXT("EditInfo", "Change the composition of this fleet");
	}
}

bool SFlareFleetInfo::IsInspectDisabled() const
{
	if (TargetFleet->IsTraveling())
	{
		return true;
	}
	else if (TargetFleet->GetCurrentSector()->IsPlayerBattleInProgress())
	{
		return true;
	}
	else
	{
		return false;
	}
}

FText SFlareFleetInfo::GetInspectTradeRouteHintText() const
{
	if (TargetFleet && TargetFleet->GetCurrentTradeRoute())
	{
		return LOCTEXT("TradeRouteInfo", "Edit the trade route this fleet is assigned to");
	}
	else
	{
		return LOCTEXT("CantEditNoTradeRoute", "Assign a trade route to this fleet in the company menu");
	}
}

bool SFlareFleetInfo::IsInspectTradeRouteDisabled() const
{
	if (TargetFleet && TargetFleet->GetCurrentTradeRoute())
	{
		return false;
	}
	else
	{
		return true;
	}
}

FText SFlareFleetInfo::GetAutoTradeHintText() const
{
	if (TargetFleet->GetCurrentTradeRoute())
	{
		return LOCTEXT("CantAutoTradeOnTradeRoute", "Fleets assigned to a trade route can't do automatic trading");
	}
	else if (TargetFleet == TargetFleet->GetGame()->GetPC()->GetPlayerFleet())
	{
		return LOCTEXT("CantAutoTradeWithPlayerFleet", "Your personal fleet can't trade automatically");
	}
	else
	{
		return LOCTEXT("AutoTradeInfo", "Command this fleet to start automatic trading");
	}
}

bool SFlareFleetInfo::IsAutoTradeDisabled() const
{
	if (TargetFleet->GetCurrentTradeRoute())
	{
		return true;
	}
	else if (TargetFleet == TargetFleet->GetGame()->GetPC()->GetPlayerFleet())
	{
		return true;
	}
	else
	{
		return false;
	}
}

FText SFlareFleetInfo::GetComposition() const
{
	FText Result;

	if (TargetFleet)
	{
		FText SingleShip = LOCTEXT("ShipSingle", "ship");
		FText MultipleShips = LOCTEXT("ShipPlural", "ships");
		int32 HeavyShipCount = TargetFleet->GetMilitaryShipCountBySize(EFlarePartSize::L);
		int32 LightShipCount = TargetFleet->GetMilitaryShipCountBySize(EFlarePartSize::S);
		int32 CivilianShipCount = TargetFleet->GetShipCount() - HeavyShipCount - LightShipCount;

		// Fighters
		FText LightShipText;
		if (LightShipCount > 0)
		{
			LightShipText = FText::Format(LOCTEXT("FleetCompositionLightFormat", "{0} light"),
				FText::AsNumber(LightShipCount));
		}

		// Heavies
		FText HeavyShipText;
		if (HeavyShipCount > 0)
		{
			HeavyShipText = FText::Format(LOCTEXT("FleetCompositionHeavyFormat", "{0} heavy"),
				FText::AsNumber(HeavyShipCount));

			if (LightShipCount > 0)
			{
				HeavyShipText = FText::FromString(", " + HeavyShipText.ToString());
			}
		}

		// Civilians
		FText CivilianShipText;
		if (CivilianShipCount > 0)
		{
			CivilianShipText = FText::Format(LOCTEXT("FleetCompositionCivilianFormat", "{0} civilian"),
				FText::AsNumber(CivilianShipCount));

			if (LightShipCount > 0 || HeavyShipCount > 0)
			{
				CivilianShipText = FText::FromString(", " + CivilianShipText.ToString());
			}
		}

		Result = FText::Format(LOCTEXT("FleetCompositionFormat", "({0}{1}{2})"), LightShipText, HeavyShipText, CivilianShipText);
	}

	return Result;
}

FText SFlareFleetInfo::GetCombatValue() const
{
	FText Result;

	if (TargetFleet)
	{
		if (TargetFleet->GetCombatPoints(true) > 0 || TargetFleet->GetCombatPoints(false) > 0)
		{
			Result = FText::Format(LOCTEXT("GetCombatValueFormat", "{0}/{1}"),
				FText::AsNumber(TargetFleet->GetCombatPoints(true)),
				FText::AsNumber(TargetFleet->GetCombatPoints(false)));
		}
		else
		{
			Result = LOCTEXT("GetCombatValueZero", "0");
		}
	}

	return Result;
}

FText SFlareFleetInfo::GetDescription() const
{
	FText Result;
	
	if (TargetFleet)
	{
		FText FleetAssignedText;
		if (TargetFleet->GetCurrentTradeRoute())
		{
			FleetAssignedText = UFlareGameTools::AddLeadingSpace(FText::Format(LOCTEXT("FleetAssignedFormat", "- {0}{1}"),
				TargetFleet->GetCurrentTradeRoute()->GetTradeRouteName(),
				(TargetFleet->GetCurrentTradeRoute()->IsPaused() ? UFlareGameTools::AddLeadingSpace(LOCTEXT("FleetTradeRoutePausedFormat", "(Paused)")) : FText())));
		}

		FText FleetDescriptionText = FText::Format(LOCTEXT("FleetFormat", "{0} ({1} / {2}){3}"),
			TargetFleet->GetFleetName(),
			FText::AsNumber(TargetFleet->GetShipCount()),
			FText::AsNumber(TargetFleet->GetMaxShipCount()),
			FleetAssignedText);

		Result = FText::Format(LOCTEXT("FleetInfoFormat", "{0} - {1}"), TargetFleet->GetStatusInfo(), FleetDescriptionText);
	}

	return Result;
}

EVisibility SFlareFleetInfo::GetCompanyFlagVisibility() const
{
	// Crash mitigation - If parent is hidden, so are we, don't try to use the target (#178)
	if ((OwnerWidget.IsValid() && OwnerWidget->GetVisibility() != EVisibility::Visible)
		|| (PC && !PC->GetMenuManager()->IsUIOpen()))
	{
		return EVisibility::Collapsed;
	}

	// Check the target
	if (TargetFleet && TargetFleet->IsValidLowLevel())
	{
		UFlareCompany* TargetCompany = TargetFleet->GetFleetCompany();
		if (TargetCompany && PC && TargetCompany == PC->GetCompany())
		{
			return EVisibility::Collapsed;
		}
	}

	return EVisibility::Visible;
}


#undef LOCTEXT_NAMESPACE
