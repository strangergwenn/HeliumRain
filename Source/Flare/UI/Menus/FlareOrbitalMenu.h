#pragma once

#include "../../Flare.h"


class AFlareMenuManager;


class SFlareOrbitalMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareOrbitalMenu){}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareMenuManager>, MenuManager)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Setup the widget */
	void Setup();

	/** Enter this menu */
	void Enter();

	/** Exit this menu */
	void Exit();


protected:

	/*----------------------------------------------------
		Drawing
	----------------------------------------------------*/

	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& ClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	/** Draw an orbit path between two points */
	void DrawOrbitPath(const FSlateRect& ClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	                   FVector2D PlanetCenter, int32 RadiusA, int32 AngleA, int32 RadiusB, int32 AngleB) const;

	/** Draw a 90° arc */
	void DrawOrbitSegment(const FSlateRect& ClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
		FVector2D PlanetCenter, FVector2D PointA, FVector2D TangentA, FVector2D PointB, FVector2D TangentB) const;

	/** Get a position from polar coordinates */
	inline FVector2D GetPositionFromPolar(int32 Radius, int32 Angle) const;


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Inspect the company */
	void OnInspectCompany();

	/** Open the company leaderboard */
	void OnOpenLeaderboard();

	/** Back to the main menu */
	void OnMainMenu();

	/** Exit this menu */
	void OnExit();

	/** Open a sector */
	void OnOpenSector(TSharedPtr<int32> Index);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	// Game data
	AFlareGame*                                 Game;
	TWeakObjectPtr<class AFlareMenuManager>     MenuManager;
	
	TSharedPtr<SHorizontalBox>                  SectorsBox;
};
