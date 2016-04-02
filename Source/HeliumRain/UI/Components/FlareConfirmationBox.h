#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"


class AFlarePlayerController;


class SFlareConfirmationBox : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareConfirmationBox)
		: _FullHide(false)
		, _PC(NULL)
	{}

	SLATE_EVENT(FFlareButtonClicked, OnConfirmed)
	SLATE_EVENT(FFlareButtonClicked, OnCancelled)

	SLATE_ARGUMENT(AFlarePlayerController*, PC)
	SLATE_ARGUMENT(FText, ConfirmText)
	SLATE_ARGUMENT(FText, CancelText)
	SLATE_ARGUMENT(bool, FullHide)
			
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Get the display text */
	FText GetBuyText() const;

	/** Show this box for a specific amount */
	void Show(float Amount);

	/** Hide this box */
	void Hide();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UPROPERTY()
	AFlarePlayerController* PC;

	TSharedPtr<SFlareButton> ConfirmButton;
	TSharedPtr<SFlareButton> CancelButton;
	TSharedPtr<STextBlock> CostLabel;

	FText ConfirmText;
	bool FullHide;
	int32 Amount;


};
