#pragma once

#include "../../Flare.h"
#include "../Components/FlareNotification.h"


class AFlareHUD;


class SFlareNotifier : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareNotifier)
	{}

	SLATE_ARGUMENT(AFlareHUD*, OwnerHUD)

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

	/** Show a notification to the user */
	void Notify(FText Text, FText Info, EFlareNotification::Type Type, EFlareMenu::Type TargetMenu, void* TargetInfo);


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/
	
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
		

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** HUD reference */
	UPROPERTY()
	TWeakObjectPtr<class AFlareHUD>      OwnerHUD;

	// Slate data
	TArray< TSharedPtr<SFlareNotification> > NotificationData;
	TSharedPtr<SVerticalBox>             NotificationContainer;


};
