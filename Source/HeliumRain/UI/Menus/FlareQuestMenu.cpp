
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

			// Selected quest details
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
							.Text(this, &SFlareQuestMenu::GetSelectedQuestTitle)
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

void SFlareQuestMenu::Enter(UFlareQuest* TargetQuest)
{
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	check(QuestManager);

	if (TargetQuest)
	{
		SelectedQuest = TargetQuest;
		FLOGV("SFlareQuestMenu::Enter : got param quest '%s'", *SelectedQuest->GetQuestName().ToString());
	}
	else if (QuestManager->GetSelectedQuest())
	{
		SelectedQuest = QuestManager->GetSelectedQuest();
		FLOGV("SFlareQuestMenu::Enter : got tracked quest '%s'", *SelectedQuest->GetQuestName().ToString());
	}
	else
	{
		SelectedQuest = NULL;
		FLOG("SFlareQuestMenu::Enter : no quest");
	}

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

	SelectedQuest = NULL;
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
		.HAlign(HAlign_Left)
		[
			SNew(SHorizontalBox)

			// Title
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(Theme.SmallContentPadding)
			[
				SNew(SFlareButton)
				.Width(8.4)
				.Text(FText::Format(LOCTEXT("ActiveQuestTitleFormat", "{0} ({1} / {2})"),
					Quest->GetQuestName(),
					FText::AsNumber(QuestProgress->SuccessfullSteps.Num() + 1),
					FText::AsNumber(QuestDescription->Steps.Num())))
				.HelpText(LOCTEXT("SelectActiveQuestInfo", "Take a closer look at this quest"))
				.OnClicked(this, &SFlareQuestMenu::OnQuestSelected, Quest)
				.IsDisabled(this, &SFlareQuestMenu::IsSelectQuestButtonDisabled, Quest)
			]

			// Select button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SFlareButton)
				.Width(2.5)
				.Icon(FFlareStyleSet::GetIcon("Travel"))
				.Text(LOCTEXT("SelectQuest", "Track"))
				.HelpText(LOCTEXT("SelectQuestInfo", "Activate this quest and track its progress"))
				.OnClicked(this, &SFlareQuestMenu::OnQuestTracked, Quest)
				.IsDisabled(this, &SFlareQuestMenu::IsTrackQuestButtonDisabled, Quest)
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
		.HAlign(HAlign_Left)
		[
			SNew(SHorizontalBox)
			
			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(SFlareButton)
				.Width(12)
				.Text(FText::Format(LOCTEXT("SelectPreviousQuestFormat", "{0} ({1})"),
					Quest->GetQuestName(),
					Quest->GetStatusText()))
				.HelpText(LOCTEXT("SelectPreviousQuestInfo", "Take a closer look at this quest"))
				.OnClicked(this, &SFlareQuestMenu::OnQuestSelected, Quest)
				.IsDisabled(this, &SFlareQuestMenu::IsSelectQuestButtonDisabled, Quest)
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

	CurrentQuestStepIndex = 0;
	QuestDetails->ClearChildren();

	// Get active quest
	if (SelectedQuest)
	{
		const FFlareQuestDescription* SelectedQuestDescription = SelectedQuest->GetQuestDescription();
		bool IsActiveQuest = (QuestManager->IsQuestActive(SelectedQuestDescription->Identifier));

		// Header
		QuestDetails->AddSlot()
		.Padding(FMargin(0, 0, 0, 20))
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TextFont)
			.Text(SelectedQuestDescription->QuestDescription)
		];

		// Get progress info
		FFlareQuestProgressSave* SelectedQuestProgress = SelectedQuest->Save();
		int32 CompletedSteps = IsActiveQuest ? SelectedQuestProgress->SuccessfullSteps.Num() : SelectedQuestDescription->Steps.Num();
		CurrentQuestStepIndex = CompletedSteps;

		// List all quest steps
		for (int32 QuestStepIndex = 0; QuestStepIndex < SelectedQuestDescription->Steps.Num(); QuestStepIndex++)
		{
			const FFlareQuestStepDescription& QuestStep = SelectedQuestDescription->Steps[QuestStepIndex];
			FFlarePlayerObjectiveData QuestStepData;
			SelectedQuest->AddConditionObjectives(&QuestStepData, QuestStep.EndConditions);

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
			if (QuestStepIndex < CompletedSteps)
			{
				TSharedPtr<SVerticalBox> DetailBox = AddQuestDetail(QuestStepIndex);

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
						.Visibility(IsActiveQuest ? EVisibility::Visible : EVisibility::Hidden)
					]
				];
				
				// Detailed text
				DetailBox->AddSlot()
				.AutoHeight()
				.Padding(Theme.SmallContentPadding)
				[
					SNew(STextBlock)
					.WrapTextAt(0.5 * Theme.ContentWidth)
					.TextStyle(&Theme.TextFont)
					.Text(this, &SFlareQuestMenu::GetQuestStepDescription, QuestStepIndex)
					.Visibility(this, &SFlareQuestMenu::GetQuestStepDescriptionVisibility, QuestStepIndex)
				];
			}

			// Current step
			else if (QuestStepIndex == CompletedSteps)
			{
				TSharedPtr<SVerticalBox> DetailBox = AddQuestDetail(QuestStepIndex);
				
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
					.Text(this, &SFlareQuestMenu::GetQuestStepDescription, QuestStepIndex)
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
			.Text(LOCTEXT("NoSelectedQuest", "No quest selected."))
		];
	}
}

TSharedPtr<SVerticalBox> SFlareQuestMenu::AddQuestDetail(int32 QuestStepIndex)
{
	TSharedPtr<SVerticalBox> Temp;

	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor ObjectiveColor = Theme.ObjectiveColor;
	ObjectiveColor.A = FFlareStyleSet::GetDefaultTheme().DefaultAlpha;

	QuestDetails->AddSlot()
	.AutoHeight()
	.Padding(Theme.SmallContentPadding)
	[
		SNew(SButton)
		.ContentPadding(FMargin(0))
		.ButtonStyle(FCoreStyle::Get(), "NoBorder")
		.OnClicked(this, &SFlareQuestMenu::OnQuestStepSelected, QuestStepIndex)
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
						.Text(FText::AsNumber(QuestStepIndex + 1))
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
		]
	];

	return Temp;
}


/*----------------------------------------------------
	Content callbacks
----------------------------------------------------*/

FText SFlareQuestMenu::GetSelectedQuestTitle() const
{
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	check(QuestManager);

	// Get selected quest
	if (SelectedQuest && SelectedQuest->GetCurrentStepDescription())
	{
		const FFlareQuestDescription* SelectedQuestDescription = SelectedQuest->GetQuestDescription();

		if (QuestManager->IsQuestActive(SelectedQuestDescription->Identifier))
		{
			FFlareQuestProgressSave* SelectedQuestProgress = SelectedQuest->Save();

			return FText::Format(LOCTEXT("SelectedActiveQuestTitleFormat", "Selected quest : {0} ({1} / {2})"),
				SelectedQuest->GetQuestName(),
				FText::AsNumber(SelectedQuestProgress->SuccessfullSteps.Num() + 1),
				FText::AsNumber(SelectedQuestDescription->Steps.Num()));
		}
		else
		{
			return FText::Format(LOCTEXT("SelectedQuestTitleFormat", "Selected quest : {0}"),
				SelectedQuest->GetQuestName());
		}
	}
	else
	{
		return LOCTEXT("SelectedQuestTitleEmpty", "Selected quest");
	}

	return FText();
}

FText SFlareQuestMenu::GetQuestStepDescription(int32 QuestStepIndex) const
{
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	check(QuestManager);

	// Only show this for the currently selected quest step
	if (SelectedQuest && CurrentQuestStepIndex == QuestStepIndex)
	{
		const FFlareQuestDescription* CurrentQuestDescription = SelectedQuest->GetQuestDescription();
		check(CurrentQuestDescription);
		check(QuestStepIndex >= 0 && QuestStepIndex < CurrentQuestDescription->Steps.Num());
				
		const FFlareQuestStepDescription& QuestStep = CurrentQuestDescription->Steps[QuestStepIndex];
		return SelectedQuest->FormatTags(QuestStep.StepDescription);
	}

	return FText();
}

EVisibility SFlareQuestMenu::GetQuestStepDescriptionVisibility(int32 QuestStepIndex) const
{
	return (CurrentQuestStepIndex == QuestStepIndex ? EVisibility::Visible : EVisibility::Collapsed);
}

bool SFlareQuestMenu::IsTrackQuestButtonDisabled(UFlareQuest* Quest) const
{
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	check(QuestManager);

	return (Quest == QuestManager->GetSelectedQuest());
}

bool SFlareQuestMenu::IsSelectQuestButtonDisabled(UFlareQuest* Quest) const
{
	return (Quest == SelectedQuest);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareQuestMenu::OnQuestTracked(UFlareQuest* Quest)
{
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	check(QuestManager);
	QuestManager->SelectQuest(Quest);

	OnQuestSelected(Quest);
}

void SFlareQuestMenu::OnQuestSelected(UFlareQuest* Quest)
{
	SelectedQuest = Quest;
	FillQuestDetails();
}

FReply SFlareQuestMenu::OnQuestStepSelected(int32 QuestStepIndex)
{
	CurrentQuestStepIndex = QuestStepIndex;
	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE
