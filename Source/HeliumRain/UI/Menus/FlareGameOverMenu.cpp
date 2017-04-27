
#include "FlareGameOverMenu.h"
#include "../../Flare.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareSaveGame.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"

#include "Runtime/Engine/Classes/Engine/UserInterfaceSettings.h"
#include "Runtime/Engine/Classes/Engine/RendererSettings.h"


#define LOCTEXT_NAMESPACE "FlareGameOverMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareGameOverMenu::Construct(const FArguments& InArgs)
{
	// Style data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Settings
	int32 Width = 1.5 * Theme.ContentWidth;
	int32 TextWidth = Width - Theme.ContentPadding.Left - Theme.ContentPadding.Right;

	// Buttons
	TSharedPtr<SFlareButton> ExitButton;

	// Build structure
	ChildSlot
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SNew(SBorder)
		.BorderImage(FFlareStyleSet::GetImage("Black"))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				.WidthOverride(this, &SFlareGameOverMenu::GetWidth)
				.HeightOverride(this, &SFlareGameOverMenu::GetHeight)
				[
					SNew(SBorder)
					.BorderImage(FFlareStyleSet::GetImage("Story_GameOver"))
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.VAlign(VAlign_Top)
						.Padding(Theme.ContentPadding)
						.AutoHeight()
						[
							SNew(SHorizontalBox)
						
							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Left)
							[
								SNew(SBox)
								.WidthOverride(2 * Theme.ButtonWidth)
							]

							// Title
							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Center)
							.Padding(Theme.ContentPadding)
							.AutoWidth()
							[
								SNew(STextBlock)
								.Justification(ETextJustify::Center)
								.TextStyle(&Theme.SpecialTitleFont)
								.Text(LOCTEXT("YouDied", "FLEET DESTROYED"))
							]

							// Exit
							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Right)
							[
								SAssignNew(ExitButton, SFlareButton)
								.Width(2)
								.Height(2)
								.Transparent(true)
								.OnClicked(this, &SFlareGameOverMenu::OnQuit)
							]
						]

						// Bottom pane
						+ SVerticalBox::Slot()
						.VAlign(VAlign_Bottom)
						[
							SNew(SBackgroundBlur)
							.BlurRadius(10)
							.BlurStrength(1)
							.Padding(Theme.ContentPadding)
							[
								SNew(STextBlock)
								.Justification(ETextJustify::Center)
								.TextStyle(&Theme.NameFont)
								.Text(LOCTEXT("ShipRecovery", "Your fleet was destroyed and you have been injured. The Nema Colonial Administration is providing you a new start, and a new ship. Try not to destroy this one."))
								.WrapTextAt(TextWidth)
							]
						]
					]
				]
			]
		]
	];

	ExitButton->GetContainer()->SetContent(
		SNew(SBox)
		.WidthOverride(64)
		.HeightOverride(64)
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetIcon("Close"))
		]);
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareGameOverMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareGameOverMenu::Enter()
{
	FLOG("SFlareGameOverMenu::Enter");
	
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
}

void SFlareGameOverMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareGameOverMenu::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	float ViewportScale = GetDefault<UUserInterfaceSettings>(UUserInterfaceSettings::StaticClass())->GetDPIScaleBasedOnSize(FIntPoint(ViewportSize.X, ViewportSize.Y));

	// Aspect ratio
	float AspectRatio = 16.0 / 9.0;
	if (ViewportSize.X / AspectRatio < ViewportSize.Y)
	{
		MaxWidth = ViewportSize.X / ViewportScale;
		MaxHeight = ViewportSize.X / AspectRatio / ViewportScale;
	}
	else
	{
		MaxWidth = ViewportSize.Y * AspectRatio / ViewportScale;
		MaxHeight = ViewportSize.Y / ViewportScale;
	}
}

FOptionalSize SFlareGameOverMenu::GetWidth() const
{
	return MaxWidth;
}

FOptionalSize SFlareGameOverMenu::GetHeight() const
{
	return MaxHeight;
}

void SFlareGameOverMenu::OnQuit()
{
	MenuManager->CloseMenu();
}

#undef LOCTEXT_NAMESPACE

