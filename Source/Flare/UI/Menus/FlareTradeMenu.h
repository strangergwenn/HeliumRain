#pragma once

#include "../../Flare.h"

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

	void FillTradeBlock(UFlareSimulatedSpacecraft* TargetSpacecraft, UFlareSimulatedSpacecraft* OtherSpacecraft, TSharedPtr<SVerticalBox> TargetBlock);

	/** Exit this menu */
	void Exit();


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

//	/** Get the travel text */
//	FText GetTravelText() const;

//	/** Visibility setting for the travel button */
//	EVisibility GetTravelVisibility() const;

	/** Get the title */
	FText GetTitle() const;

//	/** Get the sector's description */
//	FText GetSectorDescription() const;

//	/** Get the sector's location */
//	FText GetSectorLocation() const;

	/** Go back to the previous menu*/
	void OnBackClicked();

	void OnSelectSpacecraft(UFlareSimulatedSpacecraft*Spacecraft);

	void OnTransferResources(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource, TSharedPtr<uint32> Quantity);

//	/** Move the selected fleet here */
//	void OnTravelHereClicked();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// HUD reference
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>    MenuManager;

	// Menu components
	TSharedPtr<SVerticalBox>                 LeftBlock;
	TSharedPtr<SVerticalBox>                 RightBlock;
	UFlareSimulatedSector*                   TargetSector;
	UFlareSimulatedSpacecraft*               TargetLeftSpacecraft;
	UFlareSimulatedSpacecraft*               TargetRightSpacecraft;
};
