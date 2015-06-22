
#include "../../Flare.h"
#include "FlareNotification.h"
#include "../../Player/FlareHUD.h"

#define LOCTEXT_NAMESPACE "FlareNotification"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareNotification::Construct(const FArguments& InArgs)
{
	// Settings
	NotificationTimeout = 10;
	NotificationScroll = 150;
	NotificationEnterTime = 0.4;
	NotificationExitTime = 0.7;
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
	OwnerHUD = InArgs._OwnerHUD;
	TargetMenu = InArgs._TargetMenu;
	TargetInfo = InArgs._TargetInfo;
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
									.TextStyle(&FFlareStyleSet::GetDefaultTheme().NameFont)
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
									.TextStyle(&FFlareStyleSet::GetDefaultTheme().TextFont)
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

bool SFlareNotification::IsFinished() const
{
	return (Lifetime >= NotificationTimeout);
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
	float AnimationTime = NotificationExitTime / 2;
	float TimeToFade = NotificationTimeout - Lifetime - 2 * AnimationTime;
	float TimeToRemove = NotificationTimeout - Lifetime - AnimationTime;

	// Disappear if finished
	if (TimeToFade <= 0 && Button->GetVisibility() == EVisibility::Visible)
	{
		LastHeight = GetDesiredSize().Y;
		Button->SetVisibility(EVisibility::Collapsed);
	}

	// Update render data
	if (Lifetime <= NotificationEnterTime)
	{
		CurrentAlpha = FMath::InterpEaseOut(0.0f, 1.0f, FMath::Clamp(Lifetime / NotificationEnterTime, 0.0f, 1.0f), Ease);
		CurrentMargin = NotificationScroll * FMath::InterpEaseOut(1.0f, 0.0f, FMath::Clamp(Lifetime / NotificationEnterTime, 0.0f, 1.0f), Ease);
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
		OwnerHUD->OpenMenu(TargetMenu, TargetInfo);
	}

	// Set the lifetime to "almost done"
	Lifetime = FMath::Max(Lifetime, NotificationTimeout - NotificationExitTime);
	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE
