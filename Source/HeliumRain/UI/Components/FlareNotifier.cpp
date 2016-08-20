
#include "../../Flare.h"
#include "FlareNotifier.h"
#include "../Components/FlareObjectiveInfo.h"
#include "../../Player/FlareMenuManager.h"

#define LOCTEXT_NAMESPACE "FlareNotifier"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareNotifier::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Right)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight() + 10, 0, 0))
	[
		SNew(SBox)
		.HeightOverride(800)
		.VAlign(VAlign_Top)
		[
			SNew(SVerticalBox)

			// Objective
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SFlareObjectiveInfo)
				.PC(MenuManager->GetPC())
				.Visibility(this, &SFlareNotifier::GetObjectiveVisibility)
			]

			// Notifications
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(NotificationContainer, SVerticalBox)
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareNotifier::Notify(FText Text, FText Info, FName Tag, EFlareNotification::Type Type, bool Pinned, EFlareMenu::Type TargetMenu, FFlareMenuParameterData TargetInfo)
{
	// Remove notification with the same tag.
	if (Tag != NAME_None)
	{
		for (int Index = 0; Index < NotificationData.Num(); Index++)
		{
			if (NotificationData[Index]->IsDuplicate(Tag))
			{
				NotificationData[Index]->Finish(true);
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
			.Pinned(Pinned)
			.TargetMenu(TargetMenu)
			.TargetInfo(TargetInfo)
		];

	// Store a reference to it
	NotificationData.Add(NotificationEntry);
}

void SFlareNotifier::FlushNotifications()
{
	for (auto& NotificationEntry : NotificationData)
	{
		NotificationEntry->Finish();
	}
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareNotifier::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	int32 NotificationCount = 0;

	// Don't show notifications in story menu
	if (MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Story
		|| MenuManager->GetNextMenu() == EFlareMenu::MENU_Story)
	{
		NotificationContainer->SetVisibility(EVisibility::Hidden);
	}
	else
	{
		NotificationContainer->SetVisibility(EVisibility::SelfHitTestInvisible);
	}

	// Tick parent
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

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

EVisibility SFlareNotifier::GetObjectiveVisibility() const
{
	if (MenuManager->IsUIOpen())
	{
		return EVisibility::Collapsed;
	}
	else
	{
		return EVisibility::SelfHitTestInvisible;
	}
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

bool SFlareNotifier::IsFirstNotification(SFlareNotification* Notification)
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
