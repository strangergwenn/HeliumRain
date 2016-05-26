#pragma once

#include "../../Flare.h"


class AFlareGame;
class AFlareMenuManager;
class FInterfaceContainer;


class SFlareLeaderboardMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareLeaderboardMenu){}

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
	void Enter();

	/** Exit this menu */
	void Exit();


protected:
	

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Target item generator */
	TSharedRef<ITableRow> GenerateCompanyInfo(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable);
	

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	// Game data
	AFlareGame*                                Game;
	TWeakObjectPtr<class AFlareMenuManager>    MenuManager;
	
	// Slate data
	TArray< TSharedPtr<FInterfaceContainer> >                   CompanyListData;
	TSharedPtr< SListView< TSharedPtr<FInterfaceContainer> > >  CompanyList;


};
