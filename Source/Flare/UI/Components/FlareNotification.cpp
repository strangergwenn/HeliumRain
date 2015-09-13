
#include "../../Flare.h"
#include "FlareNotification.h"
#include "../../Player/FlareMenuManager.h"

#define LOCTEXT_NAMESPACE "FlareNotification"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareNotification::Construct(const FArguments& InArgs)
{
	// Settings
	NotificationFinishDuration = 2.0;
	NotificationScroll = 150;
	NotificationEnterDuration = 0.4;
	NotificationExitDuration = 0.7;
	int32 NotificatioNWidth = 400;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	int32 NotificationTextWidth = NotificatioNWidth - Theme.ContentPadding.Left - Theme.ContentPadding.Right;
	FLinearColor ShadowColor = FLinearColor::Black;
	ShadowColor.A = 0;

	// Args and state
	Lifetime = 0;
	LastHeight = 0;
	CurrentAlpha = 0;
	CurrentMargin = 0;
	Text = InArgs._Text;
	MenuManager = InArgs._MenuManager;
	TargetMenu = InArgs._TargetMenu;
	TargetInfo = InArgs._TargetInfo;
	Tag = InArgs._Tag;
	NotificationTimeout = InArgs._Timeout;
	FLOGV("SFlareNotification::Construct : notifying '%s'", *InArgs._Text.ToString());

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Right)
	[
		SNew(SBox)
		.Padding(this, &SFlareNotification::GetNotificationMargins)
		[
			// Dummy container so that SBox doesn't stop ticking on fade
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
			[
				SAssignNew(Button, SButton)
				.ContentPadding(FMargin(0))
				.ButtonStyle(FCoreStyle::Get(), "NoBorder")
				.OnClicked(this, &SFlareNotification::OnNotificationClicked)
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
							.ColorAndOpacity(this, &SFlareNotification::GetNotificationColor, InArgs._Type)
						]
					]

					// Text
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(NotificatioNWidth)
						[
							SNew(SBorder)
							.BorderImage(&Theme.BackgroundBrush)
							.BorderBackgroundColor(this, &SFlareNotification::GetNotificationBackgroundColor)
							.Padding(Theme.ContentPadding)
							[
								SNew(SVerticalBox)

								// Header
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(Theme.SmallContentPadding)
								[
									SNew(STextBlock)
									.Text(InArgs._Text)
									.WrapTextAt(NotificationTextWidth)
									.TextStyle(&Theme.NameFont)
									.ColorAndOpacity(this, &SFlareNotification::GetNotificationTextColor)
									.ShadowColorAndOpacity(ShadowColor)
								]

								// Info
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
									.Text(InArgs._Info)
									.WrapTextAt(NotificationTextWidth)
									.TextStyle(&Theme.TextFont)
									.ColorAndOpacity(this, &SFlareNotification::GetNotificationTextColor)
									.ShadowColorAndOpacity(ShadowColor)
								]
							]
						]
					]
				]
			]
		]
	];

	SetVisibility(EVisibility::Visible);
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

bool SFlareNotification::IsFinished() const
{
	return (NotificationTimeout > 0 && Lifetime >= NotificationTimeout);
}

bool SFlareNotification::IsDuplicate(const FName& OtherTag) const
{
	return (OtherTag == Tag);
}

void SFlareNotification::Finish(bool Now)
{
	// 1s to finish quietly
	if (NotificationTimeout == 0)
	{
		NotificationTimeout  = Lifetime + (Now ? NotificationExitDuration : NotificationFinishDuration);
	}
	else
	{
		float Timeout = NotificationTimeout - (Now ? NotificationExitDuration : NotificationFinishDuration);
		Lifetime = FMath::Max(Lifetime, Timeout);
	}
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareNotification::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// Update lifetime
	Lifetime += InDeltaTime;
	float Ease = 2;
	float AnimationTime = NotificationExitDuration / 2;
	float TimeToFade = (NotificationTimeout > 0 ? NotificationTimeout - Lifetime - 2 * AnimationTime : 2 * AnimationTime);
	float TimeToRemove = (NotificationTimeout > 0 ? NotificationTimeout - Lifetime - AnimationTime : AnimationTime);

	// Disappear if finished
	if (TimeToFade <= 0 && Button->GetVisibility() == EVisibility::Visible)
	{
		LastHeight = GetDesiredSize().Y;
		Button->SetVisibility(EVisibility::Collapsed);
	}

	// Update render data
	if (Lifetime <= NotificationEnterDuration)
	{
		CurrentAlpha = FMath::InterpEaseOut(0.0f, 1.0f, FMath::Clamp(Lifetime / NotificationEnterDuration, 0.0f, 1.0f), Ease);
		CurrentMargin = NotificationScroll * FMath::InterpEaseOut(1.0f, 0.0f, FMath::Clamp(Lifetime / NotificationEnterDuration, 0.0f, 1.0f), Ease);
	}
	else
	{
		CurrentAlpha = FMath::InterpEaseOut(0.0f, 1.0f, FMath::Clamp(TimeToFade / AnimationTime, 0.0f, 1.0f), Ease);
		CurrentMargin = LastHeight * FMath::InterpEaseOut(0.0f, 1.0f, FMath::Clamp(TimeToRemove / AnimationTime, 0.0f, 1.0f), Ease);
	}
}

FSlateColor SFlareNotification::GetNotificationColor(EFlareNotification::Type Type) const
{
	// Get color
	FLinearColor Result;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	switch (Type)
	{
		case EFlareNotification::NT_General:  Result = Theme.ObjectiveColor; break;
		case EFlareNotification::NT_Help:     Result = Theme.InfoColor;      break;
		case EFlareNotification::NT_Military: Result = Theme.CombatColor;    break;
		case EFlareNotification::NT_Quest:	  Result = Theme.QuestColor;     break;
		case EFlareNotification::NT_Trading:  Result = Theme.TradingColor;   break;
	}

	// Update alpha and return
	Result.A = CurrentAlpha * FFlareStyleSet::GetDefaultTheme().DefaultAlpha;
	return Result;
}

FSlateColor SFlareNotification::GetNotificationTextColor() const
{
	FLinearColor Result = FFlareStyleSet::GetDefaultTheme().NeutralColor;
	Result.A = CurrentAlpha * FFlareStyleSet::GetDefaultTheme().DefaultAlpha;
	return Result;
}

FSlateColor SFlareNotification::GetNotificationBackgroundColor() const
{
	FLinearColor Result = FFlareStyleSet::GetDefaultTheme().NeutralColor;
	Result.A = CurrentAlpha;
	return Result;
}

FMargin SFlareNotification::GetNotificationMargins() const
{
	FMargin Result(0);

	Result.Bottom = CurrentMargin;

	return Result;
}

FReply SFlareNotification::OnNotificationClicked()
{
	// Call if necessary
	if (TargetMenu != EFlareMenu::MENU_None)
	{
		MenuManager->OpenMenu(TargetMenu, TargetInfo);
	}

	// Set the lifetime to "almost done"
	Finish();
	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE
