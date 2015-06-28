
#include "../../Flare.h"
#include "FlareMainMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareSaveGame.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareMainMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareMainMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	AFlarePlayerController* PC = MenuManager->GetPC();
	TSharedPtr<SHorizontalBox> Temp;
	SaveSlotCount = 5;

	// Alpha color
	FLinearColor AlphaColor = FLinearColor::White;
	AlphaColor.A = Theme.DefaultAlpha;

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(30))
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		[
			SNew(SHorizontalBox)

			// Icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(20))
			[
				SNew(SImage)
				.Image(FFlareStyleSet::Get().GetBrush("/Brushes/SB_Icon_HeliumRain"))
			]

			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TitleFont)
				.Text(LOCTEXT("MainMenu", "HELIUM RAIN"))
			]
		]

		// Save slots
		+ SVerticalBox::Slot()
		.Padding(FMargin(0, 50))
		[
			SAssignNew(Temp, SHorizontalBox)
		]

		// Actions
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.Padding(FMargin(20, 0))
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("Settings", "Settings"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Settings))
				.OnClicked(this, &SFlareMainMenu::OnOpenSettings)
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("Quit", "Quit game"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Quit))
				.OnClicked(this, &SFlareMainMenu::OnQuitGame)
			]
		]
	];

	// Add save slots
	for (int32 Index = 1; Index <= SaveSlotCount; Index++)
	{
		TSharedPtr<SFlareButton> TempButton;

		Temp->AddSlot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		[
			SNew(SBorder)
			.BorderImage(FFlareStyleSet::Get().GetBrush("/Brushes/SB_Black"))
			.BorderBackgroundColor(AlphaColor)
			[
				SNew(SVerticalBox)

				// Slot N°
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.TitlePadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.SubTitleFont)
					.Text(FText::FromString(FString::FromInt(Index)))
				]

				// Company emblem
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Center)
					[
						SNew(SImage)
						.Image(this, &SFlareMainMenu::GetSaveIcon, Index)
					]
				]

				// Description
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(this, &SFlareMainMenu::GetText, Index)
				]

				// Launch
				+ SVerticalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.AutoHeight()
				[
					SAssignNew(TempButton, SFlareButton)
					.OnClicked(this, &SFlareMainMenu::OnOpenSlot, TSharedPtr<int32>(new int32(Index)))
					.Width(6)
					.Height(1)
				]
			]
		];

		// Add button content
		TempButton->GetContainer()->SetContent(
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SImage)
				.Image(this, &SFlareMainMenu::GetButtonIcon, Index)
			]

			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TextFont)
				.Text(this, &SFlareMainMenu::GetButtonText, Index)
			]
		);
	}
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareMainMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}

void SFlareMainMenu::Enter()
{
	FLOG("SFlareMainMenu::Enter");

	// Look for existing saves
	for (int32 Index = 1; Index <= SaveSlotCount; Index++)
	{
		UFlareSaveGame* Save = AFlareGame::LoadSaveFile(Index);
		if (Save)
		{
			FLOGV("SFlareMainMenu::Enter : found valid save data in slot %d", Index);
			SaveSlots.Add(Save);
		}
		else
		{
			SaveSlots.Add(NULL);
		}
	}
	FLOG("SFlareMainMenu::Enter : all slots found");

	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
}

void SFlareMainMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FText SFlareMainMenu::GetText(int32 Index) const
{
	int32 RealIndex = Index - 1;

	if (RealIndex < SaveSlots.Num() && SaveSlots[RealIndex])
	{
		FFlarePlayerSave& PlayerData = SaveSlots[RealIndex]->PlayerData;

		FString CompanyString = LOCTEXT("Company", "Company : ").ToString() + PlayerData.CompanyIdentifier.ToString();
		FString ShipString = LOCTEXT("Flying", "Player ship : ").ToString() + PlayerData.CurrentShipName;

		return FText::FromString(CompanyString + "\n" + ShipString);
	}
	else
	{
		return FText::FromString("");
	}
}

const FSlateBrush* SFlareMainMenu::GetSaveIcon(int32 Index) const
{
	int32 RealIndex = Index - 1;

	if (RealIndex < SaveSlots.Num() && SaveSlots[RealIndex])
	{
		return FFlareStyleSet::GetIcon("Company");
	}
	else
	{
		return FFlareStyleSet::GetIcon("HeliumRain");
	}
}

FText SFlareMainMenu::GetButtonText(int32 Index) const
{
	int32 RealIndex = Index - 1;

	if (RealIndex < SaveSlots.Num() && SaveSlots[RealIndex])
	{
		return LOCTEXT("Load", "Load saved game");
	}
	else
	{
		return LOCTEXT("Create", "Start a new game");
	}
}

const FSlateBrush* SFlareMainMenu::GetButtonIcon(int32 Index) const
{
	int32 RealIndex = Index - 1;

	if (RealIndex < SaveSlots.Num() && SaveSlots[RealIndex])
	{
		return FFlareStyleSet::GetIcon("Load");
	}
	else
	{
		return FFlareStyleSet::GetIcon("New");
	}
}

void SFlareMainMenu::OnOpenSlot(TSharedPtr<int32> Index)
{
	AFlarePlayerController* PC = MenuManager->GetPC();
	AFlareGame* Game = MenuManager->GetGame();

	if (PC && Game)
	{
		// Load the world
		bool WorldLoaded = Game->LoadWorld(PC, *Index);
		if (!WorldLoaded)
		{
			Game->CreateWorld(PC);
		}

		// Go to the ship
		MenuManager->OpenMenu(EFlareMenu::MENU_FlyShip, PC->GetShipPawn());
	}
}

void SFlareMainMenu::OnOpenSettings()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Settings);
}

void SFlareMainMenu::OnQuitGame()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Quit);
}

#undef LOCTEXT_NAMESPACE

