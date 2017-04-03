#pragma once

#include "../../Flare.h"


class AFlareMenuManager;


class SFlareGameOverMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareGameOverMenu){}

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
		Callbacks
	----------------------------------------------------*/

	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
	/** Aspect ratio X */
	FOptionalSize GetWidth() const;

	/** Aspect ratio Y */
	FOptionalSize GetHeight() const;

	/** Exit */
	void OnQuit();
	

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	// Gameplay data
	float                                      MaxWidth;
	float                                      MaxHeight;

	// Slate data
	TWeakObjectPtr<class AFlareMenuManager>    MenuManager;


};
