#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareListItem.h"
#include "../Widgets/FlareConfirmationBox.h"
#include "../../Spacecrafts/FlareSpacecraftComponent.h"
#include "../../Player/FlarePlayerController.h"
#include "../Widgets/FlareSpacecraftInfo.h"


class SFlareShipMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareShipMenu){}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareMenuManager>, MenuManager)
	
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

	/** Load a spacecraft data and setup the interface */
	void LoadTargetSpacecraft();

	/** Load a ship */
	void LoadPart(FName InternalName);
	
	/** Update the part list, preselect an item */
	void UpdatePartList(FFlareSpacecraftComponentDescription* SelectItem);


protected:

	/*----------------------------------------------------
		Content callbacks
	----------------------------------------------------*/

	/** Title icon callback */
	const FSlateBrush* GetTitleIcon() const;

	/** Title callback */
	FText GetTitleText() const;

	/** Is it a station */
	EVisibility GetEngineVisibility() const;

	/** Get a Slate brush for the RCS icon */
	const FSlateBrush* GetRCSIcon() const;

	/** RCS callback */
	FText GetRCSText() const;

	/** Get a Slate brush for the engine icon */
	const FSlateBrush* GetEngineIcon() const;

	/** Engine callback */
	FText GetEngineText() const;

	/** Get a Slate brush for the weapon icons */
	const FSlateBrush* GetWeaponIcon(TSharedPtr<int32> Index) const;

	/** Weapon callback */
	FText GetWeaponText(TSharedPtr<int32> Index) const;

	/** Part list generator */
	TSharedRef<ITableRow> GeneratePartInfo(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable);


	/*----------------------------------------------------
		Action callbacks
	----------------------------------------------------*/

	/** Show engines */
	void ShowEngines();

	/** Show RCSs */
	void ShowRCSs();

	/** Show weapons */
	void ShowWeapons(TSharedPtr<int32> WeaponIndex);

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

	// Menu manager
	TWeakObjectPtr<class AFlareMenuManager>         MenuManager;
	
	//  UI
	bool                                            CanEdit;
	TSharedPtr<STextBlock>                          ObjectName;
	TSharedPtr<STextBlock>                          ObjectClassName;
	TSharedPtr<STextBlock>                          ObjectDescription;
	TSharedPtr<SFlareSpacecraftInfo>                 ObjectActionMenu;
	TSharedPtr<SFlareShipList>                      ShipList;
	TSharedPtr<SVerticalBox>                        ShipCustomizationBox;
	TSharedPtr<SHorizontalBox>                      WeaponButtonBox;

	// Spacecraft data
	IFlareSpacecraftInterface*                      TargetSpacecraft;
	FFlareSpacecraftSave*                           TargetSpacecraftData;
	FFlareSpacecraftComponentDescription*           RCSDescription;
	FFlareSpacecraftComponentDescription*           EngineDescription;
	TArray<FFlareSpacecraftComponentDescription*>   WeaponDescriptions;
	int32                                           CurrentWeaponIndex;
	
	// List of parts being shown right now 
	TArray< FFlareSpacecraftComponentDescription* >               PartListData;
	TArray< TSharedPtr<FInterfaceContainer> >                     PartListDataShared;
	TSharedPtr< SListView< TSharedPtr<FInterfaceContainer> > >    PartList;

	// Parts list UI
	TSharedPtr<SFlareListItem>                      PreviousSelection;
	TSharedPtr<SVerticalBox>                        ShipPartCustomizationBox;
	TSharedPtr<STextBlock>                          ShipPartPickerTitle;
	TSharedPtr<SFlareConfirmationBox>               BuyConfirmation;
	TSharedPtr<SHorizontalBox>                      PartCharacteristicBox;

	// Part list data
	int32                                           CurrentPartIndex;
	int32                                           CurrentEquippedPartIndex;
	int32                                           ShipPartIndex;
	

};
