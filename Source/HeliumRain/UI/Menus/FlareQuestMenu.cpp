
#include "../../Flare.h"
#include "FlareQuestMenu.h"

#include "../../Game/FlareGame.h"
#include "../../Quests/FlareQuest.h"
#include "../../Quests/FlareQuestStep.h"
#include "../../Quests/FlareQuestCondition.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"

#include "../Components/FlareButton.h"
#include "../Components/FlareObjectiveInfo.h"


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
					.WidthOverride(Theme.ContentWidth)
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.TitlePadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(LOCTEXT("OngoingQuestsTitle", "Ongoing contracts"))
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(OngoingQuestList, SVerticalBox)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.TitlePadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(LOCTEXT("AvailableQuestsTitle", "Available contracts"))
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
							.Text(LOCTEXT("PreviousQuestsTitle", "Previous contracts"))
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
						SAssignNew(QuestDetails, SVerticalBox)
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
	FillOngoingQuestList();
	FillPreviousQuestList();
	FillQuestDetails();

	SlatePrepass(FSlateApplicationBase::Get().GetApplicationScale());
}

void SFlareQuestMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);

	OngoingQuestList->ClearChildren();
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

	// Get list of availabed quests
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
				.HelpText(LOCTEXT("SelectAvailableQuestInfo", "Take a closer look at this contract"))
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
				.HelpText(LOCTEXT("AcceptQuestInfo", "Accept this contract"))
				.OnClicked(this, &SFlareQuestMenu::OnQuestAccepted, Quest)
			]
		];
	}

	// No available quest
	if (AvailableQuests.Num() == 0)
	{
		AvailableQuestList->AddSlot()
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TextFont)
			.Text(LOCTEXT("NoAvailableQuest", "No available contract."))
		];
	}
}

void SFlareQuestMenu::FillOngoingQuestList()
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	FCHECK(QuestManager);

	OngoingQuestList->ClearChildren();
	TArray<UFlareQuest*>& OngoingQuests = QuestManager->GetOngoingQuests();

	// Get list of ongoing quests
	for (int32 QuestIndex = 0; QuestIndex < OngoingQuests.Num(); QuestIndex++)
	{
		UFlareQuest* Quest = OngoingQuests[QuestIndex];

		FText TrackedQuest = (Quest == QuestManager->GetSelectedQuest()) ? LOCTEXT("TrackedQuest", "(Tracked)") : FText();

		OngoingQuestList->AddSlot()
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
				.Text(FText::Format(LOCTEXT("OngoingQuestTitleFormat", "{0} {1}"),
					Quest->GetQuestName(),
					TrackedQuest))
				.HelpText(LOCTEXT("SelectOngoingQuestInfo", "Take a closer look at this contract"))
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
				.HelpText(LOCTEXT("SelectQuestInfo", "Select this contract and track its progress"))
				.OnClicked(this, &SFlareQuestMenu::OnQuestTracked, Quest)
				.IsDisabled(this, &SFlareQuestMenu::IsTrackButtonDisabled, Quest)
			]

			// Abandon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(0))
			[
				SNew(SFlareButton)
				.Transparent(true)
				.Text(FText())
				.HelpText(LOCTEXT("AbandonQuestInfo", "Abandon this contract"))
				.Icon(FFlareStyleSet::GetIcon("Stop"))
				.OnClicked(this, &SFlareQuestMenu::OnQuestAbandoned, Quest)
				.Width(1)
			]
		];
	}

	// No ongoing quest
	if (OngoingQuests.Num() == 0)
	{
		OngoingQuestList->AddSlot()
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TextFont)
			.Text(LOCTEXT("NoOngoingQuest", "No ongoing contract."))
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

		// Don't show expired quests
		if (Quest->GetStatus() == EFlareQuestStatus::ABANDONED)
		{
			continue;
		}

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
				.HelpText(LOCTEXT("SelectPreviousQuestInfo", "Take a closer look at this contract"))
				.OnClicked(this, &SFlareQuestMenu::OnQuestSelected, Quest)
				.Color(this, &SFlareQuestMenu::GetQuestColor, Quest)
			]
		];
	}

	// No previous quest
	if (PreviousQuestList->NumSlots() == 0)
	{
		PreviousQuestList->AddSlot()
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TextFont)
			.Text(LOCTEXT("NoPreviousQuest", "No previous contract."))
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

	// Get selected quest
	if (SelectedQuest)
	{
		UFlareCompany* ClientCompany = SelectedQuest->GetClient();

		// Header
		QuestDetails->AddSlot()
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			[
				SNew(SVerticalBox)

				// Title
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.TitlePadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.SubTitleFont)
					.Text(SelectedQuest->GetQuestName())
				]

				// Description
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.WrapTextAt(0.8 * Theme.ContentWidth)
					.Text(SelectedQuest->GetQuestDescription())
				]

				// Client info
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text((ClientCompany ? FText::Format(LOCTEXT("QuestInfoFormat", "This contract is offered by {0}"), ClientCompany->GetCompanyName()):FText()))
				]
			]

			// Emblem
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(Theme.ContentPadding)
			.VAlign(VAlign_Top)
			[
				SNew(SImage)
				.Image(ClientCompany ? ClientCompany->GetEmblem() : NULL)
			]
		];

		// Get progress info
		CurrentQuestStep = SelectedQuest->GetCurrentStep();

		// List all quest steps
		for (UFlareQuestStep* QuestStep : SelectedQuest->GetSteps())
		{
			FLOGV ("QuestStep end condition count %d", QuestStep->GetEndCondition()->GetAllConditions().Num());

			// Generate condition text
			FText StepConditionsText;
			for(UFlareQuestCondition* Condition : QuestStep->GetEndCondition()->GetAllConditions())
			{
				FLOGV ("condition %s (%d)", *Condition->GetInitialLabel().ToString(), Condition->GetConditionIndex());
				StepConditionsText = FText::Format(LOCTEXT("StepConditionFormat", "{0}{1}{2}"),
					StepConditionsText,
					(Condition->GetConditionIndex() > 0 ? FText::FromString("\n") : FText()),
					Condition->GetInitialLabel());
			}

			// Completed step
			if (QuestStep->IsCompleted())
			{
				TSharedPtr<SVerticalBox> DetailBox = AddQuestDetail(QuestStep);
				bool IsOngoingQuest = (QuestManager->IsQuestOngoing(SelectedQuest));

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
						.Visibility(IsOngoingQuest ? EVisibility::Visible : EVisibility::Hidden)
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
								
				// Detailed progression widget for the current step of the current quest
				if (QuestStep == SelectedQuest->GetCurrentStep() && SelectedQuest->GetStatus() == EFlareQuestStatus::ONGOING)
				{
					// Step description
					DetailBox->AddSlot()
					.AutoHeight()
					.Padding(Theme.SmallContentPadding)
					[
						SNew(STextBlock)
						.WrapTextAt(0.9 * Theme.ContentWidth)
						.TextStyle(&Theme.NameFont)
						.Text(this, &SFlareQuestMenu::GetQuestStepDescription, QuestStep)
					];

					// Condition widget
					DetailBox->AddSlot()
					.AutoHeight()
					.Padding(Theme.SmallContentPadding)
					[
						SNew(SFlareObjectiveInfo)
						.PC(MenuManager->GetPC())
						.Width(0.9 * Theme.ContentWidth)
						.ConditionsOnly(true)
						.QuestStep(QuestStep)
					];
				}

				// Simple info
				else
				{
					// Condition text
					DetailBox->AddSlot()
					.AutoHeight()
					.Padding(Theme.SmallContentPadding)
					[
						SNew(STextBlock)
						.WrapTextAt(0.9 * Theme.ContentWidth)
						.TextStyle(&Theme.NameFont)
						.Text(StepConditionsText)
					];

					// Step description
					DetailBox->AddSlot()
					.AutoHeight()
					.Padding(Theme.SmallContentPadding)
					[
						SNew(STextBlock)
						.WrapTextAt(0.9 * Theme.ContentWidth)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareQuestMenu::GetQuestStepDescription, QuestStep)
					];
				}

				// Stop listing steps at this point if it's an ongoing or pending quest
				if (QuestManager->IsQuestOngoing(SelectedQuest) || QuestManager->IsQuestAvailable(SelectedQuest))
				{
					break;
				}
			}
		}

		// Expiration warning
		if (SelectedQuest->GetStatus() == EFlareQuestStatus::AVAILABLE)
		{
			QuestDetails->AddSlot()
			.AutoHeight()
			.Padding(Theme.TitlePadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.SubTitleFont)
				.Text(LOCTEXT("ExpirationTitle", "Contract expiration"))
			];

			QuestDetails->AddSlot()
			.AutoHeight()
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.WrapTextAt(0.9 * Theme.ContentWidth)
				.TextStyle(&Theme.TextFont)
				.Text(SelectedQuest->GetQuestExpiration())
			];
		}
				
		// Reward & penalty
		QuestDetails->AddSlot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			// Reward
			+ SHorizontalBox::Slot()
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.Padding(Theme.TitlePadding)
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
					[
						SNew(STextBlock)
						.TextStyle(&Theme.SubTitleFont)
						.Text(LOCTEXT("RewardTitle", "Rewards on success"))
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.WrapTextAt(0.4 * Theme.ContentWidth)
					.TextStyle(&Theme.TextFont)
					.Text(SelectedQuest->GetQuestReward())
				]
			]

			// Penalty
			+ SHorizontalBox::Slot()
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.Padding(Theme.TitlePadding)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					[
						SNew(SImage)
						.Image(FFlareStyleSet::GetIcon("Delete"))
					]

					+ SHorizontalBox::Slot()
					[
						SNew(STextBlock)
						.TextStyle(&Theme.SubTitleFont)
						.Text(LOCTEXT("PenaltyTitle", "Penalties on failure"))
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.WrapTextAt(0.4 * Theme.ContentWidth)
					.TextStyle(&Theme.TextFont)
					.Text(SelectedQuest->GetQuestPenalty())
				]
			]
		];
	}

	// No selected quest ?
	else
	{
		QuestDetails->AddSlot()
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TextFont)
			.Text(LOCTEXT("NoSelectedQuest", "No contract selected."))
		];
	}

	SlatePrepass(FSlateApplicationBase::Get().GetApplicationScale());
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

FText SFlareQuestMenu::GetQuestStepDescription(UFlareQuestStep* QuestStep) const
{
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	FCHECK(QuestManager);

	// Hide this for completed quests
	if (SelectedQuest && (CurrentQuestStep == QuestStep || !QuestManager->IsQuestOngoing(SelectedQuest)))
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
	if (SelectedQuest && (CurrentQuestStep == QuestStep || !QuestManager->IsQuestOngoing(SelectedQuest)))
	{
		return EVisibility::Visible;
	}
	else
	{
		return EVisibility::Collapsed;
	}
}

bool SFlareQuestMenu::IsTrackButtonDisabled(UFlareQuest* Quest) const
{
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	FCHECK(QuestManager);

	return (Quest == QuestManager->GetSelectedQuest());
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
			case EFlareQuestStatus::ONGOING:
			case EFlareQuestStatus::SUCCESSFUL:  return Theme.NeutralColor;
			case EFlareQuestStatus::ABANDONED:  
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
	FillOngoingQuestList();
	FillQuestDetails();
}

void SFlareQuestMenu::OnQuestAbandoned(UFlareQuest* Quest)
{
	MenuManager->Confirm(LOCTEXT("ConfirmAbandon", "ARE YOU SURE ?"),
		LOCTEXT("ConfirmAbandonInfo", "Do you really want to abandon this contract ?"),
		FSimpleDelegate::CreateSP(this, &SFlareQuestMenu::OnQuestAbandonedConfirmed, Quest));
}

void SFlareQuestMenu::OnQuestAbandonedConfirmed(UFlareQuest* Quest)
{
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	FCHECK(QuestManager);
	QuestManager->AbandonQuest(Quest);

	FillOngoingQuestList();
	FillPreviousQuestList();
	FillQuestDetails();
}

void SFlareQuestMenu::OnQuestTracked(UFlareQuest* Quest)
{
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	FCHECK(QuestManager);
	QuestManager->SelectQuest(Quest);

	SelectedQuest = Quest;
	FillOngoingQuestList();
	FillQuestDetails();
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
