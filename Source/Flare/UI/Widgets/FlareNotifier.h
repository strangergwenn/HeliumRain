#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "FlareNotifier.generated.h"


/** Notification data storage */
USTRUCT()
struct FFlareNotificationData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	int32               Index;
	TSharedPtr<SWidget> Widget;
	float               Lifetime;

};

/** Possible notification types */
UENUM()
namespace EFlareNotification
{
	enum Type
	{
		NT_General,
		NT_Help,
		NT_Trading,
		NT_Military
	};
}


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
	void Notify(FText Text, EFlareNotification::Type Type, EFlareMenu::Type TargetMenu, void* TargetInfo);


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/
	
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	/** Get the current margin */
	FMargin GetNotificationMargin(int32 Index) const;

	/** Get the current color */
	FSlateColor GetNotificationColor(int32 Index) const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** HUD reference */
	UPROPERTY()
	TWeakObjectPtr<class AFlareHUD>      OwnerHUD;

	// Settings
	float                                NotificationTimeout;
	float                                NotificationScroll;
	float                                NotificationScrollTime;

	// Notification timeout data
	int32                                NotificationIndex;
	TArray<FFlareNotificationData>       NotificationData;

	// Slate data
	TSharedPtr<SVerticalBox>             NotificationContainer;



};
