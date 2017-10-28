#pragma once

#include "../../Flare.h"
#include "FlareButton.h"


class UFlareCompany;
class UFlareSimulatedSector;


class SFlareSectorButton : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareSectorButton)
		: _Sector(NULL)
		, _PlayerCompany(NULL)
	{}

	SLATE_ARGUMENT(UFlareSimulatedSector*, Sector)
	SLATE_ARGUMENT(UFlareCompany*, PlayerCompany)
	SLATE_EVENT(FFlareButtonClicked, OnClicked)
			
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Mouse entered (tooltip) */
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	/** Mouse left (tooltip) */
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	/** Should we display fleets */
	bool ShouldDisplayFleets() const;

	/** Get the visibility of text */
	EVisibility GetBottomTextVisibility() const;

	/** Get the text to display */
	FText GetSectorTitle() const;

	/** Get the text to display */
	FText GetSectorText() const;

	/** Brush callback */
	const FSlateBrush* GetBackgroundBrush() const;

	/** Get the main color */
	FSlateColor GetMainColor() const;

	/** Get the color to use for the border */
	FSlateColor GetBorderColor() const;

	/** Get the shadow color */
	FLinearColor GetShadowColor() const;

	/** Mouse clicked */
	FReply OnButtonClicked();


protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/
	
	// Data
	FFlareButtonClicked            OnClicked;
	UFlareSimulatedSector*         Sector;
	UFlareCompany*                 PlayerCompany;

	// Slate data
	TSharedPtr<STextBlock>         TextBlock;
	TSharedPtr<SHorizontalBox>     FleetBox;


};
