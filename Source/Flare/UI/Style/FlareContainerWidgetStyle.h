#pragma once

#include "../../Flare.h"
#include "FlareContainerWidgetStyle.generated.h"


USTRUCT()
struct FFlareContainerStyle : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	/*----------------------------------------------------
	Class defaults
	----------------------------------------------------*/

	static const FName TypeName;
	const FName GetTypeName() const override
	{
		return TypeName;
	}

	static const FFlareContainerStyle& GetDefault()
	{
		static FFlareContainerStyle Default;
		return Default;
	}

	void GetResources(TArray<const FSlateBrush*>& OutBrushes) const
	{
		OutBrushes.Add(&BackgroundBrush);
		OutBrushes.Add(&ActiveBackgroundBrush);
	}


	/*----------------------------------------------------
		Style data
	----------------------------------------------------*/
	
	/** Background brush */
	UPROPERTY(EditAnywhere, Category = Content) FSlateBrush BackgroundBrush;

	/** Background brush */
	UPROPERTY(EditAnywhere, Category = Content) FSlateBrush ActiveBackgroundBrush;

	/** Button padding for the inner box */
	UPROPERTY(EditAnywhere, Category = Behaviour) FMargin BorderPadding;
	
};


/*----------------------------------------------------
	Wrapper class
----------------------------------------------------*/

UCLASS(hidecategories=Object, MinimalAPI)
class UFlareContainerWidgetStyle : public USlateWidgetStyleContainerBase
{
	GENERATED_UCLASS_BODY()

public:
	
	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return static_cast<const struct FSlateWidgetStyle*>(&m_style);
	}

protected:

	UPROPERTY(Category = Appearance, EditAnywhere, meta = (ShowOnlyInnerProperties))
	FFlareContainerStyle m_style;

};
