
#pragma once

#include "../../Flare.h"


class SFlarePlanetaryBox : public SPanel
{

public:

	/** Slot class for this box */
	class FSlot : public TSlotBase<FSlot>
	{
	public:

		virtual ~FSlot() {}

		FSizeParam SizeParam;

		EHorizontalAlignment HAlignment;

		EVerticalAlignment VAlignment;

		TAttribute<FMargin> SlotPadding;

		TAttribute<float> MaxSize;

	public:

		/** Default values for a slot */
		FSlot()
			: TSlotBase<FSlot>()
			, SizeParam(FStretch(1))
			, HAlignment(HAlign_Fill)
			, VAlignment(VAlign_Fill)
			, SlotPadding(FMargin(0))
			, MaxSize(0.0f)
		{ }

		FSlot& AutoWidth()
		{
			SizeParam = FAuto();
			return *this;
		}

		FSlot& MaxWidth(const TAttribute< float >& InMaxWidth)
		{
			MaxSize = InMaxWidth;
			return *this;
		}

		FSlot& FillWidth(const TAttribute< float >& StretchCoefficient)
		{
			SizeParam = FStretch(StretchCoefficient);
			return *this;
		}

		FSlot& Padding(float Uniform)
		{
			SlotPadding = FMargin(Uniform);
			return *this;
		}

		FSlot& Padding(float Horizontal, float Vertical)
		{
			SlotPadding = FMargin(Horizontal, Vertical);
			return *this;
		}

		FSlot& Padding(float Left, float Top, float Right, float Bottom)
		{
			SlotPadding = FMargin(Left, Top, Right, Bottom);
			return *this;
		}

		FSlot& Padding(TAttribute<FMargin> InPadding)
		{
			SlotPadding = InPadding;
			return *this;
		}

		FSlot& HAlign(EHorizontalAlignment InHAlignment)
		{
			HAlignment = InHAlignment;
			return *this;
		}

		FSlot& VAlign(EVerticalAlignment InVAlignment)
		{
			VAlignment = InVAlignment;
			return *this;
		}

		FSlot& Expose(FSlot*& OutVarToInit)
		{
			OutVarToInit = this;
			return *this;
		}
		/*
		FSlot& operator[](TSharedRef<SWidget> InWidget)
		{
		SFlarePlanetaryBox::FSlot::operator[](InWidget);
		return *this;
		}*/
	};

public:

	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlarePlanetaryBox)
	{
		_Visibility = EVisibility::SelfHitTestInvisible;
	}
	
	SLATE_SUPPORTS_SLOT(SFlarePlanetaryBox::FSlot)

	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Construct this widget */
	void Construct(const FArguments& InArgs);

	/** Set a new planet image (call before ClearChildren) */
	void SetPlanetImage(const FSlateBrush* Image)
	{
		PlanetImage = Image;
	}

	/** Set the radius */
	void SetRadius(int NewRadius)
	{
		Radius = NewRadius;
	}

	/** Create a slot */
	static FSlot& Slot()
	{
		return *(new FSlot());
	}

	/** Add a slot */
	FSlot& AddSlot()
	{
		SFlarePlanetaryBox::FSlot& NewSlot = *new SFlarePlanetaryBox::FSlot();
		this->Children.Add(&NewSlot);
		return NewSlot;
	}

	/** Insert a slot at a specific index */
	FSlot& InsertSlot(int32 Index = INDEX_NONE)
	{
		if (Index == INDEX_NONE)
		{
			return AddSlot();
		}
		SFlarePlanetaryBox::FSlot& NewSlot = *new SFlarePlanetaryBox::FSlot();
		this->Children.Insert(&NewSlot, Index);
		return NewSlot;
	}

	/** Removes the slot which contains the specified SWidget */
	int32 RemoveSlot(const TSharedRef<SWidget>& SlotWidget);

	/** Removes all children from the box */
	void ClearChildren();

	/** Get the number of slots */
	int32 NumSlots() const
	{
		return this->Children.Num();
	}
	

	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;

	virtual FVector2D ComputeDesiredSize(float) const override;

	virtual FChildren* GetChildren() override;
	

protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/

	/** Radius of the planetary system */
	int32                                      Radius;

	/** SLate brush to use for the planet image */
	const FSlateBrush*                         PlanetImage;

	/** List of all children */
	TPanelChildren<FSlot>                      Children;


public:

	SFlarePlanetaryBox()
	{
	}

};
