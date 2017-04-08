
#include "../../Flare.h"
#include "FlareCompanyInfo.h"
#include "../../Game/FlareCompany.h"
#include "../../Game/AI/FlareAIBehavior.h"
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
		FNumberFormattingOptions RankFormat;
		RankFormat.MinimumIntegralDigits = 2;
		RankText = FText::Format(LOCTEXT("RankFormat", "{0}/"), FText::AsNumber(InArgs._Rank, &RankFormat));
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
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(0.5 * Theme.ContentWidth)
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

				// Combat value
				+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(FFlareStyleSet::GetIcon("CombatValue"))
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					[
						SNew(STextBlock)
						.Text(this, &SFlareCompanyInfo::GetCompanyCombatValue)
						.TextStyle(&Theme.TextFont)
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("CompanyCombatValue", "combat value"))
						.TextStyle(&Theme.TextFont)
					]
				]

				// Research value
				+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(FFlareStyleSet::GetIcon("ResearchValue"))
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					[
						SNew(STextBlock)
						.Text(this, &SFlareCompanyInfo::GetCompanyResearchValue)
						.TextStyle(&Theme.TextFont)
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("CompanyResearchValue", "research spent"))
						.TextStyle(&Theme.TextFont)
					]
				]

				// Full value
				+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(FFlareStyleSet::GetIcon("CostValue"))
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					[
						SNew(STextBlock)
						.Text(this, &SFlareCompanyInfo::GetCompanyValue)
						.TextStyle(&Theme.TextFont)
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("CompanyCreditsValue", "credits value"))
						.TextStyle(&Theme.TextFont)
					]
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
		]

		// Details
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.Padding(Theme.ContentPadding)
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
					SNew(SBox)
					.WidthOverride(8 * Theme.ButtonWidth)
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

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)

							// Confidence text
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(STextBlock)
								.Text(this, &SFlareCompanyInfo::GetConfidenceText)
								.TextStyle(&Theme.TextFont)
							]

							// Confidence value
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(STextBlock)
								.Text(this, &SFlareCompanyInfo::GetConfidenceTextValue)
								.ColorAndOpacity(this, &SFlareCompanyInfo::GetConfidenceColor)
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

				// Tribute
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(Theme.SmallContentPadding)
				[
					SNew(SFlareButton)
					.Text(this, &SFlareCompanyInfo::GetTributeText)
					.HelpText(this, &SFlareCompanyInfo::GetTributeHelpText)
					.OnClicked(this, &SFlareCompanyInfo::OnPayTribute)
					.IsDisabled(this, &SFlareCompanyInfo::IsTributeDisabled)
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

FText SFlareCompanyInfo::GetCompanyCombatValue() const
{
	FText Result;

	if (Company)
	{
		CompanyValue CompanyValue = Company->GetCompanyValue(NULL, false);

		if (CompanyValue.ArmyCurrentCombatPoints > 0 || CompanyValue.ArmyTotalCombatPoints > 0)
		{
			Result = FText::Format(LOCTEXT("CompanyCombatValueFormat", "{0}/{1}"),
				FText::AsNumber(CompanyValue.ArmyCurrentCombatPoints),
				FText::AsNumber(CompanyValue.ArmyTotalCombatPoints));
		}
		else
		{
			Result = LOCTEXT("CompanyCombatZero", "0");
		}
	}

	return Result;
}

FText SFlareCompanyInfo::GetCompanyResearchValue() const
{
	FText Result;

	if (Company)
	{
		Result = FText::AsNumber(Company->GetResearchSpent());
	}

	return Result;
}

FText SFlareCompanyInfo::GetCompanyValue() const
{
	FText Result;

	if (Company)
	{
		Result = FText::AsNumber(UFlareGameTools::DisplayMoney(Company->GetCompanyValue().TotalValue));
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
		return FText::Format(LOCTEXT("CompanyInfoFormat", "{0} credits in bank\n{1}, {2} "),
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

FText SFlareCompanyInfo::GetConfidenceText() const
{
	FText Result;

	if (Player && Player->GetCompany() != Company)
	{
		return LOCTEXT("ConfidenceInfo", "Confidence level : ");
	}

	return Result;
}

FText SFlareCompanyInfo::GetConfidenceTextValue() const
{
	FText Result;

	if (Player && Company && Player->GetCompany() != Company)
	{
		int32 Confidence = 50 + Company->GetConfidenceLevel(Player->GetCompany()) * 50;
		return FText::FromString(FString::FromInt(Confidence) + "%");
	}

	return Result;
}

FSlateColor SFlareCompanyInfo::GetConfidenceColor() const
{
	FLinearColor Result;

	if (Player && Company)
	{
		float Reputation = Company->GetReputation(Player->GetCompany());
		float Confidence = Company->GetConfidenceLevel(Player->GetCompany());
		const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

		if (Reputation <= -100 && Confidence > 0)
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

FText SFlareCompanyInfo::GetTributeText() const
{
	if (!Player || !Company)
	{
		return FText();
	}

	// They don't like us
	else if (Company->GetPlayerHostility() == EFlareHostility::Hostile)
	{
		return FText::Format(LOCTEXT("TributeWarFormat", "Pay tribute ({0} credits)"),
			FText::AsNumber(UFlareGameTools::DisplayMoney(Player->GetCompany()->GetTributeCost(Company))));
	}

	else
	{
		return LOCTEXT("TributePeace", "Pay tribute");
	}
}

FText SFlareCompanyInfo::GetTributeHelpText() const
{
	if (!Player || !Company)
	{
		return FText();
	}

	// They don't like us
	else if (Company->GetPlayerHostility() == EFlareHostility::Hostile)
	{
		return LOCTEXT("TributeWarInfo", "Pay tribute to make this company neutral towards you");
	}

	else
	{
		return LOCTEXT("TributePeaceInfo", "You are at peace with this faction !");
	}
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

bool SFlareCompanyInfo::IsTributeDisabled() const
{
	if (!Player || !Company)
	{
		return true;
	}

	UFlareCompany* PlayerCompany = Player->GetCompany();
	if (Company->GetPlayerHostility() == EFlareHostility::Hostile)
	{
		return (PlayerCompany->GetTributeCost(Company) > PlayerCompany->GetMoney());
	}
	else
	{
		return true;
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
		if (Company->GetAI()->GetBehavior()->ProposeTributeToPlayer && Company->GetTributeCost(Player->GetCompany()) < Company->GetMoney())
		{
			return FText::Format(LOCTEXT("AcceptTributeFormat", "Accept tribute ({0} credits)"),
				FText::AsNumber(UFlareGameTools::DisplayMoney(Company->GetTributeCost(Player->GetCompany()))));
		}
		else
		{
			return LOCTEXT("RequestPeace", "Request peace");
		}
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
		if (Company->GetAI()->GetBehavior()->ProposeTributeToPlayer && Company->GetTributeCost(Player->GetCompany()) < Company->GetMoney())
		{
			return LOCTEXT("AcceptTributeHelp", "Accept a tribute from this company and stop hostilities (you are currently at war with this company). If you declare war to this company again in the few next weeks you will gain a diplomatic malus.");
		}
		else
		{
			return LOCTEXT("RequestPeaceHelp", "Request peace with this company and stop hostilities (you are currently at war with this company). If you declare war to this company again in the few next weeks you will gain a diplomatic malus.");
		}
	}

	// We are at peace
	else
	{
		return LOCTEXT("DeclareWarHelp", "Declare war to this company (you are currently on good terms with this company)");
	}
}

void SFlareCompanyInfo::OnPayTribute()
{
	if (Player && Company)
	{
		Player->GetCompany()->PayTribute(Company);

		FText Text = LOCTEXT("TributePaid", "Tribute paid");
		FText InfoText = FText::Format(LOCTEXT("TributePaidFormat", "You have paid tribute to {0}."), Company->GetCompanyName());
		Player->Notify(Text, InfoText, NAME_None, EFlareNotification::NT_Military);
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

			if (Company->GetAI()->GetBehavior()->ProposeTributeToPlayer && Company->GetTributeCost(Player->GetCompany()) < Company->GetMoney())
			{
				Company->PayTribute(Player->GetCompany());
				FText Text = LOCTEXT("TributeReceived", "Tribute received");
				FText InfoText = FText::Format(LOCTEXT("TributeReceivedFormat", "Tribute received from {0}."), Company->GetCompanyName());
				Player->Notify(Text, InfoText, NAME_None, EFlareNotification::NT_Military);
			}
			else
			{
				FText Text = LOCTEXT("PeaceRequested", "Peace requested");
				FText InfoText = FText::Format(LOCTEXT("PeaceRequestedFormat", "You are seeking peace with {0}."), Company->GetCompanyName());
				Player->Notify(Text, InfoText, NAME_None, EFlareNotification::NT_Military);
			}
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
