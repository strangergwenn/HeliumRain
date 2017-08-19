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
	void Confirm(FText Title, FText Text, FSimpleDelegate OnConfirmed, FSimpleDelegate OnCancel, FSimpleDelegate OnIgnore);
	
	/** Is this open */
	bool IsOpen() const;

	/** Close this window */
	void Close();


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/
	
	/** Get the info text */
	FText GetText() const;

	/** Get the info title */
	FText GetTitle() const;

	/** Confirmed action */
	void OnConfirmed();

	/** Cancelled action */
	void OnCancelled();

	/** Ignored action */
	void OnIgnored();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** HUD reference */
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>      MenuManager;

	// Slate data
	FText                                        InfoText;
	FText                                        InfoTitle;
	FSimpleDelegate                              OnConfirmedCB;
	FSimpleDelegate                              OnCancelCB;
	FSimpleDelegate                              OnIgnoreCB;

	// Widgets
	TSharedPtr<SFlareButton>                     OKButton;
	TSharedPtr<SFlareButton>                     CancelButton;
	TSharedPtr<SFlareButton>                     IgnoreButton;


};
