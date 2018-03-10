
#include "FlareCompanyMenu.h"
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

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SFlareTabView)

		// Content block
		+ SFlareTabView::Slot()
		.Header(LOCTEXT("CompanyMainTab", "Company"))
		.HeaderHelp(LOCTEXT("CompanyMainTabHelp", "General information about your company"))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.Padding(Theme.TitlePadding)
			.AutoHeight()
			[
				SNew(STextBlock)
				.TextStyle(&Theme.SubTitleFont)
				.Text(LOCTEXT("CompanyInfoTitle", "Company"))
			]

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
			SNew(SBox)
			.WidthOverride(Theme.ContentWidth)
			.HAlign(HAlign_Fill)
			[
				SNew(SVerticalBox)

				// Title
				+ SVerticalBox::Slot()
				.Padding(Theme.TitlePadding)
				.AutoHeight()
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.SubTitleFont)
					.Text(LOCTEXT("CompanyLogTitle", "Transaction log"))
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
						.WidthOverride(SmallWidth)
						.HAlign(HAlign_Left)
						.Padding(Theme.ContentPadding)
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
						.Padding(Theme.ContentPadding)
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
						.WidthOverride(LargeWidth)
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
				.HAlign(HAlign_Fill)
				.AutoHeight()
				[
					SAssignNew(CompanyLog, SVerticalBox)
				]
			]
		]
	];
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

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
	TradeRouteInfo->UpdateTradeRouteList();

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

	// Sub-menus
	ShowProperty(Target);
	ShowCompanyLog(Target);

	// Refresh
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

	/*//DEBUG
	AddTransactionLog(1455,
		MenuManager->GetPC()->GetPlayerShip(),
		MenuManager->GetPC()->GetPlayerShip()->GetCurrentSector(),
		Target,
		Target,
		640000,
		FText::FromString(TEXT("Sold 125 steel")), true);

	AddTransactionLog(1453,
		MenuManager->GetPC()->GetPlayerShip(),
		MenuManager->GetPC()->GetPlayerShip()->GetCurrentSector(),
		Target,
		Target,
		-3400000,
		FText::FromString(TEXT("Bought 63 iron")), false);

	AddTransactionLog(1452,
		MenuManager->GetPC()->GetPlayerShip(),
		MenuManager->GetPC()->GetPlayerShip()->GetCurrentSector(),
		Target,
		Target,
		-13000,
		FText::FromString(TEXT("Bought 12 water")), true);*/
}

void SFlareCompanyMenu::AddTransactionLog(int64 Time, UFlareSimulatedSpacecraft* Source, UFlareSimulatedSector* Sector,
	UFlareCompany* Owner, UFlareCompany* Other, int64 Value, FText Comment, bool EvenIndex)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Format credit & debit
	FText Credit;
	FText Debit;
	if (Value >= 0)
	{
		Debit = FText::AsNumber(UFlareGameTools::DisplayMoney(Value));
	}
	else
	{
		Credit = FText::AsNumber(UFlareGameTools::DisplayMoney(Value));
	}

	// Add structure
	CompanyLog->AddSlot()
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
					.Text(UFlareGameTools::GetDisplayDate(Time))
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
					.ColorAndOpacity(Theme.FriendlyColor)
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
					.ColorAndOpacity(Theme.EnemyColor)
					.Text(Credit)
				]
			]

			// Source
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
					.Text(Source->GetNickName())
					.WrapTextAt(SmallWidth - 2 * Theme.ContentPadding.Left - 2 * Theme.ContentPadding.Right)
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
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(Sector->GetSectorName())
					.WrapTextAt(SmallWidth - 2 * Theme.ContentPadding.Left - 2 * Theme.ContentPadding.Right)
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
					.Text(Other->GetCompanyName())
					.WrapTextAt(SmallWidth - 2 * Theme.ContentPadding.Left - 2 * Theme.ContentPadding.Right)
				]
			]

			// Comment
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(LargeWidth)
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(Comment)
					.WrapTextAt(LargeWidth - 2 * Theme.ContentPadding.Left - 2 * Theme.ContentPadding.Right)
				]
			]
		]
	];
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

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


#undef LOCTEXT_NAMESPACE
