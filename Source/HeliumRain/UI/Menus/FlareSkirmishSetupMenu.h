#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareDropList.h"
#include "../Components/FlareListItem.h"

#include "SBackgroundBlur.h"


struct FFlareCompanyDescription;
class AFlareMenuManager;


class SFlareSkirmishSetupMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareSkirmishSetupMenu){}

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
		Content callbacks
	----------------------------------------------------*/

	TSharedRef<ITableRow> OnGenerateSpacecraftLine(TSharedPtr<FFlareSkirmishSpacecraftOrder> Item, const TSharedRef<STableViewBase>& OwnerTable);
	
	TSharedRef<SWidget> OnGenerateCompanyComboLine(FFlareCompanyDescription Item);

	FText OnGetCurrentCompanyComboLine() const;

	bool IsStartDisabled() const;

	FText GetStartHelpText() const;

	bool CanStartPlaying(FText& Reason) const;


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Start the process of ordering a new ship */
	void OnOrderShip(bool ForPlayer);

	/** Order a new ship */
	void OnOrderShipConfirmed(FFlareSpacecraftDescription* Spacecraft);

	/** Upgrade spacecraft */
	void OnUpgradeSpacecraft(TSharedPtr<FFlareSkirmishSpacecraftOrder> Order);

	/** Close upgrade panel */
	void OnCloseUpgradePanel();

	/** Start engagement */
	void OnStartSkirmish();

	/** Quit the menu */
	void OnMainMenu();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Data
	TWeakObjectPtr<class AFlareMenuManager>                  MenuManager;

	// Widgets
	TSharedPtr<SFlareDropList<FFlareCompanyDescription>>     CompanySelector;
	TSharedPtr<SListView<TSharedPtr<FFlareSkirmishSpacecraftOrder>>>   PlayerSpacecraftList;
	TSharedPtr<SListView<TSharedPtr<FFlareSkirmishSpacecraftOrder>>>   EnemySpacecraftList;

	// Upgrade widgets
	TSharedPtr<SBackgroundBlur>                              UpgradeBox;
	TSharedPtr<SVerticalBox>                                 OrbitalEngineBox;
	TSharedPtr<SVerticalBox>                                 RCSBox;
	TSharedPtr<SHorizontalBox>                               WeaponBox;

	// State
	TArray<TSharedPtr<FFlareSkirmishSpacecraftOrder>>        PlayerSpacecraftListData;
	TArray<TSharedPtr<FFlareSkirmishSpacecraftOrder>>        EnemySpacecraftListData;
	bool                                                     IsOrderingForPlayer;

};
