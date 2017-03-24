
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
	FSlateFontInfo TitleFont(FPaths::GameContentDir() / TEXT("Slate/Fonts/Lato700.ttf"), 36);
	FSlateFontInfo MainFont(FPaths::GameContentDir() / TEXT("Slate/Fonts/Lato700.ttf"), 30);
	FSlateFontInfo SecondaryFont(FPaths::GameContentDir() / TEXT("Slate/Fonts/Lato700.ttf"), 16);

	// Settings
	TextShowTime = 15.0f;
	TextHideTime = 1.0f;
	TransitionTime = 0.5f;
	int32 Width = 1.5 * Theme.ContentWidth;
	int32 TextWidth = Width - Theme.ContentPadding.Left - Theme.ContentPadding.Right;
	
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
				.Font(TitleFont)
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
				.Font(MainFont)
				.ColorAndOpacity(this, &SFlareStoryMenu::GetTextColor)
			]

			// Image
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			.AutoHeight()
			[
				SAssignNew(Image, SImage)
				.ColorAndOpacity(this, &SFlareStoryMenu::GetTextColor)
			]

			// Subtext
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			.AutoHeight()
			[
				SAssignNew(SubText, STextBlock)
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
			.Width(2)
			.Text(LOCTEXT("Skip", "Skip"))
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

	TitleList.Add(LOCTEXT("Story1Title", "2092"));
	TextList.Add(LOCTEXT("Story1", "The Hypatia Space Telescope identifies a gas giant around the star \u03B2 Hydri"));
	SubTextList.Add(LOCTEXT("SubStory1", "The new planet, Nema, would only be one of many exoplanets in the universe, if not for moons deemed capable of supporting life.\n24 light-years away from Earth, it is close enough to be in human reach.\n"));
	ImageList.Add(FFlareStyleSet::GetImage("Story_Discovery"));

	TitleList.Add(LOCTEXT("Story2Title", "2107"));
	TextList.Add(LOCTEXT("Story2", "Interstellar colonial carrier Daedalus leaves Earth orbit"));
	SubTextList.Add(LOCTEXT("SubStory2", "Packed with a crew of 9,000, mining gear and scientific equipment to investigate the Hydri system, it will reach 20% of the speed of light.\n"));
	ImageList.Add(FFlareStyleSet::GetImage("Story_Departure"));

	TitleList.Add(LOCTEXT("Story3Title", "2230"));
	TextList.Add(LOCTEXT("Story3", "ICC Daedalus enters the orbit of Nema"));
	SubTextList.Add(LOCTEXT("SubStory3", "Its moons are barren, desolated wastelands, unable to support life. \nPumping stations are built around Nema to extract the valuable gases whithin, asteroids are broken up for materials and colonists establish outposts around the moons.\n"));
	ImageList.Add(FFlareStyleSet::GetImage("Story_Nema"));

	TitleList.Add(LOCTEXT("Story4Title", "2249"));
	TextList.Add(LOCTEXT("Story4", "Some colonists attempt a return to Earth"));
	SubTextList.Add(LOCTEXT("SubStory4", "A clash between colonists pushes some of them to attempt to fly Daedalus back to Earth, only to meet opposition from the others. The carrier is destroyed in the fight.\n"));
	ImageList.Add(FFlareStyleSet::GetImage("Story_Return"));

	TitleList.Add(LOCTEXT("Story5Title", "2250"));
	TextList.Add(LOCTEXT("Story5", "Life goes on"));
	SubTextList.Add(LOCTEXT("SubStory5", "As the colonial government has broken down, the mining and exploration companies remain. With no hope of return to the known world, life goes on, in relative peace...\n"));
	ImageList.Add(FFlareStyleSet::GetImage("Story_Peace"));
}

void SFlareStoryMenu::Enter()
{
	FLOG("SFlareStoryMenu::Enter");

	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	CurrentTime = -1.0;
	CurrentAlpha = 0;
	CurrentIndex = 0;
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
	CurrentTime += InDeltaTime;
	
	// Fading in
	if (CurrentTime <= TransitionTime)
	{
		CurrentAlpha = CurrentTime / TransitionTime;
	}

	// Shown
	else if (CurrentTime - TransitionTime <= TextShowTime)
	{
		CurrentAlpha = 1;
	}

	// Fading out
	else if (CurrentTime - TransitionTime - TextShowTime <= TransitionTime)
	{
		CurrentAlpha = 1 - (CurrentTime - TransitionTime - TextShowTime) / TransitionTime;
	}

	// Not shown
	else if (CurrentTime - TransitionTime - TextShowTime - TransitionTime <= TextHideTime)
	{
		CurrentAlpha = 0;
	}

	// Text switch : loop with the next text, or exit
	else
	{
		CurrentAlpha = 0;

		if (CurrentIndex + 1 == TextList.Num())
		{
			OnStartPlaying();
		}
		else
		{
			CurrentTime = 0;
			CurrentIndex++;
		}
	}

	// Update states
	Title->SetText(TitleList[CurrentIndex]);
	Text->SetText(TextList[CurrentIndex]);
	SubText->SetText(SubTextList[CurrentIndex]);
	Image->SetImage(ImageList[CurrentIndex]);
}

FSlateColor SFlareStoryMenu::GetTextColor() const
{
	return FLinearColor(1, 1, 1, CurrentAlpha);
}

void SFlareStoryMenu::OnStartPlaying()
{
	AFlarePlayerController* PC = MenuManager->GetPC();
	UFlareSimulatedSpacecraft* CurrentShip = PC->GetPlayerShip();
	if (CurrentShip)
	{
		UFlareSimulatedSector* Sector = CurrentShip->GetCurrentSector();
		PC->GetGame()->ActivateCurrentSector();

		FCHECK(PC->GetPlayerShip()->GetActive());

		FFlareMenuParameterData Data;
		Data.Spacecraft = PC->GetPlayerShip();
		MenuManager->OpenMenu(EFlareMenu::MENU_FlyShip, Data);
	}
}


#undef LOCTEXT_NAMESPACE

