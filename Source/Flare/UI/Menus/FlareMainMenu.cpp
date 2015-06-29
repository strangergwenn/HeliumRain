
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
	SaveSlotCount = 3;
	Initialized = false;

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
				SNew(SImage)
				.Image(FFlareStyleSet::GetIcon("HeliumRain"))
			]

			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TitleFont)
				.Text(LOCTEXT("MainMenu", "HELIUM RAIN"))
			]

			// Settings
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("Settings", "Settings"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Settings, true))
				.OnClicked(this, &SFlareMainMenu::OnOpenSettings)
			]

			// Quit
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("Quit", "Quit game"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Quit, true))
				.OnClicked(this, &SFlareMainMenu::OnQuitGame)
			]
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(200, 40))
		[
			SNew(SImage).Image(&Theme.SeparatorBrush)
		]

		// Save slots
		+ SVerticalBox::Slot()
		[
			SAssignNew(Temp, SHorizontalBox)
		]
	];

	// Add save slots
	for (int32 Index = 1; Index <= SaveSlotCount; Index++)
	{
		TSharedPtr<int32> IndexPtr(new int32(Index));

		Temp->AddSlot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		[
			SNew(SVerticalBox)

			// Slot N°
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.TitlePadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TitleFont)
				.Text(FText::FromString(FString::FromInt(Index) + "/"))
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
			.Padding(Theme.SmallContentPadding)
			[
				SNew(SFlareButton)
				.Text(this, &SFlareMainMenu::GetButtonText, Index)
				.Icon(this, &SFlareMainMenu::GetButtonIcon, Index)
				.OnClicked(this, &SFlareMainMenu::OnOpenSlot, IndexPtr)
				.Width(5)
				.Height(1)
			]

			// Delete
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.AutoHeight()
			.Padding(Theme.SmallContentPadding)
			[
				SNew(SFlareButton)
				.Text(LOCTEXT("Delete", "Delete game"))
				.Icon(FFlareStyleSet::GetIcon("Delete"))
				.OnClicked(this, &SFlareMainMenu::OnDeleteSlot, IndexPtr)
				.Width(5)
				.Height(1)
				.Visibility(this, &SFlareMainMenu::GetDeleteButtonVisibility, Index)
			]
		];
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
	UpdateSaveSlots();
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
}

void SFlareMainMenu::Exit()
{
	SetEnabled(false);
	Initialized = false;
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FText SFlareMainMenu::GetText(int32 Index) const
{
	if (IsExistingGame(Index - 1))
	{
		FFlarePlayerSave& PlayerData = SaveSlots[Index - 1]->PlayerData;

		FString CompanyString = LOCTEXT("Company", "Company : ").ToString() + PlayerData.CompanyIdentifier.ToString();
		FString ShipString = LOCTEXT("Flying", "Player ship : ").ToString() + PlayerData.CurrentShipName;

		return FText::FromString(CompanyString + "\n" + ShipString + "\n");
	}
	else
	{
		return FText::FromString("\n\n");
	}
}

const FSlateBrush* SFlareMainMenu::GetSaveIcon(int32 Index) const
{
	return (IsExistingGame(Index - 1) ? FFlareStyleSet::GetIcon("Company") : FFlareStyleSet::GetIcon("HeliumRain"));
}

EVisibility SFlareMainMenu::GetDeleteButtonVisibility(int32 Index) const
{
	return (IsExistingGame(Index - 1) ? EVisibility::Visible : EVisibility::Collapsed);
}

FText SFlareMainMenu::GetButtonText(int32 Index) const
{
	return (IsExistingGame(Index - 1) ? LOCTEXT("Load", "Load game") : LOCTEXT("Create", "New game"));
}

const FSlateBrush* SFlareMainMenu::GetButtonIcon(int32 Index) const
{
	return (IsExistingGame(Index - 1) ? FFlareStyleSet::GetIcon("Load") : FFlareStyleSet::GetIcon("New"));
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

void SFlareMainMenu::OnDeleteSlot(TSharedPtr<int32> Index)
{
	AFlareGame::DeleteSaveFile(*Index);
	UpdateSaveSlots();
}

void SFlareMainMenu::OnOpenSettings()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Settings);
}

void SFlareMainMenu::OnQuitGame()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Quit);
}


/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

void SFlareMainMenu::UpdateSaveSlots()
{
	SaveSlots.Empty();

	for (int32 Index = 1; Index <= SaveSlotCount; Index++)
	{
		UFlareSaveGame* Save = AFlareGame::LoadSaveFile(Index);
		if (Save)
		{
			FLOGV("SFlareMainMenu::UpdateSaveSlots : found valid save data in slot %d", Index);
			SaveSlots.Add(Save);
		}
		else
		{
			SaveSlots.Add(NULL);
		}
	}

	FLOG("SFlareMainMenu::UpdateSaveSlots : all slots found");
	Initialized = true;
}

bool SFlareMainMenu::IsExistingGame(int32 Index) const
{
	return Initialized && Index < SaveSlots.Num() && SaveSlots[Index];
}


#undef LOCTEXT_NAMESPACE

