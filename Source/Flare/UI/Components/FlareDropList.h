#pragma once

#include "../../Flare.h"
#include "../Components/FlareItemArray.h"


DECLARE_DELEGATE_OneParam(FFlareItemPicked, int32)


class SFlareDropList : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareDropList)
	: _HeaderStyle(&FFlareStyleSet::Get().GetWidgetStyle<FFlareButtonStyle>("/Style/DefaultButton"))
	, _ItemStyle(&FFlareStyleSet::Get().GetWidgetStyle<FFlareButtonStyle>("/Style/DefaultButton"))
	, _ContainerStyle(&FFlareStyleSet::Get().GetWidgetStyle<FFlareContainerStyle>("/Style/DefaultContainerStyle"))
	, _LineSize(1)
	, _ItemMargin(FMargin(2))
	{}

	SLATE_EVENT(FFlareItemPicked, OnItemPicked)
	SLATE_STYLE_ARGUMENT(FFlareButtonStyle, HeaderStyle)
	SLATE_STYLE_ARGUMENT(FFlareButtonStyle, ItemStyle)
	SLATE_STYLE_ARGUMENT(FFlareContainerStyle, ContainerStyle)

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

	/** Drop the list down */
	void OnHeaderClicked();

	/** Choose a color*/
	void OnItemPicked(int32 ItemIndex);

	/** Mouse out */
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	bool IsDropped;

	int32 LineSize;

	FMargin ItemMargin;

	FFlareItemPicked OnItemPickedCallback;

	const FFlareButtonStyle* ItemStyle;
	const FFlareContainerStyle* ContainerStyle;

	TSharedPtr<SFlareButton> HeaderButton;

	TSharedPtr<SFlareItemArray> ItemArray;

	TArray< TSharedRef<SWidget> > ContentArray;
	

};
