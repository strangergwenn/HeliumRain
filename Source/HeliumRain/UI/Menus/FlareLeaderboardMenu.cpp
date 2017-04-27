
#include "FlareLeaderboardMenu.h"
#include "../../Flare.h"
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
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SVerticalBox)

		// Company list
		+ SVerticalBox::Slot()
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
					.WidthOverride(Theme.ContentWidth * 1.7)
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
	SetVisibility(EVisibility::Collapsed);
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
	struct FSortByValue
	{
		FORCEINLINE bool operator()(const TSharedPtr<FInterfaceContainer> PtrA, const TSharedPtr<FInterfaceContainer> PtrB) const
		{
			return (PtrA->CompanyPtr->GetCompanyValue().TotalValue > PtrB->CompanyPtr->GetCompanyValue().TotalValue);
		}
	};

	// Sort
	CompanyListData.Sort(FSortByValue());
	CompanyList->RequestListRefresh();
}

void SFlareLeaderboardMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
	CompanyListData.Empty();
	CompanyList->RequestListRefresh();
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
		.Rank(CompanyListData.Find(Item) + 1)
	];
}


#undef LOCTEXT_NAMESPACE

