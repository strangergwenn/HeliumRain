
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
	ForcedLife = false;
	LastHeight = 0;
	CurrentAlpha = 0;
	CurrentMargin = 0;
	Text = InArgs._Text;
	MenuManager = InArgs._MenuManager;
	Notifier = InArgs._Notifier;
	TargetMenu = InArgs._TargetMenu;
	TargetInfo = InArgs._TargetInfo;
	Tag = InArgs._Tag;
	NotificationTimeout = InArgs._Pinned ? 0 : 7.0f;
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
							.Padding(Theme.SmallContentPadding)
							[
								SNew(SVerticalBox)

								// Header
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SHorizontalBox)

									// Title
									+ SHorizontalBox::Slot()
									.HAlign(HAlign_Fill)
									.Padding(Theme.SmallContentPadding)
									[
										SNew(STextBlock)
										.Text(InArgs._Text)
										.WrapTextAt(NotificationTextWidth)
										.TextStyle(&Theme.NameFont)
										.ColorAndOpacity(this, &SFlareNotification::GetNotificationTextColor)
										.ShadowColorAndOpacity(ShadowColor)
									]

									// Close button
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.HAlign(HAlign_Right)
									[
										SNew(SFlareButton)
										.Width(1)
										.Transparent(true)
										.Text(FText())
										.HelpText(LOCTEXT("DismissInfo", "Dismiss this notification"))
										.Icon(FFlareStyleSet::GetIcon("Delete"))
										.OnClicked(this, &SFlareNotification::OnNotificationDismissed)
									]
								]

								// Info
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(Theme.SmallContentPadding)
								[
									SNew(STextBlock)
									.Text(InArgs._Info)
									.WrapTextAt(NotificationTextWidth)
									.TextStyle(&Theme.TextFont)
									.ColorAndOpacity(this, &SFlareNotification::GetNotificationTextColor)
									.ShadowColorAndOpacity(ShadowColor)
								]

								// Icons
								+ SVerticalBox::Slot()
								.AutoHeight()
								.HAlign(HAlign_Left)
								.VAlign(VAlign_Center)
								.Padding(NotificatioNWidth - 55, Theme.SmallContentPadding.Top, Theme.SmallContentPadding.Right, Theme.SmallContentPadding.Bottom)
								[
									SNew(SHorizontalBox)

									// Clickable
									+ SHorizontalBox::Slot()
									.AutoWidth()
									[
										SNew(SImage)
										.Image(FFlareStyleSet::GetIcon("Clickable"))
										.Visibility(this, &SFlareNotification::GetClickableIconVisibility)
										.ColorAndOpacity(this, &SFlareNotification::GetNotificationTextColor)
									]

									// Lifetime
									+ SHorizontalBox::Slot()
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									[
										SNew(SBox)
										.WidthOverride(this, &SFlareNotification::GetLifetimeSize)
										.HeightOverride(this, &SFlareNotification::GetLifetimeSize)
										.Visibility(this, &SFlareNotification::GetLifetimeIconVisibility)
										.HAlign(HAlign_Fill)
										.VAlign(VAlign_Fill)
										[
											SNew(SImage)
											.Image(FFlareStyleSet::GetIcon("Lifetime"))
											.ColorAndOpacity(this, &SFlareNotification::GetNotificationTextColor)
										]
									]
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
		NotificationTimeout = Lifetime + NotificationFinishDuration;
	}
	else
	{
		Lifetime = FMath::Max(Lifetime, NotificationTimeout - NotificationFinishDuration);
	}

	// Exit right now
	if (Now)
	{
		Lifetime = NotificationTimeout;
	}

	ForcedLife = true;
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareNotification::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	Lifetime += InDeltaTime;

	float Ease = 2;
	float AnimationTime = NotificationExitDuration / 2;
	float TimeToFade = (NotificationTimeout > 0 ? NotificationTimeout - Lifetime - 2 * AnimationTime : 2 * AnimationTime);
	float TimeToRemove = (NotificationTimeout > 0 ? NotificationTimeout - Lifetime : AnimationTime);

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
		case EFlareNotification::NT_Info:      Result = Theme.InfoColor;      break;
		case EFlareNotification::NT_Objective: Result = Theme.ObjectiveColor; break;
		case EFlareNotification::NT_Military:  Result = Theme.CombatColor;    break;
		case EFlareNotification::NT_Quest:	   Result = Theme.QuestColor;     break;
		case EFlareNotification::NT_Economy:   Result = Theme.TradingColor;   break;
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

EVisibility SFlareNotification::GetClickableIconVisibility() const
{
	if (TargetMenu != EFlareMenu::MENU_None)
	{
		return EVisibility::Visible;
	}
	else
	{
		return EVisibility::Hidden;
	}
}

EVisibility SFlareNotification::GetLifetimeIconVisibility() const
{
	if (NotificationTimeout == 0)
	{
		return EVisibility::Collapsed;
	}
	else
	{
		return EVisibility::Visible;
	}
}

FOptionalSize  SFlareNotification::GetLifetimeSize() const
{
	float InitialSize = 24.0f;
	if (NotificationTimeout == 0)
	{
		return InitialSize;
	}
	else
	{
		return FMath::Clamp(1.0f - Lifetime / NotificationTimeout, 0.0f, 1.0f) * InitialSize;
	}
}

FMargin SFlareNotification::GetNotificationMargins() const
{
	FMargin Result(0);

	Result.Top = CurrentMargin;

	return Result;
}

void SFlareNotification::OnNotificationDismissed()
{
	Finish();
}

FReply SFlareNotification::OnNotificationClicked()
{
	if (TargetMenu != EFlareMenu::MENU_None)
	{
		MenuManager->OpenMenu(TargetMenu, TargetInfo);
	}

	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE
