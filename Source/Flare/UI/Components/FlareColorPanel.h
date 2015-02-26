#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareDropList.h"


class SFlareColorPanel : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareColorPanel){}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareHUD>, OwnerHUD)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Setup the widget with saved data */
	void Setup(struct FFlarePlayerSave& PlayerData);

	/** Chose a color for the paint */
	void OnPaintColorPicked(int32 Index);

	/** Chose a color for the lights */
	void OnLightColorPicked(int32 Index);

	/** Chose a pattern */
	void OnPatternPicked(int32 Index);
	

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UPROPERTY()
	TWeakObjectPtr<class AFlareHUD> OwnerHUD;

	TSharedPtr<SHorizontalBox> PickerList;

	TSharedPtr<SFlareDropList> PaintColorPicker;

	TSharedPtr<SFlareDropList> LightColorPicker;

	TSharedPtr<SFlareDropList> EngineColorPicker;
	
	TSharedPtr<SFlareDropList> PatternPicker;
		

};
