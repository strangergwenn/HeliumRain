#pragma once

#include "../../Flare.h"
#include "../Components/FlareNotification.h"
#include "../FlareUITypes.h"


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
	void Notify(FText Text, FText Info, FName Tag, EFlareNotification::Type Type, bool Pinned, EFlareMenu::Type TargetMenu, FFlareMenuParameterData TargetInfo = FFlareMenuParameterData());

	/** Remove all notifications with the given tag */
	void ClearNotifications(FName Tag, bool Now);

	/** Remove all notifications from the screen */
	void FlushNotifications();

	/** Hide all notifications from the screen */
	void HideNotifications();


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/
	
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	/** Get the current background */
	const FSlateBrush* GetBackgroundBrush() const;

	/** Get the visibility of the objective info */
	EVisibility GetObjectiveVisibility() const;

	/** Get the text for the show/hide button */
	FText GetHideText() const;

	/** Is the button visible ? */
	EVisibility GetHideButtonVisibility() const;

	/** Show/hide button was clicked */
	void OnHideClicked();


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
	TSharedPtr<SFlareButton>                        NotificationVisibleButton;
	bool                                            NotificationsVisible;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	/** Is this the first notification ? */
	bool IsFirstNotification(SFlareNotification* Notification);

	/** Are we showing notifications ? */
	bool AreNotificationsVisible() const;


};
