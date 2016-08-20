
#include "../../Flare.h"
#include "FlareQuestMenu.h"

#include "../../Game/FlareGame.h"
#include "../../Quests/FlareQuest.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"

#include "../Components/FlareButton.h"


#define LOCTEXT_NAMESPACE "FlareQuestMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareQuestMenu::Construct(const FArguments& InArgs)
{
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SVerticalBox)
					
		// Content block
		+ SVerticalBox::Slot()
		.VAlign(VAlign_Top)
		.Padding(Theme.ContentPadding)
		[
			SNew(SHorizontalBox)

			// Quest list
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			[
				SNew(SScrollBox)
				.Style(&Theme.ScrollBoxStyle)
				.ScrollBarStyle(&Theme.ScrollBarStyle)

				+ SScrollBox::Slot()
				[
					SNew(SBox)
					.WidthOverride(0.75 * Theme.ContentWidth)
					.Padding(Theme.ContentPadding)
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(LOCTEXT("PreviousQuestsTitle", "Active quests"))
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(QuestList, SVerticalBox)
						]
					]
				]
			]

			// Tracked quest details
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			[
				SNew(SScrollBox)
				.Style(&Theme.ScrollBoxStyle)
				.ScrollBarStyle(&Theme.ScrollBarStyle)

				+ SScrollBox::Slot()
				[
					SNew(SBox)
					.WidthOverride(Theme.ContentWidth)
					.Padding(Theme.ContentPadding)
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(LOCTEXT("CurrentQuestTitle", "Tracked quest"))
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(QuestDetails, SVerticalBox)
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

void SFlareQuestMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareQuestMenu::Enter(UFlareQuest* Sector)
{
	FLOG("SFlareQuestMenu::Enter");

	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	
	FillQuestList();
	FillQuestDetails();
}

void SFlareQuestMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);

	QuestList->ClearChildren();
	QuestDetails->ClearChildren();
}


/*----------------------------------------------------
	Internal methods
----------------------------------------------------*/

void SFlareQuestMenu::FillQuestList()
{
	QuestList->ClearChildren();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	check(QuestManager);

	// Get list of active quests
	TArray<UFlareQuest*>& ActiveQuests = QuestManager->GetActiveQuests();
	for (int32 QuestIndex = 0; QuestIndex < ActiveQuests.Num(); QuestIndex++)
	{
		UFlareQuest* Quest = ActiveQuests[QuestIndex];
		FFlareQuestProgressSave* ActiveQuestProgress = Quest->Save();
		const FFlareQuestDescription* ActiveQuestDescription = Quest->GetQuestDescription();

		QuestList->AddSlot()
		.Padding(Theme.SmallContentPadding)
		[
			SNew(SHorizontalBox)

			// Select button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SFlareButton)
				.Width(3)
				.Icon(FFlareStyleSet::GetIcon("Travel"))
				.Text(LOCTEXT("SelectQuest", "Track"))
				.HelpText(LOCTEXT("SelectQuestInfo", "Select this quest as the main quest"))
				.OnClicked(this, &SFlareQuestMenu::OnQuestSelected, Quest)
			]

			// Title
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.NameFont)
				.Text(FText::Format(LOCTEXT("QuestTitleFormat", "{0} ({1} / {2})"),
					Quest->GetQuestName(),
					FText::AsNumber(ActiveQuestProgress->SuccessfullSteps.Num() + 1),
					FText::AsNumber(ActiveQuestDescription->Steps.Num())))
			]
		];
	}

	// No active quest
	if (ActiveQuests.Num() == 0)
	{
		QuestDetails->AddSlot()
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TextFont)
			.Text(LOCTEXT("NoActiveQuest", "No active quest."))
		];
	}
}

void SFlareQuestMenu::FillQuestDetails()
{
	QuestDetails->ClearChildren();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	check(QuestManager);

	// Get active quest
	UFlareQuest* ActiveQuest = QuestManager->GetSelectedQuest();
	if (ActiveQuest)
	{
		FFlareQuestProgressSave* ActiveQuestProgress = ActiveQuest->Save();
		const FFlareQuestDescription* ActiveQuestDescription = ActiveQuest->GetQuestDescription();

		// Header
		QuestDetails->AddSlot()
		[
			SNew(STextBlock)
			.TextStyle(&Theme.NameFont)
			.Text(FText::Format(LOCTEXT("QuestTitleFormat", "{0} ({1} / {2})"),
				ActiveQuest->GetQuestName(),
				FText::AsNumber(ActiveQuestProgress->SuccessfullSteps.Num() + 1),
				FText::AsNumber(ActiveQuestDescription->Steps.Num())))
		];

		// List all quest steps
		int32 CompletedSteps = ActiveQuestProgress->SuccessfullSteps.Num();
		for (int32 QuestIndex = 0; QuestIndex < ActiveQuestDescription->Steps.Num(); QuestIndex++)
		{
			const FFlareQuestStepDescription& QuestStep = ActiveQuestDescription->Steps[QuestIndex];

			// Completed step
			if (QuestIndex < CompletedSteps)
			{
				QuestDetails->AddSlot()
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(FText::Format(LOCTEXT("QuestSuccessfulStepFormat", "{0} - {1} (done)"),
						FText::AsNumber(QuestIndex + 1),
						FText::FromName(QuestStep.Identifier)))
				];
			}

			// Current step
			else if (QuestIndex == CompletedSteps)
			{
				QuestDetails->AddSlot()
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(FText::Format(LOCTEXT("QuestSuccessfulStepFormat", "{0} - {1} (current)"),
						FText::AsNumber(QuestIndex + 1),
						FText::FromName(QuestStep.Identifier)))
				];
			}

			// Future steps
			else if (QuestIndex > CompletedSteps)
			{
				QuestDetails->AddSlot()
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(FText::Format(LOCTEXT("QuestSuccessfulStepFormat", "{0} - {1} (todo)"),
						FText::AsNumber(QuestIndex + 1),
						FText::FromName(QuestStep.Identifier)))
				];
			}
		}
	}

	// No active quest ?
	else
	{
		QuestDetails->AddSlot()
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TextFont)
			.Text(LOCTEXT("NoTrackedQuest", "No tracked quest."))
		];
	}
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareQuestMenu::OnQuestSelected(UFlareQuest* Quest)
{
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	check(QuestManager);

	QuestManager->SelectQuest(Quest);
	FillQuestDetails();
}


#undef LOCTEXT_NAMESPACE
