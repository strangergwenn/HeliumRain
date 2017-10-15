#pragma once

#include "../../Flare.h"
#include "../Components/FlareColorPanel.h"
#include "../Components/FlareList.h"
#include "../Components/FlareCompanyInfo.h"
#include "../Components/FlareTradeRouteInfo.h"


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
		Callbacks
	----------------------------------------------------*/

	/** Can we validate new details */
	bool IsRenameDisabled() const;


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

	/** Confirm details */
	void OnRename();

	/** Emblem picked */
	void OnEmblemPicked(int32 Index);
		

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
	TSharedPtr<SFlareList>                   ShipList;
	TSharedPtr<SFlareCompanyInfo>            CompanyInfo;
	TSharedPtr<SFlareTradeRouteInfo>         TradeRouteInfo;
	TSharedPtr<SEditableText>                CompanyName;
	TSharedPtr<SFlareDropList<int32>>        EmblemPicker;


};
