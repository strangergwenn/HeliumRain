
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
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.ContentPadding)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("Sector 1", "Sector 1"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_None, true))
				.OnClicked(this, &SFlareOrbitalMenu::OnOpenSector, TSharedPtr<int32>(new int32(1)))
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("Sector 2", "Sector 2"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_None, true))
				.OnClicked(this, &SFlareOrbitalMenu::OnOpenSector, TSharedPtr<int32>(new int32(2)))
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("Sector 3", "Sector 3"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_None, true))
				.OnClicked(this, &SFlareOrbitalMenu::OnOpenSector, TSharedPtr<int32>(new int32(3)))
			]
		]
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

	DrawOrbitPath(ClippingRect, OutDrawElements, LayerId, PlanetCenter, 200, 0, 200, 360);

	//DrawOrbitPath(ClippingRect, OutDrawElements, LayerId, PlanetCenter, 350, 45, 300, 225);

	//DrawOrbitPath(ClippingRect, OutDrawElements, LayerId, PlanetCenter, 350, -90, 300, 90);

	return SCompoundWidget::OnPaint(Args, AllottedGeometry, ClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

void SFlareOrbitalMenu::DrawOrbitPath(const FSlateRect& ClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	FVector2D PlanetCenter, int32 RadiusA, int32 AngleA, int32 RadiusB, int32 AngleB) const
{
	// Setup initial data
	float InitialDistance = AngleB - AngleA;
	int32 InitialAngleA = AngleA;
	int32 DrawnDistance = 0;
	int32 DrawnSegments = 0;
	int32 MaxAngleInSegment = 90;
	
	// Compute initial points
	FVector2D PointA = GetPositionFromPolar(RadiusA, AngleA);
	FVector2D PointB = GetPositionFromPolar(RadiusB, AngleB);

	// Draw a series of segments until the path has been done.
	do
	{
		// Compute intermediate angles
		int32 CurrentAngleA = AngleA + DrawnSegments * MaxAngleInSegment;
		int32 CurrentAngleB = FMath::Min(CurrentAngleA + MaxAngleInSegment, AngleB);
		float CurrentAngleARad = FMath::DegreesToRadians(CurrentAngleA - InitialAngleA);
		float CurrentAngleBRad = FMath::DegreesToRadians(CurrentAngleB - InitialAngleA);

		// Polar equation of an ellipse : point A
		float CurrentRadiusA = (RadiusA * RadiusB) / FMath::Sqrt(
			FMath::Square(RadiusB * FMath::Cos(CurrentAngleARad)) +
			FMath::Square(RadiusA * FMath::Sin(CurrentAngleARad))
			);
		FVector2D CurrentPointA = GetPositionFromPolar(CurrentRadiusA, CurrentAngleA);;

		// Polar equation of an ellipse : point B
		float CurrentRadiusB = (RadiusA * RadiusB) / FMath::Sqrt(
			FMath::Square(RadiusB * FMath::Cos(CurrentAngleBRad)) +
			FMath::Square(RadiusA * FMath::Sin(CurrentAngleBRad))
			);
		FVector2D CurrentPointB = GetPositionFromPolar(CurrentRadiusB, CurrentAngleB);

		// Tangents vary depending on the angle and ellipse parameters
		float TangentSubdivisionRatio = ((CurrentAngleB - CurrentAngleA) / 90.0f);
		float TangentLength = TangentSubdivisionRatio * FMath::Sqrt(2);// *(1 + 0.3 * FMath::Sin((PI * (CurrentAngleA - InitialAngleA)) / 180));

		 // Compute intermediate tangents
		FVector2D CurrentTangentA = TangentLength * FVector2D(-CurrentPointA.Y, CurrentPointA.X);
		FVector2D CurrentTangentB = TangentLength * FVector2D(-CurrentPointB.Y, CurrentPointB.X);
		
		// Draw
		DrawOrbitSegment(ClippingRect, OutDrawElements, LayerId, PlanetCenter, CurrentPointA, CurrentTangentA, CurrentPointB, CurrentTangentB);
		DrawnDistance += (CurrentAngleB - CurrentAngleA);
		DrawnSegments++;

	} while (DrawnDistance < InitialDistance);
}

void SFlareOrbitalMenu::DrawOrbitSegment(const FSlateRect& ClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	FVector2D PlanetCenter, FVector2D PointA, FVector2D TangentA, FVector2D PointB, FVector2D TangentB) const
{
	// Get the rendering parametrs
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor WireColor = Theme.NeutralColor;
	WireColor.A = Theme.DefaultAlpha;
	
	// Draw
	FSlateDrawElement::MakeDrawSpaceSpline(
		OutDrawElements,
		LayerId,
		PlanetCenter + PointA,
		TangentA,
		PlanetCenter + PointB,
		TangentB,
		ClippingRect,
		8,
		ESlateDrawEffect::None,
		WireColor
		);
}

inline FVector2D SFlareOrbitalMenu::GetPositionFromPolar(int32 Radius, int32 Angle) const
{
	float X, Y;
	FMath::PolarToCartesian(Radius, FMath::DegreesToRadians(Angle), X, Y);
	return FVector2D(X, Y);
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

void SFlareOrbitalMenu::OnOpenSector(TSharedPtr<int32> Index)
{
	// Fred
}


#undef LOCTEXT_NAMESPACE

