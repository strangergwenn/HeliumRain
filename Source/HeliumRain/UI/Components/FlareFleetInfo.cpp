
#include "../../Flare.h"
#include "FlareFleetInfo.h"
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
						SNew(STextBlock)
						.Text(this, &SFlareFleetInfo::GetName)
						.TextStyle(&Theme.NameFont)
						.ColorAndOpacity(this, &SFlareFleetInfo::GetTextColor)
					]

					// Fleet description
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(this, &SFlareFleetInfo::GetDescription)
						.TextStyle(&Theme.TextFont)
					]
				]

				// Company line
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.SmallContentPadding)
				[
					SAssignNew(CompanyFlag, SFlareCompanyFlag)
					.Player(InArgs._Player)
					.Visibility(this, &SFlareFleetInfo::GetCompanyFlagVisibility)
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
						.Text(LOCTEXT("Inspect", "DETAILS"))
						.HelpText(LOCTEXT("InspectInfo", "Take a closer look at this fleet"))
						.OnClicked(this, &SFlareFleetInfo::OnInspect)
						.Width(2.8)
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
	}
	else if (TargetFleet && TargetFleet->IsValidLowLevel())
	{
		InspectButton->SetVisibility(EVisibility::Visible);
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
		if (TargetFleet == PC->GetPlayerFleet())
		{
			return Theme.FriendlyColor;
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

FText SFlareFleetInfo::GetDescription() const
{
	// Common text
	FText DefaultText = LOCTEXT("Default", "UNKNOWN OBJECT");

	// TODO

	return DefaultText;
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
