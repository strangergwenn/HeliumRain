#pragma once

#include "../../Flare.h"
#include "../Components/FlareColorPanel.h"
#include "../Components/FlareList.h"
#include "../Components/FlareCompanyInfo.h"


class UFlareCompany;


class SFlareTradeRouteInfo : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareTradeRouteInfo){}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareMenuManager>, MenuManager)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);
	
	/** Generate the trade route list */
	void UpdateTradeRouteList();

	/** Clear the trade route list */
	void Clear();

	/** Info text */
	FText GetDetailText(UFlareTradeRoute* TradeRoute) const;

	/** Create a new trade route and open trade route menu */
	void OnNewTradeRouteClicked();

	/** Inspect trade route */
	void OnInspectTradeRouteClicked(UFlareTradeRoute* TradeRoute);

	/** Delete trade route */
	void OnDeleteTradeRoute(UFlareTradeRoute* TradeRoute);

	/** Delete trade route (confirmed) */
	void OnDeleteTradeRouteConfirmed(UFlareTradeRoute* TradeRoute);
		

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Menu manager
	TWeakObjectPtr<class AFlareMenuManager>  MenuManager;
	
	// Menu data
	TSharedPtr<SVerticalBox>                 TradeRouteList;


};
