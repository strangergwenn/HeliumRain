
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
	
	// Create the layout
	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Left)
	[
		SNew(SBox)
		.WidthOverride(Theme.ContentWidth)
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)

			// Emblem
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Left)
			[
				SNew(SImage)
				.Image(this, &SFlareCompanyInfo::GetCompanyEmblem)
			]

			// Data
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(Theme.ContentPadding)
			[
				SNew(SVerticalBox)
				
				// Name
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(this, &SFlareCompanyInfo::GetCompanyName)
					.TextStyle(&Theme.SubTitleFont)
				]

				// Data
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(this, &SFlareCompanyInfo::GetCompanyInfo)
					.TextStyle(&Theme.TextFont)
				]
			]

			// Hostility
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.Padding(Theme.ContentPadding)
			[
				SNew(SVerticalBox)
				
				// Status
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.Text(this, &SFlareCompanyInfo::GetCompanyHostility)
					.TextStyle(&Theme.TextFont)
				]

				// Toggle player hostility
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SFlareButton)
					.Text(this, &SFlareCompanyInfo::GetToggleHostilityText)
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
		return FText::Format(LOCTEXT("CompanyInfoFormat", "{0} {1}\n{2}\n{3}"), FText::AsNumber(Company->GetMoney()), LOCTEXT("Credits", "credits"), StationText, ShipText);
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
				Result = LOCTEXT("Neutral", "This company is neutral");
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
	else if (Player->GetCompany()->GetHostility(Company))
	{
		return LOCTEXT("RequestPeace", "Request peace");
	}

	// We are at peace
	else
	{
		return LOCTEXT("DeclareWar", "Declare war");
	}
}

void SFlareCompanyInfo::OnToggleHostility()
{
	if (Player && Company)
	{
		// Requesting peace
		if (Player->GetCompany()->GetHostility(Company))
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
