#pragma once

#include "../../Flare.h"
#include "../../Game/FlareGameTypes.h"
#include "FlareButton.h"


class SFlareTechnologyInfo : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareTechnologyInfo)
		: _Technology(NULL)
	{}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareMenuManager>, MenuManager)
	SLATE_ARGUMENT(const FFlareTechnologyDescription*, Technology)
	SLATE_EVENT(FFlareButtonClicked, OnClicked)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Mouse entered (tooltip) */
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	/** Mouse left (tooltip) */
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	/** Get the category color */
	FSlateColor GetCategoryColor() const;

	/** Get the background to display */
	const FSlateBrush* GetBackgroundBrush() const;

	/** Get the icon to display */
	const FSlateBrush* GetUnlockIcon() const;

	/** Cost text */
	FText GetTechnologyCost() const;
	
	/** Technology item clicked */
	FReply OnButtonClicked();



protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/

	// Spacecraft data
	TWeakObjectPtr<class AFlareMenuManager>         MenuManager;
	const FFlareTechnologyDescription*              Technology;
	FFlareButtonClicked                             OnClicked;
	

};
