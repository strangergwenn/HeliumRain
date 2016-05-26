
#include "../../Flare.h"
#include "FlareMainOverlay.h"
#include "../../Player/FlareMenuManager.h"
#include "../Components/FlareObjectiveInfo.h"

#define LOCTEXT_NAMESPACE "FlareMainOverlay"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareMainOverlay::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Right)
	.Padding(FMargin(0, 200, 0, 0))
	[
		SNew(SVerticalBox)

		// Menu list
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
		]
	
		// Notifications
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.HeightOverride(800)
			.VAlign(VAlign_Top)
			.Visibility(EVisibility::SelfHitTestInvisible)
			[
				SNew(SVerticalBox)

				// Objective
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SFlareObjectiveInfo)
					.PC(MenuManager->GetPC())
					.Visibility(EVisibility::SelfHitTestInvisible)
				]

				// Notifications
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(NotificationContainer, SVerticalBox)
				]
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareMainOverlay::Open()
{

}

void SFlareMainOverlay::Close()
{

}

void SFlareMainOverlay::Notify(FText Text, FText Info, FName Tag, EFlareNotification::Type Type, float Timeout, EFlareMenu::Type TargetMenu, void* TargetInfo, FName TargetSpacecraft)
{
	// Remove notification with the same tag.
	if (Tag != NAME_None)
	{
		for (int Index = 0; Index < NotificationData.Num(); Index++)
		{
			if (NotificationData[Index]->IsDuplicate(Tag))
			{
				NotificationData[Index]->Finish();
				FLOG("SFlareMainOverlay::Notify : ignoring because it's duplicate");
			}
		}
	}

	// Add notification
	TSharedPtr<SFlareNotification> NotificationEntry;
	NotificationContainer->AddSlot()
	.AutoHeight()
	[
		SAssignNew(NotificationEntry, SFlareNotification)
		.MenuManager(MenuManager.Get())
		.Notifier(this)
		.Text(Text)
		.Info(Info)
		.Type(Type)
		.Tag(Tag)
		.Timeout(Timeout)
		.TargetMenu(TargetMenu)
		.TargetInfo(TargetInfo)
		.TargetSpacecraft(TargetSpacecraft)
	];

	// Store a reference to it
	NotificationData.Add(NotificationEntry);
}

void SFlareMainOverlay::FlushNotifications()
{
	for (auto& NotificationEntry : NotificationData)
	{
		NotificationEntry->Finish();
	}
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareMainOverlay::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
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


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

bool SFlareMainOverlay::IsFirstNotification(SFlareNotification* Notification)
{
	for (int i = 0; i < NotificationData.Num(); i++)
	{
		if (!NotificationData[i]->IsFinished())
		{
			return NotificationData[i].Get() == Notification;
		}
	}
	return false;
}

#undef LOCTEXT_NAMESPACE
