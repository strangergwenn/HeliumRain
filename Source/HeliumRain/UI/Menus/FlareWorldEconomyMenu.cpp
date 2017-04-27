
#include "FlareWorldEconomyMenu.h"
#include "../../Flare.h"
#include "../../Game/FlareGame.h"
#include "../../Economy/FlareResource.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareWorldEconomyMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareWorldEconomyMenu::Construct(const FArguments& InArgs)
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

		// Resource selector and info
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(2 * Theme.ContentWidth)
			.Padding(FMargin(0))
			.HAlign(HAlign_Fill)
			[
				SNew(SHorizontalBox)

				// Main resource info
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Top)
				.HAlign(HAlign_Left)
				.Padding(Theme.ContentPadding)
				[
					SNew(SVerticalBox)

					// Resource name
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.TitlePadding)
					[
						SNew(STextBlock)
						.Text(this, &SFlareWorldEconomyMenu::GetResourceName)
						.TextStyle(&Theme.SubTitleFont)
					]
		
					// Resource picker
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Icon
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Top)
						.HAlign(HAlign_Right)
						.Padding(Theme.ContentPadding)
						.AutoWidth()
						[
							SNew(SImage)
							.Image(this, &SFlareWorldEconomyMenu::GetResourceIcon)
						]

						// Info
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Top)
						.HAlign(HAlign_Left)
						.Padding(Theme.ContentPadding)
						[
							SNew(SVerticalBox)

							// Resource name
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(SBox)
								.WidthOverride(Theme.ContentWidth / 2)
								.Padding(FMargin(0))
								.HAlign(HAlign_Left)
								[
									SAssignNew(ResourceSelector, SComboBox<UFlareResourceCatalogEntry*>)
									.OptionsSource(&MenuManager->GetPC()->GetGame()->GetResourceCatalog()->Resources)
									.OnGenerateWidget(this, &SFlareWorldEconomyMenu::OnGenerateResourceComboLine)
									.OnSelectionChanged(this, &SFlareWorldEconomyMenu::OnResourceComboLineSelectionChanged)
									.ComboBoxStyle(&Theme.ComboBoxStyle)
									.ForegroundColor(FLinearColor::White)
									[
										SNew(STextBlock)
										.Text(this, &SFlareWorldEconomyMenu::OnGetCurrentResourceComboLine)
										.TextStyle(&Theme.TextFont)
									]
								]
							]

							// Resource description
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.SmallContentPadding)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.TextFont)
								.Text(this, &SFlareWorldEconomyMenu::GetResourceDescription)
								.WrapTextAt(Theme.ContentWidth)
							]
						]
					]
				]

				// Resource List
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Right)
				.AutoWidth()
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(Theme.ContentWidth / 2)
					.Padding(FMargin(0))
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareWorldEconomyMenu::GetResourceInfo)
						.WrapTextAt(Theme.ContentWidth / 2)
					]
				]
			]
		]

		// Content
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Center)
		.Padding(Theme.ContentPadding)
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

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Sector name
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(0.3 * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.NameFont)
								.Text(LOCTEXT("SectorNameColumnTitleInfo", "Sector"))
							]
						]

						// Production
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(0.16 * Theme.ContentWidth)
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
							.WidthOverride(0.16 * Theme.ContentWidth)
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
							.WidthOverride(0.16 * Theme.ContentWidth)
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
							.WidthOverride(0.16 * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.NameFont)
								.Text(LOCTEXT("ResourceCapacityColumnTitleInfo", "Needs"))
							]
						]

						// Price
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(0.16 * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.NameFont)
								.Text(LOCTEXT("ResourcePriceColumnTitleInfo", "Price"))
							]
						]

						// Price variation 1 day
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
								.Text(LOCTEXT("ResourcePriceVariation1dColumnTitleInfo", "1-day variation"))
								.WrapTextAt(0.3 * Theme.ContentWidth)
							]
						]

						// Price variation 40 days
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(0.25 * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.NameFont)
								.Text(LOCTEXT("ResourcePriceVariation40dColumnTitleInfo", "40-days variation"))
								.WrapTextAt(0.5 * Theme.ContentWidth)
							]
						]
					]

					// List
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(SectorList, SVerticalBox)
					]
				]
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareWorldEconomyMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareWorldEconomyMenu::Enter(FFlareResourceDescription* Resource, UFlareSimulatedSector* Sector)
{
	FLOG("SFlareWorldEconomyMenu::Enter");
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	if (Resource)
	{
		TargetResource = Resource;
	}
	WorldStats = WorldHelper::ComputeWorldResourceStats(MenuManager->GetGame());

	// Update resource selector
	ResourceSelector->RefreshOptions();
	if (TargetResource)
	{
		ResourceSelector->SetSelectedItem(MenuManager->GetPC()->GetGame()->GetResourceCatalog()->GetEntry(TargetResource));
	}

	GenerateSectorList();
}

void SFlareWorldEconomyMenu::GenerateSectorList()
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	SectorList->ClearChildren();

	if(TargetResource == NULL)
	{
		return;
	}

	// Natural order is sorted by discovery
	TArray<UFlareSimulatedSector*>& Sectors = MenuManager->GetPC()->GetCompany()->GetVisitedSectors();

	// Sector list
	for (int32 SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Sectors[SectorIndex];

		SectorList->AddSlot()
		.Padding(FMargin(0))
		[
			SNew(SBorder)
			.Padding(FMargin(1))
			.BorderImage((SectorIndex % 2 == 0 ? &Theme.EvenBrush : &Theme.OddBrush))
			[
				SNew(SHorizontalBox)

				// Sector
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(0.30 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(SFlareButton)
						.Width(6)
						.Text(this, &SFlareWorldEconomyMenu::GetSectorText, Sector)
						.Color(this, &SFlareWorldEconomyMenu::GetSectorTextColor, Sector)
						.Icon(FFlareStyleSet::GetIcon("Travel"))
						.OnClicked(this, &SFlareWorldEconomyMenu::OnOpenSector, Sector)
					]
				]

				// Production
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(0.16 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareWorldEconomyMenu::GetResourceProductionInfo, Sector)
					]
				]

				// Consumption
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(0.16 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareWorldEconomyMenu::GetResourceConsumptionInfo, Sector)
					]
				]

				// Stock
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(0.16 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareWorldEconomyMenu::GetResourceStockInfo, Sector)
					]
				]

				// Capacity
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(0.16 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareWorldEconomyMenu::GetResourceCapacityInfo, Sector)
					]
				]

				// Price
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(0.16 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.ColorAndOpacity(this, &SFlareWorldEconomyMenu::GetPriceColor, Sector)
						.Text(this, &SFlareWorldEconomyMenu::GetResourcePriceInfo, Sector)
					]
				]

				// Price variation 1 day
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
						.Text(this, &SFlareWorldEconomyMenu::GetResourcePriceVariationInfo, Sector, TSharedPtr<int32>(new int32(1)))
					]
				]

				// Price variation 40 days
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(0.25 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareWorldEconomyMenu::GetResourcePriceVariationInfo, Sector, TSharedPtr<int32>(new int32(40)))
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
						.Icon(FFlareStyleSet::GetIcon("Travel"))
						.OnClicked(this, &SFlareWorldEconomyMenu::OnOpenSectorPrices, Sector)
					]
				]
			]
		];
	}

	SlatePrepass(FSlateApplicationBase::Get().GetApplicationScale());
}

void SFlareWorldEconomyMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
	SectorList->ClearChildren();
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FSlateColor SFlareWorldEconomyMenu::GetPriceColor(UFlareSimulatedSector* Sector) const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	if (TargetResource)
	{

		FLinearColor HighPriceColor = Theme.FriendlyColor;
		FLinearColor MeanPriceColor = Theme.NeutralColor;
		FLinearColor LowPriceColor = Theme.EnemyColor;

		float ResourcePrice = Sector->GetPreciseResourcePrice(TargetResource);

		float PriceRatio = (ResourcePrice - TargetResource->MinPrice) / (float)(TargetResource->MaxPrice - TargetResource->MinPrice);

		if (PriceRatio > 0.5)
		{
			return FMath::Lerp(MeanPriceColor, HighPriceColor, 2.f * (PriceRatio - 0.5));
		}
		else
		{
			return FMath::Lerp(LowPriceColor, MeanPriceColor, 2.f * PriceRatio);
		}

		return FMath::Lerp(LowPriceColor, HighPriceColor, PriceRatio);
	}
	return Theme.FriendlyColor;
}

FText SFlareWorldEconomyMenu::GetResourceDescription() const
{
	if (TargetResource)
	{
		return TargetResource->Description;
	}

	return FText();
}

FText SFlareWorldEconomyMenu::GetResourceName() const
{
	if (TargetResource)
	{
		return TargetResource->Name;
	}

	return LOCTEXT("NoResourceSelected", "No resource selected");
}

const FSlateBrush* SFlareWorldEconomyMenu::GetResourceIcon() const
{
	if (TargetResource)
	{
		return &TargetResource->Icon;
	}

	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	return &Theme.ResourceBackground;
}

FText SFlareWorldEconomyMenu::GetResourceInfo() const
{
	if (TargetResource)
	{
		if (WorldStats.Contains(TargetResource))
		{
			FNumberFormattingOptions Format;
			Format.MaximumFractionalDigits = 1;

			// Balance info
			FText BalanceText;
			float Balance = WorldStats[TargetResource].Balance;
			if (Balance > 0)
			{
				BalanceText = FText::Format(LOCTEXT("BalanceInfoPlusFormat", "+{0} / day"),
					FText::AsNumber(Balance, &Format));
			}
			else
			{
				BalanceText = FText::Format(LOCTEXT("BalanceInfoNegFormat", "{0} / day"),
					FText::AsNumber(Balance, &Format));
			}
			

			FText Part1 = FText::Format(LOCTEXT("StockInfoFormatPart1", "\u2022Transport fee: {0} credits\n\u2022 Worldwide stock: {1}\n\u2022 Worldwide needs: {2}\n"),
										UFlareGameTools::DisplayMoney(TargetResource->TransportFee),
										FText::AsNumber(WorldStats[TargetResource].Stock),
										FText::AsNumber(WorldStats[TargetResource].Capacity));
			FText Part2 = FText::Format(LOCTEXT("StockInfoFormatPart2", "\u2022 Worldwide production: {0} / day\n\u2022 Worldwide usage: {1} / day\n"),
										FText::AsNumber(WorldStats[TargetResource].Production, &Format),
										FText::AsNumber(WorldStats[TargetResource].Consumption, &Format));

			// Generate info
			return FText::Format(LOCTEXT("StockInfoFormat",
					"{0}{1}\u2022 Balance: {2}"),
				Part1,
				Part2,
				BalanceText);
		}
	}

	return FText();
}

FText SFlareWorldEconomyMenu::GetSectorText(UFlareSimulatedSector* Sector) const
{
	return FText::Format(LOCTEXT("SectorInfoFormat", "{0} ({1})"),
		Sector->GetSectorName(),
		Sector->GetSectorFriendlynessText(MenuManager->GetPC()->GetCompany()));
}

FSlateColor SFlareWorldEconomyMenu::GetSectorTextColor(UFlareSimulatedSector* Sector) const
{
	return Sector->GetSectorFriendlynessColor(MenuManager->GetPC()->GetCompany());
}

FText SFlareWorldEconomyMenu::GetResourceProductionInfo(UFlareSimulatedSector* Sector) const
{
	if (TargetResource)
	{
		FNumberFormattingOptions Format;
		Format.MaximumFractionalDigits = 1;

		TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> Stats = SectorHelper::ComputeSectorResourceStats(Sector);
		return FText::Format(LOCTEXT("ResourceMainProductionFormat", "{0}"),
			FText::AsNumber(Stats[TargetResource].Production, &Format));
	}

	return FText();
}

FText SFlareWorldEconomyMenu::GetResourceConsumptionInfo(UFlareSimulatedSector* Sector) const
{
	if (TargetResource)
	{
		FNumberFormattingOptions Format;
		Format.MaximumFractionalDigits = 1;

		TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> Stats = SectorHelper::ComputeSectorResourceStats(Sector);
		return FText::Format(LOCTEXT("ResourceMainConsumptionFormat", "{0}"),
			FText::AsNumber(Stats[TargetResource].Consumption, &Format));
	}

	return FText();
}

FText SFlareWorldEconomyMenu::GetResourceStockInfo(UFlareSimulatedSector* Sector) const
{
	if (TargetResource)
	{
		TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> Stats = SectorHelper::ComputeSectorResourceStats(Sector);
		return FText::Format(LOCTEXT("ResourceMainStockFormat", "{0}"),
			FText::AsNumber(Stats[TargetResource].Stock));
	}

	return FText();
}


FText SFlareWorldEconomyMenu::GetResourceCapacityInfo(UFlareSimulatedSector* Sector) const
{
	if (TargetResource)
	{

		TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> Stats = SectorHelper::ComputeSectorResourceStats(Sector);
		return FText::Format(LOCTEXT("ResourceMainCapacityFormat", "{0}"),
			FText::AsNumber(Stats[TargetResource].Capacity));
	}

	return FText();
}

FText SFlareWorldEconomyMenu::GetResourcePriceInfo(UFlareSimulatedSector* Sector) const
{
	if (TargetResource)
	{
		FNumberFormattingOptions MoneyFormat;
		MoneyFormat.MaximumFractionalDigits = 2;

		int64 ResourcePrice = Sector->GetResourcePrice(TargetResource, EFlareResourcePriceContext::Default);

		return FText::Format(LOCTEXT("ResourceMainPriceFormat", "{0} credits"),
			FText::AsNumber(ResourcePrice / 100.0f, &MoneyFormat));
	}

	return FText();
}

FText SFlareWorldEconomyMenu::GetResourcePriceVariationInfo(UFlareSimulatedSector* Sector, TSharedPtr<int32> MeanDuration) const
{
	if (TargetResource)
	{
		FNumberFormattingOptions MoneyFormat;
		MoneyFormat.MaximumFractionalDigits = 2;

		int64 ResourcePrice = Sector->GetResourcePrice(TargetResource, EFlareResourcePriceContext::Default);
		int64 LastResourcePrice = Sector->GetResourcePrice(TargetResource, EFlareResourcePriceContext::Default, *MeanDuration);

		if(ResourcePrice != LastResourcePrice)
		{
			float Variation = (((float) ResourcePrice) / ((float) LastResourcePrice) - 1);

			if(FMath::Abs(Variation) >= 0.0001)
			{
				return FText::Format(LOCTEXT("ResourceVariationFormat", "{0}{1}%"),
								(Variation > 0 ?
									 FText::FText(LOCTEXT("ResourceVariationFormatSignPlus","+")) :
									 FText::FText(LOCTEXT("ResourceVariationFormatSignMinus","-"))),
							  FText::AsNumber(FMath::Abs(Variation) * 100.0f, &MoneyFormat));
			}
		}
		return FText::FText(LOCTEXT("ResourceMainPriceNoVariationFormat", "-"));
	}

	return FText();
}


TSharedRef<SWidget> SFlareWorldEconomyMenu::OnGenerateResourceComboLine(UFlareResourceCatalogEntry* Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(STextBlock)
		.Text(Item->Data.Name)
		.TextStyle(&Theme.TextFont);
}

FText SFlareWorldEconomyMenu::OnGetCurrentResourceComboLine() const
{
	UFlareResourceCatalogEntry* Item = ResourceSelector->GetSelectedItem();
	if (Item)
	{
		if (Item->Data.Name.CompareTo(Item->Data.Acronym) == 0)
		{
			return Item->Data.Name;
		}
		else
		{
			return FText::Format(LOCTEXT("ComboResourceLineFormat", "{0} ({1})"), Item->Data.Name, Item->Data.Acronym);
		}
	}
	else
	{
		return LOCTEXT("SelectResource", "Select a resource");
	}
}

void SFlareWorldEconomyMenu::OnResourceComboLineSelectionChanged(UFlareResourceCatalogEntry* Item, ESelectInfo::Type SelectInfo)
{
	if (Item)
	{
		TargetResource  = &Item->Data;
	}
	else
	{
		TargetResource = NULL;
	}

	GenerateSectorList();
}

void SFlareWorldEconomyMenu::OnOpenSector(UFlareSimulatedSector* Sector)
{
	FFlareMenuParameterData Data;
	Data.Sector = Sector;
	MenuManager->OpenMenu(EFlareMenu::MENU_Sector, Data);
}

void SFlareWorldEconomyMenu::OnOpenSectorPrices(UFlareSimulatedSector* Sector)
{
	FFlareMenuParameterData Data;
	Data.Sector = Sector;
	MenuManager->OpenMenu(EFlareMenu::MENU_ResourcePrices, Data);
}

#undef LOCTEXT_NAMESPACE

