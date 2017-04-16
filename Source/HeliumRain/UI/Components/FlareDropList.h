#pragma once

#include "../../Flare.h"
#include "../Components/FlareItemArray.h"
#include "Runtime/Slate/Public/Widgets/Colors/SColorWheel.h"


DECLARE_DELEGATE_OneParam(FFlareItemPicked, int32)


class SFlareDropList : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareDropList)
		: _LineSize(1)
		, _HeaderWidth(3)
		, _HeaderHeight(1)
		, _ItemWidth(3)
		, _ItemHeight(1)
		, _ShowColorWheel(false)
	{}

	SLATE_EVENT(FFlareItemPicked, OnItemPicked)

	SLATE_ARGUMENT(int32, LineSize)
	SLATE_ARGUMENT(float, HeaderWidth)
	SLATE_ARGUMENT(float, HeaderHeight)
	SLATE_ARGUMENT(float, ItemWidth)
	SLATE_ARGUMENT(float, ItemHeight)
	SLATE_ARGUMENT(bool, ShowColorWheel)		
	
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

	/** Selected index */
	int32 GetSelectedIndex() const;

	/** Get an item's content */
	TSharedRef<SWidget> GetItemContent(int32 ItemIndex) const;

	/** Drop the list down */
	void OnHeaderClicked();

	/** Choose a color*/
	void OnItemPicked(int32 ItemIndex);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	// Data
	bool                          IsDropped;
	bool                          HasColorWheel;
	int32                         LineSize;
	FFlareItemPicked              OnItemPickedCallback;
	
	// Slate data
	TSharedPtr<SFlareButton>      HeaderButton;
	TSharedPtr<SFlareItemArray>   ItemArray;
	TSharedPtr<SColorWheel>       ColorWheel;
	TSharedPtr<SSlider>           ColorSlider;
	TArray< TSharedRef<SWidget> > ContentArray;
	

};
