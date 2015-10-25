
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
	Company = InArgs._Company;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	// Create the layout
	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Left)
	[
		SNew(SHorizontalBox)

		// Emblem
		+ SHorizontalBox::Slot()
		.AutoWidth()
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
		int32 CompanyShipCount = Company->GetCompanyShips().Num();
		int32 CompanyStationCount = Company->GetCompanyStations().Num();
		FString ShipString = FString::FromInt(CompanyShipCount) + " ";
		FString StationString = FString::FromInt(CompanyStationCount) + " ";
		ShipString += (CompanyShipCount == 1 ? LOCTEXT("Ship", "ship").ToString() : LOCTEXT("Ships", "ships").ToString());
		StationString += (CompanyStationCount == 1 ? LOCTEXT("Station", "station").ToString() : LOCTEXT("Stations", "stations").ToString());
		FString MoneyString = FString::FromInt(Company->GetMoney()) + " " + LOCTEXT("Credits", "credits").ToString();

		return FText::FromString(MoneyString + "\n" + StationString + "\n" + ShipString);
	}

	return Result;
}

const FSlateBrush* SFlareCompanyInfo::GetCompanyEmblem() const
{
	return (Company ? Company->GetEmblem() : NULL);
}


#undef LOCTEXT_NAMESPACE
