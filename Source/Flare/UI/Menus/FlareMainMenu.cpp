
#include "../../Flare.h"
#include "FlareMainMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareSaveGame.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlareMenuManager.h"
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
			SAssignNew(SaveBox, SHorizontalBox)
		]
	];

	// Add save slots
	for (int32 Index = 1; Index <= SaveSlotCount; Index++)
	{
		TSharedPtr<int32> IndexPtr(new int32(Index));

		SaveBox->AddSlot()
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

	SaveSlots.Empty();

	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FText SFlareMainMenu::GetText(int32 Index) const
{
	if (IsExistingGame(Index - 1))
	{
		const FFlareSaveSlotInfo& SaveSlotInfo = SaveSlots[Index - 1];

		FString CompanyString = SaveSlotInfo.CompanyName.ToString();
		FString ShipString = FString::FromInt(SaveSlotInfo.CompanyShipCount) + " " + LOCTEXT("Ships", "ships").ToString();
		FString MoneyString = FString::FromInt(SaveSlotInfo.CompanyMoney) + " " + LOCTEXT("Credits", "credits").ToString();

		return FText::FromString(CompanyString + "\n" + ShipString + "\n" + MoneyString + "\n");
	}
	else
	{
		return FText::FromString("\n\n\n");
	}
}

const FSlateBrush* SFlareMainMenu::GetSaveIcon(int32 Index) const
{
	return (IsExistingGame(Index - 1) ? &SaveSlots[Index - 1].EmblemBrush : FFlareStyleSet::GetIcon("HeliumRain"));
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
	// Setup
	SaveSlots.Empty();
	FVector2D EmblemSize = 128 * FVector2D::UnitVector;
	UMaterial* BaseEmblemMaterial = Cast<UMaterial>(FFlareStyleSet::GetIcon("CompanyEmblem")->GetResourceObject());

	// Get all saves
	for (int32 Index = 1; Index <= SaveSlotCount; Index++)
	{
		FFlareSaveSlotInfo SaveSlotInfo;
		SaveSlotInfo.EmblemBrush.ImageSize = EmblemSize;
		UFlareSaveGame* Save = AFlareGame::LoadSaveFile(Index);
		SaveSlotInfo.Save = Save;

		if (Save)
		{
			// Basic setup
			AFlareGame* Game = MenuManager->GetPC()->GetGame();
			FFlareCompanySave& Company = Save->CompanyData[0];
			UFlareCustomizationCatalog* Catalog = Game->GetCustomizationCatalog();
			SaveSlotInfo.CompanyName = LOCTEXT("Company", "Mining Syndicate");
			FLOGV("SFlareMainMenu::UpdateSaveSlots : found valid save data in slot %d", Index);

			// Count player ships
			SaveSlotInfo.CompanyShipCount = 0;
			for (int32 Index = 0; Index < Save->ShipData.Num(); Index++)
			{
				const FFlareSpacecraftSave& Spacecraft = Save->ShipData[Index];

				if (Spacecraft.CompanyIdentifier == Save->PlayerData.CompanyIdentifier)
				{
					SaveSlotInfo.CompanyShipCount++;
				}
			}

			// Money
			SaveSlotInfo.CompanyMoney = 50000;

			// Emblem material
			SaveSlotInfo.Emblem = UMaterialInstanceDynamic::Create(BaseEmblemMaterial, Game->GetWorld());
			SaveSlotInfo.Emblem->SetVectorParameterValue("BasePaintColor", Catalog->GetColor(Company.CustomizationBasePaintColorIndex));
			SaveSlotInfo.Emblem->SetVectorParameterValue("PaintColor", Catalog->GetColor(Company.CustomizationPaintColorIndex));
			SaveSlotInfo.Emblem->SetVectorParameterValue("OverlayColor", Catalog->GetColor(Company.CustomizationOverlayColorIndex));
			SaveSlotInfo.Emblem->SetVectorParameterValue("GlowColor", Catalog->GetColor(Company.CustomizationLightColorIndex));

			// Create the brush dynamically
			SaveSlotInfo.EmblemBrush.SetResourceObject(SaveSlotInfo.Emblem);
		}
		else
		{
			SaveSlotInfo.Save = NULL;
			SaveSlotInfo.Emblem = NULL;
			SaveSlotInfo.EmblemBrush = FSlateNoResource();
			SaveSlotInfo.CompanyShipCount = 0;
			SaveSlotInfo.CompanyMoney = 0;
			SaveSlotInfo.CompanyName = FText::FromString("");
		}

		SaveSlots.Add(SaveSlotInfo);
	}

	FLOG("SFlareMainMenu::UpdateSaveSlots : all slots found");
	Initialized = true;
}

bool SFlareMainMenu::IsExistingGame(int32 Index) const
{
	return Initialized && Index < SaveSlots.Num() && SaveSlots[Index].Save;
}


#undef LOCTEXT_NAMESPACE

