
#include "../../Flare.h"
#include "FlareGroupStatus.h"
#include "../Components/FlareRoundButton.h"
#include "../../Game/FlareGameTypes.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Game/AI/FlareCompanyAI.h"

#define LOCTEXT_NAMESPACE "FlareGroupStatus"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareGroupStatus::Construct(const FArguments& InArgs)
{
	// Args
	PC = InArgs._PC;
	TargetShipGroup = InArgs._TargetShipGroup;

	// Setup
	GroupHealth = 1.0;
	CurrentAlpha = 0;
	FadeInTime = 0.5f;
	FadeOutTime = 1.0f;

	// Content
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	const FSlateBrush* Icon = UFlareGameTypes::GetCombatGroupIcon(TargetShipGroup);

	// Structure
	ChildSlot
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Left)
	.Padding(Theme.ContentPadding)
	[
		SNew(SHorizontalBox)

		// Button
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SFlareRoundButton)
			.ShowText(false)
			.InvertedBackground(true)
			.Icon(Icon)
			.IconColor(this, &SFlareGroupStatus::GetIconColor)
			.HighlightColor(this, &SFlareGroupStatus::GetHighlightColor)
		]
		
		// Text
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(Theme.ContentPadding)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.SmallFont)
			.Text(this, &SFlareGroupStatus::GetText)
			.ColorAndOpacity(this, &SFlareGroupStatus::GetTextColor)
			.ShadowColorAndOpacity(this, &SFlareGroupStatus::GetShadowColor)
		]
	];
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareGroupStatus::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// Get the selection state for this Group
	float IsSelected = false;
	float IsSelecting = false;
	if (PC)
	{
		UFlareTacticManager* TacticManager = PC->GetCompany()->GetTacticManager();
		IsSelected = (TacticManager->GetCurrentShipGroup() == TargetShipGroup);
		IsSelecting = PC->IsSelectingWeapon();
	}

	// Update animation state
	if (IsSelected)
	{
		CurrentAlpha += InDeltaTime / FadeInTime;
	}
	else if (IsSelecting)
	{
		if (CurrentAlpha >= 0.5f)
		{
			CurrentAlpha -= InDeltaTime / FadeOutTime;
		}
		else
		{
			CurrentAlpha += InDeltaTime / FadeInTime;
			CurrentAlpha = FMath::Clamp(CurrentAlpha, 0.0f, 0.5f);
		}
	}
	else
	{
		CurrentAlpha -= InDeltaTime / FadeOutTime;
	}
	CurrentAlpha = FMath::Clamp(CurrentAlpha, 0.0f, 1.0f);
}

FText SFlareGroupStatus::GetText() const
{
	FText Text;

	UFlareTacticManager* TacticManager = PC->GetCompany()->GetTacticManager();
	int32 TargetShipGroupCount = TacticManager->GetShipCountForShipGroup(TargetShipGroup);
	EFlareCombatTactic::Type TargetShipGroupOrder = TacticManager->GetCurrentTacticForShipGroup(TargetShipGroup);
	
	// Final string
	Text = FText::Format(LOCTEXT("GroupInfoFormat", "{0} ({1} ships)\nCurrent order : {2}\n"),
		UFlareGameTypes::GetCombatGroupDescription(TargetShipGroup),
		FText::AsNumber(TargetShipGroupCount),
		UFlareGameTypes::GetCombatTacticDescription(TargetShipGroupOrder));

	return Text;
}

FSlateColor SFlareGroupStatus::GetHighlightColor() const
{
	FLinearColor Color = FFlareStyleSet::GetHealthColor(GroupHealth, true);
	Color.A *= CurrentAlpha;
	return Color;
}

FSlateColor SFlareGroupStatus::GetIconColor() const
{
	FLinearColor NormalColor = FLinearColor::White;
	NormalColor.A *= CurrentAlpha;
	return NormalColor;
}

FSlateColor SFlareGroupStatus::GetTextColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.NeutralColor;
	NormalColor.A *= Theme.DefaultAlpha * CurrentAlpha;
	return NormalColor;
}

FLinearColor SFlareGroupStatus::GetShadowColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.SmallFont.ShadowColorAndOpacity;
	NormalColor.A *= Theme.DefaultAlpha * CurrentAlpha;
	return NormalColor;
}


#undef LOCTEXT_NAMESPACE
