
#include "FlareResourcePricesMenu.h"
#include "../../Flare.h"
#include "FlareWorldEconomyMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareSectorHelper.h"
#include "../../Economy/FlareResource.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Data/FlareResourceCatalog.h"


#define LOCTEXT_NAMESPACE "FlareResourcePricesMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareResourcePricesMenu::Construct(const FArguments& InArgs)
{
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SVerticalBox)
		
		// Content
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Center)
		[
			SNew(SScrollBox)
			.Style(&Theme.ScrollBoxStyle)
			.ScrollBarStyle(&Theme.ScrollBarStyle)

			+ SScrollBox::Slot()
			[
				SNew(SBox)
				.WidthOverride(2 * Theme.ContentWidth)
				.Padding(FMargin(0))
				[
					SNew(SVerticalBox)

					// Sector name
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.TitlePadding)
					[
						SNew(STextBlock)
						.Text(this, &SFlareResourcePricesMenu::GetSectorName)
						.TextStyle(&Theme.SubTitleFont)
					]

					// Sector picker
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.WidthOverride(Theme.ContentWidth / 2)
						.Padding(FMargin(0))
						.HAlign(HAlign_Left)
						[
							SAssignNew(SectorSelector, SFlareDropList<UFlareSimulatedSector*>)
							.OptionsSource(&KnownSectors)
							.OnGenerateWidget(this, &SFlareResourcePricesMenu::OnGenerateSectorComboLine)
							.OnSelectionChanged(this, &SFlareResourcePricesMenu::OnSectorComboLineSelectionChanged)
							.HeaderWidth(5)
							.ItemWidth(5)
							[
								SNew(SBox)
								.Padding(Theme.ListContentPadding)
								[
									SNew(STextBlock)
									.Text(this, &SFlareResourcePricesMenu::OnGetCurrentSectorComboLine)
									.TextStyle(&Theme.TextFont)
								]
							]
						]
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Icon space
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SBox)
							.WidthOverride(Theme.ResourceWidth)
							.Padding(FMargin(0))
						]

						// Production
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(0.2 * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.NameFont)
								.Text(LOCTEXT("ResourceProductionColumnTitleInfo", "Production"))
							]
						]

						// Consumption
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(0.2 * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.NameFont)
								.Text(LOCTEXT("ResourceConsumptionColumnTitleInfo", "Consumption"))
							]
						]

						// Stock
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(0.2 * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.NameFont)
								.Text(LOCTEXT("ResourceStockColumnTitleInfo", "Stock"))
							]
						]

						// Capacity
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(0.2 * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.NameFont)
								.Text(LOCTEXT("ResourceCapacityColumnTitleInfo", "Needs"))
							]
						]

						// Prices
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(0.2 * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.NameFont)
								.Text(LOCTEXT("PriceCapacityColumnTitleInfo", "Price"))
							]
						]

						// Variation
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(0.2 * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.NameFont)
								.Text(LOCTEXT("ResourcePriceAveragedVariation", "Variation"))
							]
						]

						// Transport fee
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(0.2 * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.NameFont)
								.Text(LOCTEXT("TransportFee", "Transport"))
							]
						]

						// Icon space
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(0.3 * Theme.ContentWidth)
							.Padding(FMargin(0))
						]
					]

					// List
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(ResourcePriceList, SVerticalBox)
					]
				]
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareResourcePricesMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareResourcePricesMenu::Enter(UFlareSimulatedSector* Sector)
{
	FLOG("SFlareResourcePricesMenu::Enter");
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	TargetSector = Sector;
	KnownSectors = MenuManager->GetPC()->GetCompany()->GetKnownSectors();
	SectorSelector->RefreshOptions();
	SectorSelector->SetSelectedItem(TargetSector);

	GenerateResourceList();
}

void SFlareResourcePricesMenu::GenerateResourceList()
{
	// Resource prices
	ResourcePriceList->ClearChildren();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	TArray<UFlareResourceCatalogEntry*> ResourceList = MenuManager->GetGame()->GetResourceCatalog()->GetResourceList();
	ResourceList.Sort(&SortByResourceType);

	// Resource prices
	for (int32 ResourceIndex = 0; ResourceIndex < ResourceList.Num(); ResourceIndex++)
	{
		FFlareResourceDescription& Resource = ResourceList[ResourceIndex]->Data;
		ResourcePriceList->AddSlot()
		.Padding(FMargin(1))
		[
			SNew(SBorder)
			.Padding(FMargin(1))
			.BorderImage((ResourceIndex % 2 == 0 ? &Theme.EvenBrush : &Theme.OddBrush))
			[
				SNew(SHorizontalBox)

				// Icon
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBorder)
					.Padding(FMargin(0))
					.BorderImage(&Resource.Icon)
					[
						SNew(SBox)
						.WidthOverride(Theme.ResourceWidth)
						.HeightOverride(Theme.ResourceHeight)
						.Padding(FMargin(0))
						[
							SNew(STextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(Resource.Acronym)
						]
					]
				]

				// Production
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(0.2 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareResourcePricesMenu::GetResourceProductionInfo, &Resource)
					]
				]

				// Consumption
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(0.2 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareResourcePricesMenu::GetResourceConsumptionInfo, &Resource)
					]
				]

				// Stock
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(0.2 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareResourcePricesMenu::GetResourceStockInfo, &Resource)
					]
				]

				// Capacity
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(0.2 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareResourcePricesMenu::GetResourceCapacityInfo, &Resource)
					]
				]

				// Price
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(0.2 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.ColorAndOpacity(this, &SFlareResourcePricesMenu::GetPriceColor, &Resource)
						.Text(this, &SFlareResourcePricesMenu::GetResourcePriceInfo, &Resource)
					]
				]

				// Price variation
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(0.2 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareResourcePricesMenu::GetResourcePriceVariationInfo, &Resource)
					]
				]

				// Transport fee
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(0.2 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareResourcePricesMenu::GetResourceTransportFeeInfo, &Resource)
					]
				]

				// Details
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(0.2 * Theme.ContentWidth)
					[
						SNew(SFlareButton)
						.Text(LOCTEXT("DetailButton", "Details"))
						.OnClicked(this, &SFlareResourcePricesMenu::OnShowWorldInfosClicked, &Resource)
						.Icon(FFlareStyleSet::GetIcon("Travel"))
					]
				]
			]
		];
	}
}

void SFlareResourcePricesMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
	ResourcePriceList->ClearChildren();
	TargetSector = NULL;
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FText SFlareResourcePricesMenu::GetSectorName() const
{
	if (TargetSector)
	{
		return FText::Format(LOCTEXT("SectorNameFormat", "Economy status for {0}"), TargetSector->GetSectorName());
	}

	return LOCTEXT("NoSectorSelected", "No sector selected");
}

FSlateColor SFlareResourcePricesMenu::GetPriceColor(FFlareResourceDescription* Resource) const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	if (TargetSector)
	{
		FLinearColor HighPriceColor = Theme.FriendlyColor;
		FLinearColor MeanPriceColor = Theme.NeutralColor;
		FLinearColor LowPriceColor = Theme.EnemyColor;

		float ResourcePrice = TargetSector->GetPreciseResourcePrice(Resource);

		float PriceRatio = (ResourcePrice - Resource->MinPrice) / (float) (Resource->MaxPrice - Resource->MinPrice);

		if(PriceRatio  > 0.5)
		{
			return FMath::Lerp(MeanPriceColor, HighPriceColor, 2.f * (PriceRatio - 0.5));
		}
		else
		{
			return FMath::Lerp(LowPriceColor, MeanPriceColor, 2.f * PriceRatio);
		}
	}
	return Theme.FriendlyColor;
}

void SFlareResourcePricesMenu::OnShowWorldInfosClicked(FFlareResourceDescription* Resource)
{
	FFlareMenuParameterData Data;
	Data.Sector = TargetSector;
	Data.Resource = Resource;
	MenuManager->OpenMenu(EFlareMenu::MENU_WorldEconomy, Data);
}

FText SFlareResourcePricesMenu::GetResourceProductionInfo(FFlareResourceDescription* Resource) const
{
	if (TargetSector)
	{
		FNumberFormattingOptions Format;
		Format.MaximumFractionalDigits = 1;

		TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> Stats = SectorHelper::ComputeSectorResourceStats(TargetSector);
		return FText::Format(LOCTEXT("ResourceMainProductionFormat", "{0}"),
			FText::AsNumber(Stats[Resource].Production, &Format));
	}

	return FText();
}

FText SFlareResourcePricesMenu::GetResourceConsumptionInfo(FFlareResourceDescription* Resource) const
{
	if (TargetSector)
	{
		FNumberFormattingOptions Format;
		Format.MaximumFractionalDigits = 1;

		TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> Stats = SectorHelper::ComputeSectorResourceStats(TargetSector);
		return FText::Format(LOCTEXT("ResourceMainConsumptionFormat", "{0}"),
			FText::AsNumber(Stats[Resource].Consumption, &Format));
	}

	return FText();
}

FText SFlareResourcePricesMenu::GetResourceStockInfo(FFlareResourceDescription* Resource) const
{
	if (TargetSector)
	{
		TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> Stats = SectorHelper::ComputeSectorResourceStats(TargetSector);
		return FText::Format(LOCTEXT("ResourceMainStockFormat", "{0}"),
			FText::AsNumber(Stats[Resource].Stock));
	}

	return FText();
}


FText SFlareResourcePricesMenu::GetResourceCapacityInfo(FFlareResourceDescription* Resource) const
{
	if (TargetSector)
	{

		TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> Stats = SectorHelper::ComputeSectorResourceStats(TargetSector);
		return FText::Format(LOCTEXT("ResourceMainCapacityFormat", "{0}"),
			FText::AsNumber(Stats[Resource].Capacity));
	}

	return FText();
}

FText SFlareResourcePricesMenu::GetResourcePriceInfo(FFlareResourceDescription* Resource) const
{
	if (TargetSector)
	{
		FNumberFormattingOptions MoneyFormat;
		MoneyFormat.MaximumFractionalDigits = 2;

		int64 ResourcePrice = TargetSector->GetResourcePrice(Resource, EFlareResourcePriceContext::Default);

		return FText::Format(LOCTEXT("ResourceMainPriceFormat", "{0} credits"),
			FText::AsNumber(ResourcePrice / 100.0f, &MoneyFormat));
	}

	return FText();
}

FText SFlareResourcePricesMenu::GetResourcePriceVariationInfo(FFlareResourceDescription* Resource) const
{
	if (TargetSector)
	{
		FNumberFormattingOptions MoneyFormat;
		MoneyFormat.MaximumFractionalDigits = 2;

		int32 MeanDuration = 30;
		int64 ResourcePrice = TargetSector->GetResourcePrice(Resource, EFlareResourcePriceContext::Default);
		int64 LastResourcePrice = TargetSector->GetResourcePrice(Resource, EFlareResourcePriceContext::Default, MeanDuration);

		if(ResourcePrice != LastResourcePrice)
		{
			float Variation = (((float) ResourcePrice) / ((float) LastResourcePrice) - 1);

			if(FMath::Abs(Variation) >= 0.0001)
			{
				return FText::Format(LOCTEXT("ResourceVariationFormat", "{0}{1}%"),
								(Variation > 0 ?
									 LOCTEXT("ResourceVariationFormatSignPlus","+") :
									 LOCTEXT("ResourceVariationFormatSignMinus","-")),
							  FText::AsNumber(FMath::Abs(Variation) * 100.0f, &MoneyFormat));
			}
		}

		return LOCTEXT("ResourceMainPriceNoVariationFormat", "-");
	}

	return FText();
}

FText SFlareResourcePricesMenu::GetResourceTransportFeeInfo(FFlareResourceDescription* Resource) const
{
	if (TargetSector)
	{
		FNumberFormattingOptions MoneyFormat;
		MoneyFormat.MaximumFractionalDigits = 2;

		return FText::Format(LOCTEXT("ResourceMainPriceFormat", "{0} credits"),
			FText::AsNumber(Resource->TransportFee / 100.0f, &MoneyFormat));
	}

	return FText();
}

TSharedRef<SWidget> SFlareResourcePricesMenu::OnGenerateSectorComboLine(UFlareSimulatedSector* Sector)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(SBox)
	.Padding(Theme.ListContentPadding)
	[
		SNew(STextBlock)
		.Text(Sector->GetSectorName())
		.TextStyle(&Theme.TextFont)
	];
}

void SFlareResourcePricesMenu::OnSectorComboLineSelectionChanged(UFlareSimulatedSector* Sector, ESelectInfo::Type SelectInfo)
{
	TargetSector = Sector;
	GenerateResourceList();
}

FText SFlareResourcePricesMenu::OnGetCurrentSectorComboLine() const
{
	UFlareSimulatedSector* Sector = SectorSelector->GetSelectedItem();
	if (Sector)
	{
		return Sector->GetSectorName();
	}
	else if (TargetSector)
	{
		return TargetSector->GetSectorName();
	}
	else
	{
		return LOCTEXT("SelectSector", "Select a sector");
	}
}

#undef LOCTEXT_NAMESPACE

