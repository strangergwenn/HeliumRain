
#include "HeliumRain/UI/Menus/FlareCompanyMenu.h"
#include "../../Flare.h"

#include "../Components/FlarePartInfo.h"
#include "../Components/FlareCompanyInfo.h"
#include "../Components/FlareTabView.h"

#include "../../Data/FlareSpacecraftComponentsCatalog.h"
#include "../../Data/FlareCustomizationCatalog.h"

#include "../../Game/FlareGame.h"
#include "../../Game/FlareCompany.h"
#include "../../Game/FlareGameTools.h"

#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlarePlayerController.h"

#include <Widgets/Layout/SBackgroundBlur.h>


#define LOCTEXT_NAMESPACE "FlareCompanyMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareCompanyMenu::Construct(const FArguments& InArgs)
{
	// Data
	Company = NULL;
	MenuManager = InArgs._MenuManager;
	AFlarePlayerController* PC = MenuManager->GetPC();

	// Theme
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	SmallWidth = 0.25 * Theme.ContentWidth;
	LargeWidth = 0.5 * Theme.ContentWidth;
	VeryLargeWidth = 1.0 * Theme.ContentWidth;

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SBackgroundBlur)
		.BlurRadius(this, &SFlareCompanyMenu::GetBlurRadius)
		.BlurStrength(this, &SFlareCompanyMenu::GetBlurStrength)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Padding(FMargin(0))
		[
			SAssignNew(TabView, SFlareTabView)

			// Content block
			+ SFlareTabView::Slot()
			.Header(LOCTEXT("CompanyMainTab", "Company"))
			.HeaderHelp(LOCTEXT("CompanyMainTabHelp", "General information about your company"))
			[
				SNew(SVerticalBox)

				// Company info
				+ SVerticalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoHeight()
				[
					SNew(SBox)
					.WidthOverride(0.8 * Theme.ContentWidth)
					[
						SAssignNew(CompanyInfo, SFlareCompanyInfo)
						.Player(PC)
					]
				]

				// TR info
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(TradeRouteInfo, SFlareTradeRouteInfo)
					.MenuManager(MenuManager)
				]
			]

			// Property block
			+ SFlareTabView::Slot()
			.Header(LOCTEXT("CompanyPropertyTab", "Property"))
			.HeaderHelp(LOCTEXT("CompanyPropertyTabHelp", "Fleets, ships and stations"))
			[
				SNew(SBox)
				.WidthOverride(Theme.ContentWidth)
				.HAlign(HAlign_Left)
				[
					SAssignNew(ShipList, SFlareList)
					.MenuManager(MenuManager)
					.Title(LOCTEXT("Property", "Property"))
				]
			]

			// Company customization
			+ SFlareTabView::Slot()
			.Header(LOCTEXT("CompanyCustomizationTab", "Appearance"))
			.HeaderHelp(LOCTEXT("CompanyCustomizationTabHelp", "Appearance settings for your company"))
			[
				SNew(SBox)
				.WidthOverride(Theme.ContentWidth)
				.HAlign(HAlign_Left)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.Padding(Theme.TitlePadding)
					.AutoHeight()
					[
						SNew(STextBlock)
						.TextStyle(&Theme.SubTitleFont)
						.Text(LOCTEXT("CompanyEditTitle", "Appearance"))
					]

					// Company name
					+ SVerticalBox::Slot()
					.Padding(Theme.ContentPadding)
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(LOCTEXT("EditCompanyName", "Company name"))
						]

						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Right)
						[
							SNew(SBox)
							.WidthOverride(0.4 * Theme.ContentWidth)
							[
								SNew(SBorder)
								.BorderImage(&Theme.BackgroundBrush)
								.Padding(Theme.ContentPadding)
								[
									SAssignNew(CompanyName, SEditableText)
									.AllowContextMenu(false)
									.Style(&Theme.TextInputStyle)
								]
							]
						]
			
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.HAlign(HAlign_Right)
						[
							SNew(SFlareButton)
							.Text(LOCTEXT("Rename", "Rename"))
							.HelpText(LOCTEXT("RenameInfo", "Rename this company"))
							.Icon(FFlareStyleSet::GetIcon("OK"))
							.OnClicked(this, &SFlareCompanyMenu::OnRename)
							.IsDisabled(this, &SFlareCompanyMenu::IsRenameDisabled)
							.Width(4)
						]
					]

					// Color picker
					+ SVerticalBox::Slot()
					.Padding(Theme.ContentPadding)
					.AutoHeight()
					[
						SAssignNew(ColorBox, SFlareColorPanel)
						.MenuManager(MenuManager)
					]
				
					// Emblem
					+ SVerticalBox::Slot()
					.Padding(Theme.ContentPadding)
					.VAlign(VAlign_Top)
					[
						SAssignNew(EmblemPicker, SFlareDropList<int32>)
						.LineSize(1)
						.HeaderWidth(3)
						.HeaderHeight(3)
						.ItemWidth(3)
						.ItemHeight(2.7)
						.ShowColorWheel(false)
						.MaximumHeight(600)
						.OnItemPicked(this, &SFlareCompanyMenu::OnEmblemPicked)
					]
				]
			]

			// Company log
			+ SFlareTabView::Slot()
			.Header(LOCTEXT("CompanyLogTab", "Transaction log"))
			.HeaderHelp(LOCTEXT("CompanyLogTabHelp", "Log of recent commercial operations"))
			[
				SNew(SVerticalBox)

				// Title
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Top)
				[
					SNew(SHorizontalBox)

					// Title
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Center)
					.Padding(Theme.TitlePadding)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.SubTitleFont)
						.Text(LOCTEXT("CompanyLogTitle", "Transaction log"))
					]

					// Source filter
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					.Padding(Theme.ContentPadding)
					[
						SAssignNew(SourceSelector, SFlareDropList<UFlareSimulatedSpacecraft*>)
						.OptionsSource(&SourceList)
						.OnGenerateWidget(this, &SFlareCompanyMenu::OnGenerateSourceComboLine)
						.OnSelectionChanged(this, &SFlareCompanyMenu::OnSourceComboLineSelectionChanged)
						.HeaderWidth(8)
						.ItemWidth(8)
						[
							SNew(SBox)
							.Padding(Theme.ListContentPadding)
							[
								SNew(STextBlock)
								.Text(this, &SFlareCompanyMenu::OnGetCurrentSourceComboLine)
								.TextStyle(&Theme.TextFont)
							]
						]
					]

					// Sector filter
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					.Padding(Theme.ContentPadding)
					[
						SAssignNew(SectorSelector, SFlareDropList<UFlareSimulatedSector*>)
						.OptionsSource(&SectorList)
						.OnGenerateWidget(this, &SFlareCompanyMenu::OnGenerateSectorComboLine)
						.OnSelectionChanged(this, &SFlareCompanyMenu::OnSectorComboLineSelectionChanged)
						.HeaderWidth(8)
						.ItemWidth(8)
						[
							SNew(SBox)
							.Padding(Theme.ListContentPadding)
							[
								SNew(STextBlock)
								.Text(this, &SFlareCompanyMenu::OnGetCurrentSectorComboLine)
								.TextStyle(&Theme.TextFont)
							]
						]
					]

					// Company filter
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					.Padding(Theme.ContentPadding)
					[
						SAssignNew(CompanySelector, SFlareDropList<UFlareCompany*>)
						.OptionsSource(&CompanyList)
						.OnGenerateWidget(this, &SFlareCompanyMenu::OnGenerateCompanyComboLine)
						.OnSelectionChanged(this, &SFlareCompanyMenu::OnCompanyComboLineSelectionChanged)
						.HeaderWidth(8)
						.ItemWidth(8)
						[
							SNew(SBox)
							.Padding(Theme.ListContentPadding)
							[
								SNew(STextBlock)
								.Text(this, &SFlareCompanyMenu::OnGetCurrentCompanyComboLine)
								.TextStyle(&Theme.TextFont)
							]
						]
					]
				]

				// Header
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					// Date
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(SmallWidth)
						.HAlign(HAlign_Left)
						.Padding(Theme.ContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.NameFont)
							.Text(LOCTEXT("TransactionTitleDate", "Date"))
						]
					]

					// Debit
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(SmallWidth)
						.HAlign(HAlign_Left)
						.Padding(Theme.ContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.NameFont)
							.Text(LOCTEXT("TransactionTitleDebit", "Debit"))
						]
					]

					// Credit
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(SmallWidth)
						.HAlign(HAlign_Left)
						.Padding(Theme.ContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.NameFont)
							.Text(LOCTEXT("TransactionTitleCredit", "Credit"))
						]
					]

					// Source
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(LargeWidth)
						.HAlign(HAlign_Left)
						.Padding(Theme.ContentPadding + FMargin(10, 0, 0, 0))
						[
							SNew(STextBlock)
							.TextStyle(&Theme.NameFont)
							.Text(LOCTEXT("TransactionTitleSource", "Source"))
						]
					]

					// Location
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(SmallWidth)
						.HAlign(HAlign_Left)
						.Padding(Theme.ContentPadding + FMargin(10, 0, 0, 0))
						[
							SNew(STextBlock)
							.TextStyle(&Theme.NameFont)
							.Text(LOCTEXT("TransactionTitleSector", "Sector"))
						]
					]

					// Partner
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(SmallWidth)
						.HAlign(HAlign_Left)
						.Padding(Theme.ContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.NameFont)
							.Text(LOCTEXT("TransactionTitlePartner", "Partner"))
						]
					]

					// Comment
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(VeryLargeWidth)
						.HAlign(HAlign_Left)
						.Padding(Theme.ContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.NameFont)
							.Text(LOCTEXT("TransactionTitleComment", "Comment"))
						]
					]
				]

				// Company log contents
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(CompanyLog, SVerticalBox)
				]
			]

			// Company accounting
			+ SFlareTabView::Slot()
			.Header(LOCTEXT("CompanyAccountingTab", "Accounting"))
			.HeaderHelp(LOCTEXT("CompanyAccountingTabHelp", "Company accounts"))
			[
				SNew(SVerticalBox)

				// Title
				+ SVerticalBox::Slot()
				.Padding(Theme.TitlePadding)
				.AutoHeight()
				.HAlign(HAlign_Left)
				[
					SNew(SBox)
					.WidthOverride(Theme.ContentWidth)
					.HAlign(HAlign_Fill)
					[
						SNew(SHorizontalBox)

						// Previous
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.HAlign(HAlign_Left)
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SFlareButton)
							.Width(1.5)
							.Transparent(true)
							.OnClicked(this, &SFlareCompanyMenu::OnPreviousYear)
							.IsDisabled(this, &SFlareCompanyMenu::IsPreviousYearDisabled)
							.Icon(FFlareStyleSet::GetIcon("MoveLeft"))
							.Text(FText())
							.HelpText(LOCTEXT("PreviousYearHelp", "Show accounting for the previous year"))
						]

						// Title
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.Padding(Theme.ContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(this, &SFlareCompanyMenu::GetAccountingTitle)
						]

						// Next year
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.HAlign(HAlign_Right)
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SFlareButton)
							.Width(1.5)
							.OnClicked(this, &SFlareCompanyMenu::OnNextYear)
							.IsDisabled(this, &SFlareCompanyMenu::IsNextYearDisabled)
							.Icon(FFlareStyleSet::GetIcon("MoveRight"))
							.Text(FText())
							.HelpText(LOCTEXT("NextYearHelp", "Show accounting for the next year"))
						]
					]
				]
		
				// Header
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					// Type
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(LargeWidth)
						.HAlign(HAlign_Left)
						.Padding(Theme.ContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.NameFont)
							.Text(LOCTEXT("AccountingTitleType", "Category"))
						]
					]

					// Debit
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(SmallWidth)
						.HAlign(HAlign_Left)
						.Padding(Theme.ContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.NameFont)
							.Text(LOCTEXT("AccountingTitleDebit", "Debit"))
						]
					]

					// Credit
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(SmallWidth)
						.HAlign(HAlign_Left)
						.Padding(Theme.ContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.NameFont)
							.Text(LOCTEXT("AccountingTitleCredit", "Credit"))
						]
					]
				]

				// Company accounting contents
				+ SVerticalBox::Slot()
				.HAlign(HAlign_Left)
				.AutoHeight()
				[
					SNew(SBox)
					.WidthOverride(Theme.ContentWidth)
					[
						SAssignNew(CompanyAccounting, SVerticalBox)
					]
				]
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareCompanyMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareCompanyMenu::Enter(UFlareCompany* Target)
{
	FLOG("SFlareCompanyMenu::Enter");
	SetEnabled(true);

	// Company data
	Company = Target;
	SetVisibility(EVisibility::Visible);
	CompanyInfo->SetCompany(Company);
	CompanyName->SetText(Company->GetCompanyName());
	TradeRouteInfo->Update();

	// Date
	int64 CurrentGameDate = MenuManager->GetGame()->GetGameWorld()->GetDate();
	CurrentGameYear = UFlareGameTools::GetYearFromDate(CurrentGameDate);
	CurrentAccountingYear = CurrentGameYear;

	// Player customization
	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC && Target)
	{
		// Colors
		FFlarePlayerSave Data;
		FFlareCompanyDescription Unused;
		PC->Save(Data, Unused);
		ColorBox->Setup(Data);

		// Emblems
		UFlareCustomizationCatalog* CustomizationCatalog = MenuManager->GetGame()->GetCustomizationCatalog();
		for (int i = 0; i < CustomizationCatalog->GetEmblemCount(); i++)
		{
			EmblemPicker->AddItem(SNew(SImage).Image(CustomizationCatalog->GetEmblemBrush(i)));
		}
		EmblemPicker->SetSelectedIndex(PC->GetPlayerData()->PlayerEmblemIndex);

		// Menu
		PC->GetMenuPawn()->SetCameraOffset(FVector2D(200, -30));
		if (PC->GetPlayerShip())
		{
			PC->GetMenuPawn()->ShowShip(PC->GetPlayerShip());
		}
		else
		{
			const FFlareSpacecraftComponentDescription* PartDesc = PC->GetGame()->GetShipPartsCatalog()->Get("object-safe");
			PC->GetMenuPawn()->ShowPart(PartDesc);
		}
	}

	// Add filter options
	SourceList.Add(NULL);
	SectorList.Add(NULL);
	CompanyList.Add(NULL);
	for (auto Entry : Target->GetTransactionLog())
	{
		UFlareCompany* Other = Entry.GetOtherCompany(Target->GetGame());
		UFlareSimulatedSector* Sector = Entry.GetSector(Target->GetGame());
		UFlareSimulatedSpacecraft* Source = Entry.GetSpacecraft(Target->GetGame());

		SourceList.AddUnique(Source);
		SectorList.AddUnique(Sector);
		CompanyList.AddUnique(Other);
	}

	// Reset filters
	CurrentSourceFilter = NULL;
	CurrentSectorFilter = NULL;
	CurrentCompanyFilter = NULL;

	// Sub-menus
	ShowProperty(Target);
	ShowCompanyLog(Target);
	ShowCompanyAccounting(Target);

	// Refresh
	SourceSelector->RefreshOptions();
	SectorSelector->RefreshOptions();
	CompanySelector->RefreshOptions();
	ShipList->RefreshList();
	ShipList->SetVisibility(EVisibility::Visible);
}

void SFlareCompanyMenu::Exit()
{
	SetEnabled(false);
	ShipList->Reset();
	ShipList->SetVisibility(EVisibility::Collapsed);

	EmblemPicker->ClearItems();
	TradeRouteInfo->Clear();
	CompanyLog->ClearChildren();
	CompanyAccounting->ClearChildren();

	SourceList.Empty();
	SectorList.Empty();
	CompanyList.Empty();

	CurrentSourceFilter = NULL;
	CurrentSectorFilter = NULL;
	CurrentCompanyFilter = NULL;

	Company = NULL;
	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Content helpers
----------------------------------------------------*/

void SFlareCompanyMenu::ShowProperty(UFlareCompany* Target)
{
	// Station list
	TArray<UFlareSimulatedSpacecraft*>& CompanyStations = Target->GetCompanyStations();
	for (int32 i = 0; i < CompanyStations.Num(); i++)
	{
		if (CompanyStations[i]->GetDamageSystem()->IsAlive())
		{
			ShipList->AddShip(CompanyStations[i]);
		}
	}

	// Ship list
	TArray<UFlareSimulatedSpacecraft*>& CompanyShips = Target->GetCompanyShips();
	for (int32 i = 0; i < CompanyShips.Num(); i++)
	{
		if (CompanyShips[i]->GetDamageSystem()->IsAlive())
		{
			ShipList->AddShip(CompanyShips[i]);
		}
	}
}

void SFlareCompanyMenu::ShowCompanyLog(UFlareCompany* Target)
{
	CompanyLog->ClearChildren();
	
	// Compute balances
	TMap<int64, int64> DayBalances;
	for (const FFlareTransactionLogEntry& Entry : Target->GetTransactionLog())
	{
		int64* PreviousBalance = DayBalances.Find(Entry.Date);
		if (PreviousBalance)
		{
			DayBalances.Add(Entry.Date, Entry.Amount + *PreviousBalance);			
		}
		else
		{
			DayBalances.Add(Entry.Date, Entry.Amount);
		}
	}

	// Generate the full log
	bool Even = true;
	int64 CurrentDate = 0;
	for (const FFlareTransactionLogEntry& Entry : Target->GetTransactionLog())
	{
		// Check filters
		UFlareCompany* Other = Entry.GetOtherCompany(Target->GetGame());
		UFlareSimulatedSector* Sector = Entry.GetSector(Target->GetGame());
		UFlareSimulatedSpacecraft* Source = Entry.GetSpacecraft(Target->GetGame());
		if (CurrentSourceFilter && CurrentSourceFilter != Source)
		{
			continue;
		}
		else if (CurrentSectorFilter && CurrentSectorFilter != Sector)
		{
			continue;
		}
		else if (CurrentCompanyFilter && CurrentCompanyFilter != Other)
		{
			continue;
		}
		else if (MenuManager->GetGame()->GetGameWorld()->GetDate() - Entry.Date > 30)
		{
			continue;
		}

		// Add day header if the date just changed
		if (Entry.Date != CurrentDate)
		{
			if (CurrentDate != 0)
			{
				int64* Balance = DayBalances.Find(CurrentDate);
				FCHECK(Balance);
				AddTransactionHeader(CurrentDate, *Balance, Target);
			}

			CurrentDate = Entry.Date;
		}

		// Add regular transaction log entry
		AddTransactionLog(Entry, Target, Even);
		Even = !Even;
	}

	// Add header for the last day
	int64* Balance = DayBalances.Find(CurrentDate);
	if (Balance)
	{
		AddTransactionHeader(CurrentDate, *Balance, Target);
	}
}

void SFlareCompanyMenu::ShowCompanyAccounting(UFlareCompany* Target)
{
	CompanyAccounting->ClearChildren();

	// Initialize balances
	TArray<int64> Balances;
	for (int32 Type = 0; Type < EFlareTransactionLogEntry::Type::TYPE_COUNT; Type++)
	{
		Balances.Add(0);
	}

	// Compute balances
	for (const FFlareTransactionLogEntry& Entry : Target->GetTransactionLog())
	{
		if (UFlareGameTools::GetYearFromDate(Entry.Date) == CurrentAccountingYear)
		{
			Balances[Entry.Type] += Entry.Amount;
		}
	}

	// Others
	int64 CurrentTotal = AddAccountingCategory(EFlareTransactionLogEntry::InitialMoney, Balances, Target, false);
	CurrentTotal += AddAccountingCategory(EFlareTransactionLogEntry::SendTribute, Balances, Target, true);
	CurrentTotal += AddAccountingCategory(EFlareTransactionLogEntry::QuestReward, Balances, Target, false);
	AddAccountingHeader(LOCTEXT("AccountingHeaderOthers", "Others"), CurrentTotal);

	// Stations
	CurrentTotal = AddAccountingCategory(EFlareTransactionLogEntry::PaidForRefill, Balances, Target, true);
	CurrentTotal += AddAccountingCategory(EFlareTransactionLogEntry::PaidForRepair, Balances, Target, false);
	CurrentTotal += AddAccountingCategory(EFlareTransactionLogEntry::PeoplePurchase, Balances, Target, true);
	CurrentTotal += AddAccountingCategory(EFlareTransactionLogEntry::CancelFactoryWages, Balances, Target, false);
	CurrentTotal += AddAccountingCategory(EFlareTransactionLogEntry::FactoryWages, Balances, Target, true);
	CurrentTotal += AddAccountingCategory(EFlareTransactionLogEntry::StationUpgradeFees, Balances, Target, false);
	CurrentTotal += AddAccountingCategory(EFlareTransactionLogEntry::CancelStationUpgradeFees, Balances, Target, true);
	CurrentTotal += AddAccountingCategory(EFlareTransactionLogEntry::StationConstructionFees, Balances, Target, false);
	AddAccountingHeader(LOCTEXT("AccountingHeaderStations", "Stations"), CurrentTotal);

	// Ships
	CurrentTotal = AddAccountingCategory(EFlareTransactionLogEntry::ScrapGain, Balances, Target, true);
	CurrentTotal += AddAccountingCategory(EFlareTransactionLogEntry::RecoveryFees, Balances, Target, false);
	CurrentTotal += AddAccountingCategory(EFlareTransactionLogEntry::PayRefill, Balances, Target, false);
	CurrentTotal += AddAccountingCategory(EFlareTransactionLogEntry::PayRepair, Balances, Target, true);
	CurrentTotal += AddAccountingCategory(EFlareTransactionLogEntry::UpgradeShipPart, Balances, Target, true);
	CurrentTotal += AddAccountingCategory(EFlareTransactionLogEntry::CancelOrderShip, Balances, Target, true);
	CurrentTotal += AddAccountingCategory(EFlareTransactionLogEntry::OrderShipAdvance, Balances, Target, false);
	CurrentTotal += AddAccountingCategory(EFlareTransactionLogEntry::OrderShip, Balances, Target, false);
	AddAccountingHeader(LOCTEXT("AccountingHeaderShips", "Ships"), CurrentTotal);

	// Trading
	CurrentTotal = AddAccountingCategory(EFlareTransactionLogEntry::TradeRouteResourceSell, Balances, Target, false);
	CurrentTotal += AddAccountingCategory(EFlareTransactionLogEntry::TradeRouteResourcePurchase, Balances, Target, true);
	CurrentTotal += AddAccountingCategory(EFlareTransactionLogEntry::ManualResourceSell, Balances, Target, false);
	CurrentTotal += AddAccountingCategory(EFlareTransactionLogEntry::ManualResourcePurchase, Balances, Target, true);
	AddAccountingHeader(LOCTEXT("AccountingHeaderTrading", "Trading"), CurrentTotal);

}

void SFlareCompanyMenu::AddTransactionHeader(int64 Time, int64 Balance, UFlareCompany* Target)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	CompanyLog->InsertSlot(0)
	.AutoHeight()
	[
		SNew(SHorizontalBox)
			
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.HeightOverride(2)
			[
				SNew(SImage)
				.Image(&Theme.NearInvisibleBrush)
			]
		]

		// Date
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(Theme.SmallContentPadding)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.SmallFont)
			.Text(FText::Format(LOCTEXT("TransactionDayFormat", "Transactions by {0} on {1}"),
				Target->GetCompanyName(),
				UFlareGameTools::GetDisplayDate(Time)))
		]

		// Balance
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(Theme.SmallContentPadding)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.SmallFont)
			.Text(FText::Format(LOCTEXT("TransactionBalanceFormat", "({0})"),
				FText::AsNumber(UFlareGameTools::DisplayMoney(Balance))))
			.ColorAndOpacity(Balance >= 0 ? Theme.FriendlyColor : Theme.EnemyColor)
		]

		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.HeightOverride(2)
			[
				SNew(SImage)
				.Image(&Theme.NearInvisibleBrush)
			]
		]
	];
}

void SFlareCompanyMenu::AddTransactionLog(const FFlareTransactionLogEntry& Entry, UFlareCompany* Target, bool EvenIndex)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Get data
	UFlareCompany* Other = Entry.GetOtherCompany(Target->GetGame());
	UFlareSimulatedSector* Sector = Entry.GetSector(Target->GetGame());
	UFlareSimulatedSpacecraft* Source = Entry.GetSpacecraft(Target->GetGame());
	FText Comment = Entry.GetComment(Target->GetGame());

	// Format credit & debit
	FText Credit;
	FText Debit;
	if (Entry.Amount >= 0)
	{
		Credit = FText::AsNumber(UFlareGameTools::DisplayMoney(Entry.Amount));
	}
	else
	{
		Debit = FText::AsNumber(UFlareGameTools::DisplayMoney(Entry.Amount));
	}

	// Add structure
	CompanyLog->InsertSlot(0)
	.AutoHeight()
	[
		SNew(SBorder)
		.BorderImage((EvenIndex ? &Theme.EvenBrush : &Theme.OddBrush))
		[
			SNew(SHorizontalBox)

			// Date
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Left)
			[
				SNew(SBox)
				.WidthOverride(SmallWidth)
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(UFlareGameTools::GetDisplayDate(Entry.Date))
				]
			]

			// Debit
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Left)
			[
				SNew(SBox)
				.WidthOverride(SmallWidth)
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.ColorAndOpacity(Theme.EnemyColor)
					.Text(Debit)
				]
			]

			// Credit
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Left)
			[
				SNew(SBox)
				.WidthOverride(SmallWidth)
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.ColorAndOpacity(Theme.FriendlyColor)
					.Text(Credit)
				]
			]

			// Source
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Left)
			[
				SNew(SBox)
				.WidthOverride(LargeWidth)
				.Padding(Theme.ContentPadding)
				[
					SNew(SFlareButton)
					.Width(8)
					.Text(Source ? UFlareGameTools::DisplaySpacecraftName(Source) : FText())
					.OnClicked(this, &SFlareCompanyMenu::OnTransactionLogSourceClicked, Source)
					.Visibility(Source ? EVisibility::Visible : EVisibility::Hidden)
				]
			]

			// Location
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Left)
			[
				SNew(SBox)
				.WidthOverride(SmallWidth)
				.Padding(Theme.ContentPadding)
				[
					SNew(SFlareButton)
					.Width(4)
					.Text(Sector ? Sector->GetSectorName() : FText())
					.OnClicked(this, &SFlareCompanyMenu::OnTransactionLogSectorClicked, Sector)
					.Visibility(Source ? EVisibility::Visible : EVisibility::Hidden)
				]
			]

			// Partner
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Left)
			[
				SNew(SBox)
				.WidthOverride(SmallWidth)
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(Other ? Other->GetCompanyName() : FText())
					.WrapTextAt(SmallWidth - 2 * Theme.ContentPadding.Left - 2 * Theme.ContentPadding.Right)
				]
			]

			// Comment
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(VeryLargeWidth)
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(Comment)
					.WrapTextAt(VeryLargeWidth - 2 * Theme.ContentPadding.Left - 2 * Theme.ContentPadding.Right)
				]
			]
		]
	];
}

void SFlareCompanyMenu::AddAccountingHeader(FText Text, int64 Balance)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	CompanyAccounting->InsertSlot(0)
	.AutoHeight()
	[
		SNew(SHorizontalBox)
			
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.HeightOverride(2)
			[
				SNew(SImage)
				.Image(&Theme.NearInvisibleBrush)
			]
		]

		// Text
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(Theme.SmallContentPadding)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.SmallFont)
			.Text(Text)
		]

		// Balance
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(Theme.SmallContentPadding)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.SmallFont)
			.Text(FText::Format(LOCTEXT("AccountingBalanceFormat", "({0})"),
				FText::AsNumber(UFlareGameTools::DisplayMoney(Balance))))
			.ColorAndOpacity(Balance >= 0 ? Theme.FriendlyColor : Theme.EnemyColor)
		]			

		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.HeightOverride(2)
			[
				SNew(SImage)
				.Image(&Theme.NearInvisibleBrush)
			]
		]
	];
}

int64 SFlareCompanyMenu::AddAccountingCategory(EFlareTransactionLogEntry::Type Type, TArray<int64>& Balances, UFlareCompany* Target, bool EvenIndex)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FText Category = FFlareTransactionLogEntry::GetCategoryDescription(Type);
	FCHECK(Type < Balances.Num());

	// Format credit & debit
	FText Credit;
	FText Debit;
	if (Balances[Type] >= 0)
	{
		Credit = FText::AsNumber(UFlareGameTools::DisplayMoney(Balances[Type]));
	}
	else
	{
		Debit = FText::AsNumber(UFlareGameTools::DisplayMoney(Balances[Type]));
	}

	// Add structure
	CompanyAccounting->InsertSlot(0)
	.AutoHeight()
	[
		SNew(SBorder)
		.BorderImage((EvenIndex ? &Theme.EvenBrush : &Theme.OddBrush))
		[
			SNew(SHorizontalBox)

			// Category
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Left)
			[
				SNew(SBox)
				.WidthOverride(LargeWidth)
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.ColorAndOpacity(Theme.EnemyColor)
					.Text(Category)
				]
			]

			// Debit
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Left)
			[
				SNew(SBox)
				.WidthOverride(SmallWidth)
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(Debit)
				]
			]

			// Credit
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Left)
			[
				SNew(SBox)
				.WidthOverride(SmallWidth)
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.ColorAndOpacity(Theme.FriendlyColor)
					.Text(Credit)
				]
			]
		]
	];

	return Balances[Type];
}


/*----------------------------------------------------
	Lists
----------------------------------------------------*/

TSharedRef<SWidget> SFlareCompanyMenu::OnGenerateSourceComboLine(UFlareSimulatedSpacecraft* Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(SBox)
	.Padding(Theme.ListContentPadding)
	[
		SNew(STextBlock)
		.Text(Item ? Item->GetNickName() : LOCTEXT("NoFilterBySource", "No filter"))
		.TextStyle(&Theme.TextFont)
	];
}

void SFlareCompanyMenu::OnSourceComboLineSelectionChanged(UFlareSimulatedSpacecraft* Item, ESelectInfo::Type SelectInfo)
{
	CurrentSourceFilter = Item;

	ShowCompanyLog(Company);
}

FText SFlareCompanyMenu::OnGetCurrentSourceComboLine() const
{
	UFlareSimulatedSpacecraft* Item = SourceSelector->GetSelectedItem();

	return Item ? Item->GetNickName() : LOCTEXT("FilterBySource", "Filter by source");
}


TSharedRef<SWidget> SFlareCompanyMenu::OnGenerateSectorComboLine(UFlareSimulatedSector* Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(SBox)
	.Padding(Theme.ListContentPadding)
	[
		SNew(STextBlock)
		.Text(Item ? Item->GetSectorName() : LOCTEXT("NoFilterBySector", "No filter"))
		.TextStyle(&Theme.TextFont)
	];
}

void SFlareCompanyMenu::OnSectorComboLineSelectionChanged(UFlareSimulatedSector* Item, ESelectInfo::Type SelectInfo)
{
	CurrentSectorFilter = Item;

	ShowCompanyLog(Company);
}

FText SFlareCompanyMenu::OnGetCurrentSectorComboLine() const
{
	UFlareSimulatedSector* Item = SectorSelector->GetSelectedItem();
	return Item ? Item->GetSectorName() : LOCTEXT("FilterBySector", "Filter by sector");
}

TSharedRef<SWidget> SFlareCompanyMenu::OnGenerateCompanyComboLine(UFlareCompany* Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	return SNew(SBox)
	.Padding(Theme.ListContentPadding)
	[
		SNew(STextBlock)
		.Text(Item ? Item->GetCompanyName() : LOCTEXT("NoFilterByCompany", "No filter"))
		.TextStyle(&Theme.TextFont)
	];
}

void SFlareCompanyMenu::OnCompanyComboLineSelectionChanged(UFlareCompany* Item, ESelectInfo::Type SelectInfo)
{
	CurrentCompanyFilter = Item;

	ShowCompanyLog(Company);
}

FText SFlareCompanyMenu::OnGetCurrentCompanyComboLine() const
{
	UFlareCompany* Item = CompanySelector->GetSelectedItem();
	return Item ? Item->GetCompanyName() : LOCTEXT("FilterByCompany", "Filter by company");
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

TOptional<int32> SFlareCompanyMenu::GetBlurRadius() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	return (TabView->GetCurrentTabIndex() == 3) ? Theme.BlurRadius : 0;
}

float SFlareCompanyMenu::GetBlurStrength() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	return (TabView->GetCurrentTabIndex() == 3) ? Theme.BlurStrength : 0;
}

bool SFlareCompanyMenu::IsRenameDisabled() const
{
	FString CompanyNameData = CompanyName->GetText().ToString();

	if (CompanyNameData.Len() > 25)
	{
		return true;
	}
	if (CompanyNameData == Company->GetCompanyName().ToString())
	{
		return true;
	}
	else
	{
		return false;
	}
}

void SFlareCompanyMenu::OnRename()
{
	AFlarePlayerController* PC = MenuManager->GetPC();
	FFlareCompanyDescription CompanyDescription = *PC->GetCompanyDescription();
	CompanyDescription.Name = CompanyName->GetText();
	PC->SetCompanyDescription(CompanyDescription);
}

void SFlareCompanyMenu::OnEmblemPicked(int32 Index)
{
	AFlarePlayerController* PC = MenuManager->GetPC();
	UFlareCustomizationCatalog* CustomizationCatalog = MenuManager->GetGame()->GetCustomizationCatalog();

	FFlareCompanyDescription CompanyDescription = *PC->GetCompanyDescription();
	CompanyDescription.Emblem = CustomizationCatalog->GetEmblem(Index);
	PC->SetCompanyDescription(CompanyDescription);
	PC->GetPlayerData()->PlayerEmblemIndex = EmblemPicker->GetSelectedIndex();
	PC->OnLoadComplete();
}

void SFlareCompanyMenu::OnTransactionLogSectorClicked(UFlareSimulatedSector* Sector)
{
	FFlareMenuParameterData Data;
	Data.Sector = Sector;
	MenuManager->OpenMenu(EFlareMenu::MENU_Sector, Data);
}

void SFlareCompanyMenu::OnTransactionLogSourceClicked(UFlareSimulatedSpacecraft* Source)
{
	FFlareMenuParameterData Data;
	Data.Spacecraft = Source;

	if (Source->IsStation())
	{
		MenuManager->OpenMenu(EFlareMenu::MENU_Station, Data);
	}
	else
	{
		MenuManager->OpenMenu(EFlareMenu::MENU_Ship, Data);
	}
}

void SFlareCompanyMenu::OnPreviousYear()
{
	CurrentAccountingYear--;

	ShowCompanyAccounting(Company);
}

bool SFlareCompanyMenu::IsPreviousYearDisabled() const
{
	return (CurrentAccountingYear <= UFlareGameTools::START_YEAR);
}

void SFlareCompanyMenu::OnNextYear()
{
	CurrentAccountingYear++;

	ShowCompanyAccounting(Company);
}

bool SFlareCompanyMenu::IsNextYearDisabled() const
{
	return (CurrentAccountingYear >= CurrentGameYear);
}

FText SFlareCompanyMenu::GetAccountingTitle() const
{
	return FText::Format(LOCTEXT("AccountingYearFormat", "Accounting for year {0}"),
		FText::AsNumber(CurrentAccountingYear + 1));
}

#undef LOCTEXT_NAMESPACE
