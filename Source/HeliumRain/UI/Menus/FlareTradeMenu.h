#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareShipList.h"
#include "../Components/FlareConfirmationBox.h"

class UFlareSimulatedSpacecraft;
class IFlareSectorInterface;
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
	
	/** Is the trading part visible or not */
	EVisibility GetTradingVisibility() const;

	/** Is the "back to selection" visible or not */
	EVisibility GetBackToSelectionVisibility() const;

	/** Are the transaction details visible ? */
	EVisibility GetTransactionDetailsVisibility() const;

	/** Are the transaction details visible ? */
	EVisibility GetTransactionInvalidVisibility() const;
	
	/** Get the name of the left spacecraft */
	FText GetLeftSpacecraftName() const;

	/** Get the name of the right spacecraft */
	FText GetRightSpacecraftName() const;
	
	/** Get the transaction details */
	FText GetTransactionDetails() const;

	/** Get the transaction invalid details */
	FText GetTransactionInvalidDetails() const;

	/** Get the resource prices */
	FText GetResourcePriceInfo(FFlareResourceDescription* Resource) const;
	
	/** A spacecraft has been selected, hide the list and show the cargo */
	void OnSpacecraftSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer);

	/** Start transferring a resource */
	void OnTransferResources(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource);

	/** Changed resource quantity, recompute price **/
	void OnResourceQuantityChanged(float Value);

	/** Changed resource quantity, recompute price **/
	void OnResourceQuantityEntered(const FText& TextValue);

	/** Accept a transaction */
	void OnConfirmTransaction();

	/** Cancel a transaction */
	void OnCancelTransaction();

	/** Go back to choosing a ship to trade with */
	void OnBackToSelection();

	/** Update price on confirm button */
	void UpdatePrice();

	/** Return true if the transaction is valid*/
	bool IsTransactionValid() const;

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
	TSharedPtr<SSlider>                             QuantitySlider;
	TSharedPtr<SEditableText>                       QuantityText;
	TSharedPtr<SFlareConfirmationBox>               PriceBox;
	TSharedPtr<SVerticalBox>                        ResourcePriceList;

	// Data
	UFlareSimulatedSector*                          TargetSector;
	UFlareSimulatedSpacecraft*                      TargetLeftSpacecraft;
	UFlareSimulatedSpacecraft*                      TargetRightSpacecraft;

	// Current transaction
	UFlareSimulatedSpacecraft*                      TransactionSourceSpacecraft;
	UFlareSimulatedSpacecraft*                      TransactionDestinationSpacecraft;
	FFlareResourceDescription*                      TransactionResource;
	uint32                                          TransactionQuantity;

};
