#pragma once

#include "../../Flare.h"
#include "FlareNotification.generated.h"


class AFlareMenuManager;
class SFlareNotifier;


/** Possible notification types */
UENUM()
namespace EFlareNotification
{
	enum Type
	{
		NT_Info,
		NT_Objective,
		NT_Economy,
		NT_Military,
		NT_Quest
	};
}


class SFlareNotification : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareNotification)
		: _TargetInfo(NULL)
		, _TargetSpacecraft(NAME_None)
	{}

	SLATE_ARGUMENT(AFlareMenuManager*, MenuManager)
	SLATE_ARGUMENT(SFlareNotifier*, Notifier)
	SLATE_ARGUMENT(EFlareNotification::Type, Type)
	SLATE_ARGUMENT(FText, Text)
	SLATE_ARGUMENT(FText, Info)
	SLATE_ARGUMENT(FName, Tag)
	SLATE_ARGUMENT(float, Timeout)
	SLATE_ARGUMENT(EFlareMenu::Type, TargetMenu)
	SLATE_ARGUMENT(void*, TargetInfo)
	SLATE_ARGUMENT(FName, TargetSpacecraft)

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
	
	/** Get the current color */
	FSlateColor GetNotificationColor(EFlareNotification::Type Type) const;

	/** Get the current text color */
	FSlateColor GetNotificationTextColor() const;

	/** Get the current background color */
	FSlateColor GetNotificationBackgroundColor() const;

	/** Get the margins */
	FMargin GetNotificationMargins() const;

	/** We clicked something */
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
	void*                                TargetInfo;
	FName                                TargetSpacecraft;
	FText                                Text;
	FName                                Tag;
	
	// Fade data
	TSharedPtr<SButton>                  Button;
	float                                CurrentAlpha;
	float                                CurrentMargin;
	float                                LastHeight;


};
