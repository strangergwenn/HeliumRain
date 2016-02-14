#pragma once

#include "../../Flare.h"
#include "../../Data/FlareSpacecraftCatalog.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareListItem.h"


class SFlareFactoryInfo;
class UFlareFactory;
class AFlareMenuManager;


class SFlareSpacecraftOrderOverlay : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareSpacecraftOrderOverlay)
	{}

	SLATE_ARGUMENT(AFlareMenuManager*, MenuManager)

	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);


	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Show the overlay */
	void Open(SFlareFactoryInfo* FactoryMenu);
	
	/** Close the overlay */
	void Close();


	/*----------------------------------------------------
		Content callbacks
	----------------------------------------------------*/
		
	/** Craft a line for the list */
	TSharedRef<ITableRow> OnGenerateSpacecraftLine(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/** The selection was changed */
	void OnSpacecraftSelectionChanged(TSharedPtr<FInterfaceContainer> Item, ESelectInfo::Type SelectInfo);

	/** Can we confirm */
	EVisibility GetConfirmVisibility() const;


	/*----------------------------------------------------
		Action callbacks
	----------------------------------------------------*/

	/** Confirmed, start production */
	void OnConfirmed();

	/** Don't do anything and close */
	void OnClose();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** HUD reference */
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>                   MenuManager;

	// Spacecraft building
	UFlareFactory*                                            TargetFactory;
	TArray<TSharedPtr<FInterfaceContainer>>                   SpacecraftList;
	TSharedPtr<SListView<TSharedPtr<FInterfaceContainer>>>    SpacecraftSelector;

	// Slate data
	TSharedPtr<SFlareListItem>                                PreviousSelection;
	TSharedPtr<FInterfaceContainer>                           SelectedItem;

};
