
#include "../../Flare.h"
#include "FlareNotifier.h"

#define LOCTEXT_NAMESPACE "FlareNotifier"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareNotifier::Construct(const FArguments& InArgs)
{
	// Data
	NotificationIndex = 0;
	NotificationTimeout = 10;
	NotificationScroll = 200;
	NotificationEnterTime = 0.5;
	NotificationExitTime = 0.5;
	OwnerHUD = InArgs._OwnerHUD;

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Right)
	[
		SNew(SVerticalBox)

		// Watermark
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(5))
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Watermark", "DEVELOPMENT BUILD"))
			.TextStyle(&FFlareStyleSet::GetDefaultTheme().SmallFont)
		]

		// Actual content
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0, 0, 50, 0))
		[
			SNew(SBox)
			.HeightOverride(700)
			.VAlign(VAlign_Bottom)
			[
				SAssignNew(NotificationContainer, SVerticalBox)
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareNotifier::Notify(FText Text, FText Info, EFlareNotification::Type Type, EFlareMenu::Type TargetMenu, void* TargetInfo)
{
	// Add button
	TSharedPtr<SWidget> TempContainer;
	TSharedPtr<SFlareButton> TempButton;
	NotificationContainer->InsertSlot(0)
		.AutoHeight()
		[
			SAssignNew(TempContainer, SBox)
			.Padding(this, &SFlareNotifier::GetNotificationMargin, NotificationIndex)
			[
				SAssignNew(TempButton, SFlareButton)
				.Color(this, &SFlareNotifier::GetNotificationColor, NotificationIndex)
				.OnClicked(this, &SFlareNotifier::OnNotificationClicked, NotificationIndex)
				.ButtonStyle(&FFlareStyleSet::Get().GetWidgetStyle<FFlareButtonStyle>("/Style/BuyButton"))
			]
		];

	// Get icon
	const FSlateBrush* Icon = NULL;
	switch (Type)
	{
		case EFlareNotification::NT_General:  Icon = FFlareStyleSet::GetIcon("Owned");       break;
		case EFlareNotification::NT_Help:     Icon = FFlareStyleSet::GetIcon("Help_Notif");  break;
		case EFlareNotification::NT_Military: Icon = FFlareStyleSet::GetIcon("Shell_Notif"); break;
		case EFlareNotification::NT_Trading:  Icon = FFlareStyleSet::GetIcon("Cost");        break;
	}

	// Fill button
	TempButton->GetContainer()->SetContent
	(
		SNew(SVerticalBox)

		// Header
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			// Icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SImage)
				.Image(Icon)
				.ColorAndOpacity(this, &SFlareNotifier::GetNotificationColor, NotificationIndex)
			]

			// Text
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(10, 5))
			[
				SNew(STextBlock)
				.Text(Text)
				.TextStyle(&FFlareStyleSet::GetDefaultTheme().NameFont)
				.ColorAndOpacity(this, &SFlareNotifier::GetNotificationColor, NotificationIndex)
			]
		]

		// Info
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(Info)
			.TextStyle(&FFlareStyleSet::GetDefaultTheme().TextFont)
			.ColorAndOpacity(this, &SFlareNotifier::GetNotificationColor, NotificationIndex)
		]
	);

	// Configure notification
	FFlareNotificationData NotificationEntry;
	NotificationEntry.Index = NotificationIndex;
	NotificationEntry.Widget = TempContainer;
	NotificationEntry.Lifetime = 0;
	NotificationEntry.TargetMenu = TargetMenu;
	NotificationEntry.TargetInfo = TargetInfo;

	// Add notification
	NotificationData.Add(NotificationEntry);
	NotificationIndex++;
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareNotifier::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	int NotificationCount = 0;

	// look for valid notifications
	for (auto& NotificationEntry : NotificationData)
	{
		NotificationEntry.Lifetime += InDeltaTime;
		if (NotificationEntry.Lifetime >= NotificationTimeout)
		{
			NotificationContainer->RemoveSlot(NotificationEntry.Widget.ToSharedRef());
		}
		else
		{
			NotificationCount++;
		}
	}

	// Clean up
	if (NotificationCount == 0)
	{
		NotificationData.Empty();
	}
}

FMargin SFlareNotifier::GetNotificationMargin(int32 Index) const
{
	float BottomMargin = 0;
	float TopMargin = 0;

	for (auto& NotificationEntry : NotificationData)
	{
		if (NotificationEntry.Index == Index)
		{
			// Compute alphas
			float RemainingTime = NotificationTimeout - NotificationEntry.Lifetime;
			float AlphaIn = FMath::Clamp(NotificationEntry.Lifetime / NotificationEnterTime, 0.0f, 1.0f);
			float AlphaOut = FMath::Clamp(RemainingTime / NotificationExitTime, 0.0f, 1.0f);

			// Update margin
			BottomMargin = FMath::InterpEaseOut(NotificationScroll, 0.0f, AlphaIn, 2);
			TopMargin = FMath::InterpEaseOut(0.0f, 50.0f, AlphaOut, 2);
			break;
		}
	}

	return FMargin(0, TopMargin, 0, BottomMargin);
}

FSlateColor SFlareNotifier::GetNotificationColor(int32 Index) const
{
	FLinearColor Result = FFlareStyleSet::GetDefaultTheme().NeutralColor;

	for (auto& NotificationEntry : NotificationData)
	{
		if (NotificationEntry.Index == Index)
		{
			// Find the new color
			float RemainingTime = NotificationTimeout - NotificationEntry.Lifetime - NotificationExitTime;
			float Alpha = FMath::Clamp(RemainingTime / NotificationExitTime, 0.0f, 1.0f);
			Result.A = FMath::InterpEaseOut(0.0f, 0.7f, Alpha, 2);
			break;
		}
	}

	return Result;
}

void SFlareNotifier::OnNotificationClicked(int32 Index)
{
	for (auto& NotificationEntry : NotificationData)
	{
		if (NotificationEntry.Index == Index)
		{
			// Call if necessary
			if (NotificationEntry.TargetMenu != EFlareMenu::MENU_None)
			{
				OwnerHUD->OpenMenu(NotificationEntry.TargetMenu, NotificationEntry.TargetInfo);
			}

			// Set the lifetime to "almost done"
			NotificationEntry.Lifetime = FMath::Max(NotificationEntry.Lifetime, NotificationTimeout - 2 * NotificationExitTime);
			break;
		}
	}
}


#undef LOCTEXT_NAMESPACE
