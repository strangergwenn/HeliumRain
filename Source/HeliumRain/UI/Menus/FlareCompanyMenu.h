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


	/*----------------------------------------------------
		Content helpers
	----------------------------------------------------*/

	/** Show the company property */
	void ShowProperty(UFlareCompany* Target);

	/** Show the company economy log */
	void ShowCompanyLog(UFlareCompany* Target);

	/** Generate a log line */
	void AddTransactionLog(int64 Time, int64 Value, UFlareCompany* Owner, UFlareCompany* Other,
		UFlareSimulatedSector* Sector, FText Source, FText Comment, bool EvenIndex);


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

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
	int32                                    SmallWidth;
	int32                                    LargeWidth;

	// Menu data
	TSharedPtr<SFlareColorPanel>             ColorBox;
	TSharedPtr<SFlareList>                   ShipList;
	TSharedPtr<SFlareCompanyInfo>            CompanyInfo;
	TSharedPtr<SFlareTradeRouteInfo>         TradeRouteInfo;
	TSharedPtr<SEditableText>                CompanyName;
	TSharedPtr<SFlareDropList<int32>>        EmblemPicker;
	TSharedPtr<SVerticalBox>                 CompanyLog;


};
