
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

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TitleFont)
			.Text(LOCTEXT("MainMenu", "HELIUM RAIN"))
		]

		+ SVerticalBox::Slot()
		[
			SAssignNew(Temp, SHorizontalBox)
		]
	];

	// Add save slots
	for (int32 Index = 1; Index <= SaveSlotCount; Index++)
	{
		Temp->AddSlot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				[
					SNew(STextBlock)
					.TextStyle(&Theme.SubTitleFont)
					.Text(FText::FromString(LOCTEXT("Slot", "Save slot ").ToString() + FString::FromInt(Index)))
				]

				+ SVerticalBox::Slot()
				[
					SNew(SFlareButton)
					.Text(this, &SFlareMainMenu::GetText, Index)
					.OnClicked(this, &SFlareMainMenu::OnOpenSlot, TSharedPtr<int32>(new int32(Index)))
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
		return FText::FromString(SaveSlots[RealIndex]->PlayerData.CurrentShipName);
	}
	else
	{
		return LOCTEXT("NewGame", "Start a new game");
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


#undef LOCTEXT_NAMESPACE

