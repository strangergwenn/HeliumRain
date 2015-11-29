
#include "../../Flare.h"
#include "FlareCompanyInfo.h"
#include "../../Game/FlareCompany.h"


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
				SNew(STextBlock)
				.Text(this, &SFlareCompanyInfo::GetCompanyHostility)
				.TextStyle(&Theme.TextFont)
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
		Result = Desc->Name;
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
				Result = LOCTEXT("Allied", "Allied");
				break;

			case EFlareHostility::Hostile:
				Result = LOCTEXT("Hostile", "Hostile");
				break;

			case EFlareHostility::Neutral:
				Result = LOCTEXT("Neutral", "Neutral");
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


#undef LOCTEXT_NAMESPACE
