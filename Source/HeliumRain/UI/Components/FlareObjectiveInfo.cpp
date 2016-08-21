
#include "../../Flare.h"
#include "FlareObjectiveInfo.h"
#include "../../Player/FlarePlayerController.h"

#define LOCTEXT_NAMESPACE "FlareObjectiveInfo"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareObjectiveInfo::Construct(const FArguments& InArgs)
{
	// Settings
	PC = InArgs._PC;
	CurrentAlpha = 1;
	ObjectiveEnterTime = 0.5;
	CurrentFadeTime = 0;
	LastObjectiveVersion = -1;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	int32 ObjectiveInfoWidth = 400;
	int32 ObjectiveInfoTextWidth = ObjectiveInfoWidth - Theme.SmallContentPadding.Left - Theme.SmallContentPadding.Right;
	FLinearColor ObjectiveColor = Theme.ObjectiveColor;
	ObjectiveColor.A = FFlareStyleSet::GetDefaultTheme().DefaultAlpha;
	
	// Create the layout
	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Right)
	[
		SNew(SHorizontalBox)

		// Icon
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(10)
			[
				SNew(SImage)
				.Image(&Theme.InvertedBrush)
				.ColorAndOpacity(ObjectiveColor)
				.Visibility(this, &SFlareObjectiveInfo::GetVisibility)
			]
		]

		// Text
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBorder)
			.BorderImage(&Theme.BackgroundBrush)
			.Padding(FMargin(1))
			[
				SNew(SBox)
				.WidthOverride(ObjectiveInfoWidth)
				.Visibility(this, &SFlareObjectiveInfo::GetVisibility)
				.Padding(Theme.SmallContentPadding)
				[
					SNew(SVerticalBox)

					// Header
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.SmallContentPadding)
					[
						SNew(STextBlock)
						.Text(this, &SFlareObjectiveInfo::GetName)
						.WrapTextAt(ObjectiveInfoTextWidth)
						.TextStyle(&Theme.NameFont)
						.ColorAndOpacity(this, &SFlareObjectiveInfo::GetTextColor)
						.ShadowColorAndOpacity(this, &SFlareObjectiveInfo::GetShadowColor)
					]

					// Description
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.SmallContentPadding)
					[
						SNew(STextBlock)
						.Text(this, &SFlareObjectiveInfo::GetDescription)
						.WrapTextAt(ObjectiveInfoTextWidth)
						.TextStyle(&Theme.TextFont)
						.ColorAndOpacity(this, &SFlareObjectiveInfo::GetTextColor)
						.ShadowColorAndOpacity(this, &SFlareObjectiveInfo::GetShadowColor)
					]

					// Conditions
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.SmallContentPadding)
					[
						SAssignNew(ConditionBox, SVerticalBox)
					]
				]
			]
		]
	];
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareObjectiveInfo::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	float Delta = (PC->HasObjective() ? 1 : -1) * (InDeltaTime / ObjectiveEnterTime);
	CurrentFadeTime = FMath::Clamp(CurrentFadeTime + Delta, 0.0f, 1.0f);
	CurrentAlpha = FMath::InterpEaseOut(0.0f, 1.0f, CurrentFadeTime, 2);

	const FFlarePlayerObjective* Objective = PC->GetCurrentObjective();
	if (Objective && Objective->Version != LastObjectiveVersion)
	{
		// Update structure
		FLOGV("SFlareObjectiveInfo::Tick : New objective step %d", Objective->Version);
		LastObjectiveVersion = Objective->Version;
		ConditionBox->ClearChildren();
		const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
		int32 ObjectiveInfoWidth = 400;
		int32 ObjectiveInfoTextWidth = ObjectiveInfoWidth - Theme.ContentPadding.Left - Theme.ContentPadding.Right;

		for (int ConditionIndex = 0; ConditionIndex < Objective->Data.ConditionList.Num(); ConditionIndex++)
		{
			// Update structure
			const FFlarePlayerObjectiveCondition* Condition = &Objective->Data.ConditionList[ConditionIndex];
			ConditionBox->AddSlot()
			.AutoHeight()
			[
				SNew(SVerticalBox)

				// Step description
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

				// Step progress
				+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)

					// Progress bar
					+ SHorizontalBox::Slot()
					[
						SNew(SBox)
						.WidthOverride(ObjectiveInfoWidth / 2)
						.Visibility(this, &SFlareObjectiveInfo::GetProgressVisibility, ConditionIndex)
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
	}
}

EVisibility SFlareObjectiveInfo::GetVisibility() const
{
	return PC->HasObjective() ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SFlareObjectiveInfo::GetName() const
{
	const FFlarePlayerObjective* Objective = PC->GetCurrentObjective();

	if (Objective)
	{
		return FText::Format(LOCTEXT("ObjectiveTextFormat", "{0} ({1}/{2})"),
			Objective->Data.Name, FText::AsNumber(Objective->Data.StepsDone + 1), FText::AsNumber(Objective->Data.StepsCount));
	}
	else
	{
		return LOCTEXT("UnnamedObjective", "Objective");
	}
}

FText SFlareObjectiveInfo::GetDescription() const
{
	const FFlarePlayerObjective* Objective = PC->GetCurrentObjective();
	return (Objective ? Objective->Data.Description : FText());
}

FText SFlareObjectiveInfo::GetInitialLabel(int32 ConditionIndex) const
{
	const FFlarePlayerObjective* Objective = PC->GetCurrentObjective();
	return (Objective && Objective->Data.ConditionList.Num() > ConditionIndex ?
				Objective->Data.ConditionList[ConditionIndex].InitialLabel : FText());
}

FText SFlareObjectiveInfo::GetTerminalLabel(int32 ConditionIndex) const
{
	const FFlarePlayerObjective* Objective = PC->GetCurrentObjective();
	return (Objective && Objective->Data.ConditionList.Num() > ConditionIndex ?
				Objective->Data.ConditionList[ConditionIndex].TerminalLabel : FText());
}

FText SFlareObjectiveInfo::GetCounter(int32 ConditionIndex) const
{
	const FFlarePlayerObjective* Objective = PC->GetCurrentObjective();

	if (!Objective || Objective->Data.ConditionList.Num() <= ConditionIndex)
	{
		return FText();
	}

	const FFlarePlayerObjectiveCondition* Condition = &Objective->Data.ConditionList[ConditionIndex];

	if (Condition->MaxCounter == 0)
	{
		return FText();
	}
	else
	{
		return FText::Format(LOCTEXT("ObjectiveCounterFormat", "{0} / {1}"), FText::AsNumber(Condition->Counter), FText::AsNumber(Condition->MaxCounter));
	}
}

EVisibility SFlareObjectiveInfo::GetCounterVisibility(int32 ConditionIndex) const
{
	const FFlarePlayerObjective* Objective = PC->GetCurrentObjective();
	if (!Objective || Objective->Data.ConditionList.Num() <= ConditionIndex)
	{
		return EVisibility::Collapsed;
	}

	const FFlarePlayerObjectiveCondition* Condition = &Objective->Data.ConditionList[ConditionIndex];
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
	const FFlarePlayerObjective* Objective = PC->GetCurrentObjective();
	if (!Objective || Objective->Data.ConditionList.Num() <= ConditionIndex)
	{
		return 0;
	}

	const FFlarePlayerObjectiveCondition* Condition = &Objective->Data.ConditionList[ConditionIndex];
	if (Condition->MaxCounter == 0)
	{
		return Condition->Progress / Condition->MaxProgress;
	}
	else
	{
		return Condition->Counter / Condition->MaxCounter;
	}
}

EVisibility SFlareObjectiveInfo::GetProgressVisibility(int32 ConditionIndex) const
{
	const FFlarePlayerObjective* Objective = PC->GetCurrentObjective();
	if (!Objective || Objective->Data.ConditionList.Num() <= ConditionIndex)
	{
		return EVisibility::Collapsed;
	}

	const FFlarePlayerObjectiveCondition* Condition = &Objective->Data.ConditionList[ConditionIndex];
	if (Condition->MaxProgress == 0 && Condition->MaxCounter == 0)
	{
		return EVisibility::Collapsed;
	}
	else
	{
		return EVisibility::Visible;
	}
}


#undef LOCTEXT_NAMESPACE
