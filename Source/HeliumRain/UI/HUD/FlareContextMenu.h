#pragma once

#include "../../Flare.h"
#include "../Components/FlareRoundButton.h"
#include "../Components/FlareShipStatus.h"
#include "../Components/FlareSpacecraftInfo.h"


class SFlareContextMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareContextMenu)
	{}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareHUD>, HUD)
	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareMenuManager>, MenuManager)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Set a station as content */
	void SetSpacecraft(AFlareSpacecraft* Target);
	
	/** Open the menu associated to the target */
	void OnClicked();


protected:

	/*----------------------------------------------------
		Internal
	----------------------------------------------------*/

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	/** Get the current position */
	FMargin GetContextMenuPosition() const;

	/** Get the color setting for the button */
	FSlateColor GetColor() const;
	
	/** Get the icon brush */
	const FSlateBrush* GetIcon() const;

	/** Get the current button label */
	FText GetText() const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Menu data
	TWeakObjectPtr<class AFlareHUD>            HUD;
	TWeakObjectPtr<class AFlareMenuManager>    MenuManager;
		
	// Game data
	AFlareSpacecraft*                          PlayerShip;
	AFlareSpacecraft*		                   TargetSpacecraft;
	
};
