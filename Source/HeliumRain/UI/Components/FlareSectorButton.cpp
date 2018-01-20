
#include "FlareSectorButton.h"
#include "../../Flare.h"

#include "../../Game/FlareGame.h"
#include "../../Game/FlareGameTools.h"
#include "../../Game/FlareWorld.h"
#include "../../Game/FlareTravel.h"
#include "../../Game/FlareCompany.h"
#include "../../Game/FlareSimulatedSector.h"

#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"

#include "../Menus/FlareOrbitalMenu.h"


#define LOCTEXT_NAMESPACE "FlareSectorButton"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSectorButton::Construct(const FArguments& InArgs)
{
	// Setup
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor Color = FLinearColor::White;
	Color.A = 0.85f;

	// Arguments
	OnClicked = InArgs._OnClicked;
	Sector = InArgs._Sector;
	PlayerCompany = InArgs._PlayerCompany;

	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Center)
	[
		SNew(SBorder)
		.BorderImage(FFlareStyleSet::GetIcon("LargeButtonBackground"))
		.BorderBackgroundColor(Color)
		.Padding(FMargin(25))
		[
			SNew(SVerticalBox)

			// Upper line
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.SectorButtonTextPadding)
			[
				SAssignNew(TextBlock, STextBlock)
				.Text(this, &SFlareSectorButton::GetSectorTitle)
				.WrapTextAt(3 * Theme.SectorButtonWidth)
				.TextStyle(&Theme.TextFont)
				.Justification(ETextJustify::Center)
				.ShadowColorAndOpacity(this, &SFlareSectorButton::GetShadowColor)
			]

			// Main
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				// Image box
				SNew(SBox)
				.WidthOverride(Theme.SectorButtonWidth)
				.HeightOverride(Theme.SectorButtonHeight)
				[
					// Button (behavior only, no display)
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
							.ColorAndOpacity(this, &SFlareSectorButton::GetMainColor)
						]
					]
				]
			]

			// Lower line
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.SectorButtonTextPadding)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SAssignNew(FleetBox, SHorizontalBox)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(TextBlock, STextBlock)
					.Text(this, &SFlareSectorButton::GetSectorText)
					.WrapTextAt(3 * Theme.SectorButtonWidth)
					.TextStyle(&Theme.SmallFont)
					.Justification(ETextJustify::Center)
					.ShadowColorAndOpacity(this, &SFlareSectorButton::GetShadowColor)
					.Visibility(this, &SFlareSectorButton::GetBottomTextVisibility)
				]
			]
		]
	];
}

void SFlareSectorButton::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	// Tick parent
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Get display mode
	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	EFlareOrbitalMode::Type DisplayMode = EFlareOrbitalMode::Stations;
	if (MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Orbit)
	{
		DisplayMode = MenuManager->GetOrbitMenu()->GetDisplayMode();
	}

	// Draw fleet info
	FleetBox->ClearChildren();
	if (Sector && DisplayMode == EFlareOrbitalMode::Fleets)
	{
		// Get local fleets
		for (UFlareFleet* Fleet : Sector->GetSectorFleets())
		{
			if (Fleet->GetFleetCompany()->IsPlayerCompany())
			{
				FLinearColor FleetColor = Fleet->GetFleetColor();
				const FSlateBrush* FleetIcon = (Fleet->GetCombatPoints(false) > 0) ?
					FFlareStyleSet::GetIcon("Fleet_Small_Military") : FFlareStyleSet::GetIcon("Fleet_Small");

				FleetBox->AddSlot()
				.AutoWidth()
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SImage)
						.Image(FleetIcon)
						.ColorAndOpacity(FleetColor)
					]
				];
			}
		}

		// Get incoming fleets
		for (UFlareTravel* Travel : MenuManager->GetGame()->GetGameWorld()->GetTravels())
		{
			if (Travel->GetFleet()->GetFleetCompany()->IsPlayerCompany() && Sector == Travel->GetDestinationSector())
			{
				FLinearColor FleetColor = Travel->GetFleet()->GetFleetColor();
				FText DateText = UFlareGameTools::FormatDate(Travel->GetRemainingTravelDuration(), 1);
				FleetColor.A = 0.5f;

				FleetBox->AddSlot()
				.AutoWidth()
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SImage)
						.Image(FFlareStyleSet::GetIcon("Fleet_Small"))
						.ColorAndOpacity(FleetColor)
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(DateText)
						.TextStyle(&Theme.SmallFont)
					]
				];
			}
		}

		SlatePrepass(FSlateApplicationBase::Get().GetApplicationScale());
	}
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
		FText SectorStatus = Sector->GetSectorFriendlynessText(PlayerCompany);
		FText SectorNameText = FText::Format(LOCTEXT("SectorNameFormat", "{0} ({1})"), Sector->GetSectorName(), SectorStatus);
		FText SectorInfoText = Sector->GetSectorDescription();

		// Get local fleets
		for (UFlareFleet* Fleet : Sector->GetSectorFleets())
		{
			if (Fleet->GetFleetCompany()->IsPlayerCompany())
			{
				SectorInfoText = FText::Format(LOCTEXT("SectorInfoFleetLocalFormat", "{0}\n\u2022 Your fleet {1} is here."),
					SectorInfoText,
					Fleet->GetFleetName());
			}
		}

		// Get incoming fleets
		for (UFlareTravel* Travel : MenuManager->GetGame()->GetGameWorld()->GetTravels())
		{
			if (Travel->GetFleet()->GetFleetCompany()->IsPlayerCompany() && Sector == Travel->GetDestinationSector())
			{
				FText DateText = UFlareGameTools::FormatDate(Travel->GetRemainingTravelDuration(), 1);
				SectorInfoText = FText::Format(LOCTEXT("SectorInfoFleetTravelFormat", "{0}\n\u2022 Your fleet {1} will arrive in {2}."),
					SectorInfoText,
					Travel->GetFleet()->GetFleetName(),
					DateText);
			}
		}

		MenuManager->ShowTooltip(this, SectorNameText, SectorInfoText);
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

bool SFlareSectorButton::ShouldDisplayFleets() const
{
	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	EFlareOrbitalMode::Type DisplayMode = EFlareOrbitalMode::Stations;
	if (MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Orbit)
	{
		DisplayMode = MenuManager->GetOrbitMenu()->GetDisplayMode();
	}

	if (DisplayMode == EFlareOrbitalMode::Fleets)
	{
		return true;
	}
	else
	{
		return false;
	}
}

EVisibility SFlareSectorButton::GetBottomTextVisibility() const
{
	if (ShouldDisplayFleets())
	{
		return EVisibility::Collapsed;
	}
	else
	{
		return EVisibility::Visible;
	}
}

FText SFlareSectorButton::GetSectorTitle() const
{
	if (Sector)
	{
		return Sector->GetSectorName();
	}

	return FText();
}

FText SFlareSectorButton::GetSectorText() const
{
	// Get display mode
	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	EFlareOrbitalMode::Type DisplayMode = EFlareOrbitalMode::Stations;
	if (MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Orbit)
	{
		DisplayMode = MenuManager->GetOrbitMenu()->GetDisplayMode();
	}

	// If the sector is known, display it
	if (Sector && PlayerCompany->HasVisitedSector(Sector))
	{
		FText SectorText;

		if (DisplayMode == EFlareOrbitalMode::Fleets)
		{
		}

		else if (DisplayMode == EFlareOrbitalMode::Stations && Sector->GetSectorStations().Num() > 0)
		{
			SectorText = Sector->GetSectorStations().Num() == 1 ? LOCTEXT("Station", "{0} station") : LOCTEXT("Stations", "{0} stations");
			SectorText = FText::Format(SectorText, FText::AsNumber(Sector->GetSectorStations().Num()));
		}

		else if (DisplayMode == EFlareOrbitalMode::Ships && Sector->GetSectorShips().Num() > 0)
		{
			SectorText = Sector->GetSectorShips().Num() == 1 ? LOCTEXT("Ship", "{0} ship") : LOCTEXT("Ships", "{0} ships");
			SectorText = FText::Format(SectorText, FText::AsNumber(Sector->GetSectorShips().Num()));
		}

		else if (DisplayMode == EFlareOrbitalMode::Battles)
		{
			SectorText = Sector->GetSectorBattleStateText(PlayerCompany);
		}

		return SectorText;
	}

	return FText();
}

const FSlateBrush* SFlareSectorButton::GetBackgroundBrush() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return (IsHovered() ? &Theme.SectorButtonActiveBackground : &Theme.SectorButtonBackground);
}

FSlateColor SFlareSectorButton::GetMainColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	FLinearColor Color = FLinearColor::White;

	if (Sector == MenuManager->GetPC()->GetPlayerFleet()->GetCurrentSector())
	{
		Color = Theme.FriendlyColor;
	}

	Color = (IsHovered() ? Color : Color.Desaturate(0.1));
	return FSlateColor(Color);
}

FSlateColor SFlareSectorButton::GetBorderColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	FLinearColor Color = FLinearColor::White;

	if (Sector)
	{
		if (MenuManager->GetPC()->GetCurrentObjective() && MenuManager->GetPC()->GetCurrentObjective()->IsTarget(Sector))
		{
			return Theme.ObjectiveColor;
		}
		else
		{
			Color = Sector->GetSectorFriendlynessColor(PlayerCompany);
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
