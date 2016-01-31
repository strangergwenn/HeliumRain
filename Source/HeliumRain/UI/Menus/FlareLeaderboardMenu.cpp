
#include "../../Flare.h"
#include "FlareLeaderboardMenu.h"
#include "../Components/FlareCompanyInfo.h"
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
				.Text(LOCTEXT("Leaderboard", "COMPETITION"))
			]

			// Close
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("GoOrbit", "Orbital map"))
				.HelpText(LOCTEXT("GoOrbitInfo", "Go back to the orbital map"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Orbit, true))
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
		.Padding(Theme.ContentPadding)
		.HAlign(HAlign_Center)
		[
			SNew(SScrollBox)
			.Style(&Theme.ScrollBoxStyle)
			.ScrollBarStyle(&Theme.ScrollBarStyle)

			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
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

	// Sorting rules
	struct FSortByMoney
	{
		FORCEINLINE bool operator()(const TSharedPtr<FInterfaceContainer> PtrA, const TSharedPtr<FInterfaceContainer> PtrB) const
		{
			return (PtrA->CompanyPtr->GetMoney() > PtrB->CompanyPtr->GetMoney());
		}
	};

	// Sort
	CompanyListData.Sort(FSortByMoney());
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
	return SNew(SFlareListItem, OwnerTable)
	.Content()
	[
		SNew(SFlareCompanyInfo)
		.Player(MenuManager->GetPC())
		.Company(Item->CompanyPtr)
	];
}

void SFlareLeaderboardMenu::OnExit()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Orbit);
}


#undef LOCTEXT_NAMESPACE

