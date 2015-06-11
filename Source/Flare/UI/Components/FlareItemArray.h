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
	: _ItemStyle(&FFlareStyleSet::Get().GetWidgetStyle<FFlareButtonStyle>("/Style/DefaultButton"))
	, _LineSize(1)
	, _ItemMargin(FMargin(0))
	{}

	SLATE_EVENT(FFlareItemPicked, OnItemPicked)
	SLATE_STYLE_ARGUMENT(FFlareButtonStyle, ItemStyle)

	SLATE_ARGUMENT(int32, LineSize)
	SLATE_ARGUMENT(FMargin, ItemMargin)
	
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

	/** Get an item's content */
	TSharedRef<SWidget> GetItemContent(int32 ItemIndex) const;
		
	/** Choose an item*/
	void OnItemPicked(TSharedPtr<int32> ItemIndex);
	

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	int32 CurrentIndex;

	int32 LineSize;

	FMargin ItemMargin;

	FFlareItemPicked OnItemPickedCallback;

	const FFlareButtonStyle* ItemStyle;

	TSharedPtr<SGridPanel> WidgetGrid;

	TArray< TSharedPtr<SFlareButton> > ContentArray;
	

};
