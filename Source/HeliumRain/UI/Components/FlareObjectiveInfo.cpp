
#include "FlareObjectiveInfo.h"
#include "../../Flare.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Quests/FlareQuestStep.h"

#define LOCTEXT_NAMESPACE "FlareObjectiveInfo"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareObjectiveInfo::Construct(const FArguments& InArgs)
{
	// Args
	PC = InArgs._PC;
	Width = InArgs._Width;
	QuestStep = InArgs._QuestStep;
	ConditionsOnly = InArgs._ConditionsOnly;

	// Settings
	CurrentAlpha = 1;
	ObjectiveEnterTime = 0.5;
	CurrentFadeTime = 0;
	CurrentObjectiveConditionNumber = 0;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	int32 ObjectiveInfoTextWidth = Width - Theme.SmallContentPadding.Left - Theme.SmallContentPadding.Right;
	
	// Create the layout
	ChildSlot
	[
		SNew(SVerticalBox)

		// Header
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(this, &SFlareObjectiveInfo::GetName)
			.WrapTextAt(ObjectiveInfoTextWidth)
			.TextStyle(&Theme.NameFont)
			.ColorAndOpacity(this, &SFlareObjectiveInfo::GetTextColor)
			.ShadowColorAndOpacity(this, &SFlareObjectiveInfo::GetShadowColor)
			.Visibility(ConditionsOnly ? EVisibility::Collapsed : EVisibility::Visible)
		]

		// Conditions
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.SmallContentPadding)
		[
			SAssignNew(ConditionBox, SVerticalBox)
		]
	];

	// Get objective for this quest
	if (QuestStep)
	{
		QuestObjective = FFlarePlayerObjectiveData();
		QuestStep->AddEndConditionObjectives(&QuestObjective);
	}
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareObjectiveInfo::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	int32 ObjectiveInfoTextWidth = Width - Theme.ContentPadding.Left - Theme.ContentPadding.Right;

	float Delta = (PC->HasObjective() ? 1 : -1) * (InDeltaTime / ObjectiveEnterTime);
	CurrentFadeTime = FMath::Clamp(CurrentFadeTime + Delta, 0.0f, 1.0f);
	CurrentAlpha = FMath::InterpEaseOut(0.0f, 1.0f, CurrentFadeTime, 2);

	const FFlarePlayerObjectiveData* Objective = QuestStep ? &QuestObjective : PC->GetCurrentObjective();
	if (Objective && Objective->ConditionList.Num() != CurrentObjectiveConditionNumber)
	{
		ConditionBox->ClearChildren();

		for (int ConditionIndex = 0; ConditionIndex < Objective->ConditionList.Num(); ConditionIndex++)
		{
			// Update structure
			const FFlarePlayerObjectiveCondition* Condition = &Objective->ConditionList[ConditionIndex];
			ConditionBox->AddSlot()
			.AutoHeight()
			[
				SNew(SVerticalBox)

				// Condition description (initial label)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(this, &SFlareObjectiveInfo::GetInitialLabel, ConditionIndex)
					.WrapTextAt(ObjectiveInfoTextWidth)
					.TextStyle(&Theme.TextFont)
					.ColorAndOpacity(this, &SFlareObjectiveInfo::GetTextColor)
					.ShadowColorAndOpacity(this, &SFlareObjectiveInfo::GetShadowColor)
				]

				// Condition progress
				+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)

					// Progress bar
					+ SHorizontalBox::Slot()
					[
						SNew(SBox)
						.WidthOverride(Width / 2)
						.VAlign(VAlign_Center)
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SProgressBar)
							.Style(&Theme.ProgressBarStyle)
							.Percent(this, &SFlareObjectiveInfo::GetProgress, ConditionIndex)
							.FillColorAndOpacity(this, &SFlareObjectiveInfo::GetColor)
						]
					]

					// Counter
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					[
						SNew(STextBlock)
						.Text(this, &SFlareObjectiveInfo::GetCounter, ConditionIndex)
						.WrapTextAt(ObjectiveInfoTextWidth)
						.TextStyle(&Theme.TextFont)
						.ColorAndOpacity(this, &SFlareObjectiveInfo::GetTextColor)
						.ShadowColorAndOpacity(this, &SFlareObjectiveInfo::GetShadowColor)
						.Visibility(this, &SFlareObjectiveInfo::GetCounterVisibility, ConditionIndex)
					]

					// Terminal text
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					[
						SNew(STextBlock)
						.Text(this, &SFlareObjectiveInfo::GetTerminalLabel, ConditionIndex)
						.WrapTextAt(ObjectiveInfoTextWidth)
						.TextStyle(&Theme.TextFont)
						.ColorAndOpacity(this, &SFlareObjectiveInfo::GetTextColor)
						.ShadowColorAndOpacity(this, &SFlareObjectiveInfo::GetShadowColor)
					]
				]
			];
		}

		SlatePrepass(FSlateApplicationBase::Get().GetApplicationScale());
		CurrentObjectiveConditionNumber = Objective->ConditionList.Num();
	}
}

FText SFlareObjectiveInfo::GetName() const
{
	const FFlarePlayerObjectiveData* Objective = PC->GetCurrentObjective();

	if (Objective)
	{
		return Objective->Name;
	}
	else
	{
		return LOCTEXT("UnnamedObjective", "Objective");
	}
}

FText SFlareObjectiveInfo::GetInitialLabel(int32 ConditionIndex) const
{
	const FFlarePlayerObjectiveData* Objective = QuestStep ? &QuestObjective : PC->GetCurrentObjective();

	if (Objective && Objective->ConditionList.Num() > ConditionIndex)
	{
		FText ConditionType;
		if (ConditionIndex != 0)
		{
			ConditionType = Objective->IsAndCondition ? LOCTEXT("ConditionTypeAnd", "AND:") : LOCTEXT("ConditionTypeOr", "OR:");
		}

		return FText::FromString(ConditionType.ToString() + " " + Objective->ConditionList[ConditionIndex].InitialLabel.ToString());
	}
	else
	{
		return FText();
	}
}

FText SFlareObjectiveInfo::GetTerminalLabel(int32 ConditionIndex) const
{
	const FFlarePlayerObjectiveData* Objective = QuestStep ? &QuestObjective : PC->GetCurrentObjective();

	return (Objective && Objective->ConditionList.Num() > ConditionIndex ?
				Objective->ConditionList[ConditionIndex].TerminalLabel : FText());
}

FText SFlareObjectiveInfo::GetCounter(int32 ConditionIndex) const
{
	const FFlarePlayerObjectiveData* Objective = QuestStep ? &QuestObjective : PC->GetCurrentObjective();

	if (!Objective || Objective->ConditionList.Num() <= ConditionIndex)
	{
		return FText();
	}

	const FFlarePlayerObjectiveCondition* Condition = &Objective->ConditionList[ConditionIndex];

	if (Condition->MaxCounter == 0)
	{
		return FText();
	}
	else
	{
		return FText::Format(LOCTEXT("ObjectiveCounterFormat", "{0} / {1} "), FText::AsNumber(Condition->Counter), FText::AsNumber(Condition->MaxCounter));
	}
}

EVisibility SFlareObjectiveInfo::GetCounterVisibility(int32 ConditionIndex) const
{
	const FFlarePlayerObjectiveData* Objective = QuestStep ? &QuestObjective : PC->GetCurrentObjective();

	if (!Objective || Objective->ConditionList.Num() <= ConditionIndex)
	{
		return EVisibility::Collapsed;
	}

	const FFlarePlayerObjectiveCondition* Condition = &Objective->ConditionList[ConditionIndex];
	if (Condition->MaxCounter == 0)
	{
		return EVisibility::Collapsed;
	}
	else
	{
		return EVisibility::Visible;
	}
}

FSlateColor SFlareObjectiveInfo::GetColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.NeutralColor;
	NormalColor.A *= CurrentAlpha;
	return NormalColor;
}

FSlateColor SFlareObjectiveInfo::GetTextColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.NeutralColor;
	NormalColor.A *= Theme.DefaultAlpha * CurrentAlpha;
	return NormalColor;
}

FLinearColor SFlareObjectiveInfo::GetShadowColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.SmallFont.ShadowColorAndOpacity;
	NormalColor.A *= Theme.DefaultAlpha * CurrentAlpha;
	return NormalColor;
}

TOptional<float> SFlareObjectiveInfo::GetProgress(int32 ConditionIndex) const
{
	const FFlarePlayerObjectiveData* Objective = QuestStep ? &QuestObjective : PC->GetCurrentObjective();

	if (!Objective || Objective->ConditionList.Num() <= ConditionIndex)
	{
		return 0;
	}

	const FFlarePlayerObjectiveCondition* Condition = &Objective->ConditionList[ConditionIndex];
	if (Condition->MaxCounter == 0)
	{
		return Condition->Progress / Condition->MaxProgress;
	}
	else
	{
		return (float) Condition->Counter / (float) Condition->MaxCounter;
	}
}


#undef LOCTEXT_NAMESPACE
