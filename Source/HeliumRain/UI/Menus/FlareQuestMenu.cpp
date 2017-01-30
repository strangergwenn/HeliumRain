
#include "../../Flare.h"
#include "FlareQuestMenu.h"

#include "../../Game/FlareGame.h"
#include "../../Quests/FlareQuest.h"
#include "../../Quests/FlareQuestStep.h"
#include "../../Quests/FlareQuestCondition.h"
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
							.Text(LOCTEXT("AvailableQuestsTitle", "Available quests"))
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(AvailableQuestList, SVerticalBox)
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
	FCHECK(QuestManager);

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

	FillAvailableQuestList();
	FillActiveQuestList();
	FillPreviousQuestList();
	FillQuestDetails();

	SlatePrepass(FSlateApplicationBase::Get().GetApplicationScale());
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

void SFlareQuestMenu::FillAvailableQuestList()
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	FCHECK(QuestManager);

	AvailableQuestList->ClearChildren();
	TArray<UFlareQuest*>& AvailableQuests = QuestManager->GetAvailableQuests();

	// Get list of active quests
	for (int32 QuestIndex = 0; QuestIndex < AvailableQuests.Num(); QuestIndex++)
	{
		UFlareQuest* Quest = AvailableQuests[QuestIndex];

		AvailableQuestList->AddSlot()
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
				.Width(10)
				.Text(Quest->GetQuestName())
				.HelpText(LOCTEXT("SelectActiveQuestInfo", "Take a closer look at this quest"))
				.OnClicked(this, &SFlareQuestMenu::OnQuestSelected, Quest)
				.Color(this, &SFlareQuestMenu::GetQuestColor, Quest)
			]

			// Accept button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SFlareButton)
				.Width(2.5)
				.Icon(FFlareStyleSet::GetIcon("OK"))
				.Text(LOCTEXT("AcceptQuest", "Accept"))
				.HelpText(LOCTEXT("AcceptQuestInfo", "Accept this quest"))
				.OnClicked(this, &SFlareQuestMenu::OnQuestAccepted, Quest)
			]
		];
	}

	// No active quest
	if (AvailableQuests.Num() == 0)
	{
		AvailableQuestList->AddSlot()
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TextFont)
			.Text(LOCTEXT("NoAvailableQuest", "No available quest."))
		];
	}
}

void SFlareQuestMenu::FillActiveQuestList()
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	FCHECK(QuestManager);

	ActiveQuestList->ClearChildren();
	TArray<UFlareQuest*>& ActiveQuests = QuestManager->GetActiveQuests();

	// Get list of active quests
	for (int32 QuestIndex = 0; QuestIndex < ActiveQuests.Num(); QuestIndex++)
	{
		UFlareQuest* Quest = ActiveQuests[QuestIndex];

		FText TrackedQuest = (Quest == QuestManager->GetSelectedQuest()) ? LOCTEXT("TrackedQuest", ", tracked") : FText();

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
				.Width(10)
				.Text(FText::Format(LOCTEXT("ActiveQuestTitleFormat", "{0} ({1} / {2}{3})"),
					Quest->GetQuestName(),
					FText::AsNumber(Quest->GetSuccessfullStepCount()),
					FText::AsNumber(Quest->GetStepCount()),
					TrackedQuest))
				.HelpText(LOCTEXT("SelectActiveQuestInfo", "Take a closer look at this quest"))
				.OnClicked(this, &SFlareQuestMenu::OnQuestSelected, Quest)
				.Color(this, &SFlareQuestMenu::GetQuestColor, Quest)
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
				.Visibility(this, &SFlareQuestMenu::GetTrackButtonVisibility, Quest)
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
	FCHECK(QuestManager);

	PreviousQuestList->ClearChildren();
	TArray<UFlareQuest*>& PreviousQuests = QuestManager->GetPreviousQuests();

	// Get list of previous quests
	for (int32 QuestIndex = 0; QuestIndex < PreviousQuests.Num(); QuestIndex++)
	{
		UFlareQuest* Quest = PreviousQuests[QuestIndex];

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
				.Width(10)
				.Text(FText::Format(LOCTEXT("SelectPreviousQuestFormat", "{0} ({1})"),
					Quest->GetQuestName(),
					Quest->GetStatusText()))
				.HelpText(LOCTEXT("SelectPreviousQuestInfo", "Take a closer look at this quest"))
				.OnClicked(this, &SFlareQuestMenu::OnQuestSelected, Quest)
				.Color(this, &SFlareQuestMenu::GetQuestColor, Quest)
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
	FCHECK(QuestManager);

	CurrentQuestStep = NULL;
	QuestDetails->ClearChildren();

	// Get active quest
	if (SelectedQuest)
	{
		// Header
		QuestDetails->AddSlot()
		.Padding(FMargin(0, 0, 0, 20))
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TextFont)
			.Text(SelectedQuest->GetQuestDescription())
		];

		// Get progress info
		CurrentQuestStep = SelectedQuest->GetCurrentStep();

		// List all quest steps
		for (UFlareQuestStep* QuestStep : SelectedQuest->GetSteps())
		{
			// Generate condition text
			FText StepConditionsText;
			for(UFlareQuestCondition* Condition : QuestStep->GetEndConditions())
			{
				StepConditionsText = FText::Format(LOCTEXT("StepConditionFormat", "{0}{1}{2}"),
					StepConditionsText,
					(Condition->GetConditionIndex() > 0 ? FText::FromString("\n") : FText()),
					Condition->GetInitialLabel());
			}

			// Completed step
			if (QuestStep->IsCompleted())
			{
				TSharedPtr<SVerticalBox> DetailBox = AddQuestDetail(QuestStep);
				bool IsActiveQuest = (QuestManager->IsQuestActive(SelectedQuest));

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
					.WrapTextAt(0.9 * Theme.ContentWidth)
					.TextStyle(&Theme.TextFont)
					.Text(this, &SFlareQuestMenu::GetQuestStepDescription, QuestStep)
					.Visibility(this, &SFlareQuestMenu::GetQuestStepDescriptionVisibility, QuestStep)
				];
			}

			// Current step, or steps to a pending / completed quest
			else
			{
				TSharedPtr<SVerticalBox> DetailBox = AddQuestDetail(QuestStep);
				
				// Description
				DetailBox->AddSlot()
				.AutoHeight()
				.Padding(Theme.SmallContentPadding)
				[
					SNew(STextBlock)
					.WrapTextAt(0.9 * Theme.ContentWidth)
					.TextStyle(&Theme.NameFont)
					.Text(StepConditionsText)
				];
				
				// Detailed text
				DetailBox->AddSlot()
				.AutoHeight()
				.Padding(Theme.SmallContentPadding)
				[
					SNew(STextBlock)
					.WrapTextAt(0.9 * Theme.ContentWidth)
					.TextStyle(&Theme.TextFont)
					.Text(this, &SFlareQuestMenu::GetQuestStepDescription, QuestStep)
				];

				// Stop listing steps at this point if it's an active or pending quest
				if (QuestManager->IsQuestActive(SelectedQuest) || QuestManager->IsQuestAvailable(SelectedQuest))
				{
					break;
				}
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

TSharedPtr<SVerticalBox> SFlareQuestMenu::AddQuestDetail(UFlareQuestStep* QuestStep)
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
		.OnClicked(this, &SFlareQuestMenu::OnQuestStepSelected, QuestStep)
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
						.Text(FText::AsNumber(QuestStep->GetStepIndex() + 1))
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
	FCHECK(QuestManager);

	// Get selected quest
	if (QuestManager->IsQuestActive(SelectedQuest))
	{
		return FText::Format(LOCTEXT("SelectedActiveQuestTitleFormat", "{0} ({1} / {2})"),
			SelectedQuest->GetQuestName(),
			FText::AsNumber(SelectedQuest->GetSuccessfullStepCount()),
			FText::AsNumber(SelectedQuest->GetStepCount()));
	}
	else
	{
		return SelectedQuest->GetQuestName();
	}
}

FText SFlareQuestMenu::GetQuestStepDescription(UFlareQuestStep* QuestStep) const
{
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	FCHECK(QuestManager);

	// Hide this for completed quests
	if (SelectedQuest && (CurrentQuestStep == QuestStep || !QuestManager->IsQuestActive(SelectedQuest)))
	{
		return SelectedQuest->FormatTags(QuestStep->GetStepDescription());
	}
	else
	{
		return FText();
	}
}

EVisibility SFlareQuestMenu::GetQuestStepDescriptionVisibility(UFlareQuestStep* QuestStep) const
{
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	FCHECK(QuestManager);

	// Hide this for completed quests
	if (SelectedQuest && (CurrentQuestStep == QuestStep || !QuestManager->IsQuestActive(SelectedQuest)))
	{
		return EVisibility::Visible;
	}
	else
	{
		return EVisibility::Collapsed;
	}
}

EVisibility SFlareQuestMenu::GetTrackButtonVisibility(UFlareQuest* Quest) const
{
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	FCHECK(QuestManager);

	return (Quest == QuestManager->GetSelectedQuest()) ? EVisibility::Hidden : EVisibility::Visible;
}

FSlateColor SFlareQuestMenu::GetQuestColor(UFlareQuest* Quest) const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	FCHECK(QuestManager);

	if (Quest == SelectedQuest)
	{
		return Theme.FriendlyColor;
	}
	else if (Quest == QuestManager->GetSelectedQuest())
	{
		return Theme.ObjectiveColor;
	}
	else
	{
		switch (Quest->GetStatus())
		{
			case EFlareQuestStatus::AVAILABLE:
			case EFlareQuestStatus::ACTIVE:
			case EFlareQuestStatus::SUCCESSFUL:  return Theme.NeutralColor;
			case EFlareQuestStatus::ABANDONNED:  
			case EFlareQuestStatus::FAILED:
			default:                             return Theme.EnemyColor;
		}
	}
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareQuestMenu::OnQuestAccepted(UFlareQuest* Quest)
{
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	FCHECK(QuestManager);
	QuestManager->AcceptQuest(Quest);

	FillAvailableQuestList();
	FillActiveQuestList();
}

void SFlareQuestMenu::OnQuestTracked(UFlareQuest* Quest)
{
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	FCHECK(QuestManager);
	QuestManager->SelectQuest(Quest);
}

void SFlareQuestMenu::OnQuestSelected(UFlareQuest* Quest)
{
	SelectedQuest = Quest;
	FillQuestDetails();
}

FReply SFlareQuestMenu::OnQuestStepSelected(UFlareQuestStep* QuestStep)
{
	CurrentQuestStep = QuestStep;
	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE
