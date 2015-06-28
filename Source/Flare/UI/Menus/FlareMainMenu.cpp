
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

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SFlareButton)
			.Text(this, &SFlareMainMenu::GetText, 0)
			.OnClicked(this, &SFlareMainMenu::OnOpenSlot, TSharedPtr<int32>(new int32(0)))
		]

		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SFlareButton)
			.Text(this, &SFlareMainMenu::GetText, 1)
			.OnClicked(this, &SFlareMainMenu::OnOpenSlot, TSharedPtr<int32>(new int32(1)))
		]

		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SFlareButton)
			.Text(this, &SFlareMainMenu::GetText, 2)
			.OnClicked(this, &SFlareMainMenu::OnOpenSlot, TSharedPtr<int32>(new int32(2)))
		]
	];

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

	int32 Index = 0;
	bool KeepLooking = true;

	// Look for existing saves
	while (KeepLooking)
	{
		FString SlotName = "SaveSlot" + FString::FromInt(Index);
		UFlareSaveGame* Save = AFlareGame::LoadSaveFile(SlotName);
		if (Save)
		{
			FLOGV("SFlareMainMenu::Enter : found valid save data in '%s'", *SlotName);
			SaveSlots.Add(Save);
			Index++;
		}
		else
		{
			KeepLooking = false;
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
	if (Index < SaveSlots.Num() && SaveSlots[Index])
	{
		return FText::FromString(SaveSlots[Index]->PlayerData.CurrentShipName);
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
		bool WorldLoaded = Game->LoadWorld(PC, "SaveSlot" + FString::FromInt(*Index));
		if (!WorldLoaded)
		{
			Game->CreateWorld(PC);
		}

		// Go to the ship
		MenuManager->OpenMenu(EFlareMenu::MENU_FlyShip, PC->GetShipPawn());
	}

}


#undef LOCTEXT_NAMESPACE

