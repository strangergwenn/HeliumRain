
#include "../../Flare.h"
#include "FlareOrbitalMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "STextComboBox.h"


#define LOCTEXT_NAMESPACE "FlareOrbitalMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareOrbitalMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	Game = MenuManager->GetPC()->GetGame();

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.Padding(Theme.ContentPadding)
		[
			SNew(SHorizontalBox)

			// Icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SImage).Image(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Orbit))
			]

			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TitleFont)
				.Text(LOCTEXT("Orbital", "ORBITAL MAP"))
			]

			// Quit
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("SaveQuit", "Save and quit"))
				.ToolTipText(LOCTEXT("SaveQuitInfo", "Save the game and go back to the main menu"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Main, true))
				.OnClicked(this, &SFlareOrbitalMenu::OnMainMenu)
			]
			
			// Close
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("Close", "Close"))
				.ToolTipText(LOCTEXT("CloseInfo", "Close the menu and go back to flying the ship"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Exit, true))
				.OnClicked(this, &SFlareOrbitalMenu::OnExit)
			]
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(200, 20))
		[
			SNew(SImage).Image(&Theme.SeparatorBrush)
		]

		// Content
		/*+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.ContentPadding)
		[
		]*/
	];

}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareOrbitalMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}

void SFlareOrbitalMenu::Enter()
{
	FLOG("SFlareOrbitalMenu::Enter");
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
}

void SFlareOrbitalMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Drawing
----------------------------------------------------*/

int32 SFlareOrbitalMenu::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& ClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	FVector2D PlanetCenter = ClippingRect.GetSize() / 2;

	DrawOrbitPath(AllottedGeometry, ClippingRect, OutDrawElements, LayerId, PlanetCenter, 200, 0, 200, 360);

	DrawOrbitPath(AllottedGeometry, ClippingRect, OutDrawElements, LayerId, PlanetCenter, 250, 0, 300, 360);

	DrawOrbitPath(AllottedGeometry, ClippingRect, OutDrawElements, LayerId, PlanetCenter, 350, -90, 300, 90);

	return SCompoundWidget::OnPaint(Args, AllottedGeometry, ClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

void SFlareOrbitalMenu::DrawOrbitPath(const FGeometry& AllottedGeometry, const FSlateRect& ClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
                                      FVector2D PlanetCenter, int32 RadiusA, int32 AngleA, int32 RadiusB, int32 AngleB) const
{
	// Setup initial data
	float InitialDistance = AngleB - AngleA;
	int32 InitialAngleA = AngleA;
	int32 DrawnDistance = 0;
	int32 DrawnSegments = 0;
	int32 MaxAngleInSegment = 30;

	// Get the rendering parametrs
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor WireColor = Theme.NeutralColor;
	WireColor.A = Theme.DefaultAlpha;

	// Compute initial points
	float X, Y;
	FMath::PolarToCartesian(RadiusA, FMath::DegreesToRadians(AngleA), X, Y);
	FVector2D PointA = FVector2D(X, Y);
	FMath::PolarToCartesian(RadiusB, FMath::DegreesToRadians(AngleB), X, Y);
	FVector2D PointB = FVector2D(X, Y);

	// Draw a series of segments until the path has been done.
	do
	{
		// Compute intermediate angles
		int32 CurrentAngleA = AngleA + DrawnSegments * MaxAngleInSegment;
		int32 CurrentAngleB = FMath::Min(CurrentAngleA + MaxAngleInSegment, AngleB);
		float CurrentAngleARad = FMath::DegreesToRadians(CurrentAngleA);
		float CurrentAngleBRad = FMath::DegreesToRadians(CurrentAngleB);
		
		// Polar equation of an ellipse : point A
		float CurrentRadiusA = (RadiusA * RadiusB) / FMath::Sqrt(
			FMath::Square(RadiusB * FMath::Cos(CurrentAngleARad)) +
			FMath::Square(RadiusA * FMath::Sin(CurrentAngleARad))
			);
		FMath::PolarToCartesian(CurrentRadiusA, FMath::DegreesToRadians(CurrentAngleA), X, Y);
		FVector2D CurrentPointA = FVector2D(X, Y);

		// Polar equation of an ellipse : point B
		float CurrentRadiusB = (RadiusA * RadiusB) / FMath::Sqrt(
			FMath::Square(RadiusB * FMath::Cos(CurrentAngleBRad)) +
			FMath::Square(RadiusA * FMath::Sin(CurrentAngleBRad))
			);
		FMath::PolarToCartesian(CurrentRadiusB, FMath::DegreesToRadians(CurrentAngleB), X, Y);
		FVector2D CurrentPointB = FVector2D(X, Y);

		// Compute intermediate tangents
		float TangentLength = FMath::Sqrt(2) * ((CurrentAngleB - CurrentAngleA) / 90.0f);
		FVector2D CurrentTangentA = TangentLength * FVector2D(-CurrentPointA.Y, CurrentPointA.X);
		FVector2D CurrentTangentB = TangentLength * FVector2D(-CurrentPointB.Y, CurrentPointB.X);

		// Draw
		FSlateDrawElement::MakeDrawSpaceSpline(
			OutDrawElements,
			LayerId,
			PlanetCenter + CurrentPointA,
			CurrentTangentA,
			PlanetCenter + CurrentPointB,
			CurrentTangentB,
			ClippingRect,
			5,
			ESlateDrawEffect::None,
			WireColor
			);
		
		// Finish
		DrawnDistance += (CurrentAngleB - CurrentAngleA);
		DrawnSegments++;
	}
	while (DrawnDistance < InitialDistance);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareOrbitalMenu::OnMainMenu()
{
	AFlarePlayerController* PC = MenuManager->GetPC();

	PC->GetGame()->SaveWorld(PC);
	PC->GetGame()->DeleteWorld();

	MenuManager->FlushNotifications();
	MenuManager->OpenMenu(EFlareMenu::MENU_Main);
}

void SFlareOrbitalMenu::OnExit()
{
	// TODO is it useful ?
	MenuManager->OpenMenu(EFlareMenu::MENU_Main);
}




#undef LOCTEXT_NAMESPACE

