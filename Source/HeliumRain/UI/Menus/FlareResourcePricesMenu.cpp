
#include "../../Flare.h"
#include "FlareResourcePricesMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Economy/FlareResource.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"


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
				SNew(SImage).Image(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_ResourcePrices))
			]

			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ResourcePricesTitle", "RESOURCE PRICES"))
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
				.OnClicked(this, &SFlareResourcePricesMenu::Back)
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
					SAssignNew(ResourcePriceList, SVerticalBox)
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

void SFlareResourcePricesMenu::Enter(UFlareSectorInterface* Sector)
{
	FLOG("SFlareResourcePricesMenu::Enter");
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	MenuManager->UseDarkBackground();

	TargetSector = Sector;

	// Resource prices
	ResourcePriceList->ClearChildren();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	TArray<UFlareResourceCatalogEntry*>& ResourceList = MenuManager->GetGame()->GetResourceCatalog()->GetResourceList();
	ResourceList.Sort(&SortByResourceType);

	// Header
	ResourcePriceList->AddSlot()
	.Padding(FMargin(1))
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

		// Info
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(0.8 * Theme.ContentWidth)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.NameFont)
				.Text(LOCTEXT("ResourceInfo", "Resource info"))
			]
		]

		// Info
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(0.6 * Theme.ContentWidth)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.NameFont)
				.Text(LOCTEXT("ResourcePrice", "Prices in this sector"))
			]
		]
	];

	// Resource prices
	for (int32 ResourceIndex = 0; ResourceIndex < ResourceList.Num(); ResourceIndex++)
	{
		FFlareResourceDescription& Resource = ResourceList[ResourceIndex]->Data;
		ResourcePriceList->AddSlot()
		.Padding(FMargin(1))
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

			// Info
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(SBox)
				.WidthOverride(0.8 * Theme.ContentWidth)
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(Resource.Description)
					.WrapTextAt(0.8 * Theme.ContentWidth)
				]
			]

			// Price
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(SBox)
				.WidthOverride(0.6 * Theme.ContentWidth)
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(this, &SFlareResourcePricesMenu::GetResourcePriceInfo, &Resource)
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

void SFlareResourcePricesMenu::Back()
{
	if (TargetSector)
	{
		MenuManager->OpenMenu(EFlareMenu::MENU_Sector, TargetSector);
	}
	else
	{
		MenuManager->OpenMenu(EFlareMenu::MENU_Orbit);
	}
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FText SFlareResourcePricesMenu::GetResourcePriceInfo(FFlareResourceDescription* Resource) const
{
	if (TargetSector)
	{
		FNumberFormattingOptions MoneyFormat;
		MoneyFormat.MaximumFractionalDigits = 2;

		int32 MeanDuration = 50;
		int64 ResourcePrice = TargetSector->GetResourcePrice(Resource, EFlareResourcePriceContext::Default);
		int64 LastResourcePrice = TargetSector->GetResourcePrice(Resource, EFlareResourcePriceContext::Default, MeanDuration-1);

		FText VariationText;

		if(ResourcePrice != LastResourcePrice)
		{
			float Variation = (((float) ResourcePrice) / ((float) LastResourcePrice) - 1);

			VariationText = FText::Format(LOCTEXT("ResourceVariationFormat", " ({0}{1}%)"),
							(Variation > 0 ?
								 FText::FText(LOCTEXT("ResourceVariationFormatSignPlus","+")) :
								 FText::FText(LOCTEXT("ResourceVariationFormatSignMinus","-"))),
						  FText::AsNumber(FMath::Abs(Variation) * 100.0f, &MoneyFormat));
		}

		return FText::Format(LOCTEXT("ResourceMainPriceFormat", "{0} credits{2} - Transport fee {1} credits "),
			FText::AsNumber(ResourcePrice / 100.0f, &MoneyFormat),
			FText::AsNumber(Resource->TransportFee / 100.0f, &MoneyFormat),
			VariationText);
	}

	return FText();
}


#undef LOCTEXT_NAMESPACE

