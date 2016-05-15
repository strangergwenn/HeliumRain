
#include "../../Flare.h"
#include "FlareWorldEconomyMenu.h"
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
	[
		SNew(SVerticalBox)

		// Title
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.Padding(Theme.ContentPadding)
		[
			SNew(SHorizontalBox)

			// Icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SImage).Image(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_WorldEconomy))
			]

			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("WorldEconomyTitle", "WORLD ECONOMY"))
				.TextStyle(&Theme.TitleFont)
			]

			// Close
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("Back", "Back"))
				.HelpText(LOCTEXT("BackInfo", "Go to the previous menu"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Exit, true))
				.OnClicked(this, &SFlareWorldEconomyMenu::Back)
			]
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(200, 20))
		[
			SNew(SImage).Image(&Theme.SeparatorBrush)
		]

		// Main
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(Theme.ContentPadding)
		[

			SNew(SHorizontalBox)

			// Resource List
			+ SHorizontalBox::Slot()
			.Padding(Theme.SmallContentPadding)
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

			// TODO : transport fee, stock, flow, etc
		]

		// List
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
				.WidthOverride(1.6 * Theme.ContentWidth)
				.HAlign(HAlign_Fill)
				[
					SAssignNew(SectorList, SVerticalBox)
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

void SFlareWorldEconomyMenu::Enter(FFlareResourceDescription* Resource, UFlareSectorInterface* Sector)
{
	FLOG("SFlareWorldEconomyMenu::Enter");
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	MenuManager->UseDarkBackground();

	BackSector = Sector;
	TargetResource = Resource;

	ResourceSelector->SetSelectedItem(MenuManager->GetPC()->GetGame()->GetResourceCatalog()->GetEntry(TargetResource));

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

	// Header
	SectorList->AddSlot()
	.Padding(FMargin(1))
	[
		SNew(SHorizontalBox)

		// Sector Name
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(0.25 * Theme.ContentWidth)
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.NameFont)
				.Text(LOCTEXT("SectorNameColumnTitleInfo", "Sector"))
			]
		]

		// Price
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(0.25 * Theme.ContentWidth)
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
		[
			SNew(SBox)
			.WidthOverride(0.3 * Theme.ContentWidth)
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.NameFont)
				.Text(LOCTEXT("ResourcePriceVariation1dColumnTitleInfo", "Price variation (1 day)"))
			]
		]

		// Price variation 40 days
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(0.25 * Theme.ContentWidth)
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.NameFont)
				.Text(LOCTEXT("ResourcePriceVariation40dColumnTitleInfo", "Price variation (40 days)"))
			]
		]
	];


	// Natural order is sorted bay discovery
	TArray<UFlareSimulatedSector*>& Sectors = MenuManager->GetPC()->GetCompany()->GetKnownSectors();

	// Sector list
	for (int32 SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Sectors[SectorIndex];

		SectorList->AddSlot()
		.Padding(FMargin(1))
		[
			SNew(SHorizontalBox)

			// Sector name
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
					.Text(Sector->GetSectorName())
					.WrapTextAt(0.25 * Theme.ContentWidth)
				]
			]

			// Price
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
				.WidthOverride(0.25 * Theme.ContentWidth)
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
		];
	}
}

void SFlareWorldEconomyMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
	SectorList->ClearChildren();
	TargetResource = NULL;
}

void SFlareWorldEconomyMenu::Back()
{
	if (BackSector)
	{
		MenuManager->OpenMenu(EFlareMenu::MENU_Sector, BackSector);
	}
	else
	{
		MenuManager->OpenMenu(EFlareMenu::MENU_Orbit);
	}
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/


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
				return FText::Format(LOCTEXT("ResourceVariationFormat", " ({0}{1}%)"),
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
		return FText::Format(LOCTEXT("ComboResourceLineFormat", "{0} ({1})"), Item->Data.Name, Item->Data.Acronym);
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

#undef LOCTEXT_NAMESPACE

