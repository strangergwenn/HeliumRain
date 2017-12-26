#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareDropList.h"
#include "../Components/FlareListItem.h"


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
		Lists
	----------------------------------------------------*/

	TSharedRef<ITableRow> OnGenerateSpacecraftLine(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable);

	void OnPlayerSpacecraftSelectionChanged(TSharedPtr<FInterfaceContainer> Item, ESelectInfo::Type SelectInfo);

	void OnEnemySpacecraftSelectionChanged(TSharedPtr<FInterfaceContainer> Item, ESelectInfo::Type SelectInfo);


	/*----------------------------------------------------
		Content callbacks
	----------------------------------------------------*/

	FText GetPlayerBudget() const;

	FText GetEnemyBudget() const;

	TSharedRef<SWidget> OnGenerateCompanyComboLine(FFlareCompanyDescription Item);

	FText OnGetCurrentCompanyComboLine() const;

	bool IsStartDisabled() const;

	FText GetStartHelpText() const;


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Budget changed */
	void OnBudgetSliderChanged(float Value, bool ForPlayer);

	/** Start the process of ordering a new ship */
	void OnOrderShip(bool ForPlayer);

	/** Order a new ship */
	void OnOrderShipConfirmed(FFlareSpacecraftDescription* Spacecraft);

	/** Upgrade spacecraft */
	void OnUpgradeSpacecraft();

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
	TSharedPtr<SSlider>                                      PlayerBudgetSlider;
	TSharedPtr<SSlider>                                      EnemyBudgetSlider;
	TSharedPtr<SListView<TSharedPtr<FInterfaceContainer>>>   PlayerSpacecraftList;
	TSharedPtr<SListView<TSharedPtr<FInterfaceContainer>>>   EnemySpacecraftList;
	TSharedPtr<SVerticalBox>                                 OrbitalEngineBox;
	TSharedPtr<SVerticalBox>                                 RCSBox;
	TSharedPtr<SHorizontalBox>                               WeaponBox;

	// State
	TArray<TSharedPtr<FInterfaceContainer>>                  PlayerSpacecraftListData;
	TArray<TSharedPtr<FInterfaceContainer>>                  EnemySpacecraftListData;
	bool                                                     IsOrderingForPlayer;
	TSharedPtr<SFlareListItem>                               PreviousSelection;
	TSharedPtr<FInterfaceContainer>                          SelectedItem;

};
