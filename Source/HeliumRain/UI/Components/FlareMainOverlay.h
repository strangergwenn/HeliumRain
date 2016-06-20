#pragma once

#include "../../Flare.h"


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

	/** Setup a button */
	void SetupMenuLink(TSharedPtr<SFlareButton> Button, const FSlateBrush* Icon, FText Text);


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
	
	/** Can we go back */
	bool IsBackDisabled() const;

	/** Get the name of the current menu */
	FText GetCurrentMenuName() const;

	/** Get the icon of the current menu */
	const FSlateBrush* GetCurrentMenuIcon() const;

	/** Get the spacecraft info text */
	FText GetSpacecraftInfo() const;

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
	float                                           TitleButtonWidth;
	float                                           TitleButtonHeight;

	// Slate data
	TSharedPtr<SBorder>                             Background;
	TSharedPtr<SHorizontalBox>                      MenuList;


};
