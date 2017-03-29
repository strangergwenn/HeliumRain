#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"


DECLARE_DELEGATE_OneParam(FFlareItemPicked, int32)


class SFlareItemArray : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareItemArray)
		: _LineSize(1)
		, _Width(3)
		, _Height(2)
	{}

	SLATE_EVENT(FFlareItemPicked, OnItemPicked)

	SLATE_ARGUMENT(int32, LineSize)
	SLATE_ARGUMENT(float, Width)
	SLATE_ARGUMENT(float, Height)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Add a new item */
	void AddItem(const TSharedRef< SWidget >& InContent);

	/** Delete all items */
	void ClearItems();

	/** Force the selected item */
	void SetSelectedIndex(int32 ItemIndex);

	/** Get selected index */
	int32 GetSelectedIndex() const;

	/** Get an item's content */
	TSharedRef<SWidget> GetItemContent(int32 ItemIndex) const;
		
	/** Choose an item*/
	void OnItemPicked(TSharedPtr<int32> ItemIndex);
	

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Data
	FFlareItemPicked                   OnItemPickedCallback;
	int32                              CurrentIndex;
	int32                              LineSize;
	float                              Width;
	float                              Height;
	
	// Slate data
	TSharedPtr<SGridPanel>             WidgetGrid;
	TArray< TSharedPtr<SFlareButton> > ContentArray;
	

};
