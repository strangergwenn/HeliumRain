
#include "../../Flare.h"
#include "FlareLeaderboardMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareLeaderboardMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareLeaderboardMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	Game = MenuManager->GetGame();

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
				SNew(SImage).Image(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Leaderboard))
			]

			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TitleFont)
				.Text(LOCTEXT("Leaderboard", "LEADERBOARD"))
			]
			
			// Close
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("Close", "Close"))
				.ToolTipText(LOCTEXT("CloseInfo", "Close the menu and go back to the orbital menu"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Exit, true))
				.OnClicked(this, &SFlareLeaderboardMenu::OnExit)
			]
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(200, 20))
		[
			SNew(SImage).Image(&Theme.SeparatorBrush)
		]

		// Company list
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.ContentPadding)
		[
			SNew(SBox)
			.WidthOverride(Theme.ContentWidth)
			[
				SAssignNew(CompanyList, SListView< TSharedPtr<FInterfaceContainer> >)
				.ListItemsSource(&CompanyListData)
				.SelectionMode(ESelectionMode::Single)
				.OnGenerateRow(this, &SFlareLeaderboardMenu::GenerateCompanyInfo)
			]
		]
	];

}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareLeaderboardMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}

void SFlareLeaderboardMenu::Enter()
{
	FLOG("SFlareLeaderboardMenu::Enter");

	// Reset data
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	const TArray<UFlareCompany*>& Companies = Game->GetGameWorld()->GetCompanies();
	
	// Add companies
	CompanyListData.Empty();
	for (int32 Index = 0; Index < Companies.Num(); Index++)
	{
		CompanyListData.AddUnique(FInterfaceContainer::New(Companies[Index]));
	}
	CompanyList->RequestListRefresh();
}

void SFlareLeaderboardMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

TSharedRef<ITableRow> SFlareLeaderboardMenu::GenerateCompanyInfo(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	// Item data
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	const FFlareCompanyDescription* Desc = Item->CompanyPtr->GetDescription();
	const FSlateBrush* Emblem = Item->CompanyPtr->GetEmblem();

	// Widget structure
	return SNew(SFlareListItem, OwnerTable)
		.Content()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SImage)
				.Image(Emblem)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(Desc->Name)
				.TextStyle(&Theme.NameFont)
			]
		];
}

void SFlareLeaderboardMenu::OnExit()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Orbit);
}


#undef LOCTEXT_NAMESPACE

