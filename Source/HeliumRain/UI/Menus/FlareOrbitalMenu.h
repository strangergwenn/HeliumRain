#pragma once

#include "../../Flare.h"
#include "../Components/FlarePlanetaryBox.h"
#include "../../Game/FlareSimulatedSector.h"


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

	/** Stop fast forward */
	void StopFastForward();

	/** A notification was received, stop */
	void RequestStopFastForward();

	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;


protected:

	/*----------------------------------------------------
		Drawing
	----------------------------------------------------*/
	
	/** Update the map with new data from the planetarium */
	void UpdateMap();

	/** Update the map for a specific celestial body */
	void UpdateMapForBody(TSharedPtr<SFlarePlanetaryBox> Map, const FFlareSectorCelestialBodyDescription* Body);
	

	/*----------------------------------------------------
		Content callbacks
	----------------------------------------------------*/
		
	/** Get the text for the fast-forward feature */
	FText GetFastForwardText() const;

	/** Get the icon for the fast forward button */
	const FSlateBrush* GetFastForwardIcon() const;

	/** Visibility setting for the fast-forward feature */
	bool IsFastForwardDisabled() const;

	/** Get the current date */
	FText GetDateText() const;

	/** Get the travel text */
	FText GetTravelText() const;

	/** Get a widget's position on the screen */
	FVector2D GetWidgetPosition(int32 Index) const;

	/** Get a widget's size on the screen */
	FVector2D GetWidgetSize(int32 Index) const;


	/*----------------------------------------------------
		Action callbacks
	----------------------------------------------------*/
		
	/** Open a sector */
	void OnOpenSector(TSharedPtr<int32> Index);
	
	/** Check if we can fast forward */
	void OnFastForwardClicked();

	/** Fast forward to the next event */
	void OnFastForwardConfirmed();
		

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	// Game data
	AFlareGame*                                 Game;
	TWeakObjectPtr<class AFlareMenuManager>     MenuManager;

	// Fast forward
	bool                                        FastForwardActive;
	bool                                        FastForwardStopRequested;
	float                                       FastForwardPeriod;
	float                                       TimeSinceFastForward;

	// Components
	TSharedPtr<SFlarePlanetaryBox>              NemaBox;
	TSharedPtr<SFlarePlanetaryBox>              AstaBox;
	TSharedPtr<SFlarePlanetaryBox>              AnkaBox;
	TSharedPtr<SFlarePlanetaryBox>              HelaBox;
	TSharedPtr<SFlarePlanetaryBox>              AdenaBox;
	TSharedPtr<SFlareButton>                    FastForward;
	TMap<UFlareSimulatedSector*, EFlareSectorBattleState::Type> LastSectorBattleState;

};
