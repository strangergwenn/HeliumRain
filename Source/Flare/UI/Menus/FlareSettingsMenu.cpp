
#include "../../Flare.h"
#include "FlareSettingsMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareSaveGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "STextComboBox.h"


#define LOCTEXT_NAMESPACE "FlareSettingsMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSettingsMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	Game = MenuManager->GetPC()->GetGame();

	// Game starts
	ResolutionList.Add(MakeShareable(new FString(TEXT("1920x1080"))));

	// Color
	FLinearColor Color = Theme.NeutralColor;
	Color.A = Theme.DefaultAlpha;

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.Padding(Theme.ContentPadding)
		[
			SNew(SHorizontalBox)

			// Icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SImage).Image(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Main))
			]

			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TitleFont)
				.Text(LOCTEXT("Settings", "SETTINGS"))
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
				.HelpText(LOCTEXT("BakcInfo", "Go back to the main menu"))
				.Icon(FFlareStyleSet::GetIcon("Ok"))
				.OnClicked(this, &SFlareSettingsMenu::OnExit)
			]
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(200, 20))
		[
			SNew(SImage).Image(&Theme.SeparatorBrush)
		]

		// Info
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.ContentPadding)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("GraphicsSettingsHint", "Graphics."))
			.TextStyle(&Theme.SubTitleFont)
		]

		// Main form
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.ContentPadding)
		.HAlign(HAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(Theme.ContentWidth / 2)
			.HAlign(HAlign_Fill)
			[
				SNew(SVerticalBox)

				// Resolution
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SAssignNew(ResolutionSelector, SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&ResolutionList)
					.InitiallySelectedItem(ResolutionList[0])
					.OnGenerateWidget(this, &SFlareSettingsMenu::OnGenerateComboLine)
					.OnSelectionChanged(this, &SFlareSettingsMenu::OnComboLineSelectionChanged)
					.ComboBoxStyle(&Theme.ComboBoxStyle)
					.ForegroundColor(FLinearColor::White)
					[
						SNew(STextBlock)
						.Text(this, &SFlareSettingsMenu::OnGetCurrentComboLine)
						.TextStyle(&Theme.TextFont)
					]
				]

				// Fullscreen
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Right)
				[
					SAssignNew(FullscreenButton, SFlareButton)
					.Text(LOCTEXT("Fullscreen", "Fullscreen"))
					.HelpText(LOCTEXT("FullscreenInfo", "Enable fullscreen"))
					.Toggle(true)
				]

			// Fullscreen
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.ContentPadding)
			.HAlign(HAlign_Right)
			[
				SAssignNew(VSyncButton, SFlareButton)
				.Text(LOCTEXT("V-sync", "V-sync"))
				.HelpText(LOCTEXT("VSyncInfo", "Enable v-sync"))
				.Toggle(true)
			]
			]
		]
	];

}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareSettingsMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}

void SFlareSettingsMenu::Enter()
{
	FLOG("SFlareSettingsMenu::Enter");
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		FFlarePlayerSave Data;
		FFlareCompanyDescription Unused;
		PC->Save(Data, Unused);
		//ColorBox->Setup(Data);
	}
}

void SFlareSettingsMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/


FText SFlareSettingsMenu::OnGetCurrentComboLine() const
{
	//TSharedPtr<FString> Item = ScenarioSelector->GetSelectedItem();
	//return Item.IsValid() ? FText::FromString(*Item) : FText::FromString(*ResolutionList[0]);
	return FText::FromString("1920x1080");
}

TSharedRef<SWidget> SFlareSettingsMenu::OnGenerateComboLine(TSharedPtr<FString> Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(STextBlock)
	.Text(FText::FromString(*Item))
	.TextStyle(&Theme.TextFont);
}

void SFlareSettingsMenu::OnComboLineSelectionChanged(TSharedPtr<FString> StringItem, ESelectInfo::Type SelectInfo)
{
}

void SFlareSettingsMenu::OnExit()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Main);
}




#undef LOCTEXT_NAMESPACE

