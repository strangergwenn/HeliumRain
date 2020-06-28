
#include "HeliumRain/UI/Menus/FlareTechnologyMenu.h"
#include "../../Flare.h"

#include "../Components/FlareTechnologyInfo.h"
#include "../Components/FlareTabView.h"

#include "../../Data/FlareTechnologyCatalog.h"
#include "../../Data/FlareScannableCatalog.h"

#include "../../Game/FlareGame.h"
#include "../../Game/FlareCompany.h"
#include "../../Player/FlareMenuManager.h"

#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareTechnologyMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareTechnologyMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	AFlarePlayerController* PC = MenuManager->GetPC();
	
	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SFlareTabView)

		// Technology block
		+ SFlareTabView::Slot()
		.Header(LOCTEXT("TechnologyMainTab", "Technologies"))
		.HeaderHelp(LOCTEXT("TechnologyMainTabInfo", "Technology tree & research status"))
		[
			SNew(SBox)
			.WidthOverride(2.2 * Theme.ContentWidth)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Fill)
			[
				SNew(SHorizontalBox)

				// Info block
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(0.75 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(SVerticalBox)
			
						// Title
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.TitlePadding)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("CompanyTechnologyTitle", "Company technology"))
							.TextStyle(&Theme.SubTitleFont)
						]
			
						// Company info
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(SRichTextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(this, &SFlareTechnologyMenu::GetCompanyTechnologyInfo)
							.DecoratorStyleSet(&FFlareStyleSet::Get())
							.WrapTextAt(0.7 * Theme.ContentWidth)
						]
			
						// Title
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.TitlePadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(this, &SFlareTechnologyMenu::GetTechnologyName)
						]
			
						// Description
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(this, &SFlareTechnologyMenu::GetTechnologyDescription)
							.WrapTextAt(0.7 * Theme.ContentWidth)
						]
			
						// Button
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						.HAlign(HAlign_Left)
						[
							SNew(SFlareButton)
							.Width(6)
							.Icon(FFlareStyleSet::GetIcon("ResearchValue"))
							.Text(this, &SFlareTechnologyMenu::GetTechnologyUnlockText)
							.HelpText(this, &SFlareTechnologyMenu::GetTechnologyUnlockHintText)
							.OnClicked(this, &SFlareTechnologyMenu::OnTechnologyUnlocked)
							.IsDisabled(this, &SFlareTechnologyMenu::IsUnlockDisabled)
						]
					]
				]

				// Tree block
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Top)
				[			
					SNew(SVerticalBox)
			
					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.TitlePadding)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("AvailableTechnologyTitle", "Available technologies"))
						.TextStyle(&Theme.SubTitleFont)
					]

					// Data
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					[
						SAssignNew(TechnologyTree, SScrollBox)
						.Style(&Theme.ScrollBoxStyle)
						.ScrollBarStyle(&Theme.ScrollBarStyle)
					]
				]
			]
		]

		// Scannable block
		+ SFlareTabView::Slot()
		.Header(LOCTEXT("TechnologyScannableTab", "Artifacts"))
		.HeaderHelp(LOCTEXT("TechnologyScannableTabInfo", "Artifacts are found drifting in the world and unlock research data"))
		[
			SNew(SBox)
			.WidthOverride(2.2 * Theme.ContentWidth)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				.HAlign(HAlign_Left)
				.WidthOverride(2.2 * Theme.ContentWidth)
				[
					SNew(SVerticalBox)

					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.TitlePadding)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("CompanyScannableTitle", "Artifacts found"))
						.TextStyle(&Theme.SubTitleFont)
					]

					// Count
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.Text(this, &SFlareTechnologyMenu::GetUnlockedScannableCount)
						.TextStyle(&Theme.TextFont)
						.WrapTextAt(2 * Theme.ContentWidth)
					]

					// Content
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Left)
					[
						SAssignNew(ArtifactList, SVerticalBox)
					]
				]
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareTechnologyMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareTechnologyMenu::Enter()
{
	FLOG("SFlareTechnologyMenu::Enter");

	// Menu data
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	// Sorting criteria for technologies
	struct FSortByLevelCategoryAndCost
	{
		FORCEINLINE bool operator()(const FFlareTechnologyDescription& A, const FFlareTechnologyDescription& B) const
		{
			if (A.Level > B.Level)
			{
				return true;
			}
			else if (A.Level < B.Level)
			{
				return false;
			}
			else if (A.Category < B.Category)
			{
				return true;
			}
			else if (A.Category > B.Category)
			{
				return false;
			}
			else
			{
				if (A.ResearchCost > B.ResearchCost)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
		}
	};

	// List all technologies
	SelectedTechnology = NULL;
	TArray<const FFlareTechnologyDescription*> Technologies;
	for (auto& Entry : MenuManager->GetGame()->GetTechnologyCatalog()->TechnologyCatalog)
	{
		Technologies.Add(&Entry->Data);
	}
	Technologies.Sort(FSortByLevelCategoryAndCost());

	// Add technologies to the tree
	int CurrentLevel = -1;
	TSharedPtr<SHorizontalBox> CurrentLevelRow;
	for (const FFlareTechnologyDescription* Technology : Technologies)
	{
		// Add a new row
		if (Technology->Level != CurrentLevel)
		{
			CurrentLevel = Technology->Level;

			// Row
			TechnologyTree->AddSlot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				[
					SAssignNew(CurrentLevelRow, SHorizontalBox)
				];

			// Row title
			CurrentLevelRow->AddSlot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.Padding(FMargin(0, 0, 0, 10))
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TitleFont)
					.Text(FText::Format(LOCTEXT("CurrentLevelFormat", "{0}"), FText::AsNumber(CurrentLevel)))
					.ColorAndOpacity(this, &SFlareTechnologyMenu::GetTitleTextColor, CurrentLevel)
				];
		}

		// Add entry to the row
		CurrentLevelRow->AddSlot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(SFlareTechnologyInfo)
				.MenuManager(MenuManager)
				.Technology(Technology)
				.OnClicked(FFlareButtonClicked::CreateSP(this, &SFlareTechnologyMenu::OnTechnologySelected, Technology))
			];
	}

	// List artifacts
	ArtifactList->ClearChildren();
	for (FName UnlockedScannableIdentifier : MenuManager->GetPC()->GetUnlockedScannables())
	{
		auto Scannable = MenuManager->GetGame()->GetScannableCatalog()->Get(UnlockedScannableIdentifier);
		FCHECK(Scannable);

		ArtifactList->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(SVerticalBox)

				// Upper line
				+ SVerticalBox::Slot()
				.AutoHeight()
				[						
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					[
						SNew(SImage)
						.Image(FFlareStyleSet::GetIcon("OK"))
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.TextStyle(&Theme.NameFont)
						.Text(Scannable->Name)
					]
				]

				// Lower line
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.SmallContentPadding)
				[						
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(Scannable->Description)
					.WrapTextAt(Theme.ContentWidth)
				]
			];
	}	

	SlatePrepass(FSlateApplicationBase::Get().GetApplicationScale());
}

void SFlareTechnologyMenu::Exit()
{
	SetEnabled(false);

	SelectedTechnology = NULL;
	TechnologyTree->ClearChildren();

	ArtifactList->ClearChildren();

	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

bool SFlareTechnologyMenu::IsUnlockDisabled() const
{
	if (SelectedTechnology)
	{
		FText Unused;
		return !MenuManager->GetPC()->GetCompany()->IsTechnologyAvailable(SelectedTechnology->Identifier, Unused);
	}
	else
	{
		return true;
	}
}

FText SFlareTechnologyMenu::GetCompanyTechnologyInfo() const
{
	UFlareCompany* Company = MenuManager->GetPC()->GetCompany();

	CompanyValue CompanyValue = Company->GetCompanyValue(NULL, false);

	return FText::Format(LOCTEXT("TechnologyCompanyFormat", "\u2022 You can currently research technology up to <HighlightText>level {0}</>.\n\u2022 You have <HighlightText>{1} research</> left to spend in technology.\n\u2022 You have already spent {2} research."),
		FText::AsNumber(Company->GetTechnologyLevel()),
		FText::AsNumber(Company->GetResearchAmount()),
		FText::AsNumber(Company->GetResearchSpent()));
}

FText SFlareTechnologyMenu::GetTechnologyName() const
{
	if (SelectedTechnology)
	{
		return SelectedTechnology->Name;
	}
	else
	{
		return LOCTEXT("TechnologyDetailsTitle", "Technology details");
	}
}

FText SFlareTechnologyMenu::GetTechnologyDescription() const
{
	if (SelectedTechnology)
	{
		return SelectedTechnology->Description;
	}
	else
	{
		return LOCTEXT("NotechnologySelected", "No technology selected.");
	}
}

FSlateColor SFlareTechnologyMenu::GetTitleTextColor(int32 RowLevel) const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	UFlareCompany* Company = MenuManager->GetPC()->GetCompany();

	if (RowLevel <= Company->GetTechnologyLevel())
	{
		return Theme.NeutralColor;
	}
	else
	{
		return Theme.UnknownColor;
	}
}

FText SFlareTechnologyMenu::GetTechnologyUnlockText() const
{
	UFlareCompany* Company = MenuManager->GetPC()->GetCompany();
	FText Unused;

	if (SelectedTechnology && !Company->IsTechnologyAvailable(SelectedTechnology->Identifier, Unused))
	{
		return LOCTEXT("UnlockTechImpossible", "Can't research");
	}
	else
	{
		return LOCTEXT("UnlockTechOK", "Research technology");
	}
}

FText SFlareTechnologyMenu::GetTechnologyUnlockHintText() const
{
	UFlareCompany* Company = MenuManager->GetPC()->GetCompany();
	FText Reason;

	if (SelectedTechnology && !Company->IsTechnologyAvailable(SelectedTechnology->Identifier, Reason))
	{
		return Reason;
	}
	else
	{
		return LOCTEXT("UnlockTechInfo", "Research this technology by spending some of your available budget");
	}
}

void SFlareTechnologyMenu::OnTechnologySelected(const FFlareTechnologyDescription* Technology)
{
	SelectedTechnology = Technology;
}

void SFlareTechnologyMenu::OnTechnologyUnlocked()
{
	if (SelectedTechnology)
	{
		MenuManager->GetPC()->GetCompany()->UnlockTechnology(SelectedTechnology->Identifier);
	}
}

FText SFlareTechnologyMenu::GetUnlockedScannableCount() const
{
	return FText::Format(LOCTEXT("ScannableCountFormat", "You have found {0} out of {1} artifacts. Explore the world and discover new sectors to unlock more research data !"),
		FText::AsNumber(MenuManager->GetPC()->GetUnlockedScannableCount()),
		FText::AsNumber(MenuManager->GetGame()->GetScannableCatalog()->ScannableCatalog.Num()));
}


#undef LOCTEXT_NAMESPACE
