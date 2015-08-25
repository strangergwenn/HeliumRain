#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"


class AFlareMenuManager;


class SFlareConfirmationOverlay : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareConfirmationOverlay)
	{}

	SLATE_ARGUMENT(AFlareMenuManager*, MenuManager)

	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);


	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Show the confirmation overlay */
	void Confirm(FText Text, FSimpleDelegate OnConfirmed);
	

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/
	
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	/** Confirmed action */
	void OnConfirmed();

	/** Cancelled action */
	void OnCancelled();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** HUD reference */
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>      MenuManager;

	// Slate data
	TAttribute<FText>                            InfoText;
	FSimpleDelegate                              OnConfirmedCB;


};
