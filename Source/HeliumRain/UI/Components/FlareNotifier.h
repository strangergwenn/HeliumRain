#pragma once

#include "../../Flare.h"
#include "../Components/FlareNotification.h"


class AFlareMenuManager;
class SFlareButton;


class SFlareNotifier : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareNotifier)
	{}

	SLATE_ARGUMENT(AFlareMenuManager*, MenuManager)

	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Construction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);


	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Show a notification to the user */
	void Notify(FText Text, FText Info, FName Tag, EFlareNotification::Type Type, float Timeout, EFlareMenu::Type TargetMenu, void* TargetInfo = NULL, FName TargetSpacecraft = NAME_None);

	/** Remvoe all notifications from the screen */
	void FlushNotifications();


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/
	
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Menu manager
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>         MenuManager;
	
	// Slate data
	TArray< TSharedPtr<SFlareNotification> >        NotificationData;
	TSharedPtr<SVerticalBox>                        NotificationContainer;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	virtual bool IsFirstNotification(SFlareNotification* Notification);

};
