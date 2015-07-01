#pragma once

#include "../../Flare.h"
#include "../../Game/FlareCompany.h"
#include "../Components/FlareColorPanel.h"
#include "../Widgets/FlareShipList.h"


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


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Get the company name */
	FText GetCompanyName() const;

	/** Get the company info */
	FText GetCompanyInfo() const;

	/** Go back to the dahsboard */
	void OnDashboardClicked();


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


};
