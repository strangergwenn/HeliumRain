#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"


/** Interface storage structure */
class FInterfaceContainer
{
public:

	// Ship
	static TSharedPtr<FInterfaceContainer> New(IFlareSpacecraftInterface* InObject)
	{
		return MakeShareable(new FInterfaceContainer(InObject));
	}
	FInterfaceContainer(IFlareSpacecraftInterface* InPtr)
		: ShipInterfacePtr(InPtr)
		, PartDescription(NULL)
	{}
	IFlareSpacecraftInterface* ShipInterfacePtr;

	// Part info
	static TSharedPtr<FInterfaceContainer> New(FFlareSpacecraftComponentDescription* InObject)
	{
		return MakeShareable(new FInterfaceContainer(InObject));
	}
	FInterfaceContainer(FFlareSpacecraftComponentDescription* InPtr)
		: ShipInterfacePtr(NULL)
		, PartDescription(InPtr)
	{}
	FFlareSpacecraftComponentDescription* PartDescription;
};


/** List item class */
class SFlareListItem : public STableRow< TSharedPtr<FInterfaceContainer> >
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareListItem)
		: _Content()
		, _ButtonStyle(&FFlareStyleSet::Get().GetWidgetStyle<FFlareButtonStyle>("/Style/PartButton"))
	{}

	SLATE_DEFAULT_SLOT(FArguments, Content)
	SLATE_STYLE_ARGUMENT(FFlareButtonStyle, ButtonStyle)

	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Brush callback */
	const FSlateBrush* GetDecoratorBrush() const;

	/** Brush callback */
	const FSlateBrush* GetBackgroundBrush() const;

	/** Mouse click	*/
	void SetSelected(bool Selected);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	bool IsSelected;

	const FFlareButtonStyle* ButtonStyle;

	UPROPERTY()
	TSharedPtr<SBorder> InnerContainer;


public:

	/*----------------------------------------------------
		Get & Set
	----------------------------------------------------*/

	TSharedPtr<SBorder> GetContainer() const
	{
		return InnerContainer;
	}


};
