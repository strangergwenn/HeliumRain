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

	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime );

protected:

	/*----------------------------------------------------
		Drawing
	----------------------------------------------------*/
	
	/** Update the map with new data from the planetarium */
	void UpdateMap();

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


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	// Game data
	AFlareGame*                                 Game;
	TWeakObjectPtr<class AFlareMenuManager>     MenuManager;
	
	TSharedPtr<SHorizontalBox>                  SectorsBox;
	TSharedPtr<SVerticalBox>                  TravelsBox;
	int64                                       LastUpdateTime;
};
