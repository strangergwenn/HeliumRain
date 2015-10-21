#pragma once

#include "../../Flare.h"
#include "../Components/FlarePlanetaryBox.h"


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

	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime );

protected:

	/*----------------------------------------------------
		Drawing
	----------------------------------------------------*/
	
	/** Update the map with new data from the planetarium */
	void UpdateMap();

	/** Update the map for a specific celestial body */
	void UpdateMapForBody(TSharedPtr<SFlarePlanetaryBox> Map, const FFlareSectorCelestialBodyDescription* Body);

	/** Update the list of current travels */
	void UpdateTravels();


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Inspect the company */
	void OnInspectCompany();

	/** Open the company leaderboard */
	void OnOpenLeaderboard();

	/** Back to the main menu */
	void OnMainMenu();
	
	/** Open a sector */
	void OnOpenSector(TSharedPtr<int32> Index);

	/** Get a widget's position on the screen */
	FVector2D GetWidgetPosition(int32 Index) const;

	/** Get a widget's size on the screen */
	FVector2D GetWidgetSize(int32 Index) const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	// Game data
	AFlareGame*                                 Game;
	TWeakObjectPtr<class AFlareMenuManager>     MenuManager;
	int64                                       LastUpdateTime;

	// Components
	TSharedPtr<SFlarePlanetaryBox>              NemaBox;
	TSharedPtr<SFlarePlanetaryBox>              AstaBox;
	TSharedPtr<SFlarePlanetaryBox>              AnkaBox;
	TSharedPtr<SFlarePlanetaryBox>              AriadneBox;
	TSharedPtr<SVerticalBox>                    TravelsBox;
};
