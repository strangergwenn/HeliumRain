
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
	int32 LabelWidth = Theme.ContentWidth / 4;

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SOverlay)

		// Main
		+ SOverlay::Slot()
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
			.Padding(Theme.TitlePadding)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SkirmishSettingsTitle", "Settings"))
				.TextStyle(&Theme.SubTitleFont)
			]
	
			// Opponent selector
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Left)
			.AutoHeight()
			.Padding(Theme.ContentPadding)
			[
				SNew(SHorizontalBox)

				// Text
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(LabelWidth)
					.Padding(FMargin(0, 10, 0, 0))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("EnemyLabel", "Enemy company"))
						.TextStyle(&Theme.TextFont)
					]
				]

				// List
				+ SHorizontalBox::Slot()
				.AutoWidth()
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

						// Title
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Left)
						.Padding(Theme.TitlePadding)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("PlayerFleetTitle", "Player fleet"))
							.TextStyle(&Theme.SubTitleFont)
						]
					
						// Add ship
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Left)
						.AutoHeight()
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SFlareButton)
								.Transparent(true)
								.Width(6)
								.Text(LOCTEXT("AddPlayerShip", "Add player ship"))
								.OnClicked(this, &SFlareSkirmishSetupMenu::OnOrderShip, true)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SFlareButton)
								.Transparent(true)
								.Width(6)
								.Text(LOCTEXT("ClearPlayerFleet", "Clear fleet"))
								.OnClicked(this, &SFlareSkirmishSetupMenu::OnClearFleet, true)
							]
						]

						+ SVerticalBox::Slot()
						[
							SNew(SScrollBox)
							.Style(&Theme.ScrollBoxStyle)
							.ScrollBarStyle(&Theme.ScrollBarStyle)

							+ SScrollBox::Slot()
							.Padding(Theme.ContentPadding)
							[
								SAssignNew(PlayerSpacecraftList, SListView<TSharedPtr<FFlareSkirmishSpacecraftOrder>>)
								.ListItemsSource(&PlayerSpacecraftListData)
								.SelectionMode(ESelectionMode::None)
								.OnGenerateRow(this, &SFlareSkirmishSetupMenu::OnGenerateSpacecraftLine)
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

						// Title
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Left)
						.Padding(Theme.TitlePadding)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("EnemyFleetTitle", "Enemy fleet"))
							.TextStyle(&Theme.SubTitleFont)
						]

						// Add ship
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Left)
						.AutoHeight()
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SFlareButton)
								.Transparent(true)
								.Width(6)
								.Text(LOCTEXT("AddEnemyShip", "Add enemy ship"))
								.OnClicked(this, &SFlareSkirmishSetupMenu::OnOrderShip, false)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SFlareButton)
								.Transparent(true)
								.Width(6)
								.Text(LOCTEXT("ClearEnemyFleet", "Clear fleet"))
								.OnClicked(this, &SFlareSkirmishSetupMenu::OnClearFleet, false)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SFlareButton)
								.Transparent(true)
								.Width(6)
								.Text(LOCTEXT("AutomaticFleet", "Automatic fleet"))
								//.OnClicked(this, &SFlareSkirmishSetupMenu::OnAutoCreateEnemyFleet, false)
								// TODO 1075
							]
						]

						+ SVerticalBox::Slot()
						[
							SNew(SScrollBox)
							.Style(&Theme.ScrollBoxStyle)
							.ScrollBarStyle(&Theme.ScrollBarStyle)

							+ SScrollBox::Slot()
							.Padding(Theme.ContentPadding)
							[
								SAssignNew(EnemySpacecraftList, SListView<TSharedPtr<FFlareSkirmishSpacecraftOrder>>)
								.ListItemsSource(&EnemySpacecraftListData)
								.SelectionMode(ESelectionMode::None)
								.OnGenerateRow(this, &SFlareSkirmishSetupMenu::OnGenerateSpacecraftLine)
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
		]

		// Customization overlay
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SAssignNew(UpgradeBox, SBackgroundBlur)
			.BlurRadius(Theme.BlurRadius)
			.BlurStrength(Theme.BlurStrength)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Visibility(EVisibility::Collapsed)
			[		
				SNew(SBorder)
				.BorderImage(&Theme.BackgroundBrush)
				[	
					SNew(SVerticalBox)

					// Top line
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.TitlePadding)
					[
						SNew(SHorizontalBox)
						
						// Icon
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SImage)
							.Image(FFlareStyleSet::GetIcon("ShipUpgradeMedium"))
						]

						// Title
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("ShipUpgradeTitle", "Upgrade spacecraft"))
							.TextStyle(&Theme.TitleFont)
						]

						// Close button
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Right)
						.AutoWidth()
						[
							SNew(SFlareButton)
							.Text(FText())
							.Icon(FFlareStyleSet::GetIcon("Delete"))
							.OnClicked(this, &SFlareSkirmishSetupMenu::OnCloseUpgradePanel)
							.Width(1)
						]
					]

					// Upgrades
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.TitlePadding)
					[
						SNew(SHorizontalBox)

						// Engine
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SBox)
							.WidthOverride(0.3 * Theme.ContentWidth)
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(Theme.TitlePadding)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("EngineUpgradeTitle", "Orbital engine"))
									.TextStyle(&Theme.SubTitleFont)
								]

								+ SVerticalBox::Slot()
								[
									SAssignNew(OrbitalEngineBox, SVerticalBox)
								]
							]
						]

						// RCS
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SBox)
							.WidthOverride(0.3 * Theme.ContentWidth)
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(Theme.TitlePadding)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("RCSUpgradeTitle", "RCS"))
									.TextStyle(&Theme.SubTitleFont)
								]

								+ SVerticalBox::Slot()
								[
									SAssignNew(RCSBox, SVerticalBox)
								]
							]
						]

						// Weapons
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SBox)
							.WidthOverride(Theme.ContentWidth)
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(Theme.TitlePadding)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("WeaponUpgradeTitle", "Weapons"))
									.TextStyle(&Theme.SubTitleFont)
									
								]

								+ SVerticalBox::Slot()
								[
									SAssignNew(WeaponBox, SHorizontalBox)
								]
							]
						]
					]
				]
			]
		]
	];
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

	// Empty lists
	PlayerSpacecraftListData.Empty();
	EnemySpacecraftListData.Empty();
	PlayerSpacecraftList->RequestListRefresh();
	EnemySpacecraftList->RequestListRefresh();

	// TODO 1075 : fill customization options
	// TODO 1075 : handle customization settings
}

void SFlareSkirmishSetupMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);

	// Empty lists
	PlayerSpacecraftListData.Empty();
	EnemySpacecraftListData.Empty();
	PlayerSpacecraftList->RequestListRefresh();
	EnemySpacecraftList->RequestListRefresh();
}



/*----------------------------------------------------
	Content callbacks
----------------------------------------------------*/

TSharedRef<ITableRow> SFlareSkirmishSetupMenu::OnGenerateSpacecraftLine(TSharedPtr<FFlareSkirmishSpacecraftOrder> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FFlareSpacecraftDescription* Desc = Item->Description;

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
		
		// Upgrade button
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(Theme.ContentPadding)
		[
			SNew(SFlareButton)
			.Text(FText())
			.Icon(FFlareStyleSet::GetIcon("ShipUpgradeSmall"))
			.OnClicked(this, &SFlareSkirmishSetupMenu::OnUpgradeSpacecraft, Item)
			.Width(1)
		]
	];
}

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
	if (!CanStartPlaying(Unused))
	{
		return true;
	}

	return false;
}

FText SFlareSkirmishSetupMenu::GetStartHelpText() const
{
	UFlareSkirmishManager* SkirmishManager = MenuManager->GetGame()->GetSkirmishManager();

	FText Reason;
	if (!CanStartPlaying(Reason))
	{
		return Reason;
	}

	return LOCTEXT("StartHelpText", "Start the skirmish now");
}

bool SFlareSkirmishSetupMenu::CanStartPlaying(FText& Reason) const
{
	Reason = FText();

	if (PlayerSpacecraftListData.Num() == 0)
	{
		Reason = LOCTEXT("SkirmishCantStartNoPlayer", "You don't have enough ships to start playing");
		return false;
	}
	else if (EnemySpacecraftListData.Num() == 0)
	{
		Reason = LOCTEXT("SkirmishCantStartNoEnemy", "Your enemy doesn't have enough ships to start playing");
		return false;
	}

	return true;
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
	auto Order = FFlareSkirmishSpacecraftOrder::New(Spacecraft);

	if (IsOrderingForPlayer)
	{
		PlayerSpacecraftListData.AddUnique(Order);
		PlayerSpacecraftList->RequestListRefresh();
	}
	else
	{
		EnemySpacecraftListData.AddUnique(Order);
		EnemySpacecraftList->RequestListRefresh();
	}
}

void SFlareSkirmishSetupMenu::OnUpgradeSpacecraft(TSharedPtr<FFlareSkirmishSpacecraftOrder> Order)
{
	// Fill options

	UpgradeBox->SetVisibility(EVisibility::Visible);
}

void SFlareSkirmishSetupMenu::OnCloseUpgradePanel()
{
	UpgradeBox->SetVisibility(EVisibility::Collapsed);
}

void SFlareSkirmishSetupMenu::OnStartSkirmish()
{
	UFlareSkirmishManager* Skirmish = MenuManager->GetGame()->GetSkirmishManager();

	// Add ships
	for (auto Order : PlayerSpacecraftListData)
	{
		Skirmish->AddShip(true, *Order.Get());
	}
	for (auto Order : EnemySpacecraftListData)
	{
		Skirmish->AddShip(false, *Order.Get());
	}

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

