#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"


/** Interface storage structure */
class FInterfaceContainer
{
public:

	// Ship
	static TSharedPtr<FInterfaceContainer> New(IFlareShipInterface* InObject)
	{
		return MakeShareable(new FInterfaceContainer(InObject));
	}
	FInterfaceContainer(IFlareShipInterface* InPtr)
		: ShipInterfacePtr(InPtr)
		, StationInterfacePtr(NULL)
		, PartDescription(NULL)
	{}
	IFlareShipInterface* ShipInterfacePtr;

	// Station
	static TSharedPtr<FInterfaceContainer> New(IFlareStationInterface* InObject)
	{
		return MakeShareable(new FInterfaceContainer(InObject));
	}
	FInterfaceContainer(IFlareStationInterface* InPtr)
		: ShipInterfacePtr(NULL)
		, StationInterfacePtr(InPtr)
		, PartDescription(NULL)
	{}
	IFlareStationInterface* StationInterfacePtr;

	// Part info
	static TSharedPtr<FInterfaceContainer> New(FFlareShipComponentDescription* InObject)
	{
		return MakeShareable(new FInterfaceContainer(InObject));
	}
	FInterfaceContainer(FFlareShipComponentDescription* InPtr)
		: ShipInterfacePtr(NULL)
		, StationInterfacePtr(NULL)
		, PartDescription(InPtr)
	{}
	FFlareShipComponentDescription* PartDescription;
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
		, _ContainerStyle(&FFlareStyleSet::Get().GetWidgetStyle<FFlareContainerStyle>("/Style/DefaultContainerStyle"))
	{}

	SLATE_DEFAULT_SLOT(FArguments, Content)
	SLATE_STYLE_ARGUMENT(FFlareButtonStyle, ButtonStyle)
	SLATE_STYLE_ARGUMENT(FFlareContainerStyle, ContainerStyle)

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
	const FFlareContainerStyle* ContainerStyle;

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
