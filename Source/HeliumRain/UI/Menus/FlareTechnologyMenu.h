#pragma once

#include "../../Flare.h"
#include "../FlareUITypes.h"

struct FFlareTechnologyDescription;


class SFlareTechnologyMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareTechnologyMenu){}

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
	void Enter();

	/** Exit this menu */
	void Exit();


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Company info text */
	FText GetCompanyTechnologyInfo() const;

	/** Technology name text */
	FText GetTechnologyName() const;

	/** Technology description text */
	FText GetTechnologyDescription() const;

	/** Coloring for the levle text */
	FSlateColor GetTitleTextColor(int32 RowLevel) const;

	/** Is the unlocking of technology disabled */
	bool IsUnlockDisabled() const;

	/** Unlock button text */
	FText GetTechnologyUnlockText() const;

	/** Unlock button hint text */
	FText GetTechnologyUnlockHintText() const;

	/** Technology selected */
	void OnTechnologySelected(const FFlareTechnologyDescription* Technology);
	
	/** The selected technology is unlocked */
	void OnTechnologyUnlocked();

	/** Count of unlocked scannables */
	FText GetUnlockedScannableCount() const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// General data
	TWeakObjectPtr<class AFlareMenuManager>         MenuManager;
	const FFlareTechnologyDescription*              SelectedTechnology;
	
	// Slate objects
	TSharedPtr<SScrollBox>                          TechnologyTree;
	TSharedPtr<SVerticalBox>                        ArtifactList;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	const FFlareTechnologyDescription* GetSelectedTechnology()
	{
		return SelectedTechnology;
	}

};
