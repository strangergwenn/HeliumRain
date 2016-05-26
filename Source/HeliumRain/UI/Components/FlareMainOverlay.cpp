
#include "../../Flare.h"
#include "FlareMainOverlay.h"
#include "../Components/FlareObjectiveInfo.h"

#include "../../Player/FlareHUD.h"
#include "../../Player/FlareMenuManager.h"

#include "Runtime/Engine/Classes/Engine/UserInterfaceSettings.h"
#include "Runtime/Engine/Classes/Engine/RendererSettings.h"

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
	.HAlign(HAlign_Fill)
	[
		SNew(SVerticalBox)

		// Menu list
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0))
		.VAlign(VAlign_Top)
		[
			SNew(SBorder)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(FMargin(0))
			.BorderImage(&Theme.BackgroundBrush)
			[
				SAssignNew(MenuList, SHorizontalBox)

				// Title
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					.HAlign(HAlign_Fill)
					.WidthOverride(0.8 * Theme.ContentWidth)
					[
						SNew(SHorizontalBox)

						// Title icon
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SImage)
							.Image(this, &SFlareMainOverlay::GetCurrentMenuIcon)
						]

						// Title text
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.AutoWidth()
						.Padding(FMargin(20, 0))
						[
							SNew(STextBlock)
							.TextStyle(&Theme.TitleFont)
							.Text(this, &SFlareMainOverlay::GetCurrentMenuName)
						]
					]
				]
			]
		]
	
		// Notifications
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Right)
		.Padding(FMargin(0, 200, 0, 0))
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

	// Add menus
	AddMenuLink(EFlareMenu::MENU_Ship);
	AddMenuLink(EFlareMenu::MENU_Sector);
	AddMenuLink(EFlareMenu::MENU_Orbit);
	AddMenuLink(EFlareMenu::MENU_Company);
	AddMenuLink(EFlareMenu::MENU_Settings);
	AddMenuLink(EFlareMenu::MENU_Quit);
	AddMenuLink(EFlareMenu::MENU_Exit, true);

	// Init
	Close();
}

void SFlareMainOverlay::AddMenuLink(EFlareMenu::Type Menu, bool AlignRight)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	TSharedPtr<SFlareButton> Button;

	// Create button
	if (AlignRight)
	{
		MenuList->AddSlot()
		.HAlign(HAlign_Right)
		[
			SAssignNew(Button, SFlareButton)
			.Width(3)
			.Height(2.6666667)
			.Transparent(true)
			.OnClicked(this, &SFlareMainOverlay::OnOpenMenu, Menu)
		];
	}
	else
	{
		MenuList->AddSlot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		[
			SAssignNew(Button, SFlareButton)
			.Width(3)
			.Height(2.6666667)
			.Transparent(true)
			.OnClicked(this, &SFlareMainOverlay::OnOpenMenu, Menu)
		];

	}

	// Fill button contents
	Button->GetContainer()->SetHAlign(HAlign_Center);
	Button->GetContainer()->SetVAlign(VAlign_Fill);
	Button->GetContainer()->SetPadding(FMargin(0, 20));
	Button->GetContainer()->SetContent(
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(64)
			.HeightOverride(64)
			[
				SNew(SImage)
				.Image(AFlareMenuManager::GetMenuIcon(Menu))
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TextFont)
			.Text(AFlareMenuManager::GetMenuName(Menu))
		]
	);
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareMainOverlay::Open()
{
	IsOverlayVisible = true;
	MenuList->SetVisibility(EVisibility::Visible);
}

void SFlareMainOverlay::Close()
{
	IsOverlayVisible = false;
	MenuList->SetVisibility(EVisibility::Collapsed);
}

bool SFlareMainOverlay::IsOpen() const
{
	return IsOverlayVisible;
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

	// Get 2D position
	FVector2D MousePosition = MenuManager->GetPC()->GetMousePosition();
	FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	float ViewportScale = GetDefault<UUserInterfaceSettings>(UUserInterfaceSettings::StaticClass())->GetDPIScaleBasedOnSize(FIntPoint(ViewportSize.X, ViewportSize.Y));

	// Visibility check
	if (!IsOverlayVisible || (MousePosition.Y / ViewportScale) > AFlareMenuManager::GetMainOverlayHeight())
	{
		SetVisibility(EVisibility::HitTestInvisible);
	}
	else
	{
		SetVisibility(EVisibility::Visible);
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
			NotificationCount++;
		}
	}

	// Clean up the list when no notification is active
	if (NotificationCount == 0)
	{
		NotificationData.Empty();
	}
}

FText SFlareMainOverlay::GetCurrentMenuName() const
{
	FText Name = AFlareMenuManager::GetMenuName(MenuManager->GetCurrentMenu());
	return FText::FromString(Name.ToString().ToUpper());
}

const FSlateBrush* SFlareMainOverlay::GetCurrentMenuIcon() const
{
	return AFlareMenuManager::GetMenuIcon(MenuManager->GetCurrentMenu());
}

void SFlareMainOverlay::OnOpenMenu(EFlareMenu::Type Menu)
{
	if (MenuManager->IsSpacecraftMenu(Menu))
	{
		MenuManager->OpenMenuSpacecraft(Menu);
	}
	else
	{
		MenuManager->OpenMenu(Menu);
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
