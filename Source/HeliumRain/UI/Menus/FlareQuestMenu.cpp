
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
					.WidthOverride(Theme.ContentWidth)
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.TitlePadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(LOCTEXT("ActiveQuestsTitle", "Active quests"))
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(ActiveQuestList, SVerticalBox)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.TitlePadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(LOCTEXT("PreviousQuestsTitle", "Previous quests"))
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(PreviousQuestList, SVerticalBox)
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
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.TitlePadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(this, &SFlareQuestMenu::GetActiveQuestTitle)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
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

	FillActiveQuestList();
	FillPreviousQuestList();
	FillQuestDetails();
}

void SFlareQuestMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);

	ActiveQuestList->ClearChildren();
	PreviousQuestList->ClearChildren();
	QuestDetails->ClearChildren();
}


/*----------------------------------------------------
	Internal methods
----------------------------------------------------*/

void SFlareQuestMenu::FillActiveQuestList()
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	check(QuestManager);

	ActiveQuestList->ClearChildren();
	TArray<UFlareQuest*>& ActiveQuests = QuestManager->GetActiveQuests();

	// Get list of active quests
	for (int32 QuestIndex = 0; QuestIndex < ActiveQuests.Num(); QuestIndex++)
	{
		UFlareQuest* Quest = ActiveQuests[QuestIndex];
		FFlareQuestProgressSave* QuestProgress = Quest->Save();
		const FFlareQuestDescription* QuestDescription = Quest->GetQuestDescription();

		ActiveQuestList->AddSlot()
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
				.IsDisabled(this, &SFlareQuestMenu::GetTrackQuestVisibility, Quest)
			]

			// Title
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.NameFont)
				.Text(FText::Format(LOCTEXT("ActiveQuestTitleFormat", "{0} ({1} / {2})"),
					Quest->GetQuestName(),
					FText::AsNumber(QuestProgress->SuccessfullSteps.Num() + 1),
					FText::AsNumber(QuestDescription->Steps.Num())))
			]
		];
	}

	// No active quest
	if (ActiveQuests.Num() == 0)
	{
		ActiveQuestList->AddSlot()
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TextFont)
			.Text(LOCTEXT("NoActiveQuest", "No active quest."))
		];
	}
}

void SFlareQuestMenu::FillPreviousQuestList()
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	check(QuestManager);

	PreviousQuestList->ClearChildren();
	TArray<UFlareQuest*>& PreviousQuests = QuestManager->GetPreviousQuests();

	// Get list of previous quests
	for (int32 QuestIndex = 0; QuestIndex < PreviousQuests.Num(); QuestIndex++)
	{
		UFlareQuest* Quest = PreviousQuests[QuestIndex];
		FFlareQuestProgressSave* QuestProgress = Quest->Save();
		const FFlareQuestDescription* QuestDescription = Quest->GetQuestDescription();
		
		PreviousQuestList->AddSlot()
		.Padding(Theme.SmallContentPadding)
		[
			SNew(SHorizontalBox)
			
			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TextFont)
				.Text(FText::Format(LOCTEXT("PreviousQuestTitleFormat", "{0} ({1})"),
					Quest->GetQuestName(),
					Quest->GetStatusText()))
			]
		];
	}

	// No previous quest
	if (PreviousQuests.Num() == 0)
	{
		PreviousQuestList->AddSlot()
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TextFont)
			.Text(LOCTEXT("NoPreviousQuest", "No previous quest."))
		];
	}
}

void SFlareQuestMenu::FillQuestDetails()
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	check(QuestManager);

	QuestDetails->ClearChildren();

	// Get active quest
	UFlareQuest* ActiveQuest = QuestManager->GetSelectedQuest();
	if (ActiveQuest)
	{
		FFlareQuestProgressSave* ActiveQuestProgress = ActiveQuest->Save();
		const FFlareQuestDescription* ActiveQuestDescription = ActiveQuest->GetQuestDescription();

		// Header
		QuestDetails->AddSlot()
		.Padding(Theme.ContentPadding)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TextFont)
			.Text(ActiveQuestDescription->QuestDescription)
		];

		// List all quest steps
		int32 CompletedSteps = ActiveQuestProgress->SuccessfullSteps.Num();
		for (int32 QuestIndex = 0; QuestIndex < ActiveQuestDescription->Steps.Num(); QuestIndex++)
		{
			const FFlareQuestStepDescription& QuestStep = ActiveQuestDescription->Steps[QuestIndex];
			FFlarePlayerObjectiveData QuestStepData;
			ActiveQuest->AddConditionObjectives(&QuestStepData, QuestStep.EndConditions);

			// Generate condition text
			FText StepConditionsText;
			for (int ConditionIndex = 0; ConditionIndex < QuestStepData.ConditionList.Num(); ConditionIndex++)
			{
				StepConditionsText = FText::Format(LOCTEXT("StepConditionFormat", "{0}{1}{2}"),
					StepConditionsText,
					(ConditionIndex > 0 ? FText::FromString("\n") : FText()),
					QuestStepData.ConditionList[ConditionIndex].InitialLabel);
			}

			// Completed step
			if (QuestIndex < CompletedSteps)
			{
				TSharedPtr<SVerticalBox> DetailBox = AddQuestDetail(QuestIndex);

				// Description
				DetailBox->AddSlot()
				.AutoHeight()
				.Padding(Theme.SmallContentPadding)
				[
					SNew(SHorizontalBox)

					// Text
					+ SHorizontalBox::Slot()
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(StepConditionsText)
					]
					
					// Icon
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					[
						SNew(SImage)
						.Image(FFlareStyleSet::GetIcon("OK"))
					]
				];
			}

			// Current step
			else if (QuestIndex == CompletedSteps)
			{
				TSharedPtr<SVerticalBox> DetailBox = AddQuestDetail(QuestIndex);
				
				// Description
				DetailBox->AddSlot()
				.AutoHeight()
				.Padding(Theme.SmallContentPadding)
				[
					SNew(STextBlock)
					.WrapTextAt(0.5 * Theme.ContentWidth)
					.TextStyle(&Theme.NameFont)
					.Text(StepConditionsText)
				];
				
				// Detailed text
				DetailBox->AddSlot()
				.AutoHeight()
				.Padding(Theme.SmallContentPadding)
				[
					SNew(STextBlock)
					.WrapTextAt(0.5 * Theme.ContentWidth)
					.TextStyle(&Theme.TextFont)
					.Text(this, &SFlareQuestMenu::GetActiveQuestDescription)
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

TSharedPtr<SVerticalBox> SFlareQuestMenu::AddQuestDetail(int32 QuestIndex)
{
	TSharedPtr<SVerticalBox> Temp;

	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor ObjectiveColor = Theme.ObjectiveColor;
	ObjectiveColor.A = FFlareStyleSet::GetDefaultTheme().DefaultAlpha;

	QuestDetails->AddSlot()
	.AutoHeight()
	.Padding(Theme.SmallContentPadding)
	[
		SNew(SHorizontalBox)

		// Icon
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(40)
			[
				SNew(SBorder)
				.BorderImage(&Theme.InvertedBrush)
				.BorderBackgroundColor(ObjectiveColor)
				.Padding(Theme.SmallContentPadding)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.NameFont)
					.Text(FText::AsNumber(QuestIndex + 1))
				]
			]
		]

		// Text
		+ SHorizontalBox::Slot()
		[
			SNew(SBorder)
			.BorderImage(&Theme.BackgroundBrush)
			.Padding(Theme.SmallContentPadding)
			[
				SAssignNew(Temp, SVerticalBox)
			]
		]
	];

	return Temp;
}


/*----------------------------------------------------
	Content callbacks
----------------------------------------------------*/

FText SFlareQuestMenu::GetActiveQuestTitle() const
{
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	check(QuestManager);

	// Get active quest
	UFlareQuest* ActiveQuest = QuestManager->GetSelectedQuest();
	if (ActiveQuest && ActiveQuest->GetCurrentStepDescription())
	{
		FFlareQuestProgressSave* ActiveQuestProgress = ActiveQuest->Save();
		const FFlareQuestDescription* ActiveQuestDescription = ActiveQuest->GetQuestDescription();

		return FText::Format(LOCTEXT("TrackedQuestTitleFormat", "Tracked quest : {0} ({1} / {2})"),
			ActiveQuest->GetQuestName(),
			FText::AsNumber(ActiveQuestProgress->SuccessfullSteps.Num() + 1),
			FText::AsNumber(ActiveQuestDescription->Steps.Num()));
	}
	else
	{
		return LOCTEXT("TrackedQuestTitleEmpty", "Tracked quest");
	}

	return FText();
}

FText SFlareQuestMenu::GetActiveQuestDescription() const
{
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	check(QuestManager);

	// Get active quest
	UFlareQuest* ActiveQuest = QuestManager->GetSelectedQuest();
	if (ActiveQuest && ActiveQuest->GetCurrentStepDescription())
	{
		return ActiveQuest->FormatTags(ActiveQuest->GetCurrentStepDescription()->StepDescription);
	}

	return FText();
}

bool SFlareQuestMenu::GetTrackQuestVisibility(UFlareQuest*  Quest) const
{
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	check(QuestManager);

	return (Quest == QuestManager->GetSelectedQuest());
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
