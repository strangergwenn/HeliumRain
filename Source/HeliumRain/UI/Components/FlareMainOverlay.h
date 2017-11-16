#pragma once

#include "../../Flare.h"
#include "../../Game/FlareGameTypes.h"
#include "../FlareUITypes.h"
#include "SBackgroundBlur.h"


class AFlareMenuManager;
class SFlareButton;


class SFlareMainOverlay : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareMainOverlay)
	{}

	SLATE_ARGUMENT(AFlareMenuManager*, MenuManager)

	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Construction
	----------------------------------------------------*/
	
	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Add a menu link */
	void AddMenuLink(EFlareMenu::Type Menu);

	/** Setup a button (simple) */
	void SetupMenuLink(TSharedPtr<SFlareButton> Button, EFlareMenu::Type Menu);

	/** Setup a button */
	void SetupMenuLinkSmall(TSharedPtr<SFlareButton> Button, const FSlateBrush* Icon, FText Text);


	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Open the menu list */
	void Open();

	/** Close the menu list */
	void Close();

	/** Is the overlay open */
	bool IsOpen() const;
	

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/
	
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
	/** Are the main buttons visible */
	EVisibility GetGameButtonVisibility(EFlareMenu::Type Menu) const;

	/** Is this disabled */
	bool IsGameButtonDisabled(EFlareMenu::Type Menu) const;

	/** Get overlay position for animation */
	FVector2D GetOverlayPosition() const;

	/** Get overlay size for animation */
	FVector2D GetOverlaySize() const;

	/** Can we go back */
	bool IsBackDisabled() const;

	/** Can we close the overlay */
	bool IsCloseDisabled() const;

	/** Can we close the overlay */
	EVisibility GetCloseVisibility() const;

	/** Get icon for the close button */
	const FSlateBrush* GetCloseIcon() const;
	
	/** Get the key binding for this menu */
	FText GetMenuKey(EFlareMenu::Type Menu) const;

	/** Get the name of the current menu */
	FText GetCurrentMenuName() const;

	/** Get the icon of the current menu */
	const FSlateBrush* GetCurrentMenuIcon() const;

	/** Get the icon color for the current menu */
	FSlateColor GetMenuIconColor(EFlareMenu::Type Menu) const;

	/** Get the current background */
	const FSlateBrush* GetBackgroundBrush() const;
	
	/** Get the player info text */
	FText GetPlayerInfo() const;

	/** Get the player info text visibility */
	EVisibility GetPlayerInfoVisibility() const;
	
	/** Switch menu */
	void OnOpenMenu(EFlareMenu::Type Menu);

	/** Go back */
	void OnBack();

	/** Close the menu */
	void OnCloseMenu();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Menu manager
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>         MenuManager;

	// General data
	bool                                            IsOverlayVisible;
	float                                           OverlayFadeAlpha;
	float                                           OverlayFadeDuration;
	float                                           TitleButtonWidth;
	float                                           TitleButtonHeight;
	FText                                           PlayerInfoText;

	// Slate data
	TSharedPtr<SBackgroundBlur>                     Background;
	TSharedPtr<SImage>                              Border;
	TSharedPtr<SHorizontalBox>                      MenuList;


};
