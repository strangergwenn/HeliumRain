
#include "../../Flare.h"
#include "FlareSectorButton.h"
#include "../../Game/FlareCompany.h"
#include "../../Game/FlareSimulatedSector.h"
#include "../../Player/FlareMenuManager.h"


#define LOCTEXT_NAMESPACE "FlareSectorButton"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSectorButton::Construct(const FArguments& InArgs)
{
	// Setup
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Arguments
	OnClicked = InArgs._OnClicked;
	Sector = InArgs._Sector;
	PlayerCompany = InArgs._PlayerCompany;

	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Center)
	[
		SNew(SVerticalBox)

		// Container
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			// Image box
			SNew(SBox)
			.WidthOverride(Theme.SectorButtonWidth)
			.HeightOverride(Theme.SectorButtonHeight)
			[
				// Button (behaviour only, no display)
				SNew(SButton)
				.OnClicked(this, &SFlareSectorButton::OnButtonClicked)
				.ContentPadding(FMargin(0))
				.ButtonStyle(FCoreStyle::Get(), "NoBorder")
				[
					// Background
					SNew(SBorder)
					.Padding(Theme.SectorButtonPadding)
					.BorderImage(&Theme.SectorButtonBorder)
					.BorderBackgroundColor(this, &SFlareSectorButton::GetBorderColor)
					[
						// Icon
						SNew(SImage)
						.Image(this, &SFlareSectorButton::GetBackgroundBrush)
					]
				]
			]
		]

		// Button text
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.SectorButtonTextPadding)
		[
			SAssignNew(TextBlock, STextBlock)
			.Text(this, &SFlareSectorButton::GetSectorText)
			.WrapTextAt(3 * Theme.SectorButtonWidth)
			.TextStyle(&Theme.SmallFont)
			.Justification(ETextJustify::Center)
			.ShadowColorAndOpacity(this, &SFlareSectorButton::GetShadowColor)
		]
	];
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareSectorButton::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SWidget::OnMouseEnter(MyGeometry, MouseEvent);

	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	if (MenuManager && Sector)
	{
		MenuManager->ShowTooltip(this, Sector->GetSectorDescription());
	}
}

void SFlareSectorButton::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SWidget::OnMouseLeave(MouseEvent);

	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	if (MenuManager)
	{
		MenuManager->HideTooltip(this);
	}
}

FText SFlareSectorButton::GetSectorText() const
{
	FText SectorText;
	
	if (Sector)
	{
		FString	SectorTitle = Sector->GetSectorName().ToString();

		if (PlayerCompany->HasVisitedSector(Sector))
		{
			FString ShipString = (Sector->GetSectorShips().Num() == 1 ? LOCTEXT("Ship", "ship").ToString() : LOCTEXT("Ships", "ships").ToString());
			FString StationString = (Sector->GetSectorStations().Num() == 1 ? LOCTEXT("Station", "station").ToString() : LOCTEXT("Stations", "stations").ToString());

			if (Sector->GetSectorShips().Num() > 0)
			{
				SectorTitle += "\n" + FString::FromInt(Sector->GetSectorShips().Num()) + " " + ShipString;
			}

			if (Sector->GetSectorStations().Num() > 0)
			{
				SectorTitle += "\n" + FString::FromInt(Sector->GetSectorStations().Num()) + " " + StationString;
			}
		}

		SectorText = FText::FromString(SectorTitle);
	}

	return SectorText;
}

const FSlateBrush* SFlareSectorButton::GetBackgroundBrush() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return (IsHovered() ? &Theme.SectorButtonActiveBackground : &Theme.SectorButtonBackground);
}

FSlateColor SFlareSectorButton::GetBorderColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor Color = FLinearColor::White;

	if (Sector)
	{
		switch(Sector->GetSectorFriendlyness(PlayerCompany))
		{
			case EFlareSectorFriendlyness::NotVisited:
				Color = Theme.UnknownColor;
				break;
			case EFlareSectorFriendlyness::Neutral:
				Color = Theme.NeutralColor;
				break;
			case EFlareSectorFriendlyness::Friendly:
				Color = Theme.FriendlyColor;
				break;
			case EFlareSectorFriendlyness::Contested:
				Color = Theme.DisputedColor;
				break;
			case EFlareSectorFriendlyness::Hostile:
				Color = Theme.EnemyColor;
				break;
		}
	}

	Color = (IsHovered() ? Color : Color.Desaturate(0.1));
	return FSlateColor(Color);
}

FLinearColor SFlareSectorButton::GetShadowColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor Color = Theme.SmallFont.ShadowColorAndOpacity;
	FLinearColor AlphaColor = FLinearColor::White;
	Color.A = AlphaColor.A;
	return Color;
}

FReply SFlareSectorButton::OnButtonClicked()
{
	if (OnClicked.IsBound())
	{
		OnClicked.Execute();
	}

	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE
