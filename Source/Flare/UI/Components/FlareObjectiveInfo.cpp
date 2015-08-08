
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
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	int32 ObjectiveInfoWidth = 400;
	int32 ObjectiveInfoTextWidth = ObjectiveInfoWidth - Theme.ContentPadding.Left - Theme.ContentPadding.Right;
	
	// Create the layout
	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Left)
	[
		SNew(SHorizontalBox)

		// Icon
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			.Visibility(this, &SFlareObjectiveInfo::GetVisibility)
			[
				SNew(SImage)
				.Image(FFlareStyleSet::GetIcon("Objective"))
				.ColorAndOpacity(this, &SFlareObjectiveInfo::GetColor)
			]
		]

		// Text
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(ObjectiveInfoWidth)
			.Visibility(this, &SFlareObjectiveInfo::GetVisibility)
			[
				SNew(SVerticalBox)

				// Header
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.Text(this, &SFlareObjectiveInfo::GetName)
					.WrapTextAt(ObjectiveInfoTextWidth)
					.TextStyle(&Theme.NameFont)
					.ColorAndOpacity(this, &SFlareObjectiveInfo::GetTextColor)
					.ShadowColorAndOpacity(this, &SFlareObjectiveInfo::GetShadowColor)
				]

				// Progress bar
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(SProgressBar)
					.Style(&Theme.ProgressBarStyle)
					.Percent(this, &SFlareObjectiveInfo::GetProgress)
					.FillColorAndOpacity(this, &SFlareObjectiveInfo::GetColor)
				]

				// Info
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.Text(this, &SFlareObjectiveInfo::GetInfo)
					.WrapTextAt(ObjectiveInfoTextWidth)
					.TextStyle(&Theme.TextFont)
					.ColorAndOpacity(this, &SFlareObjectiveInfo::GetTextColor)
					.ShadowColorAndOpacity(this, &SFlareObjectiveInfo::GetShadowColor)
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
}

EVisibility SFlareObjectiveInfo::GetVisibility() const
{
	return PC->HasObjective() ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SFlareObjectiveInfo::GetName() const
{
	const FFlarePlayerObjective* Objective = PC->GetCurrentObjective();
	return (Objective ? Objective->Name : FText::FromString(""));
}

FText SFlareObjectiveInfo::GetInfo() const
{
	const FFlarePlayerObjective* Objective = PC->GetCurrentObjective();
	return (Objective ? Objective->Info : FText::FromString(""));
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

TOptional<float> SFlareObjectiveInfo::GetProgress() const
{
	const FFlarePlayerObjective* Objective = PC->GetCurrentObjective();
	return (Objective ? Objective->Progress : 0);
}

#undef LOCTEXT_NAMESPACE
