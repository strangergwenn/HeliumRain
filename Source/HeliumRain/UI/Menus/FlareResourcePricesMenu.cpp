
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

		// Selector and info
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(ECONOMY_TABLE_WIDTH_FULL * Theme.ContentWidth)
			.Padding(FMargin(0))
			.HAlign(HAlign_Fill)
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
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Top)
			[
					SNew(SHorizontalBox)

					// Sector picker
					+ SHorizontalBox::Slot()
					.AutoWidth()
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

					// Include hubs
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Right)
					[
						SAssignNew(IncludeTradingHubsButton, SFlareButton)
						.Text(LOCTEXT("IncludeHubs", "Include Trading Hubs"))
						.Toggle(true)
						.Width(6)
					]
				]
			]
		]
	
		// Content
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Center)
		[
			SNew(SScrollBox)
			.Style(&Theme.ScrollBoxStyle)
			.ScrollBarStyle(&Theme.ScrollBarStyle)

			+ SScrollBox::Slot()
			.Padding(Theme.ContentPadding)
			[
				SNew(SBox)
				.WidthOverride(ECONOMY_TABLE_WIDTH_FULL * Theme.ContentWidth)
				.Padding(FMargin(0))
				[
					SNew(SVerticalBox)

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
							[
								SNew(SFlareButton)
								.Text(FText())
								.Width(1)
								.Transparent(true)
								.Icon(this, &SFlareResourcePricesMenu::GetSortIcon, EFlareEconomySort::ES_Resource)
								.OnClicked(this, &SFlareResourcePricesMenu::ToggleSortType, EFlareEconomySort::ES_Resource)
							]
						]

						// Production
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("ResourceProductionColumnTitleInfo", "Production"))
								.Width(ECONOMY_TABLE_BUTTON_SMALL)
								.Transparent(true)
								.Icon(this, &SFlareResourcePricesMenu::GetSortIcon, EFlareEconomySort::ES_Production)
								.OnClicked(this, &SFlareResourcePricesMenu::ToggleSortType, EFlareEconomySort::ES_Production)
							]
						]

						// Consumption
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("ResourceConsumptionColumnTitleInfo", "Usage"))
								.Width(ECONOMY_TABLE_BUTTON_SMALL)
								.Transparent(true)
								.Icon(this, &SFlareResourcePricesMenu::GetSortIcon, EFlareEconomySort::ES_Consumption)
								.OnClicked(this, &SFlareResourcePricesMenu::ToggleSortType, EFlareEconomySort::ES_Consumption)
							]
						]

						// Stock
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("ResourceStockColumnTitleInfo", "Stock"))
								.Width(ECONOMY_TABLE_BUTTON_SMALL)
								.Transparent(true)
								.Icon(this, &SFlareResourcePricesMenu::GetSortIcon, EFlareEconomySort::ES_Stock)
								.OnClicked(this, &SFlareResourcePricesMenu::ToggleSortType, EFlareEconomySort::ES_Stock)
							]
						]

						// Capacity
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("ResourceCapacityColumnTitleInfo", "Needs"))
								.Width(ECONOMY_TABLE_BUTTON_SMALL)
								.Transparent(true)
								.Icon(this, &SFlareResourcePricesMenu::GetSortIcon, EFlareEconomySort::ES_Needs)
								.OnClicked(this, &SFlareResourcePricesMenu::ToggleSortType, EFlareEconomySort::ES_Needs)
							]
						]

						// Prices
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("PriceCapacityColumnTitleInfo", "Price"))
								.Width(ECONOMY_TABLE_BUTTON_SMALL)
								.Transparent(true)
								.Icon(this, &SFlareResourcePricesMenu::GetSortIcon, EFlareEconomySort::ES_Price)
								.OnClicked(this, &SFlareResourcePricesMenu::ToggleSortType, EFlareEconomySort::ES_Price)
							]
						]

						// Variation
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("ResourcePriceAveragedVariation", "Variation"))
								.Width(ECONOMY_TABLE_BUTTON_SMALL)
								.Transparent(true)
								.Icon(this, &SFlareResourcePricesMenu::GetSortIcon, EFlareEconomySort::ES_Variation)
								.OnClicked(this, &SFlareResourcePricesMenu::ToggleSortType, EFlareEconomySort::ES_Variation)
							]
						]

						// Transport fee
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("TransportFee", "Transport"))
								.Width(ECONOMY_TABLE_BUTTON_SMALL)
								.Transparent(true)
								.Icon(this, &SFlareResourcePricesMenu::GetSortIcon, EFlareEconomySort::ES_Transport)
								.OnClicked(this, &SFlareResourcePricesMenu::ToggleSortType, EFlareEconomySort::ES_Transport)
							]
						]

						// Icon space
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ECONOMY_TABLE_WIDTH_MEDIUM * Theme.ContentWidth)
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

	// Defaults
	IsCurrentSortDescending = false;
	CurrentSortType = EFlareEconomySort::ES_Resource;
	IncludeTradingHubsButton->SetActive(false);

	// Fill sector list
	TargetSector = Sector;
	KnownSectors = MenuManager->GetPC()->GetCompany()->GetKnownSectors();
	SectorSelector->RefreshOptions();
	SectorSelector->SetSelectedItem(TargetSector);

	GenerateResourceList();
}

void SFlareResourcePricesMenu::GenerateResourceList()
{
	ResourcePriceList->ClearChildren();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Get resource list
	TArray<UFlareResourceCatalogEntry*> ResourceList = MenuManager->GetGame()->GetResourceCatalog()->GetResourceList();

	// Apply the current sort
	ResourceList.Sort([this](UFlareResourceCatalogEntry& R1, UFlareResourceCatalogEntry& R2)
	{
		bool Result = false;

		// Get sorting data
		TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> Stats = SectorHelper::ComputeSectorResourceStats(this->TargetSector);
		int64 ResourcePrice1 = this->TargetSector->GetResourcePrice(&R1.Data, EFlareResourcePriceContext::Default);
		int64 ResourcePrice2 = this->TargetSector->GetResourcePrice(&R2.Data, EFlareResourcePriceContext::Default);
		int64 LastResourcePrice1 = this->TargetSector->GetResourcePrice(&R1.Data, EFlareResourcePriceContext::Default, 30);
		int64 LastResourcePrice2 = this->TargetSector->GetResourcePrice(&R2.Data, EFlareResourcePriceContext::Default, 30);
		float Variation1 = (((float)ResourcePrice1) / ((float)LastResourcePrice1) - 1);
		float Variation2 = (((float)ResourcePrice2) / ((float)LastResourcePrice2) - 1);

		// Apply sort
		switch (this->CurrentSortType)
		{
		case EFlareEconomySort::ES_Resource:
			Result = R1.Data.DisplayIndex > R2.Data.DisplayIndex;
			break;
		case EFlareEconomySort::ES_Production:
			Result = (Stats[&R1.Data].Production > Stats[&R2.Data].Production);
			break;
		case EFlareEconomySort::ES_Consumption:
			Result = (Stats[&R1.Data].Consumption > Stats[&R2.Data].Consumption);
			break;
		case EFlareEconomySort::ES_Stock:
			Result = (Stats[&R1.Data].Stock > Stats[&R2.Data].Stock);
			break;
		case EFlareEconomySort::ES_Needs:
			Result = (Stats[&R1.Data].Capacity > Stats[&R2.Data].Capacity);
			break;
		case EFlareEconomySort::ES_Price:
			Result = ResourcePrice1 > ResourcePrice2;
			break;
		case EFlareEconomySort::ES_Variation:
			Result = Variation1 > Variation2;
			break;
		case EFlareEconomySort::ES_Transport:
			Result = R1.Data.TransportFee > R2.Data.TransportFee;
			break;
		}

		return this->IsCurrentSortDescending ? Result : !Result;
	});

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
					.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
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
					.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
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
					.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
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
					.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
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
					.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
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
					.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
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
					.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
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
					.WidthOverride(ECONOMY_TABLE_WIDTH_MEDIUM * Theme.ContentWidth)
					[
						SNew(SFlareButton)
						.Text(LOCTEXT("DetailButton", "Details"))
						.OnClicked(this, &SFlareResourcePricesMenu::OnShowWorldInfosClicked, &Resource)
						.Icon(FFlareStyleSet::GetIcon("Travel"))
						.Width(4)
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

const FSlateBrush* SFlareResourcePricesMenu::GetSortIcon(EFlareEconomySort::Type Type) const
{
	if (Type == CurrentSortType)
	{
		if (IsCurrentSortDescending)
		{
			return FFlareStyleSet::GetIcon("MoveDown");
		}
		else
		{
			return FFlareStyleSet::GetIcon("MoveUp");
		}
	}
	else
	{
		return FFlareStyleSet::GetIcon("MoveUpDown");
	}
}

void SFlareResourcePricesMenu::ToggleSortType(EFlareEconomySort::Type Type)
{
	if (Type == CurrentSortType)
	{
		IsCurrentSortDescending = !IsCurrentSortDescending;
	}
	else
	{
		IsCurrentSortDescending = true;
		CurrentSortType = Type;
	}

	GenerateResourceList();
}

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

