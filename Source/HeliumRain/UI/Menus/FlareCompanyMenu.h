#pragma once

#include "../../Flare.h"
#include "../Components/FlareColorPanel.h"
#include "../Components/FlareShipList.h"
#include "../Components/FlareCompanyInfo.h"


class UFlareCompany;


class SFlareCompanyMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareCompanyMenu){}

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
	void Enter(UFlareCompany* Target);

	/** Exit this menu */
	void Exit();

	/** Generate the trade route list */
	void UpdateTradeRouteList();


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Get the company name */
	FText GetCompanyName() const;
	
	/** Go back to the orbital map */
	void OnOrbit();

	/** Create a new trade route and open trade route menu */
	void OnNewTradeRouteClicked();

	/** Inspect trade route */
	void OnInspectTradeRouteClicked(UFlareTradeRoute* TradeRoute);

	/** Delete trade route */
	void OnDeleteTradeRoute(UFlareTradeRoute* TradeRoute);

    EVisibility GetTradeRouteVisibility() const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Menu manager
	TWeakObjectPtr<class AFlareMenuManager>  MenuManager;

	// Gameplay data
	UFlareCompany*                           Company;

	// Menu data
	TSharedPtr<SFlareColorPanel>             ColorBox;
	TSharedPtr<SFlareShipList>               ShipList;
	TSharedPtr<SFlareCompanyInfo>            CompanyInfo;
	TSharedPtr<SVerticalBox>                 TradeRouteList;


};
