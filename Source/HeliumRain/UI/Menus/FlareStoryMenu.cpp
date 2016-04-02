
#include "../../Flare.h"
#include "FlareStoryMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareSaveGame.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareStoryMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareStoryMenu::Construct(const FArguments& InArgs)
{
	// Style data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FSlateFontInfo MainFont(FPaths::GameContentDir() / TEXT("Slate/Fonts/Lato700.ttf"), 42);
	FSlateFontInfo SecondaryFont(FPaths::GameContentDir() / TEXT("Slate/Fonts/Lato700.ttf"), 20);

	// Settings
	TextShowTime = 15.0f;
	TextHideTime = 1.0f;
	TransitionTime = 0.5f;
	int32 Width = 1.5 * Theme.ContentWidth;
	int32 TextWidth = Width - Theme.ContentPadding.Left - Theme.ContentPadding.Right;

	// Setup
	CurrentTime = -2.0;
	CurrentTextAlpha = 0;
	CurrentTextIndex = 0;

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SVerticalBox)
		
		// Main
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(Theme.ContentPadding)
		[
			SNew(SVerticalBox)

			// Title
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SAssignNew(Title, STextBlock)
				.Justification(ETextJustify::Center)
				.Font(MainFont)
				.WrapTextAt(TextWidth)
				.ColorAndOpacity(this, &SFlareStoryMenu::GetTextColor)
			]

			// Text
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SAssignNew(Text, STextBlock)
				.Justification(ETextJustify::Center)
				.Font(SecondaryFont)
				.WrapTextAt(TextWidth)
				.ColorAndOpacity(this, &SFlareStoryMenu::GetTextColor)
			]
		]

		// Skip
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		[
			SNew(SFlareButton)
			.Transparent(true)
			.Width(4)
			.Text(LOCTEXT("Skip", "Skip introduction"))
			.OnClicked(this, &SFlareStoryMenu::OnStartPlaying)
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareStoryMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);

	TitleList.Add(LOCTEXT("Story1Title", "2045"));
	TextList.Add(LOCTEXT("Story1", "The Kranz Space Telescope identifies a gas giant around \u03B2 Hydri, 24 light-years away from Earth. One of the many exoplanets in the universe, it is deemed capable of supporting life, and close enough to be in human reach.\n"));

	TitleList.Add(LOCTEXT("Story2Title", "2089"));
	TextList.Add(LOCTEXT("Story2", "Interstellar colonial carrier Daedalus leaves Earth orbit with a crew of 9,000. Packed with mining gear and scientific equipment to investigate the Hydri system, it will reach 20% the speed of light.\n"));

	TitleList.Add(LOCTEXT("Story3Title", "2213"));
	TextList.Add(LOCTEXT("Story3", "ICC Daedalus enters the orbit of Nema, the giant planet orbiting \u03B2 Hydri. Its moons are barren, desolated wastelands, unable to support life. Pumping stations are built around Nema to extract the valuable gases it's made of.\n"));

	TitleList.Add(LOCTEXT("Story4Title", "2219"));
	TextList.Add(LOCTEXT("Story4", "Daedalus is destroyed when some of the colonists attempt a return to Earth, only to meet military opposition from the others. As the colonial government breaks down, companies remain as the only social structures.\n"));

	TitleList.Add(LOCTEXT("Story5Title", "2224"));
	TextList.Add(LOCTEXT("Story5", "With no hope of return to the known world, life goes on, in relative peace...\n"));
}

void SFlareStoryMenu::Enter()
{
	FLOG("SFlareStoryMenu::Enter");
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	MenuManager->UseDarkBackground();
}

void SFlareStoryMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareStoryMenu::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	
	// Fading in
	if (CurrentTime <= TransitionTime)
	{
		CurrentTextAlpha = CurrentTime / TransitionTime;
	}

	// Shown
	else if (CurrentTime - TransitionTime <= TextShowTime)
	{
		CurrentTextAlpha = 1;
	}

	// Fading out
	else if (CurrentTime - TransitionTime - TextShowTime <= TransitionTime)
	{
		CurrentTextAlpha = 1 - (CurrentTime - TransitionTime - TextShowTime) / TransitionTime;
	}

	// Not shown
	else if (CurrentTime - TransitionTime - TextShowTime - TransitionTime <= TextHideTime)
	{
		CurrentTextAlpha = 0;
	}

	// Text switch : loop with the next text, or exit
	else
	{
		CurrentTextAlpha = 0;

		if (CurrentTextIndex + 1 == TextList.Num())
		{
			OnStartPlaying();
		}
		else
		{
			CurrentTime = 0;
			CurrentTextIndex++;
		}
	}

	// Update states
	CurrentTime += InDeltaTime;
	Title->SetText(TitleList[CurrentTextIndex]);
	Text->SetText(TextList[CurrentTextIndex]);
}

FSlateColor SFlareStoryMenu::GetTextColor() const
{
	return FLinearColor(1, 1, 1, CurrentTextAlpha);
}

void SFlareStoryMenu::OnStartPlaying()
{
	AFlarePlayerController* PC = MenuManager->GetPC();
	MenuManager->OpenMenu(EFlareMenu::MENU_Orbit, PC->GetShipPawn());
}


#undef LOCTEXT_NAMESPACE

