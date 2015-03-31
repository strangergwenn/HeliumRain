
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
	TSharedPtr<SWidget> Temp;

	NotificationContainer->InsertSlot(0)
		.AutoHeight()
		[
			SAssignNew(Temp, STextBlock)
			.Text(Text)
			.ColorAndOpacity(this, &SFlareNotifier::GetNotificationColor, NotificationIndex)
		];

	FFlareNotificationData NotificationEntry;
	NotificationEntry.Index = NotificationIndex;
	NotificationEntry.Widget = Temp;
	NotificationEntry.Lifetime = 5;

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
		NotificationEntry.Lifetime -= InDeltaTime;
		if (NotificationEntry.Lifetime <= 0)
		{
			NotificationContainer->RemoveSlot(NotificationEntry.Widget.ToSharedRef());
		}
	}
}

FSlateColor SFlareNotifier::GetNotificationColor(int32 Index) const
{
	FLinearColor Result = FLinearColor::White;

	for (auto& NotificationEntry : NotificationData)
	{
		if (NotificationEntry.Index == Index)
		{
			Result.A = FMath::InterpEaseInOut(1.0f, 0.0f, 1 - NotificationEntry.Lifetime / 5.0f, 2);
		}
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE
