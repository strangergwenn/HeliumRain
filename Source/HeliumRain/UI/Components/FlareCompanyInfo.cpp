
#include "../../Flare.h"
#include "FlareCompanyInfo.h"
#include "../../Game/FlareCompany.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareCompanyInfo"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareCompanyInfo::Construct(const FArguments& InArgs)
{
	// Data
	Player = InArgs._Player;
	Company = InArgs._Company;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	// Rank
	FText RankText;
	if (InArgs._Rank >= 0)
	{
		RankText = FText::Format(LOCTEXT("RankFormat", "{0}/"), FText::AsNumber(InArgs._Rank));
	}

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)

		// Rank
		+ SHorizontalBox::Slot()
		.Padding(Theme.ContentPadding)
		.AutoWidth()
		.HAlign(HAlign_Fill)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TitleFont)
			.Text(RankText)
		]

		// Emblem
		+ SHorizontalBox::Slot()
		.Padding(Theme.ContentPadding)
		.AutoWidth()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		[
			SNew(SImage)
			.Image(this, &SFlareCompanyInfo::GetCompanyEmblem)
		]

		// Data
		+ SHorizontalBox::Slot()
		.Padding(Theme.ContentPadding)
		.HAlign(HAlign_Fill)
		[
			SNew(SVerticalBox)
				
			// Name
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.SmallContentPadding)
			[
				SNew(STextBlock)
				.Text(this, &SFlareCompanyInfo::GetCompanyName)
				.TextStyle(&Theme.SubTitleFont)
				.ColorAndOpacity(this, &SFlareCompanyInfo::GetWarColor)
			]

			// Data
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.SmallContentPadding)
			[
				SNew(STextBlock)
				.Text(this, &SFlareCompanyInfo::GetCompanyInfo)
				.TextStyle(&Theme.TextFont)
			]
		]

		// Details
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.Padding(Theme.ContentPadding)
		.AutoWidth()
		[
			SNew(SVerticalBox)

			// Description
			+ SVerticalBox::Slot()
			.Padding(Theme.SmallContentPadding)
			.HAlign(HAlign_Left)
			.AutoHeight()
			[
				SNew(SBox)
				.WidthOverride(0.7 * Theme.ContentWidth)
				[
					SNew(STextBlock)
					.Text(this, &SFlareCompanyInfo::GetCompanyDescription)
					.WrapTextAt(0.7 * Theme.ContentWidth)
					.TextStyle(&Theme.NameFont)
				]
			]
				
			// Reputation
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.SmallContentPadding)
			[
				SNew(SHorizontalBox)

				// Reputation & Hostility
				+ SHorizontalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Reputation text
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(this, &SFlareCompanyInfo::GetReputationText)
							.TextStyle(&Theme.TextFont)
						]

						// Reputation value
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(this, &SFlareCompanyInfo::GetReputationTextValue)
							.ColorAndOpacity(this, &SFlareCompanyInfo::GetReputationColor)
							.TextStyle(&Theme.TextFont)
						]
					]

					// Hostility
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(this, &SFlareCompanyInfo::GetCompanyHostility)
						.TextStyle(&Theme.TextFont)
					]
				]

				// Toggle player hostility
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(Theme.SmallContentPadding)
				[
					SNew(SFlareButton)
					.Text(this, &SFlareCompanyInfo::GetToggleHostilityText)
					.HelpText(this, &SFlareCompanyInfo::GetToggleHostilityHelpText)
					.OnClicked(this, &SFlareCompanyInfo::OnToggleHostility)
					.Visibility(this, &SFlareCompanyInfo::GetToggleHostilityVisibility)
					.Width(4)
				]
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareCompanyInfo::SetCompany(UFlareCompany* NewCompany)
{
	Company = NewCompany;
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FText SFlareCompanyInfo::GetCompanyName() const
{
	FText Result;

	if (Company)
	{
		const FFlareCompanyDescription* Desc = Company->GetDescription();
		if (Desc)
		{
			Result = Desc->Name;
		}
	}

	return Result;
}

FText SFlareCompanyInfo::GetCompanyInfo() const
{
	FText Result;

	if (Company)
	{
		// Stations
		int32 CompanyStationCount = Company->GetCompanyStations().Num();
		FText StationText = FText::Format(LOCTEXT("StationInfoFormat", "{0} {1}"),
			FText::AsNumber(CompanyStationCount), CompanyStationCount == 1 ? LOCTEXT("Station", "station") : LOCTEXT("Stations", "stations"));

		// Ships
		int32 CompanyShipCount = Company->GetCompanyShips().Num();
		FText ShipText = FText::Format(LOCTEXT("ShipInfoFormat", "{0} {1}"),
			FText::AsNumber(CompanyShipCount), CompanyShipCount == 1 ? LOCTEXT("Ship", "ship") : LOCTEXT("Ships", "ships"));
		
		// Full string
		return FText::Format(LOCTEXT("CompanyInfoFormat", "Valued at {0} credits\n{1} credits in bank\n{2} owned\n{3} owned"),
			FText::AsNumber(UFlareGameTools::DisplayMoney(Company->GetCompanyValue().TotalValue)),
			FText::AsNumber(UFlareGameTools::DisplayMoney(Company->GetMoney())),
			StationText,
			ShipText);
	}

	return Result;
}

FText SFlareCompanyInfo::GetCompanyDescription() const
{
	FText Result;

	if (Company)
	{
		const FFlareCompanyDescription* Desc = Company->GetDescription();
		if (Desc)
		{
			Result = Desc->Description;
		}
	}

	return Result;
}

FText SFlareCompanyInfo::GetReputationText() const
{
	FText Result;

	if (Player && Player->GetCompany() != Company)
	{
		return LOCTEXT("ReputationInfo", "Reputation level : ");
	}

	return Result;
}

FText SFlareCompanyInfo::GetReputationTextValue() const
{
	FText Result;

	if (Player && Company && Player->GetCompany() != Company)
	{
		int32 Reputation = Company->GetReputation(Player->GetCompany());
		return FText::AsNumber(Reputation);
	}

	return Result;
}

FSlateColor SFlareCompanyInfo::GetReputationColor() const
{
	FLinearColor Result;

	if (Player && Company)
	{
		float Reputation = Company->GetReputation(Player->GetCompany());
		const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

		if (Reputation <= -100)
		{
			return Theme.EnemyColor;
		}
		else if (Reputation >= 100)
		{
			return Theme.FriendlyColor;
		}
		else
		{
			return Theme.NeutralColor;
		}
	}

	return Result;
}

FSlateColor SFlareCompanyInfo::GetWarColor() const
{
	FLinearColor Result;

	if (Player && Company)
	{
		const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

		if (Company->GetPlayerWarState() == EFlareHostility::Hostile)
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


FText SFlareCompanyInfo::GetCompanyHostility() const
{
	FText Result;

	if (Company)
	{
		EFlareHostility::Type Hostiliy = Company->GetPlayerHostility();

		switch (Hostiliy)
		{
			case EFlareHostility::Friendly:
				Result = LOCTEXT("Allied", "This company is an ally");
				break;

			case EFlareHostility::Hostile:
				Result = LOCTEXT("Hostile", "This company is hostile");
				break;

			case EFlareHostility::Neutral:
				if (Company->GetPlayerWarState() == EFlareHostility::Hostile)
				{
					Result = LOCTEXT("NeutralSeekPeace", "This company is seeking peace");
				}
				else
				{
					Result = LOCTEXT("NeutralNotSeekPeace", "This company is neutral");
				}
				break;

			case EFlareHostility::Owned:
			default:
				break;
		}
	}

	return Result;
}

const FSlateBrush* SFlareCompanyInfo::GetCompanyEmblem() const
{
	return (Company ? Company->GetEmblem() : NULL);
}

EVisibility SFlareCompanyInfo::GetToggleHostilityVisibility() const
{
	if (Player && Player->GetCompany() == Company)
	{
		return EVisibility::Collapsed;
	}
	else
	{
		return EVisibility::Visible;
	}
}

FText SFlareCompanyInfo::GetToggleHostilityText() const
{
	if (!Player || !Company)
	{
		return FText();
	}

	// We are at war
	else if (Player->GetCompany()->GetHostility(Company) == EFlareHostility::Hostile)
	{
		return LOCTEXT("RequestPeace", "Request peace");
	}

	// We are at peace
	else
	{
		return LOCTEXT("DeclareWar", "Declare war");
	}
}

FText SFlareCompanyInfo::GetToggleHostilityHelpText() const
{
	if (!Player || !Company)
	{
		return FText();
	}

	// We are at war
	else if (Player->GetCompany()->GetHostility(Company) == EFlareHostility::Hostile)
	{
		return LOCTEXT("RequestPeaceHelp", "Request peace with this company and stop hostilities (you are currently at war with this company)");
	}

	// We are at peace
	else
	{
		return LOCTEXT("DeclareWarHelp", "Declare war to this company (you are currently on good terms with this company)");
	}
}

void SFlareCompanyInfo::OnToggleHostility()
{
	if (Player && Company)
	{
		// Requesting peace
		if (Player->GetCompany()->GetHostility(Company) == EFlareHostility::Hostile)
		{
			Player->GetCompany()->SetHostilityTo(Company, false);

			FText Text = LOCTEXT("PeaceRequested", "Peace requested");
			FText InfoText = FText::Format(LOCTEXT("PeaceRequestedFormat", "You are seeking peace with {0}."), Company->GetCompanyName());
			Player->Notify(Text, InfoText, NAME_None, EFlareNotification::NT_Military);
		}

		// Declaring war
		else
		{
			Player->GetCompany()->SetHostilityTo(Company, true);

			FText Text = LOCTEXT("WarDeclared", "War has been declared");
			FText InfoText = FText::Format(LOCTEXT("WarDeclaredFormat", "You have declared war to {0} !"), Company->GetCompanyName());
			Player->Notify(Text, InfoText, NAME_None, EFlareNotification::NT_Military);
		}
	}
}

#undef LOCTEXT_NAMESPACE
