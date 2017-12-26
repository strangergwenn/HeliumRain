
#include "FlareSkirmishSetupMenu.h"
#include "../../Flare.h"

#include "../../Data/FlareCompanyCatalog.h"
#include "../../Data/FlareCustomizationCatalog.h"

#include "../../Game/FlareGame.h"
#include "../../Game/FlareSaveGame.h"
#include "../../Game/FlareSkirmishManager.h"

#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareSkirmishSetupMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSkirmishSetupMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	int32 Width = 1.25 * Theme.ContentWidth;
	UFlareSkirmishManager* SkirmishManager = MenuManager->GetGame()->GetSkirmishManager();

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
			SNew(STextBlock)
			.Text(LOCTEXT("SkirmishSetupTitle", "New skirmish"))
			.TextStyle(&Theme.SpecialTitleFont)
		]

		// Settings title
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Left)
		.Padding(Theme.ContentPadding)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SkirmishSettingsTitle", "Settings"))
			.TextStyle(&Theme.SubTitleFont)
		]
	
		// Opponent selector
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.AutoHeight()
		.Padding(Theme.ContentPadding)
		[
			SAssignNew(CompanySelector, SFlareDropList<FFlareCompanyDescription>)
			.OptionsSource(&MenuManager->GetPC()->GetGame()->GetCompanyCatalog()->Companies)
			.OnGenerateWidget(this, &SFlareSkirmishSetupMenu::OnGenerateCompanyComboLine)
			.HeaderWidth(8)
			.ItemWidth(8)
			[
				SNew(SBox)
				.Padding(Theme.ListContentPadding)
				[
					SNew(STextBlock)
					.Text(this, &SFlareSkirmishSetupMenu::OnGetCurrentCompanyComboLine)
					.TextStyle(&Theme.TextFont)
				]
			]
		]

		// Fleet title
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Left)
		.Padding(Theme.ContentPadding)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SkirmishFleetTitle", "Fleets"))
			.TextStyle(&Theme.SubTitleFont)
		]

		// Lists
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Fill)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			
			// Player fleet
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(Width)
				[
					SNew(SVerticalBox)
					
					// Add ship
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Left)
					.AutoHeight()
					[
						SNew(SFlareButton)
						.Transparent(true)
						.Width(8)
						.Text(LOCTEXT("AddPlayerShip", "Add player ship"))
						.OnClicked(this, &SFlareSkirmishSetupMenu::OnOrderShip, true)
					]

					+ SVerticalBox::Slot()
					[
						SNew(SScrollBox)
						.Style(&Theme.ScrollBoxStyle)
						.ScrollBarStyle(&Theme.ScrollBarStyle)

						+ SScrollBox::Slot()
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(PlayerSpacecraftList, SListView<TSharedPtr<FInterfaceContainer>>)
							.ListItemsSource(&PlayerSpacecraftListData)
							.SelectionMode(ESelectionMode::Single)
							.OnGenerateRow(this, &SFlareSkirmishSetupMenu::OnGenerateSpacecraftLine)
							.OnSelectionChanged(this, &SFlareSkirmishSetupMenu::OnPlayerSpacecraftSelectionChanged)
						]
					]
				]
			]

			// Enemy fleet
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(Width)
				[
					SNew(SVerticalBox)

					// Add ship
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Left)
					.AutoHeight()
					[
						SNew(SFlareButton)
						.Transparent(true)
						.Width(8)
						.Text(LOCTEXT("AddEnemyShip", "Add enemy ship"))
						.OnClicked(this, &SFlareSkirmishSetupMenu::OnOrderShip, false)
					]

					+ SVerticalBox::Slot()
					[
						SNew(SScrollBox)
						.Style(&Theme.ScrollBoxStyle)
						.ScrollBarStyle(&Theme.ScrollBarStyle)

						+ SScrollBox::Slot()
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(EnemySpacecraftList, SListView<TSharedPtr<FInterfaceContainer>>)
							.ListItemsSource(&EnemySpacecraftListData)
							.SelectionMode(ESelectionMode::Single)
							.OnGenerateRow(this, &SFlareSkirmishSetupMenu::OnGenerateSpacecraftLine)
							.OnSelectionChanged(this, &SFlareSkirmishSetupMenu::OnEnemySpacecraftSelectionChanged)
						]
					]
				]
			]

		]

		// Start
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(Theme.ContentPadding)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SFlareButton)
				.Transparent(true)
				.Width(4)
				.Text(LOCTEXT("Back", "Back"))
				.OnClicked(this, &SFlareSkirmishSetupMenu::OnMainMenu)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SFlareButton)
				.Transparent(true)
				.Width(4)
				.Icon(FFlareStyleSet::GetIcon("Load"))
				.Text(LOCTEXT("Start", "Start skirmish"))
				.HelpText(this, &SFlareSkirmishSetupMenu::GetStartHelpText)
				.IsDisabled(this, &SFlareSkirmishSetupMenu::IsStartDisabled)
				.OnClicked(this, &SFlareSkirmishSetupMenu::OnStartSkirmish)
			]
		]
	];

	// TODO 1075 : budget picker
	// TODO 1075 : spacecraft customization
	// TODO 1075 : sector settings
	// TODO 1075 : company customization

}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareSkirmishSetupMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareSkirmishSetupMenu::Enter()
{
	FLOG("SFlareSkirmishSetupMenu::Enter");
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	CompanySelector->RefreshOptions();

	// Start doing the setup
	MenuManager->GetGame()->GetSkirmishManager()->StartSetup();
	MenuManager->GetGame()->GetSkirmishManager()->SetAllowedValue(true, 100);
	MenuManager->GetGame()->GetSkirmishManager()->SetAllowedValue(false, 100);

	PlayerSpacecraftListData.Empty();
	EnemySpacecraftListData.Empty();
	PlayerSpacecraftList->RequestListRefresh();
	EnemySpacecraftList->RequestListRefresh();

}

void SFlareSkirmishSetupMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);

	PlayerSpacecraftListData.Empty();
	EnemySpacecraftListData.Empty();
	PlayerSpacecraftList->RequestListRefresh();
	EnemySpacecraftList->RequestListRefresh();
}


/*----------------------------------------------------
	Lists
----------------------------------------------------*/

TSharedRef<ITableRow> SFlareSkirmishSetupMenu::OnGenerateSpacecraftLine(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FFlareSpacecraftDescription* Desc = Item->SpacecraftDescriptionPtr;

	// Structure
	return SNew(SFlareListItem, OwnerTable)
	.Width(32)
	.Height(2)
	.Content()
	[
		SNew(SHorizontalBox)

		// Picture
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(Theme.ContentPadding)
		.VAlign(VAlign_Top)
		[
			SNew(SImage)
			.Image(&Desc->MeshPreviewBrush)
		]

		// Main infos
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(Theme.ContentPadding)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.SmallContentPadding)
			[
				SNew(STextBlock)
				.Text(Desc->Name)
				.TextStyle(&Theme.NameFont)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.SmallContentPadding)
			[
				SNew(SBox)
				.WidthOverride(Theme.ContentWidth)
				[
					SNew(STextBlock)
					.Text(Desc->Description)
					.TextStyle(&Theme.TextFont)
					.WrapTextAt(Theme.ContentWidth)
				]
			]
		]
	];
}

void SFlareSkirmishSetupMenu::OnPlayerSpacecraftSelectionChanged(TSharedPtr<FInterfaceContainer> Item, ESelectInfo::Type SelectInfo)
{
	FLOG("SFlareSkirmishSetupMenu::OnPlayerSpacecraftSelectionChanged");

	TSharedPtr<SFlareListItem> ItemWidget = StaticCastSharedPtr<SFlareListItem>(PlayerSpacecraftList->WidgetFromItem(Item));
	SelectedItem = Item;

	// Update selection
	if (PreviousSelection.IsValid())
	{
		PreviousSelection->SetSelected(false);
	}
	if (ItemWidget.IsValid())
	{
		ItemWidget->SetSelected(true);
		PreviousSelection = ItemWidget;
	}
}

void SFlareSkirmishSetupMenu::OnEnemySpacecraftSelectionChanged(TSharedPtr<FInterfaceContainer> Item, ESelectInfo::Type SelectInfo)
{
	FLOG("SFlareSkirmishSetupMenu::OnEnemySpacecraftSelectionChanged");

	TSharedPtr<SFlareListItem> ItemWidget = StaticCastSharedPtr<SFlareListItem>(EnemySpacecraftList->WidgetFromItem(Item));
	SelectedItem = Item;

	// Update selection
	if (PreviousSelection.IsValid())
	{
		PreviousSelection->SetSelected(false);
	}
	if (ItemWidget.IsValid())
	{
		ItemWidget->SetSelected(true);
		PreviousSelection = ItemWidget;
	}
}


/*----------------------------------------------------
	Content callbacks
----------------------------------------------------*/

TSharedRef<SWidget> SFlareSkirmishSetupMenu::OnGenerateCompanyComboLine(FFlareCompanyDescription Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(SBox)
	.Padding(Theme.ListContentPadding)
	[
		SNew(STextBlock)
		.Text(Item.Name)
		.TextStyle(&Theme.TextFont)
	];
}

FText SFlareSkirmishSetupMenu::OnGetCurrentCompanyComboLine() const
{
	const FFlareCompanyDescription Item = CompanySelector->GetSelectedItem();
	return Item.Name;
}

bool SFlareSkirmishSetupMenu::IsStartDisabled() const
{
	UFlareSkirmishManager* SkirmishManager = MenuManager->GetGame()->GetSkirmishManager();

	FText Unused;
	if (!SkirmishManager->CanStartPlaying(Unused))
	{
		return true;
	}

	return false;
}

FText SFlareSkirmishSetupMenu::GetStartHelpText() const
{
	UFlareSkirmishManager* SkirmishManager = MenuManager->GetGame()->GetSkirmishManager();

	FText Reason;
	if (!SkirmishManager->CanStartPlaying(Reason))
	{
		return Reason;
	}

	return LOCTEXT("StartHelpText", "Start the skirmish now");
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareSkirmishSetupMenu::OnOrderShip(bool ForPlayer)
{
	IsOrderingForPlayer = ForPlayer;

	FFlareMenuParameterData Data;
	Data.OrderForPlayer = ForPlayer;
	Data.Skirmish = MenuManager->GetGame()->GetSkirmishManager();
	MenuManager->OpenSpacecraftOrder(Data, FOrderDelegate::CreateSP(this, &SFlareSkirmishSetupMenu::OnOrderShipConfirmed));
}

void SFlareSkirmishSetupMenu::OnOrderShipConfirmed(FFlareSpacecraftDescription* Spacecraft)
{
	MenuManager->GetGame()->GetSkirmishManager()->AddShip(IsOrderingForPlayer, Spacecraft);

	if (IsOrderingForPlayer)
	{
		PlayerSpacecraftListData.AddUnique(FInterfaceContainer::New(Spacecraft));
		PlayerSpacecraftList->RequestListRefresh();
	}
	else
	{
		EnemySpacecraftListData.AddUnique(FInterfaceContainer::New(Spacecraft));
		EnemySpacecraftList->RequestListRefresh();
	}
}

void SFlareSkirmishSetupMenu::OnStartSkirmish()
{
	UFlareSkirmishManager* Skirmish = MenuManager->GetGame()->GetSkirmishManager();

	// Override defaults with current color settings
	FFlareCompanyDescription& PlayerCompanyData = Skirmish->GetData().PlayerCompanyData;
	const FFlareCompanyDescription* CurrentCompanyData = MenuManager->GetPC()->GetCompanyDescription();
	PlayerCompanyData.CustomizationBasePaintColor = CurrentCompanyData->CustomizationBasePaintColor;
	PlayerCompanyData.CustomizationPaintColor = CurrentCompanyData->CustomizationPaintColor;
	PlayerCompanyData.CustomizationOverlayColor = CurrentCompanyData->CustomizationOverlayColor;
	PlayerCompanyData.CustomizationLightColor = CurrentCompanyData->CustomizationLightColor;
	PlayerCompanyData.CustomizationPatternIndex = CurrentCompanyData->CustomizationPatternIndex;

	// Set enemy name
	Skirmish->GetData().EnemyCompanyName = CompanySelector->GetSelectedItem().ShortName;

	// Create the game
	Skirmish->StartPlay();
}

void SFlareSkirmishSetupMenu::OnMainMenu()
{
	MenuManager->GetGame()->GetSkirmishManager()->EndSkirmish();

	MenuManager->OpenMenu(EFlareMenu::MENU_Main);
}


#undef LOCTEXT_NAMESPACE

