
#include "FlareTechnologyInfo.h"
#include "../../Flare.h"
#include "../Menus/FlareTechnologyMenu.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"

#define LOCTEXT_NAMESPACE "FlareTechnologyInfo"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareTechnologyInfo::Construct(const FArguments& InArgs)
{
	// Params
	MenuManager = InArgs._MenuManager;
	Technology = InArgs._Technology;
	OnClicked = InArgs._OnClicked;

	// Data
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	TSharedPtr<SButton> Button;
	
	// Layout
	ChildSlot
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Top)
	.Padding(FMargin(1))
	[
		SNew(SVerticalBox)
		
		// Header
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.HeightOverride(2)
			[
				SNew(SImage)
				.Image(&Theme.InvertedBrush)
				.ColorAndOpacity(this, &SFlareTechnologyInfo::GetCategoryColor)
			]
		]

		// Content
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			// Button (behaviour only, no display)
			SAssignNew(Button, SButton)
			.OnClicked(this, &SFlareTechnologyInfo::OnButtonClicked)
			.ContentPadding(FMargin(0))
			.ButtonStyle(FCoreStyle::Get(), "NoBorder")
			[
				SNew(SBorder)
				.Padding(FMargin(0))			
				.BorderImage(this, &SFlareTechnologyInfo::GetBackgroundBrush)
				.Padding(Theme.SmallContentPadding)
				[
					SNew(SBox)
					.WidthOverride(4 * Theme.ResourceWidth)
					.HeightOverride(Theme.ResourceHeight)
					.Padding(FMargin(0))
					[
						SNew(SVerticalBox)
						
						// Title
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.SmallContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.NameFont)
							.Text(Technology->Name)
						]
						
						// Cost info
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Right)
						.VAlign(VAlign_Bottom)
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							[
								SNew(SImage)
								.Image(this, &SFlareTechnologyInfo::GetUnlockIcon)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.TextFont)
								.Text(this, &SFlareTechnologyInfo::GetTechnologyCost)
							]
						]
					]
				]
			]
		]
	];

	// Don't intercept clicks if it's not interactive
	if (!OnClicked.IsBound())
	{
		Button->SetVisibility(EVisibility::HitTestInvisible);
	}
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareTechnologyInfo::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SWidget::OnMouseEnter(MyGeometry, MouseEvent);
}

void SFlareTechnologyInfo::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SWidget::OnMouseLeave(MouseEvent);
}

FSlateColor SFlareTechnologyInfo::GetCategoryColor() const
{
	FText Unused;
	FLinearColor Result;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Get color
	if (MenuManager->GetPC()->GetCompany()->IsTechnologyAvailable(Technology->Identifier, Unused))
	{
		switch (Technology->Category)
		{
			case EFlareTechnologyCategory::General:   Result = Theme.InfoColor;      break;
			case EFlareTechnologyCategory::Military:  Result = Theme.EnemyColor;     break;
			case EFlareTechnologyCategory::Economy:   Result = Theme.TradingColor;   break;
		}
	}
	else
	{
		Result = Theme.UnknownColor;
	}

	// Update alpha and return
	Result.A = FFlareStyleSet::GetDefaultTheme().DefaultAlpha;
	return Result;
}

const FSlateBrush* SFlareTechnologyInfo::GetBackgroundBrush() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	if (MenuManager->GetTechnologyMenu()->GetSelectedTechnology() == Technology)
	{
		return &Theme.NearInvisibleBrush;
	}
	else
	{
		return &Theme.BackgroundBrush;
	}
}

const FSlateBrush* SFlareTechnologyInfo::GetUnlockIcon() const
{
	if (MenuManager->GetPC()->GetCompany()->IsTechnologyUnlocked(Technology->Identifier))
	{
		return FFlareStyleSet::GetIcon("Owned");
	}
	else
	{
		return FFlareStyleSet::GetIcon("ResearchValue");
	}
}

FText SFlareTechnologyInfo::GetTechnologyCost() const
{
	if (MenuManager->GetPC()->GetCompany()->IsTechnologyUnlocked(Technology->Identifier))
	{
		return FText();
	}
	else
	{
		return FText::AsNumber(MenuManager->GetPC()->GetCompany()->GetTechnologyCost(Technology));
	}
}

FReply SFlareTechnologyInfo::OnButtonClicked()
{
	if (Technology)
	{
		OnClicked.ExecuteIfBound();
	}

	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE
