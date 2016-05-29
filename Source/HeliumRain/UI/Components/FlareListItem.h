#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../../Spacecrafts/FlareSimulatedSpacecraft.h"


/** Interface storage structure */
class FInterfaceContainer
{
public:

	// Ship
	static TSharedPtr<FInterfaceContainer> New(UFlareSimulatedSpacecraft* InObject)
	{
		return MakeShareable(new FInterfaceContainer(InObject));
	}
	FInterfaceContainer(UFlareSimulatedSpacecraft* InPtr)
		: ShipInterfacePtr(InPtr)
		, SpacecraftDescriptionPtr(NULL)
		, PartDescription(NULL)
		, FleetPtr(NULL)
	{}
	UFlareSimulatedSpacecraft* ShipInterfacePtr;

	// Ship description
	static TSharedPtr<FInterfaceContainer> New(FFlareSpacecraftDescription* InObject)
	{
		return MakeShareable(new FInterfaceContainer(InObject));
	}
	FInterfaceContainer(FFlareSpacecraftDescription* InPtr)
		: ShipInterfacePtr(NULL)
		, SpacecraftDescriptionPtr(InPtr)
		, PartDescription(NULL)
		, FleetPtr(NULL)
	{}
	FFlareSpacecraftDescription* SpacecraftDescriptionPtr;

	// Part info
	static TSharedPtr<FInterfaceContainer> New(FFlareSpacecraftComponentDescription* InObject)
	{
		return MakeShareable(new FInterfaceContainer(InObject));
	}
	FInterfaceContainer(FFlareSpacecraftComponentDescription* InPtr)
		: ShipInterfacePtr(NULL)
		, SpacecraftDescriptionPtr(NULL)
		, PartDescription(InPtr)
		, FleetPtr(NULL)
	{}
	FFlareSpacecraftComponentDescription* PartDescription;

	// Company info
	static TSharedPtr<FInterfaceContainer> New(UFlareCompany* InObject)
	{
		return MakeShareable(new FInterfaceContainer(InObject));
	}
	FInterfaceContainer(UFlareCompany* InPtr)
		: ShipInterfacePtr(NULL)
		, SpacecraftDescriptionPtr(NULL)
		, CompanyPtr(InPtr)
		, FleetPtr(NULL)
	{}
	UFlareCompany* CompanyPtr;

	// Fleet info
	static TSharedPtr<FInterfaceContainer> New(UFlareFleet* InObject)
	{
		return MakeShareable(new FInterfaceContainer(InObject));
	}
	FInterfaceContainer(UFlareFleet* InPtr)
		: ShipInterfacePtr(NULL)
		, SpacecraftDescriptionPtr(NULL)
		, CompanyPtr(NULL)
		, FleetPtr(InPtr)
	{}
	UFlareFleet* FleetPtr;
};


/** List item class */
class SFlareListItem : public STableRow< TSharedPtr<FInterfaceContainer> >
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareListItem)
		: _Content()
		, _Width(5)
		, _Height(2)
	{}

	SLATE_DEFAULT_SLOT(FArguments, Content)
	SLATE_ARGUMENT(int32, Width)
	SLATE_ARGUMENT(int32, Height)

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
