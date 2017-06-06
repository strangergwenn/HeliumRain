
#include "../../Flare.h"
#include "FlareEULAMenu.h"
#include "../Components/FlareButton.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareSaveGame.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareEULAMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareEULAMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	int32 Width = 1.75 * Theme.ContentWidth;
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
				.Text(LOCTEXT("DONT-TRANSLATE-EULA", "END-USER LICENCE AGREEMENT"))
				.TextStyle(&Theme.SpecialTitleFont)
			]
		]

		// Main
		+ SVerticalBox::Slot()
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

				// Text
				+ SVerticalBox::Slot()
				[
					SNew(SScrollBox)
					.Style(&Theme.ScrollBoxStyle)
					.ScrollBarStyle(&Theme.ScrollBarStyle)

					+ SScrollBox::Slot()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("DONT-TRANSLATE-EULA-Text", "Helium Rain includes the Unreal\u00AE Engine code and other code, materials, and information " \
							"(the 'Epic Materials') from Epic Games, Inc. ('Epic'). All Epic Materials are provided on an 'as is' and 'as available' basis, " \
							"'with all faults' and without warranty of any kind. Deimos Games, Epic, and Epic's affiliates disclaim all warranties, conditions, common law duties, " \
							"and representations (express, implied, oral, and written) with respect to the Epic Materials, including without limitation all express, " \
							"implied, and statutory warranties and conditions of any kind, such as title, non-interference with your enjoyment, authority, non-infringement, " \
							"merchantability, fitness or suitability for any purpose (whether or not Epic knows or has reason to know of any such purpose), system integration, " \
							"accuracy or completeness, results, reasonable care, workmanlike effort, lack of negligence, and lack of viruses, whether alleged to arise under law, " \
							"by reason of custom or usage in the trade, or by course of dealing. \n\n" \
							"Without limiting the generality of the foregoing, Deimos Games, Epic, and Epic's affiliates make no warranty that \n" \
							"    (1) any of the Epic Materials will operate properly, including as integrated in Helium Rain, \n" \
							"    (2) that the Epic Materials will meet your requirements, \n" \
							"    (3) that the operation of the Epic Materials will be uninterrupted, bug free, or error free in any or all circumstances, \n" \
							"    (4) that any defects in the Epic Materials can or will be corrected, \n" \
							"    (5) that the Epic Materials are or will be in compliance with a platform manufacturer's rules or requirements, or \n" \
							"    (6) that a platform manufacturer has approved or will approve Helium Rain, or will not revoke approval of Helium Rain for any or no reason. \n\n" \
							"Any warranty against infringement that may be provided in Section 2-312 of the Uniform Commercial Code or in any other comparable statute is expressly disclaimed " \
							"by Deimos Games and Epic. Deimos Games, Epic, and Epic's affiliates do not guarantee continuous, error-free, virus-free, or secure operation of or access to the " \
							"Epic Materials. \n\n" \
							"This paragraph will apply to the maximum extent permitted by applicable law. To the maximum extent permitted by applicable law, neither Deimos Games, " \
							"Epic, Epic's licensors, nor its or their affiliates, nor any of Deimos Games's or Epic's service providers, shall be liable in any way for loss or damage of any kind " \
							"resulting from the use or inability to use the Epic Materials or otherwise in connection with this Agreement, including but not limited to loss of goodwill, " \
							"work stoppage, computer failure, or malfunction, or any and all other commercial damages or losses. In no event will Deimos Games, Epic, Epic's licensors, " \
							"nor its or their affiliates, nor any of Deimos Games's or Epic's service providers be liable for any loss of profits or any indirect, incidental, consequential, " \
							"special, punitive, or exemplary damages, or any other damages arising out of or in connection with this Agreement or the Epic Materials, or the delay or inability " \
							"to use or lack of functionality of the Epic Materials, even in the event of Deimos Games's, Epic's, or Epic's affiliates' fault, tort (including negligence), " \
							"strict liability, indemnity, product liability, breach of contract, breach of warranty, or otherwise and even if Deimos Games, Epic or Epic's affiliates " \
							"have been advised of the possibility of such damages. \n\n" \
							"These limitations and exclusions regarding damages apply even if any remedy fails to provide adequate " \
							"compensation. Because some states or jurisdictions do not allow the exclusion or the limitation of liability for consequential or incidental damages, " \
							"in such states or jurisdictions, the liability of Deimos Games, Epic, Epic's licensors, its and their affiliates, and any of Deimos Games's or Epic's service " \
							"providers shall be limited to the full extent permitted by law.")) // FString neded here
						.WrapTextAt(TextWidth)
						.TextStyle(&Theme.NameFont)
					]
				]
			]
		]

		// Back
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		[
			SNew(SFlareButton)
			.Transparent(true)
			.Width(3)
			.Text(LOCTEXT("Back", "Back"))
			.OnClicked(this, &SFlareEULAMenu::OnMainMenu)
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareEULAMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareEULAMenu::Enter()
{
	FLOG("SFlareEULAMenu::Enter");
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
}

void SFlareEULAMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareEULAMenu::OnMainMenu()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Main);
}


#undef LOCTEXT_NAMESPACE

