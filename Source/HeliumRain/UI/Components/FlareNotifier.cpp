
#include "FlareNotifier.h"
#include "../../Flare.h"
#include "../Components/FlareObjectiveInfo.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Quests/FlareQuest.h"

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
						.BorderImage(&Theme.BackgroundBrush)
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
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareNotifier::Notify(FText Text, FText Info, FName Identifier, EFlareNotification::Type Type, bool Pinned, EFlareMenu::Type TargetMenu, FFlareMenuParameterData TargetInfo)
{
	// Remove notification with the same tag.
	ClearNotifications(Identifier, true);

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

EVisibility SFlareNotifier::GetObjectiveVisibility() const
{
	UFlareQuestManager* QuestManager = MenuManager->GetGame()->GetQuestManager();

	if (MenuManager->GetCurrentMenu() != EFlareMenu::MENU_Story
		&& MenuManager->GetNextMenu() != EFlareMenu::MENU_Story
		&& MenuManager->GetCurrentMenu() != EFlareMenu::MENU_GameOver
		&& MenuManager->GetNextMenu() != EFlareMenu::MENU_GameOver
		&& QuestManager
		&& QuestManager->GetSelectedQuest())
	{
		return EVisibility::SelfHitTestInvisible;
	}
	else
	{
		return EVisibility::Collapsed;
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
