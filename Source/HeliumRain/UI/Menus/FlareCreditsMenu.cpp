
#include "../../Flare.h"
#include "FlareCreditsMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareSaveGame.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareCreditsMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareCreditsMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FSlateFontInfo TitleFont(FPaths::GameContentDir() / TEXT("Slate/Fonts/Lato700.ttf"), 64);
	FSlateFontInfo MainFont(FPaths::GameContentDir() / TEXT("Slate/Fonts/Lato700.ttf"), 28);
	FSlateFontInfo SecondaryFont(FPaths::GameContentDir() / TEXT("Slate/Fonts/Lato700.ttf"), 16);
	int32 Width = 1.5 * Theme.ContentWidth;
	int32 TextWidth = Width - Theme.ContentPadding.Left - Theme.ContentPadding.Right;

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SVerticalBox)

		// Title
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(Theme.ContentPadding)
		[
			SNew(SHorizontalBox)

			// Icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SImage).Image(FFlareStyleSet::GetImage("HeliumRain"))
			]

			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DONT-TRANSLATE-Title", "HELIUM RAIN"))
				.Font(TitleFont)
			]
		]

		// Main
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(Theme.ContentPadding)
		[
			SNew(SBox)
			.WidthOverride(Width)
			.HAlign(HAlign_Fill)
			[
				SNew(SVerticalBox)
				
				// Separator
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0, 20))
				[
					SNew(SImage).Image(&Theme.SeparatorBrush)
				]

				// Team 1
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					// Stranger
					+ SHorizontalBox::Slot()
					.Padding(Theme.ContentPadding)
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("DONT-TRANSLATE-Stranger", "Gwenna\u00EBl ARBONA 'Stranger'"))
							.Font(MainFont)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("DONT-TRANSLATE-Stranger-Info", "Art \u2022 UI \u2022 Game design"))
							.Font(SecondaryFont)
						]
					]

					// Niavok
					+ SHorizontalBox::Slot()
					.Padding(Theme.ContentPadding)
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("DONT-TRANSLATE-Niavok", "Fr\u00E9d\u00E9ric BERTOLUS 'Niavok'"))
							.Font(MainFont)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("DONT-TRANSLATE-Niavok-Info", "Gameplay \u2022 Game design"))
							.Font(SecondaryFont)
						]
					]
				]

				// Team 2
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					// Daisy
					+ SHorizontalBox::Slot()
					.Padding(Theme.ContentPadding)
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("DONT-TRANSLATE-Daisy", "Daisy HERBAULT"))
							.Font(MainFont)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("DONT-TRANSLATE-Daisy-Info", "Music"))
							.Font(SecondaryFont)
						]
					]

					// Grom
					+ SHorizontalBox::Slot()
					.Padding(Theme.ContentPadding)
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("DONT-TRANSLATE-Grom", "J\u00E9r\u00F4me MILLION-ROUSSEAU"))
							.Font(MainFont)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("DONT-TRANSLATE-Grom-Info", "Game logo \u2022 Communication"))
							.Font(SecondaryFont)
						]
					]
				]

				// Thanks
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("DONT-TRANSLATE-Special Thanks", "Special thanks : Johanna and our friends at NERD and Parrot"))
					.Font(SecondaryFont)
				]

				// Separator
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0, 20))
				[
					SNew(SImage).Image(&Theme.SeparatorBrush)
				]
				
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("DONT-TRANSLATE-Engine-Info", "Helium Rain uses the Unreal\u00AE Engine. Unreal\u00AE is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere."))
					.Font(SecondaryFont)
					.WrapTextAt(TextWidth)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("DONT-TRANSLATE-Engine-Info2", "Unreal\u00AE Engine, Copyright 1998 - 2016, Epic Games, Inc. All rights reserved."))
					.Font(SecondaryFont)
					.WrapTextAt(TextWidth)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("DONT-TRANSLATE-Sound-Info", "Helium Rain uses sound from http://www.freesfx.co.uk ."))
					.Font(SecondaryFont)
					.WrapTextAt(TextWidth)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("DONT-TRANSLATE-Assets-Info", "Helium Rain uses game assets by 'Imphenzia' and 'Gargore'."))
					.Font(SecondaryFont)
					.WrapTextAt(TextWidth)
				]
			]
		]

		// Back
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		[
			SNew(SFlareButton)
			.Transparent(true)
			.Width(3)
			.Text(LOCTEXT("Back", "Back"))
			.OnClicked(this, &SFlareCreditsMenu::OnMainMenu)
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareCreditsMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}

void SFlareCreditsMenu::Enter()
{
	FLOG("SFlareCreditsMenu::Enter");
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	MenuManager->UseDarkBackground();
}

void SFlareCreditsMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareCreditsMenu::OnMainMenu()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Main);
}


#undef LOCTEXT_NAMESPACE

