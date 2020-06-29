#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"

#include <Widgets/SCompoundWidget.h>
#include <Widgets/Layout/SWidgetSwitcher.h>
#include <Widgets/SBoxPanel.h>


class SFlareTabView : public SCompoundWidget
{
public:

	/*----------------------------------------------------
		Tab slot class
	----------------------------------------------------*/

	class FSlot : public TSlotBase<FSlot>
	{
	public:

		FSlot()
			: TSlotBase<FSlot>()
			, HeaderText()
			, HeaderHelpText()
		{
		}

		FSlot& Header(FText Text)
		{
			HeaderText = Text;
			return *this;
		}

		FSlot& HeaderHelp(FText Text)
		{
			HeaderHelpText = Text;
			return *this;
		}

		FSlot& Expose(FSlot*& OutVarToInit)
		{
			OutVarToInit = this;
			return *this;
		}

		FText HeaderText;
		FText HeaderHelpText;
	};


	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareTabView)
	{}

	SLATE_SUPPORTS_SLOT(FSlot)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Creates a new widget slot */
	static SFlareTabView::FSlot& Slot()
	{
		return *(new SFlareTabView::FSlot());
	}

	/** Get the current widget slot to activate */
	int32 GetCurrentTabIndex() const;


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Check if this tab is the current tab */
	bool IsCurrentTab(int32 Index) const;

	/** Set the current tab index */
	void SetCurrentTabIndex(int32 Index);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Data
	int32                                         CurrentTabIndex;

	// Widgets
	TSharedPtr<SHorizontalBox>                    Header;
	TSharedPtr<SWidgetSwitcher>                   Content;
	
	

};
