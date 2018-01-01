
#include "FlareSkirmishSetupMenu.h"
#include "../../Flare.h"

#include "../../Data/FlareCompanyCatalog.h"
#include "../../Data/FlareSpacecraftCatalog.h"
#include "../../Data/FlareSpacecraftComponentsCatalog.h"
#include "../../Data/FlareCustomizationCatalog.h"
#include "../../Data/FlareOrbitalMap.h"

#include "../../Game/FlareGame.h"
#include "../../Game/FlareSaveGame.h"
#include "../../Game/FlareSkirmishManager.h"
#include "../../Game/FlareGameUserSettings.h"

#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareSkirmishSetupMenu"

#define MAX_ASTEROIDS           50
#define MAX_DEBRIS_PERCENTAGE   25


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSkirmishSetupMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	int32 Width = 0.9 * Theme.ContentWidth;
	int32 LabelWidth = Theme.ContentWidth / 5;
	int32 ListHeight = 880;

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
	
			// Content
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// Sector settings
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
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
							.Text(LOCTEXT("SkirmishSectorSettingssTitle", "Settings"))
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
								.Padding(FMargin(0, 20, 0, 0))
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
	
						// Planet selector
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
								.Padding(FMargin(0, 20, 0, 0))
								[
									SNew(STextBlock)
									.Text(LOCTEXT("PlanetLabel", "Planetary body"))
									.TextStyle(&Theme.TextFont)
								]
							]

							// List
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.ContentPadding)
							[
								SAssignNew(PlanetSelector, SFlareDropList<FFlareSectorCelestialBodyDescription>)
								.OptionsSource(&MenuManager->GetPC()->GetGame()->GetOrbitalBodies()->OrbitalBodies)
								.OnGenerateWidget(this, &SFlareSkirmishSetupMenu::OnGeneratePlanetComboLine)
								.HeaderWidth(8)
								.ItemWidth(8)
								[
									SNew(SBox)
									.Padding(Theme.ListContentPadding)
									[
										SNew(STextBlock)
										.Text(this, &SFlareSkirmishSetupMenu::OnGetCurrentPlanetComboLine)
										.TextStyle(&Theme.TextFont)
									]
								]
							]
						]

						// Altitude
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Left)
						.AutoHeight()
						[
							SNew(SBox)
							.WidthOverride(Theme.ContentWidth)
							[
								SNew(SHorizontalBox)

								// Text
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(Theme.ContentPadding)
								[
									SNew(SBox)
									.WidthOverride(LabelWidth)
									[
										SNew(STextBlock)
										.Text(LOCTEXT("AltitudeLabel", "Altitude"))
										.TextStyle(&Theme.TextFont)
									]
								]

								// Slider
								+ SHorizontalBox::Slot()
								.VAlign(VAlign_Center)
								.Padding(Theme.ContentPadding)
								[
									SAssignNew(AltitudeSlider, SSlider)
									.Value(0)
									.Style(&Theme.SliderStyle)
								]

								// Label
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(Theme.ContentPadding)
								[
									SNew(SBox)
									.WidthOverride(LabelWidth)
									[
										SNew(STextBlock)
										.TextStyle(&Theme.TextFont)
										.Text(this, &SFlareSkirmishSetupMenu::GetAltitudeValue)
									]
								]
							]
						]

						// Asteroids
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Left)
						.AutoHeight()
						[
							SNew(SBox)
							.WidthOverride(Theme.ContentWidth)
							[
								SNew(SHorizontalBox)

								// Text
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(Theme.ContentPadding)
								[
									SNew(SBox)
									.WidthOverride(LabelWidth)
									[
										SNew(STextBlock)
										.Text(LOCTEXT("AsteroidLabel", "Asteroids"))
										.TextStyle(&Theme.TextFont)
									]
								]

								// Slider
								+ SHorizontalBox::Slot()
								.VAlign(VAlign_Center)
								.Padding(Theme.ContentPadding)
								[
									SAssignNew(AsteroidSlider, SSlider)
									.Value(0)
									.Style(&Theme.SliderStyle)
									.OnValueChanged(this, &SFlareSkirmishSetupMenu::OnAsteroidSliderChanged, true)
								]

								// Label
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(Theme.ContentPadding)
								[
									SNew(SBox)
									.WidthOverride(LabelWidth)
									[
										SNew(STextBlock)
										.TextStyle(&Theme.TextFont)
										.Text(this, &SFlareSkirmishSetupMenu::GetAsteroidValue)
									]
								]
							]
						]

						// Debris
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Left)
						.AutoHeight()
						[
							SNew(SBox)
							.WidthOverride(Theme.ContentWidth)
							[
								SNew(SHorizontalBox)

								// Text
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(Theme.ContentPadding)
								[
									SNew(SBox)
									.WidthOverride(LabelWidth)
									[
										SNew(STextBlock)
										.Text(LOCTEXT("DebrisLabel", "Debris density"))
										.TextStyle(&Theme.TextFont)
									]
								]

								// Slider
								+ SHorizontalBox::Slot()
								.VAlign(VAlign_Center)
								.Padding(Theme.ContentPadding)
								[
									SAssignNew(DebrisSlider, SSlider)
									.Value(0)
									.Style(&Theme.SliderStyle)
									.OnValueChanged(this, &SFlareSkirmishSetupMenu::OnDebrisSliderChanged, true)
								]

								// Label
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(Theme.ContentPadding)
								[
									SNew(SBox)
									.WidthOverride(LabelWidth)
									[
										SNew(STextBlock)
										.TextStyle(&Theme.TextFont)
										.Text(this, &SFlareSkirmishSetupMenu::GetDebrisValue)
									]
								]
							]
						]

						// Icy
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Left)
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(SHorizontalBox)

							// Icy
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.SmallContentPadding)
							[
								SAssignNew(IcyButton, SFlareButton)
								.Text(LOCTEXT("Icy", "Icy sector"))
								.HelpText(LOCTEXT("IcyInfo", "This sector will encompass an icy cloud of water particles"))
								.Toggle(true)
							]

							// Debris
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.SmallContentPadding)
							[
								SAssignNew(MetalDebrisButton, SFlareButton)
								.Text(LOCTEXT("MetallicDebris", "Battle debris"))
								.HelpText(LOCTEXT("MetallicDebrisInfo", "This sector will feature remains of a battle, instead of asteroid fragments"))
								.Toggle(true)
							]
						]
					]
				]
			
				// Player fleet
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
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
							.Text(this, &SFlareSkirmishSetupMenu::GetPlayerFleetTitle)
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
								.Width(4)
								.Text(LOCTEXT("AddPlayerShip", "Add ship"))
								.HelpText(this, &SFlareSkirmishSetupMenu::GetAddToPlayerFleetText)
								.IsDisabled(this, &SFlareSkirmishSetupMenu::IsAddToPlayerFleetDisabled)
								.OnClicked(this, &SFlareSkirmishSetupMenu::OnOrderShip, true)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SFlareButton)
								.Transparent(true)
								.Width(4)
								.Text(LOCTEXT("ClearPlayerFleet", "Clear fleet"))
								.OnClicked(this, &SFlareSkirmishSetupMenu::OnClearFleet, true)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SFlareButton)
								.Transparent(true)
								.Width(4)
								.Text(LOCTEXT("SortFleet", "Sort fleet"))
								.OnClicked(this, &SFlareSkirmishSetupMenu::OnSortPlayerFleet)
							]
						]

						+ SVerticalBox::Slot()
						[
							SNew(SBox)
							.HeightOverride(ListHeight)
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
				]

				// Enemy fleet
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
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
							.Text(this, &SFlareSkirmishSetupMenu::GetEnemyFleetTitle)
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
								.Width(4)
								.Text(LOCTEXT("AddEnemyShip", "Add ship"))
								.HelpText(this, &SFlareSkirmishSetupMenu::GetAddToEnemyFleetText)
								.IsDisabled(this, &SFlareSkirmishSetupMenu::IsAddToEnemyFleetDisabled)
								.OnClicked(this, &SFlareSkirmishSetupMenu::OnOrderShip, false)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SFlareButton)
								.Transparent(true)
								.Width(4)
								.Text(LOCTEXT("ClearEnemyFleet", "Clear fleet"))
								.OnClicked(this, &SFlareSkirmishSetupMenu::OnClearFleet, false)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SFlareButton)
								.Transparent(true)
								.Width(4)
								.Text(LOCTEXT("AutomaticFleet", "Automatic fleet"))
								.OnClicked(this, &SFlareSkirmishSetupMenu::OnAutoCreateEnemyFleet)
							]
						]

						+ SVerticalBox::Slot()
						[
							SNew(SBox)
							.HeightOverride(ListHeight)
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
							.WidthOverride(0.25 * Theme.ContentWidth)
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(Theme.TitlePadding)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("EngineUpgradeTitle", "Orbital engines"))
									.TextStyle(&Theme.SubTitleFont)
								]

								+ SVerticalBox::Slot()
								.VAlign(VAlign_Top)
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
							.WidthOverride(0.25 * Theme.ContentWidth)
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
								.VAlign(VAlign_Top)
								[
									SAssignNew(RCSBox, SVerticalBox)
								]
							]
						]

						// Weapons
						+ SHorizontalBox::Slot()
						.AutoWidth()
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
							.VAlign(VAlign_Top)
							[
								SNew(SBox)
								.HeightOverride(900)
								[
									SNew(SScrollBox)
									.Style(&Theme.ScrollBoxStyle)
									.ScrollBarStyle(&Theme.ScrollBarStyle)

									+ SScrollBox::Slot()
									[
										SAssignNew(WeaponBox, SHorizontalBox)
									]
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

	FCHECK(PlayerSpacecraftListData.Num() == 0);
	FCHECK(EnemySpacecraftListData.Num() == 0);

	// Setup lists
	CompanySelector->RefreshOptions();
	PlanetSelector->RefreshOptions();
	CompanySelector->SetSelectedIndex(0);
	PlanetSelector->SetSelectedIndex(0);

	// Start doing the setup
	MenuManager->GetGame()->GetSkirmishManager()->StartSetup();

	// Empty lists
	PlayerSpacecraftListData.Empty();
	EnemySpacecraftListData.Empty();
	PlayerSpacecraftList->RequestListRefresh();
	EnemySpacecraftList->RequestListRefresh();
}

void SFlareSkirmishSetupMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);

	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	FCHECK(PlayerSpacecraftListData.Num() <= MyGameSettings->MaxShipsInSector);
	FCHECK(EnemySpacecraftListData.Num() <= MyGameSettings->MaxShipsInSector);

	// Empty lists
	WeaponBox->ClearChildren();
	PlayerSpacecraftListData.Empty();
	EnemySpacecraftListData.Empty();
	PlayerSpacecraftList->RequestListRefresh();
	EnemySpacecraftList->RequestListRefresh();
}


/*----------------------------------------------------
	Content callbacks
----------------------------------------------------*/

FText SFlareSkirmishSetupMenu::GetAltitudeValue() const
{
	return FText::AsNumber(FMath::RoundToInt(100.0f * AltitudeSlider->GetValue()));
}

FText SFlareSkirmishSetupMenu::GetAsteroidValue() const
{
	return FText::AsNumber(FMath::RoundToInt(MAX_ASTEROIDS * AsteroidSlider->GetValue()));
}

FText SFlareSkirmishSetupMenu::GetDebrisValue() const
{
	return FText::AsNumber(FMath::RoundToInt(100.0f * DebrisSlider->GetValue()));
}

FText SFlareSkirmishSetupMenu::GetPlayerFleetTitle() const
{
	UFlareSpacecraftComponentsCatalog* Catalog = MenuManager->GetGame()->GetShipPartsCatalog();
	int32 CombatValue = 0;

	for (auto Order : PlayerSpacecraftListData)
	{
		CombatValue += Order->Description->CombatPoints;

		for (auto Weapon : Order->WeaponTypes)
		{
			CombatValue += Catalog->Get(Weapon)->CombatPoints;
		}
	}

	return FText::Format(LOCTEXT("PlayerFleetTitleFormat", "Player fleet ({0})"), CombatValue);
}

FText SFlareSkirmishSetupMenu::GetEnemyFleetTitle() const
{
	UFlareSpacecraftComponentsCatalog* Catalog = MenuManager->GetGame()->GetShipPartsCatalog();
	int32 CombatValue = 0;

	for (auto Order : EnemySpacecraftListData)
	{
		CombatValue += Order->Description->CombatPoints;

		for (auto Weapon : Order->WeaponTypes)
		{
			CombatValue += Catalog->Get(Weapon)->CombatPoints;
		}
	}

	return FText::Format(LOCTEXT("EnemyFleetTitleFormat", "Enemy fleet ({0})"), CombatValue);
}

TSharedRef<ITableRow> SFlareSkirmishSetupMenu::OnGenerateSpacecraftLine(TSharedPtr<FFlareSkirmishSpacecraftOrder> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	const FFlareSpacecraftDescription* Desc = Item->Description;

	// Structure
	return SNew(SFlareListItem, OwnerTable)
	.Width(16)
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
					.WrapTextAt(0.65 * Theme.ContentWidth)
				]
			]
		]
		
		// Action buttons
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(Theme.ContentPadding)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SFlareButton)
				.Text(FText())
				.HelpText(LOCTEXT("UpgradeSpacecraftInfo", "Upgrade this spacecraft"))
				.Icon(FFlareStyleSet::GetIcon("ShipUpgradeSmall"))
				.OnClicked(this, &SFlareSkirmishSetupMenu::OnUpgradeSpacecraft, Item)
				.Width(1)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SFlareButton)
				.Text(FText())
				.HelpText(LOCTEXT("DuplicateSpacecraftInfo", "Add a copy of this spacecraft and upgrades"))
				.Icon(FFlareStyleSet::GetIcon("New"))
				.OnClicked(this, &SFlareSkirmishSetupMenu::OnDuplicateSpacecraft, Item)
				.Width(1)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SFlareButton)
				.Text(FText())
				.HelpText(LOCTEXT("RemoveSpacecraftInfo", "Remove this spacecraft"))
				.Icon(FFlareStyleSet::GetIcon("Delete"))
				.OnClicked(this, &SFlareSkirmishSetupMenu::OnRemoveSpacecraft, Item)
				.Width(1)
			]
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

FText Capitalize(const FText& Text)
{
	FString Lowercase = Text.ToString().ToLower();

	FString FirstChar = Lowercase.Left(1).ToUpper();
	FString Remainder = Lowercase.Right(Lowercase.Len() - 1);

	return FText::FromString(FirstChar + Remainder);
}

TSharedRef<SWidget> SFlareSkirmishSetupMenu::OnGeneratePlanetComboLine(FFlareSectorCelestialBodyDescription Item)
{	
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(SBox)
	.Padding(Theme.ListContentPadding)
	[
		SNew(STextBlock)
		.Text(Capitalize(Item.CelestialBodyName))
		.TextStyle(&Theme.TextFont)
	];
}

FText SFlareSkirmishSetupMenu::OnGetCurrentPlanetComboLine() const
{
	const FFlareSectorCelestialBodyDescription Item = PlanetSelector->GetSelectedItem();
	
	return Capitalize(Item.CelestialBodyName);
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

FText SFlareSkirmishSetupMenu::GetAddToPlayerFleetText() const
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());

	if (PlayerSpacecraftListData.Num() >= MyGameSettings->MaxShipsInSector)
	{
		return LOCTEXT("TooManyPlayerShips", "You can't add more ships than the limit defined in settings");
	}
	else
	{
		return LOCTEXT("OKPlayerShips", "Add a new ship to the player fleet");
	}
}

FText SFlareSkirmishSetupMenu::GetAddToEnemyFleetText() const
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());

	if (EnemySpacecraftListData.Num() >= MyGameSettings->MaxShipsInSector)
	{
		return LOCTEXT("TooManyEnemyShips", "You can't add more ships than the limit defined in settings");
	}
	else
	{
		return LOCTEXT("OKEnemyShips", "Add a new ship to the enemy fleet");
	}
}

bool SFlareSkirmishSetupMenu::IsAddToPlayerFleetDisabled() const
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());

	return (PlayerSpacecraftListData.Num() >= MyGameSettings->MaxShipsInSector);
}

bool SFlareSkirmishSetupMenu::IsAddToEnemyFleetDisabled() const
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());

	return (EnemySpacecraftListData.Num() >= MyGameSettings->MaxShipsInSector);
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


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareSkirmishSetupMenu::OnAsteroidSliderChanged(float Value, bool ForPlayer)
{
}

void SFlareSkirmishSetupMenu::OnDebrisSliderChanged(float Value, bool ForPlayer)
{
}

void SFlareSkirmishSetupMenu::OnClearFleet(bool ForPlayer)
{
	if (ForPlayer)
	{
		PlayerSpacecraftListData.Empty();
		PlayerSpacecraftList->RequestListRefresh();
	}
	else
	{
		EnemySpacecraftListData.Empty();
		EnemySpacecraftList->RequestListRefresh();
	}
}

static bool SortByCombatValue(const TSharedPtr<FFlareSkirmishSpacecraftOrder>& Ship1, const TSharedPtr<FFlareSkirmishSpacecraftOrder>& Ship2)
{
	FCHECK(Ship1->Description);
	FCHECK(Ship2->Description);

	if (Ship1->Description->CombatPoints > Ship2->Description->CombatPoints)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void SFlareSkirmishSetupMenu::OnSortPlayerFleet()
{
	PlayerSpacecraftListData.Sort(SortByCombatValue);
	PlayerSpacecraftList->RequestListRefresh();
}

void SFlareSkirmishSetupMenu::OnAutoCreateEnemyFleet()
{
	// Settings
	float NerfRatio = 0.9f;
	float ShipRatio = 0.9f;
	float DiversityRatio = 0.6f;

	// Data
	UFlareSpacecraftCatalog* SpacecraftCatalog = MenuManager->GetGame()->GetSpacecraftCatalog();
	UFlareSpacecraftComponentsCatalog* PartsCatalog = MenuManager->GetGame()->GetShipPartsCatalog();

	// Get player value
	int32 PlayerLargeShips = 0;
	int32 PlayerCombatValue = 0;
	for (auto Order : PlayerSpacecraftListData)
	{
		PlayerCombatValue += Order->Description->CombatPoints;

		for (auto Weapon : Order->WeaponTypes)
		{
			PlayerCombatValue += PartsCatalog->Get(Weapon)->CombatPoints;
		}

		if (Order->Description->Size == EFlarePartSize::L)
		{
			PlayerLargeShips++;
		}
	}

	// Define objectives
	bool IsHighValueFleet = (PlayerCombatValue > 25);
	int32 TargetCombatValue = IsHighValueFleet ? NerfRatio * PlayerCombatValue : PlayerCombatValue;
	int32 TargetShipCombatValue = IsHighValueFleet ? ShipRatio * TargetCombatValue : TargetCombatValue;
	int32 CurrentCombatValue = 0;

	// Add ships, starting with the most powerful
	EnemySpacecraftListData.Empty();
	for (int32 Index = SpacecraftCatalog->ShipCatalog.Num() - 1; Index >= 0; Index--)
	{
		for (const UFlareSpacecraftCatalogEntry* Spacecraft : SpacecraftCatalog->ShipCatalog)
		{
			int32 RemainingCombatValue = TargetShipCombatValue - CurrentCombatValue;

			if (Spacecraft->Data.CombatPoints > 0 && Spacecraft->Data.CombatPoints < RemainingCombatValue)
			{
				// Define how much of this ship we want
				float RawShipCount = (DiversityRatio * RemainingCombatValue) / (float)Spacecraft->Data.CombatPoints;
				int32 ShipCount = IsHighValueFleet ? FMath::FloorToInt(RawShipCount) :  FMath::CeilToInt(RawShipCount);

				// Order all copies
				for (int OrderIndex = 0; OrderIndex < ShipCount; OrderIndex++)
				{
					TSharedPtr<FFlareSkirmishSpacecraftOrder> Order = FFlareSkirmishSpacecraftOrder::New(&Spacecraft->Data);
					Order->ForPlayer = false;
					SetOrderDefaults(Order);
					EnemySpacecraftListData.Add(Order);
					CurrentCombatValue += Spacecraft->Data.CombatPoints;
				}
			}
		}

		// Value exceeded, break off
		if (CurrentCombatValue >= TargetShipCombatValue)
		{
			break;
		}
	}

	// Upgrade ships, starting with the least powerful
	for (int32 Index = EnemySpacecraftListData.Num() - 1; Index >= 0; Index--)
	{
		// Value exceeded, break off
		int32 RemainingCombatValue = TargetCombatValue - CurrentCombatValue;
		if (RemainingCombatValue <= 0)
		{
			break;
		}

		// Get data
		TSharedPtr<FFlareSkirmishSpacecraftOrder> Order = EnemySpacecraftListData[Index];
		FFlareSpacecraftComponentDescription* BestAntiLargeWeapon = NULL;
		FFlareSpacecraftComponentDescription* BestAntiSmallWeapon = NULL;
		float BestAntiLarge = 0;
		float BestAntiSmall = 0;

		// Find best weapons against specific archetypes
		TArray<FFlareSpacecraftComponentDescription*> WeaponList;
		PartsCatalog->GetWeaponList(WeaponList, Order->Description->Size);
		for (FFlareSpacecraftComponentDescription* Weapon : WeaponList)
		{
			int32 UpgradeCombatValue = Order->WeaponTypes.Num() * Weapon->CombatPoints;
			if (UpgradeCombatValue > RemainingCombatValue)
			{
				continue;
			}

			// TODO : this algorithm always ends up using missiles if it upgrades at all, make it more diverse

			float AntiLarge = Weapon->WeaponCharacteristics.AntiLargeShipValue;
			if (AntiLarge > BestAntiLarge)
			{
				BestAntiLarge = AntiLarge;
				BestAntiLargeWeapon = Weapon;
			}

			float AntiSmall = Weapon->WeaponCharacteristics.AntiSmallShipValue;
			if (AntiSmall > BestAntiSmall)
			{
				BestAntiSmall = AntiSmall;
				BestAntiSmallWeapon = Weapon;
			}
		}

		// Assign one anti-L weapon to each L ship, else use anti small
		FFlareSpacecraftComponentDescription* UpgradeWeapon = NULL;
		if (PlayerLargeShips && BestAntiLargeWeapon)
		{
			UpgradeWeapon = BestAntiLargeWeapon;
			PlayerLargeShips--;
		}
		else if (BestAntiSmallWeapon)
		{
			UpgradeWeapon = BestAntiSmallWeapon;
		}

		// Upgrade
		if (UpgradeWeapon)
		{
			for (int32 WeaponIndex = 0; WeaponIndex < Order->WeaponTypes.Num(); WeaponIndex++)
			{
				Order->WeaponTypes[WeaponIndex] = UpgradeWeapon->Identifier;
				CurrentCombatValue += UpgradeWeapon->CombatPoints;
			}
		}
	}

	EnemySpacecraftList->RequestListRefresh();
}

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

	Order->ForPlayer = IsOrderingForPlayer;
	SetOrderDefaults(Order);

	// Add ship
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
	const FFlareSpacecraftDescription* Desc = Order->Description;
	UFlareSpacecraftComponentsCatalog* Catalog = MenuManager->GetGame()->GetShipPartsCatalog();
	TArray<FFlareSpacecraftComponentDescription*> PartList;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Prepare data
	FText Text = LOCTEXT("PickComponentFormat", "{0}\n({1} value)");
	FText HelpText = LOCTEXT("PickComponentHelp", "Upgrade with this component");

	// Reset lists
	OrbitalEngineBox->ClearChildren();
	RCSBox->ClearChildren();
	WeaponBox->ClearChildren();
	UpgradeBox->SetVisibility(EVisibility::Visible);

	// Engines
	Catalog->GetEngineList(PartList, Desc->Size);
	for (auto Engine : PartList)
	{
		FLinearColor Color = (Order->EngineType == Engine->Identifier) ? Theme.FriendlyColor : Theme.NeutralColor;

		OrbitalEngineBox->InsertSlot(0)
		.Padding(FMargin(0, 10))
		[
			SNew(SFlareRoundButton)
			.Icon(&Engine->MeshPreviewBrush)
			.Text(Engine->Name)
			.HelpText(HelpText)
			.InvertedBackground(true)
			.HighlightColor(Color)
			.OnClicked(this, &SFlareSkirmishSetupMenu::OnUpgradeEngine, Order, Engine->Identifier)
		];
	}

	// RCS
	PartList.Empty();
	Catalog->GetRCSList(PartList, Desc->Size);
	for (auto RCS : PartList)
	{
		FLinearColor Color = (Order->RCSType == RCS->Identifier) ? Theme.FriendlyColor : Theme.NeutralColor;

		RCSBox->InsertSlot(0)
		.Padding(FMargin(0, 10))
		[
			SNew(SFlareRoundButton)
			.Icon(&RCS->MeshPreviewBrush)
			.Text(RCS->Name)
			.HelpText(HelpText)
			.InvertedBackground(true)
			.HighlightColor(Color)
			.OnClicked(this, &SFlareSkirmishSetupMenu::OnUpgradeRCS, Order, RCS->Identifier)
		];
	}

	// Weapons
	PartList.Empty();
	Catalog->GetWeaponList(PartList, Desc->Size);
	for (int32 Index = 0; Index < Order->Description->WeaponGroups.Num(); Index++)
	{
		// Create vertical structure
		const FFlareSpacecraftSlotGroupDescription& Slot = Order->Description->WeaponGroups[Index];
		TSharedPtr<SVerticalBox> WeaponSlot;
		WeaponBox->AddSlot()
		[
			SNew(SBox)
			.WidthOverride(0.25 * Theme.ContentWidth)
			[
				SAssignNew(WeaponSlot, SVerticalBox)
			]
		];

		for (auto Weapon : PartList)
		{
			FText NameText = FText::Format(Text, Weapon->Name, FText::AsNumber(Weapon->CombatPoints));
			FLinearColor Color = (Order->WeaponTypes[Index] == Weapon->Identifier) ? Theme.FriendlyColor : Theme.NeutralColor;

			WeaponSlot->AddSlot()
			.Padding(FMargin(0, 10))
			[
				SNew(SFlareRoundButton)
				.Icon(&Weapon->MeshPreviewBrush)
				.Text(NameText)
				.HelpText(HelpText)
				.InvertedBackground(true)
				.HighlightColor(Color)
				.OnClicked(this, &SFlareSkirmishSetupMenu::OnUpgradeWeapon, Order, Index, Weapon->Identifier)
			];
		}
	}

	SlatePrepass(FSlateApplicationBase::Get().GetApplicationScale());
}

void SFlareSkirmishSetupMenu::OnRemoveSpacecraft(TSharedPtr<FFlareSkirmishSpacecraftOrder> Order)
{
	if (Order->ForPlayer)
	{
		PlayerSpacecraftListData.Remove(Order);
		PlayerSpacecraftList->RequestListRefresh();
	}
	else
	{
		EnemySpacecraftListData.Remove(Order);
		EnemySpacecraftList->RequestListRefresh();
	}
}

void SFlareSkirmishSetupMenu::OnDuplicateSpacecraft(TSharedPtr<FFlareSkirmishSpacecraftOrder> Order)
{
	// Copy order
	TSharedPtr<FFlareSkirmishSpacecraftOrder> NewOrder = FFlareSkirmishSpacecraftOrder::New(Order->Description);
	NewOrder->EngineType = Order->EngineType;
	NewOrder->RCSType = Order->RCSType;
	NewOrder->WeaponTypes = Order->WeaponTypes;
	NewOrder->ForPlayer = Order->ForPlayer;

	// Add order
	if (Order->ForPlayer)
	{
		PlayerSpacecraftListData.Add(NewOrder);
		PlayerSpacecraftList->RequestListRefresh();
	}
	else
	{
		EnemySpacecraftListData.Add(NewOrder);
		EnemySpacecraftList->RequestListRefresh();
	}
}

void SFlareSkirmishSetupMenu::OnUpgradeEngine(TSharedPtr<FFlareSkirmishSpacecraftOrder> Order, FName Upgrade)
{
	Order->EngineType = Upgrade;
	OnUpgradeSpacecraft(Order);
}

void SFlareSkirmishSetupMenu::OnUpgradeRCS(TSharedPtr<FFlareSkirmishSpacecraftOrder> Order, FName Upgrade)
{
	Order->RCSType = Upgrade;
	OnUpgradeSpacecraft(Order);
}

void SFlareSkirmishSetupMenu::OnUpgradeWeapon(TSharedPtr<FFlareSkirmishSpacecraftOrder> Order, int32 GroupIndex, FName Upgrade)
{
	const FFlareSpacecraftDescription* Desc = Order->Description;

	for (int32 Index = 0; Index < Order->Description->WeaponGroups.Num(); Index++)
	{
		if (Index == GroupIndex)
		{
			Order->WeaponTypes[Index] = Upgrade;
		}
	}

	OnUpgradeSpacecraft(Order);
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
	PlayerSpacecraftListData.Empty();
	for (auto Order : EnemySpacecraftListData)
	{
		Skirmish->AddShip(false, *Order.Get());
	}
	EnemySpacecraftListData.Empty();

	// Set sector settings
	Skirmish->GetData().SectorAltitude = AltitudeSlider->GetValue();
	Skirmish->GetData().AsteroidCount = MAX_ASTEROIDS * AsteroidSlider->GetValue();
	Skirmish->GetData().MetallicDebris = MetalDebrisButton->IsActive();
	Skirmish->GetData().SectorDescription.CelestialBodyIdentifier = PlanetSelector->GetSelectedItem().CelestialBodyIdentifier;
	Skirmish->GetData().SectorDescription.IsIcy = IcyButton->IsActive();
	Skirmish->GetData().SectorDescription.DebrisFieldInfo.DebrisFieldDensity = MAX_DEBRIS_PERCENTAGE * DebrisSlider->GetValue();

	// Override company color with current settings 
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


/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

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

void SFlareSkirmishSetupMenu::SetOrderDefaults(TSharedPtr<FFlareSkirmishSpacecraftOrder> Order)
{
	if (Order->Description->Size == EFlarePartSize::S)
	{
		Order->EngineType = FName("engine-thresher");
		Order->RCSType = FName("rcs-coral");

		for (auto& Slot : Order->Description->WeaponGroups)
		{
			Order->WeaponTypes.Add(FName("weapon-eradicator"));
		}
	}
	else
	{
		Order->EngineType = FName("pod-thera");
		Order->RCSType = FName("rcs-rift");

		for (auto& Slot : Order->Description->WeaponGroups)
		{
			Order->WeaponTypes.Add(FName("weapon-artemis"));
		}
	}
}


#undef LOCTEXT_NAMESPACE

