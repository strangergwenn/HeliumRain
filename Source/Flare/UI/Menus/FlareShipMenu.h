#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareListItem.h"
#include "../Widgets/FlareConfirmationBox.h"
#include "../../Ships/FlareShipComponent.h"
#include "../../Player/FlarePlayerController.h"
#include "../Widgets/FlareTargetActions.h"


class SFlareShipMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareShipMenu){}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareHUD>, OwnerHUD)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Setup the widget with saved data */
	void Setup();

	/** Enter this menu */
	void Enter(IFlareSpacecraftInterface* Target, bool IsEditable = false);

	/** Exit this menu */
	void Exit();

	/** Go to the ship view */
	void LoadTargetShip();

	/** Load a ship */
	void LoadPart(FName InternalName);
	
	/** Update the part list, preselect an item */
	void UpdatePartList(FFlareShipComponentDescription* SelectItem);


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Brush callback */
	const FSlateBrush* GetIconBrush() const;

	/** Title callback */
	FText GetTitleText() const;

	/** Show engines */
	void ShowEngines();

	/** Show RCSs */
	void ShowRCSs();

	/** Show weapons */
	void ShowWeapons(TSharedPtr<int32> WeaponIndex);

	/** Part list generator */
	TSharedRef<ITableRow> GeneratePartInfo(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/** Chose a part */
	void OnPartPicked(TSharedPtr<FInterfaceContainer> Item, ESelectInfo::Type SelectInfo);

	/** Confirmed a part */
	void OnPartConfirmed();

	/** Cancelled a part */
	void OnPartCancelled();

	/** Go back to the dahsboard */
	void OnDashboardClicked();
	

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** HUD reference */
	UPROPERTY()
	TWeakObjectPtr<class AFlareHUD> OwnerHUD;

	/** Target ship to use for customization */
	UPROPERTY()
	IFlareSpacecraftInterface* CurrentShipTarget;

	/** Save data for the ship currently being customized */
	UPROPERTY()
	FFlareShipSave* CurrentShipData;


	// Main UI
	bool                               CanEdit;
	TSharedPtr<STextBlock>             ObjectName;
	TSharedPtr<STextBlock>             ObjectDescription;
	TSharedPtr<SFlareTargetActions>    ObjectActionMenu;

	// Ship UI
	TSharedPtr<SVerticalBox>           ShipCustomizationBox;
	TSharedPtr<SFlareButton>           EngineButton;
	TSharedPtr<SFlareButton>           RCSButton;

	// Ship weapons
	TSharedPtr<SVerticalBox>           WeaponButtonBox;
	int32                              CurrentWeaponIndex;

	/** List of parts being shown right now */
	UPROPERTY()
	TArray< FFlareShipComponentDescription* >                        PartListData;
	TArray< TSharedPtr<FInterfaceContainer> >                     PartListDataShared;
	TSharedPtr< SListView< TSharedPtr<FInterfaceContainer> > >    PartList;

	// Parts list UI
	TSharedPtr<SFlareListItem>         PreviousSelection;
	TSharedPtr<SVerticalBox>           ShipPartCustomizationBox;
	TSharedPtr<STextBlock>             ShipPartPickerTitle;
	TSharedPtr<SFlareConfirmationBox>  BuyConfirmation;
	TSharedPtr<SHorizontalBox>         PartCharacteristicBox;

	// Part list data
	int32                              CurrentPartIndex;
	int32                              CurrentEquippedPartIndex;
	int32                              ShipPartIndex;
	

};
