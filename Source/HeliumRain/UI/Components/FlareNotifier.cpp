
#include "FlareNotifier.h"
#include "../../Flare.h"

#include "../../Game/FlareGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Quests/FlareQuest.h"

#include "FlareButton.h"
#include "FlareObjectiveInfo.h"

#include "SBackgroundBlur.h"

#define LOCTEXT_NAMESPACE "FlareNotifier"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareNotifier::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	int32 ObjectiveInfoWidth = 370;
	FLinearColor ObjectiveColor = Theme.ObjectiveColor;
	ObjectiveColor.A = FFlareStyleSet::GetDefaultTheme().DefaultAlpha;
	NotificationsVisible = true;

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Bottom)
	.HAlign(HAlign_Right)
	.Padding(FMargin(0))
	[
		SNew(SBox)
		.HeightOverride(800)
		.VAlign(VAlign_Bottom)
		[
			SNew(SVerticalBox)

			// Notifications
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(NotificationContainer, SVerticalBox)
			]

			// Objective
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// Icon
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(2)
					[
						SNew(SImage)
						.Image(&Theme.InvertedBrush)
						.ColorAndOpacity(ObjectiveColor)
						.Visibility(this, &SFlareNotifier::GetObjectiveVisibility)
					]
				]

				// Text
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBackgroundBlur)
					.BlurRadius(Theme.BlurRadius)
					.BlurStrength(Theme.BlurStrength)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.Padding(FMargin(0))
					[
						SNew(SBorder)
						.BorderImage(this, &SFlareNotifier::GetBackgroundBrush)
						[
							SNew(SBox)
							.WidthOverride(ObjectiveInfoWidth)
							.Visibility(this, &SFlareNotifier::GetObjectiveVisibility)
							.Padding(Theme.SmallContentPadding)
							[
								SNew(SFlareObjectiveInfo)
								.PC(MenuManager->GetPC())
								.Width(ObjectiveInfoWidth)
							]
						]
					]
				]
			]

			// Hide button
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			[
				SAssignNew(NotificationVisibleButton, SFlareButton)
				.Width(3)
				.Small(true)
				.Transparent(true)
				.Text(this, &SFlareNotifier::GetHideText)
				.OnClicked(this, &SFlareNotifier::OnHideClicked)
				.Visibility(this, &SFlareNotifier::GetHideButtonVisibility)
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

bool SFlareNotifier::Notify(FText Text, FText Info, FName Identifier, EFlareNotification::Type Type, bool Pinned, EFlareMenu::Type TargetMenu, FFlareMenuParameterData TargetInfo)
{
	// search for recent notification with the same tag

	bool DuplicateFound = false;

	if (Identifier != NAME_None)
	{
		for (int Index = 0; Index < NotificationData.Num(); Index++)
		{
			if (NotificationData[Index]->IsDuplicate(Identifier))
			{
				DuplicateFound = true;
				if(NotificationData[Index]->IsRecent())
				{
					return false;
				}
			}
		}
	}

	// Remove notification with the same tag.
	if(DuplicateFound)
	{
		ClearNotifications(Identifier, true);
	}

	// Add notification
	TSharedPtr<SFlareNotification> NotificationEntry;
	NotificationContainer->InsertSlot(0)
		.AutoHeight()
		[
			SAssignNew(NotificationEntry, SFlareNotification)
			.MenuManager(MenuManager.Get())
			.Notifier(this)
			.Text(Text)
			.Info(Info)
			.Type(Type)
			.Tag(Identifier)
			.Pinned(Pinned)
			.TargetMenu(TargetMenu)
			.TargetInfo(TargetInfo)
		];

	// Store a reference to it
	NotificationData.Add(NotificationEntry);
	NotificationsVisible = true;

	return true;
}

void SFlareNotifier::ClearNotifications(FName Identifier, bool Now)
{
	if (Identifier != NAME_None)
	{
		for (int Index = 0; Index < NotificationData.Num(); Index++)
		{
			if (NotificationData[Index]->IsDuplicate(Identifier))
			{
				NotificationData[Index]->Finish(Now);
			}
		}
	}
}

void SFlareNotifier::FlushNotifications()
{
	for (auto& NotificationEntry : NotificationData)
	{
		NotificationEntry->Finish();
	}
}

void SFlareNotifier::HideNotifications()
{
	NotificationsVisible = false;
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareNotifier::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	// Tick parent
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (!MenuManager->GetPC()->IsGameBusy())
	{
		// Don't show notifications in story menu
		if (   MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Story
			|| MenuManager->GetNextMenu() == EFlareMenu::MENU_Story
			|| MenuManager->GetCurrentMenu() == EFlareMenu::MENU_GameOver
			|| MenuManager->GetNextMenu() == EFlareMenu::MENU_GameOver)
		{
			NotificationContainer->SetVisibility(EVisibility::Hidden);
		}
		else
		{
			NotificationContainer->SetVisibility(EVisibility::SelfHitTestInvisible);
		}

		// Destroy notifications when they're done with the animation
		int32 NotificationCount = 0;
		for (auto& NotificationEntry : NotificationData)
		{
			if (NotificationEntry->IsFinished())
			{
				NotificationContainer->RemoveSlot(NotificationEntry.ToSharedRef());
			}
			else
			{
				NotificationEntry->SetVisibility(EVisibility::Visible);
				NotificationCount++;
			}
		}

		// Clean up the list when no notification is active
		if (NotificationCount == 0)
		{
			NotificationData.Empty();
		}
	}
}

const FSlateBrush* SFlareNotifier::GetBackgroundBrush() const
{
	return MenuManager->GetPC()->GetBackgroundDecorator();
}

EVisibility SFlareNotifier::GetObjectiveVisibility() const
{
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();

	if (MenuManager->GetCurrentMenu() != EFlareMenu::MENU_Story
		&& MenuManager->GetNextMenu() != EFlareMenu::MENU_Story
		&& MenuManager->GetCurrentMenu() != EFlareMenu::MENU_GameOver
		&& MenuManager->GetNextMenu() != EFlareMenu::MENU_GameOver
		&& QuestManager
		&& QuestManager->GetSelectedQuest()
		&& NotificationsVisible)
	{
		return EVisibility::SelfHitTestInvisible;
	}
	else
	{
		return EVisibility::Collapsed;
	}
}

FText SFlareNotifier::GetHideText() const
{
	// Count objects
	int32 Count = NotificationData.Num();
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();
	if (QuestManager && QuestManager->GetSelectedQuest())
	{
		Count++;
	}

	// Format text
	if (NotificationsVisible)
	{
		if (Count > 1)
		{
			return FText::Format(LOCTEXT("HideButtonVisiblePlural", "Hide {0} items"), FText::AsNumber(Count));
		}
		else
		{
			return LOCTEXT("HideButtonVisible", "Hide one item");
		}
	}
	else
	{
		if (Count > 1)
		{
			return FText::Format(LOCTEXT("HideButtonMaskedPlural", "Show {0} items"), FText::AsNumber(Count));
		}
		else
		{
			return LOCTEXT("HideButtonMaskedSingle", "Show one item");
		}
	}
}

EVisibility SFlareNotifier::GetHideButtonVisibility() const
{
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();

	if (NotificationData.Num() > 0 || (QuestManager && QuestManager->GetSelectedQuest()))
	{
		return EVisibility::Visible;
	}
	else
	{
		return EVisibility::Collapsed;
	}
}

void SFlareNotifier::OnHideClicked()
{
	NotificationsVisible = !NotificationsVisible;
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

bool SFlareNotifier::AreNotificationsVisible() const
{
	return NotificationsVisible;
}


#undef LOCTEXT_NAMESPACE
