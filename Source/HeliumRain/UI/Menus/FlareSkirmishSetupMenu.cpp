
#include "FlareSkirmishSetupMenu.h"
#include "../../Flare.h"

#include "../../Data/FlareCompanyCatalog.h"
#include "../../Data/FlareSpacecraftComponentsCatalog.h"
#include "../../Data/FlareCustomizationCatalog.h"

#include "../../Game/FlareGame.h"
#include "../../Game/FlareSaveGame.h"
#include "../../Game/FlareSkirmishManager.h"
#include "../../Game/FlareGameUserSettings.h"

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
	int32 ListHeight = 700;

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

			// TODO 1075 : orbital body picker
			// TODO 1075 : sector type picker (rock, metla debris)
			// TODO 1075 : icy checkbox
			// TODO 1075 : altitude slider
			// TODO 1075 : asteroid slider
			// TODO 1075 : debris intensity slider

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
								.Width(6)
								.Text(LOCTEXT("AddPlayerShip", "Add player ship"))
								.HelpText(this, &SFlareSkirmishSetupMenu::GetAddToPlayerFleetText)
								.IsDisabled(this, &SFlareSkirmishSetupMenu::IsAddToPlayerFleetDisabled)
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
								.Width(6)
								.Text(LOCTEXT("AddEnemyShip", "Add enemy ship"))
								.HelpText(this, &SFlareSkirmishSetupMenu::GetAddToEnemyFleetText)
								.IsDisabled(this, &SFlareSkirmishSetupMenu::IsAddToEnemyFleetDisabled)
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

	CompanySelector->RefreshOptions();

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

	// Empty lists
	PlayerSpacecraftListData.Empty();
	EnemySpacecraftListData.Empty();
	PlayerSpacecraftList->RequestListRefresh();
	EnemySpacecraftList->RequestListRefresh();
}



/*----------------------------------------------------
	Content callbacks
----------------------------------------------------*/

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
					.WrapTextAt(0.9 * Theme.ContentWidth)
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
void SFlareSkirmishSetupMenu::OnAutoCreateEnemyFleet()
{
	EnemySpacecraftListData.Empty();

	// TODO 1075 : do a real algorithm
	EnemySpacecraftListData = PlayerSpacecraftListData;

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

	// Set default upgrades
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
	FFlareSpacecraftDescription* Desc = Order->Description;
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
		FFlareSpacecraftSlotGroupDescription& Slot = Order->Description->WeaponGroups[Index];
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
	FFlareSpacecraftDescription* Desc = Order->Description;

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

