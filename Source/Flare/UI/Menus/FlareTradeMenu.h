#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareShipList.h"

class UFlareSimulatedSpacecraft;
class UFlareSimulatedSector;
struct FFlareResourceDescription;


class SFlareTradeMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareTradeMenu){}

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
	void Enter(UFlareSimulatedSector* ParentSector, UFlareSimulatedSpacecraft* LeftSpacecraft, UFlareSimulatedSpacecraft* RightSpacecraft);

	/** Fill a content pane with the trading information for Target spacecraft to deal with Other */
	void FillTradeBlock(UFlareSimulatedSpacecraft* TargetSpacecraft, UFlareSimulatedSpacecraft* OtherSpacecraft, TSharedPtr<SHorizontalBox> TargetBlock);

	/** Exit this menu */
	void Exit();


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/
	
	/** Get the title */
	FText GetTitle() const;

	/** Get the name of the left spacecraft */
	FText GetLeftSpacecraftName() const;

	/** Get the name of the right spacecraft */
	FText GetRightSpacecraftName() const;
	
	/** Go back to the previous menu*/
	void OnBackClicked();

	/** A spacecraft has been selected, hide the list and show the cargo */
	void OnSpacecraftSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer);

	/** Resources need to be transferred, do it now */
	void OnTransferResources(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource, TSharedPtr<uint32> Quantity);
	
	/** Go back to choosing a ship to trade with */
	void OnBackToSelection();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// HUD reference
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>         MenuManager;

	// Menu components
	TSharedPtr<SFlareShipList>                      ShipList;
	TSharedPtr<SHorizontalBox>                      LeftCargoBay;
	TSharedPtr<SHorizontalBox>                      RightCargoBay;
	TSharedPtr<STextBlock>                          RightShipText;
	TSharedPtr<SFlareButton>                        BackToShipSelection;

	// Data
	UFlareSimulatedSector*                          TargetSector;
	UFlareSimulatedSpacecraft*                      TargetLeftSpacecraft;
	UFlareSimulatedSpacecraft*                      TargetRightSpacecraft;

};
