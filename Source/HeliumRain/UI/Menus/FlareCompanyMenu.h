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

	/** Show the company accounting */
	void ShowCompanyAccounting(UFlareCompany* Target);

	/** Generate a log line separator for days */
	void AddTransactionHeader(int64 Time, int64 Balance, UFlareCompany* Target);

	/** Generate a log line */
	void AddTransactionLog(const FFlareTransactionLogEntry& Entry, UFlareCompany* Target, bool EvenIndex);
	
	/** Generate a log line separator for days */
	void AddAccountingHeader(FText Text, int64 Balance);

	/** Generate an accounting line */
	int64 AddAccountingCategory(EFlareTransactionLogEntry::Type, TArray<int64>& Balances, UFlareCompany* Target, bool EvenIndex);


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Confirm details */
	void OnRename();

	/** Emblem picked */
	void OnEmblemPicked(int32 Index);

	/** Spacecraft picked */
	void OnTransactionLogSectorClicked(UFlareSimulatedSector* Sector);

	/** Spacecraft picked */
	void OnTransactionLogSourceClicked(UFlareSimulatedSpacecraft* Source);

	/** Move accounting to the previous year */
	void OnPreviousYear();

	/** End-year detection */
	bool IsPreviousYearDisabled() const;

	/** Move accounting to the next year */
	void OnNextYear();

	/** End-year detection */
	bool IsNextYearDisabled() const;

	/** Title with current year */
	FText GetAccountingTitle() const;


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
	int32                                    VeryLargeWidth;
	int64                                    CurrentAccountingYear;
	int64                                    CurrentGameYear;

	// Menu data
	TSharedPtr<SFlareColorPanel>             ColorBox;
	TSharedPtr<SFlareList>                   ShipList;
	TSharedPtr<SFlareCompanyInfo>            CompanyInfo;
	TSharedPtr<SFlareTradeRouteInfo>         TradeRouteInfo;
	TSharedPtr<SEditableText>                CompanyName;
	TSharedPtr<SFlareDropList<int32>>        EmblemPicker;
	TSharedPtr<SVerticalBox>                 CompanyLog;
	TSharedPtr<SVerticalBox>                 CompanyAccounting;

};
