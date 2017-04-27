
#include "FlareMainOverlay.h"
#include "../../Flare.h"

#include "../../Game/FlareGameUserSettings.h"
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
	TitleButtonWidth = 2.0f;
	TitleButtonHeight = AFlareMenuManager::GetMainOverlayHeight() / (float)(Theme.ButtonHeight);
	FLinearColor HintColor = Theme.NeutralColor;
	HintColor.A = 0.3f;

	// State
	OverlayFadeAlpha = 0;
	OverlayFadeDuration = 0.2f;

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SCanvas)

		// Main widget
		+ SCanvas::Slot()
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Center)
		.Position(TAttribute<FVector2D>::Create(TAttribute<FVector2D>::FGetter::CreateSP(this, &SFlareMainOverlay::GetOverlayPosition)))
		.Size(TAttribute<FVector2D>::Create(TAttribute<FVector2D>::FGetter::CreateSP(this, &SFlareMainOverlay::GetOverlaySize)))
		[
			SNew(SVerticalBox)

			// Container
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(Background, SBackgroundBlur)
				.BlurRadius(Theme.BlurRadius)
				.BlurStrength(Theme.BlurStrength)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.Padding(FMargin(0))
				[
					SNew(SBorder)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.Padding(FMargin(0))
					.BorderImage(FFlareStyleSet::Get().GetBrush("/Brushes/SB_Black"))
					.BorderBackgroundColor(FLinearColor(1, 1, 1, 0.7f))
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SAssignNew(MenuList, SHorizontalBox)

							// Title
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SBox)
								.HAlign(HAlign_Fill)
								.WidthOverride(0.625 * Theme.ContentWidth)
								[
									SNew(SHorizontalBox)

									// Title icon
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.VAlign(VAlign_Center)
									[
										SNew(SImage)
										.Image(this, &SFlareMainOverlay::GetCurrentMenuIcon)
									]

									// Title text
									+ SHorizontalBox::Slot()
									.VAlign(VAlign_Center)
									.AutoWidth()
									[
										SNew(SVerticalBox)

										+ SVerticalBox::Slot()
										.AutoHeight()
										.Padding(Theme.SmallContentPadding)
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TitleFont)
											.Text(this, &SFlareMainOverlay::GetCurrentMenuName)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										.Padding(Theme.SmallContentPadding)
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.Text(this, &SFlareMainOverlay::GetPlayerInfo)
											.Visibility(this, &SFlareMainOverlay::GetPlayerInfoVisibility)
										]
									]
								]
							]
						]

						// Bottom border
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Fill)
						[
							SAssignNew(Border, SImage)
							.Image(&Theme.NearInvisibleBrush)
						]
					]
				]
			]

			// Helper
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.Padding(Theme.SmallContentPadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.SmallFont)
				.Text(this, &SFlareMainOverlay::GetHelperText)
				.ColorAndOpacity(HintColor)
				.Visibility(this, &SFlareMainOverlay::GetHintVisibility)
			]
		]
	];

	// Add general gameplay menus
	AddMenuLink(EFlareMenu::MENU_Ship);
	AddMenuLink(EFlareMenu::MENU_Sector);
	AddMenuLink(EFlareMenu::MENU_Orbit);
	AddMenuLink(EFlareMenu::MENU_Company);
	AddMenuLink(EFlareMenu::MENU_Fleet);
	AddMenuLink(EFlareMenu::MENU_Leaderboard);
	AddMenuLink(EFlareMenu::MENU_Technology);
	AddMenuLink(EFlareMenu::MENU_Quest);
	
	// Main
	TSharedPtr<SFlareButton> MainButton;
	MenuList->AddSlot()
	.HAlign(HAlign_Right)
	[
		SAssignNew(MainButton, SFlareButton)
		.Width(TitleButtonWidth)
		.Height(TitleButtonHeight)
		.Transparent(true)
		.OnClicked(this, &SFlareMainOverlay::OnOpenMenu, EFlareMenu::MENU_Main)
		.Visibility(this, &SFlareMainOverlay::GetGameButtonVisibility)
	];
	SetupMenuLink(MainButton, EFlareMenu::MENU_Main);
	
	// Settings
	TSharedPtr<SFlareButton> SettingsButton;
	MenuList->AddSlot()
	.HAlign(HAlign_Right)
	.AutoWidth()
	[
		SAssignNew(SettingsButton, SFlareButton)
		.Width(TitleButtonWidth)
		.Height(TitleButtonHeight)
		.Transparent(true)
		.OnClicked(this, &SFlareMainOverlay::OnOpenMenu, EFlareMenu::MENU_Settings)
	];
	SetupMenuLink(SettingsButton, EFlareMenu::MENU_Settings);
	
	// Back, exit
	TSharedPtr<SFlareButton> BackButton;
	TSharedPtr<SFlareButton> ExitButton;
	MenuList->AddSlot()
	.AutoWidth()
	.HAlign(HAlign_Right)
	[
		SNew(SVerticalBox)

		// Back button
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(BackButton, SFlareButton)
			.Width(TitleButtonWidth)
			.Height(TitleButtonHeight / 2)
			.Transparent(true)
			.OnClicked(this, &SFlareMainOverlay::OnBack)
			.IsDisabled(this, &SFlareMainOverlay::IsBackDisabled)
		]

		// Exit button
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(ExitButton, SFlareButton)
			.Width(TitleButtonWidth)
			.Height(TitleButtonHeight / 2)
			.Transparent(true)
			.OnClicked(this, &SFlareMainOverlay::OnCloseMenu)
			.IsDisabled(this, &SFlareMainOverlay::IsCloseDisabled)
			.Visibility(this, &SFlareMainOverlay::GetCloseVisibility)
		]
	];

	// Back, exit config
	SetupMenuLinkSmall(BackButton, FFlareStyleSet::GetIcon("Back"), FText(), FString());
	ExitButton->GetContainer()->SetHAlign(HAlign_Center);
	ExitButton->GetContainer()->SetVAlign(VAlign_Fill);
	ExitButton->GetContainer()->SetPadding(FMargin(0, 5));
	ExitButton->GetContainer()->SetContent(
		SNew(SVerticalBox)

		// Icon
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(64)
			.HeightOverride(64)
			[
				SNew(SImage)
				.Image(this, &SFlareMainOverlay::GetCloseIcon)
			]
		]);

	// Init
	IsOverlayVisible = false;
}

void SFlareMainOverlay::AddMenuLink(EFlareMenu::Type Menu)
{
	TSharedPtr<SFlareButton> Button;

	// Create button
	MenuList->AddSlot()
	.AutoWidth()
	.HAlign(HAlign_Right)
	[
		SAssignNew(Button, SFlareButton)
		.Width(TitleButtonWidth)
		.Height(TitleButtonHeight)
		.Transparent(true)
		.OnClicked(this, &SFlareMainOverlay::OnOpenMenu, Menu)
		.Visibility(this, &SFlareMainOverlay::GetGameButtonVisibility)
		.IsDisabled(this, &SFlareMainOverlay::IsGameButtonDisabled, Menu)
	];

	// Fill button contents
	SetupMenuLink(Button, Menu);
}

void SFlareMainOverlay::SetupMenuLink(TSharedPtr<SFlareButton> Button, EFlareMenu::Type Menu)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	Button->GetContainer()->SetHAlign(HAlign_Center);
	Button->GetContainer()->SetVAlign(VAlign_Fill);
	Button->GetContainer()->SetPadding(FMargin(0, 25));

	Button->GetContainer()->SetContent(
		SNew(SVerticalBox)

		// Icon
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

		// Text
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.SmallFont)
			.Text(AFlareMenuManager::GetMenuName(Menu))
		]

		// Shortcut
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.SmallFont)
			.Text(this, &SFlareMainOverlay::GetMenuKey, Menu)
		]
	);
}

void SFlareMainOverlay::SetupMenuLinkSmall(TSharedPtr<SFlareButton> Button, const FSlateBrush* Icon, FText Text, FString Key)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	Button->GetContainer()->SetHAlign(HAlign_Center);
	Button->GetContainer()->SetVAlign(VAlign_Fill);
	Button->GetContainer()->SetPadding(FMargin(0, 5));

	Button->GetContainer()->SetContent(
		SNew(SVerticalBox)

		// Icon
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(64)
			.HeightOverride(64)
			[
				SNew(SImage)
				.Image(Icon)
			]
		]

		// Text
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.SmallFont)
			.Text(Text)
		]

		// Shortcut
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.SmallFont)
			.Text(FText::Format(LOCTEXT("ShortcutKeyFormat", "({0})"), FText::FromString(Key)))
		]
	);
}

/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareMainOverlay::Open()
{
	IsOverlayVisible = true;
}

void SFlareMainOverlay::Close()
{
	IsOverlayVisible = false;
}

bool SFlareMainOverlay::IsOpen() const
{
	return IsOverlayVisible;
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareMainOverlay::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	AFlarePlayerController* PC = MenuManager->GetPC();
	
	// Get 2D position
	FVector2D MousePosition = MenuManager->GetPC()->GetMousePosition();
	FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	float ViewportScale = GetDefault<UUserInterfaceSettings>(UUserInterfaceSettings::StaticClass())->GetDPIScaleBasedOnSize(FIntPoint(ViewportSize.X, ViewportSize.Y));

	// Visibility check
	float Height = AFlareMenuManager::GetMainOverlayHeight() + 2.0f;
	if (!IsOverlayVisible || (MousePosition.Y / ViewportScale) > Height)
	{
		SetVisibility(EVisibility::HitTestInvisible);
	}
	else
	{
		SetVisibility(EVisibility::Visible);
	}

	// Player info text
	PlayerInfoText = FText();
	if (PC->GetShipPawn())
	{
		PlayerInfoText =  FText::Format(LOCTEXT("PlayerInfoFormat", "{0}\n{1} credits available"),
			PC->GetShipPawn()->GetShipStatus(),
			FText::AsNumber(PC->GetCompany()->GetMoney() / 100));
	}
	else if (MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Main)
	{
		PlayerInfoText = FText();
	}
	else if (MenuManager->GetCurrentMenu() == EFlareMenu::MENU_NewGame)
	{
		PlayerInfoText = LOCTEXT("NewGameHint", "Please describe your company");
	}
	else if (MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Settings)
	{
		PlayerInfoText = LOCTEXT("SettingsHint", "Changes will be applied and saved");
	}
	else if (MenuManager->GetCurrentMenu() == EFlareMenu::MENU_None
		|| MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Credits
		|| MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Story)
	{
		PlayerInfoText = FText();
	}
	else if (!MenuManager->IsFading())
	{
		PlayerInfoText = LOCTEXT("FastForwarding", "Time is passing by...\n");
	}

	// Fade opening & closing
	if (IsOverlayVisible)
	{
		OverlayFadeAlpha += InDeltaTime / OverlayFadeDuration;
	}
	else
	{
		OverlayFadeAlpha -= InDeltaTime / OverlayFadeDuration;
	}
	OverlayFadeAlpha = FMath::Clamp(OverlayFadeAlpha, 0.0f, 1.0f);
}

EVisibility SFlareMainOverlay::GetGameButtonVisibility() const
{
	if (MenuManager->GetPC()->GetShipPawn())
	{
		return EVisibility::Visible;
	}
	else if (MenuManager->GetCurrentMenu() == EFlareMenu::MENU_None
		|| MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Main
		|| MenuManager->GetCurrentMenu() == EFlareMenu::MENU_NewGame
		|| MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Settings
		|| MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Credits
		|| MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Story)
	{
		return EVisibility::Hidden;
	}
	else
	{
		return EVisibility::Visible;
	}
}

bool SFlareMainOverlay::IsGameButtonDisabled(EFlareMenu::Type Menu) const
{
	return false;
}

FVector2D SFlareMainOverlay::GetOverlayPosition() const
{
	FVector2D ScreenSize = GetCachedGeometry().GetLocalSize();

	float Alpha = FMath::InterpEaseOut(-1.0f, 0.0f, OverlayFadeAlpha, 2.0f);
	float VirtualHeight = Alpha * AFlareMenuManager::GetMainOverlayHeight() - 5;

	return FVector2D(ScreenSize.X / 2, VirtualHeight);
}

FVector2D SFlareMainOverlay::GetOverlaySize() const
{
	FVector2D ScreenSize = GetCachedGeometry().GetLocalSize();
	return FVector2D(ScreenSize.X, AFlareMenuManager::GetMainOverlayHeight() + 50);
}

bool SFlareMainOverlay::IsBackDisabled() const
{
	if (MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Main)
	{
		return true;
	}
	else
	{
		return !MenuManager->GetPC()->GetShipPawn() && !MenuManager->HasPreviousMenu();
	}
}

bool SFlareMainOverlay::IsCloseDisabled() const
{
	if (MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Main)
	{
		return false;
	}
	else if (MenuManager->GetPC()->GetShipPawn())
	{
		return false;
	}
	else
	{
		return true;
	}
}

EVisibility SFlareMainOverlay::GetCloseVisibility() const
{
	if (MenuManager->GetGame()->IsLoadedOrCreated() || MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Main)
	{
		return EVisibility::Visible;
	}
	else
	{
		return EVisibility::Hidden;
	}
}

const FSlateBrush* SFlareMainOverlay::GetCloseIcon() const
{
	if (MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Main)
	{
		return FFlareStyleSet::GetIcon("Quit");
	}
	else
	{
		return FFlareStyleSet::GetIcon("Close");
	}
}

FText SFlareMainOverlay::GetMenuKey(EFlareMenu::Type Menu) const
{
	return FText::Format(LOCTEXT("MenuKeyFormat", "<{0}>"), FText::FromString(AFlareMenuManager::GetMenuKey(Menu)));
}

FText SFlareMainOverlay::GetHelperText() const
{
	if (!MenuManager->IsMenuOpen())
	{
		FString KeyName = AFlareMenuManager::GetKeyNameFromActionName(FName("ToggleOverlay"));
		return FText::Format(LOCTEXT("ToggleHelperFormat", "<{0}>"), FText::FromString(KeyName));
	}
	else
	{
		return FText();
	}
}

FText SFlareMainOverlay::GetCurrentMenuName() const
{
	FText Name;
	
	if (MenuManager->IsMenuOpen())
	{
		Name = AFlareMenuManager::GetMenuName(MenuManager->GetCurrentMenu());
		if (Name.ToString().Len() == 0)
		{
			Name = AFlareMenuManager::GetMenuName(MenuManager->GetPreviousMenu());
		}
	}

	if (Name.ToString().Len() == 0)
	{
		Name = LOCTEXT("FlyingText", "Flying");
	}

	return FText::FromString(Name.ToString().ToUpper());
}

const FSlateBrush* SFlareMainOverlay::GetCurrentMenuIcon() const
{
	if (MenuManager->IsMenuOpen())
	{
		if (MenuManager->GetCurrentMenu() != EFlareMenu::MENU_None)
		{
			return AFlareMenuManager::GetMenuIcon(MenuManager->GetCurrentMenu());
		}
		else if (MenuManager->GetPreviousMenu() != EFlareMenu::MENU_None)
		{
			return AFlareMenuManager::GetMenuIcon(MenuManager->GetPreviousMenu());
		}
	}

	return AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Sector);
}

FText SFlareMainOverlay::GetPlayerInfo() const
{
	return PlayerInfoText;
}

EVisibility SFlareMainOverlay::GetPlayerInfoVisibility() const
{
	return (PlayerInfoText.ToString().Len() ? EVisibility::Visible : EVisibility::Collapsed);
}

EVisibility SFlareMainOverlay::GetHintVisibility() const
{
	return (MenuManager->GetPC()->GetNavHUD()->IsHUDVisible()) ? EVisibility::Visible : EVisibility::Hidden;
}

void SFlareMainOverlay::OnOpenMenu(EFlareMenu::Type Menu)
{
	MenuManager->OpenMenu(Menu);
}

void SFlareMainOverlay::OnBack()
{
	if (MenuManager->HasPreviousMenu())
	{
		MenuManager->Back();
		MenuManager->GetPC()->ClientPlaySound(MenuManager->GetPC()->GetSoundManager()->NegativeClickSound);
	}
	else if (MenuManager->GetPC()->GetShipPawn())
	{
		OnCloseMenu();
	}
}

void SFlareMainOverlay::OnCloseMenu()
{
	if (MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Main)
	{
		MenuManager->OpenMenu(EFlareMenu::MENU_Quit);
	}
	else if (MenuManager->IsMenuOpen())
	{
		MenuManager->CloseMenu();
	}
	else
	{
		MenuManager->CloseMainOverlay();
	}
}



#undef LOCTEXT_NAMESPACE
