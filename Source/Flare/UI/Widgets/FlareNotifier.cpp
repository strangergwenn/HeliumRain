
#include "../../Flare.h"
#include "FlareNotifier.h"

#define LOCTEXT_NAMESPACE "FlareNotifier"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareNotifier::Construct(const FArguments& InArgs)
{
	// Data
	OwnerHUD = InArgs._OwnerHUD;
	NotificationIndex = 0;
	NotificationTimeout = 10;
	NotificationScroll = 200;
	NotificationScrollTime = 1;

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Right)
	.Padding(FMargin(0, 0, 50, 0))
	[
		SNew(SBox)
		.HeightOverride(700)
		.VAlign(VAlign_Bottom)
		[
			SAssignNew(NotificationContainer, SVerticalBox)
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareNotifier::Notify(FText Text, EFlareNotification::Type Type, EFlareMenu::Type TargetMenu, void* TargetInfo)
{
	TSharedPtr<SWidget> TempContainer;
	TSharedPtr<SFlareButton> TempButton;

	// Add button
	NotificationContainer->InsertSlot(0)
		.AutoHeight()
		[
			SAssignNew(TempContainer, SBox)
			.Padding(this, &SFlareNotifier::GetNotificationMargin, NotificationIndex)
			[
				SAssignNew(TempButton, SFlareButton)
			]
		];

	// Fill button
	TempButton->GetContainer()->SetContent
	(
		SNew(STextBlock)
		.Text(Text)
		.ColorAndOpacity(this, &SFlareNotifier::GetNotificationColor, NotificationIndex)
	);

	// Configure notification
	FFlareNotificationData NotificationEntry;
	NotificationEntry.Index = NotificationIndex;
	NotificationEntry.Widget = TempContainer;
	NotificationEntry.Lifetime = 0;
	NotificationData.Add(NotificationEntry);
	NotificationIndex++;
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareNotifier::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	for (auto& NotificationEntry : NotificationData)
	{
		NotificationEntry.Lifetime += InDeltaTime;
		if (NotificationEntry.Lifetime >= NotificationTimeout)
		{
			NotificationContainer->RemoveSlot(NotificationEntry.Widget.ToSharedRef());
		}
	}
}

FMargin SFlareNotifier::GetNotificationMargin(int32 Index) const
{
	float BottomMargin = 0;

	for (auto& NotificationEntry : NotificationData)
	{
		if (NotificationEntry.Index == Index)
		{
			float Alpha = FMath::Clamp(NotificationEntry.Lifetime / NotificationScrollTime, 0.0f, 1.0f);
			BottomMargin = FMath::InterpEaseOut(NotificationScroll, 0.0f, Alpha, 2);
		}
	}

	return FMargin(0, 0, 0, BottomMargin);
}

FSlateColor SFlareNotifier::GetNotificationColor(int32 Index) const
{
	FLinearColor Result = FLinearColor::White;

	for (auto& NotificationEntry : NotificationData)
	{
		if (NotificationEntry.Index == Index)
		{
			float Alpha = FMath::Clamp(NotificationEntry.Lifetime / NotificationTimeout, 0.0f, 1.0f);
			Result.A = FMath::InterpEaseOut(1.0f, 0.0f, Alpha, 2);
		}
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE
