#pragma once

#include "../../Flare.h"
#include "../../Game/FlareGameTypes.h"


class SFlareNotifier;
class AFlareMenuManager;


class SFlareNotification : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareNotification)
		: _TargetInfo(FFlareMenuParameterData())
	{}

	SLATE_ARGUMENT(AFlareMenuManager*, MenuManager)
	SLATE_ARGUMENT(SFlareNotifier*, Notifier)
	SLATE_ARGUMENT(EFlareNotification::Type, Type)
	SLATE_ARGUMENT(FText, Text)
	SLATE_ARGUMENT(FText, Info)
	SLATE_ARGUMENT(FName, Tag)
	SLATE_ARGUMENT(bool, Pinned)
	SLATE_ARGUMENT(EFlareMenu::Type, TargetMenu)
	SLATE_ARGUMENT(FFlareMenuParameterData, TargetInfo)

	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Can we safely delete this ? */
	bool IsFinished() const;

	/** Check if this notification is similar to... */
	bool IsDuplicate(const FName& OtherTag) const;

	/** Complete this notification */
	void Finish(bool Now = true);

	
	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/
	
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	/** Blur intensity */
	TOptional<int32> GetNotificationBlurRadius() const;

	/** Blur intensity */
	float GetNotificationBlurStrength() const;

	/** Get the current color */
	FSlateColor GetNotificationColor(EFlareNotification::Type Type) const;

	/** Get the current text color */
	FSlateColor GetNotificationTextColor() const;

	/** Get the current background color */
	FSlateColor GetNotificationBackgroundColor() const;

	/** Get the visibility of the clickable icon */
	EVisibility GetClickableIconVisibility() const;

	/** Get the visibility of the lifetime icon */
	EVisibility GetLifetimeIconVisibility() const;

	/** Get the size of the lifetime icon */
	FOptionalSize GetLifetimeSize() const;

	/** Get the margins */
	FMargin GetNotificationMargins() const;

	/** We clicked the close button */
	void OnNotificationDismissed();

	/** We clicked the notification */
	FReply OnNotificationClicked();

	

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Settings
	float                                NotificationFinishDuration;
	float                                NotificationTimeout;
	float                                NotificationScroll;
	float                                NotificationEnterDuration;
	float                                NotificationExitDuration;

	// HUD reference
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager> MenuManager;

	SFlareNotifier*                      Notifier;

	// Gameplay
	float                                Lifetime;
	bool                                 ForcedLife;
	TEnumAsByte<EFlareMenu::Type>        TargetMenu;
	FFlareMenuParameterData              TargetInfo;
	FText                                Text;
	FName                                Tag;
	
	// Fade data
	TSharedPtr<SButton>                  Button;
	float                                CurrentAlpha;
	float                                CurrentMargin;
	float                                LastHeight;


};
