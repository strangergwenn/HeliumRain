
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

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Right)
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

void SFlareNotifier::Notify(FText Text, FText Info, EFlareNotification::Type Type, EFlareMenu::Type TargetMenu, void* TargetInfo)
{
	// Make sure it's unique
	if (NotificationData.Num() > 0 && NotificationData.Last()->IsDuplicate(Text, TargetMenu))
	{
		FLOG("SFlareNotifier::Notify : deleting previous because it's duplicate");
		NotificationData.Last()->Finish();
	}

	// Add notification
	TSharedPtr<SFlareNotification> NotificationEntry;
	NotificationContainer->InsertSlot(0)
	.AutoHeight()
	[
		SAssignNew(NotificationEntry, SFlareNotification)
		.OwnerHUD(OwnerHUD.Get())
		.Text(Text)
		.Info(Info)
		.Type(Type)
		.TargetMenu(TargetMenu)
		.TargetInfo(TargetInfo)
	];

	// Store a reference to it
	NotificationData.Add(NotificationEntry);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareNotifier::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	int32 NotificationCount = 0;

	// Destroy notifications when they're done with the animation
	for (auto& NotificationEntry : NotificationData)
	{
		if (NotificationEntry->IsFinished())
		{
			NotificationContainer->RemoveSlot(NotificationEntry.ToSharedRef());
		}
		else
		{
			NotificationCount++;
		}
	}

	// Clean up the list when no notification is active
	if (NotificationCount == 0)
	{
		NotificationData.Empty();
	}
}


#undef LOCTEXT_NAMESPACE
