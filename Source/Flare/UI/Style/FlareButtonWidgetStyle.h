#pragma once

#include "../../Flare.h"
#include "FlareButtonWidgetStyle.generated.h"


USTRUCT()
struct FFlareButtonStyle : public FSlateWidgetStyle
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

	static const FFlareButtonStyle& GetDefault()
	{
		static FFlareButtonStyle Default;
		return Default;
	}

	void GetResources(TArray<const FSlateBrush*>& OutBrushes) const
	{
		OutBrushes.Add(&DecoratorBrush);
		OutBrushes.Add(&ActiveDecoratorBrush);
	}


	/*----------------------------------------------------
		Style data
	----------------------------------------------------*/
	
	/** Active selection brush */
	UPROPERTY(EditAnywhere, Category = Content) FSlateBrush DecoratorBrush;

	/** Active selection brush */
	UPROPERTY(EditAnywhere, Category = Content) FSlateBrush ActiveDecoratorBrush;

	/** Panel width */
	UPROPERTY(EditAnywhere, Category = Behaviour) int32 Width;

	/** Panel height */
	UPROPERTY(EditAnywhere, Category = Behaviour) int32 Height;

	/** Button content padding */
	UPROPERTY(EditAnywhere, Category = Behaviour) FMargin ContentPadding;
	
};


/*----------------------------------------------------
	Wrapper class
----------------------------------------------------*/

UCLASS(hidecategories=Object, MinimalAPI)
class UFlareButtonWidgetStyle : public USlateWidgetStyleContainerBase
{
	GENERATED_UCLASS_BODY()

public:
	
	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return static_cast<const struct FSlateWidgetStyle*>(&m_style);
	}

protected:

	UPROPERTY(Category = Appearance, EditAnywhere, meta = (ShowOnlyInnerProperties))
		FFlareButtonStyle m_style;

};
