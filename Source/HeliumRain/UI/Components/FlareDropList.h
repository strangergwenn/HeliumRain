#pragma once

#include "../../Flare.h"
#include "../Components/FlareItemArray.h"
#include "../Style/FlareStyleSet.h"

#include "Runtime/Slate/Public/Widgets/Colors/SColorWheel.h"
#include "SSimpleGradient.h"


DECLARE_DELEGATE_OneParam(FFlareItemPicked, int32)


template< typename OptionType >
class SFlareDropList : public SCompoundWidget
{
	typedef typename TSlateDelegates< OptionType >::FOnGenerateWidget FFlareOnGenerateWidget;
	typedef typename TSlateDelegates< OptionType >::FOnSelectionChanged FFlareOnSelectionChanged;

	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareDropList)
		: _OptionsSource()
		, _OnSelectionChanged()
		, _OnGenerateWidget()
		, _OnItemPicked()
		, _OnColorPicked()
		, _LineSize(1)
		, _HeaderWidth(3)
		, _HeaderHeight(1)
		, _ItemWidth(3)
		, _ItemHeight(1)
		, _MaximumHeight(300)
		, _ShowColorWheel(false)
	{}
	
	SLATE_ARGUMENT(const TArray< OptionType >*, OptionsSource)

	SLATE_EVENT(FFlareOnSelectionChanged, OnSelectionChanged)
	SLATE_EVENT(FFlareOnGenerateWidget, OnGenerateWidget)
	SLATE_EVENT(FFlareItemPicked, OnItemPicked)
	SLATE_EVENT(FFlareColorPicked, OnColorPicked)

	SLATE_ARGUMENT(int32, LineSize)
	SLATE_ARGUMENT(float, HeaderWidth)
	SLATE_ARGUMENT(float, HeaderHeight)
	SLATE_ARGUMENT(float, ItemWidth)
	SLATE_ARGUMENT(float, ItemHeight)
	SLATE_ARGUMENT(int32, MaximumHeight)
	SLATE_ARGUMENT(bool, ShowColorWheel)

	SLATE_DEFAULT_SLOT(FArguments, Content)
	
	SLATE_END_ARGS()


public:
	

	/*----------------------------------------------------
		Construct
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs)
	{
		const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

		// Data
		IsDropped = false;
		HasColorWheel = InArgs._ShowColorWheel;
		LineSize = InArgs._LineSize;
		OnItemPickedCallback = InArgs._OnItemPicked;
		OnColorPickedCallback = InArgs._OnColorPicked;
	

		OnSelectionChanged = InArgs._OnSelectionChanged;
		OnGenerateWidget = InArgs._OnGenerateWidget;
		OptionsSource = InArgs._OptionsSource;

		// Layout
		ChildSlot
		[
			SNew(SHorizontalBox)

			// Border
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SImage)
				.Image(&Theme.NearInvisibleBrush)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SVerticalBox)

				// List header
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Left)
				[
					SNew(SOverlay)

					// Header
					+ SOverlay::Slot()
					[
						SAssignNew(HeaderButton, SFlareButton)
						.OnClicked(this, &SFlareDropList<OptionType>::OnHeaderClicked)
						.Width(InArgs._HeaderWidth)
						.Height(InArgs._HeaderHeight)
					]

					// Icon
					+ SOverlay::Slot()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Bottom)
					[
						SNew(SImage)
						.Image(FFlareStyleSet::GetIcon("Droplist"))
						.Visibility(EVisibility::HitTestInvisible)
					]
				]

				// Item picker below
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBox)
					.MaxDesiredHeight(InArgs._MaximumHeight)
					.MinDesiredHeight(0)
					[
						SAssignNew(ItemScrollBox, SScrollBox)
						.Style(&Theme.ScrollBoxStyle)
						.ScrollBarStyle(&Theme.ScrollBarStyle)

						+ SScrollBox::Slot()
						[
							SAssignNew(ItemArray, SFlareItemArray)
							.OnItemPicked(this, &SFlareDropList<OptionType>::OnItemPicked)
							.LineSize(LineSize)
							.Width(InArgs._ItemWidth)
							.Height(InArgs._ItemHeight)
						]
					]
				]

				// Color picker
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBorder)
					.BorderImage(&Theme.BackgroundBrush)
					.Visibility(this, &SFlareDropList<OptionType>::GetColorPickerVisibility)
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SBox)
							.WidthOverride(LineSize * Theme.ButtonWidth * InArgs._ItemWidth)
							.HeightOverride(LineSize * Theme.ButtonWidth * InArgs._ItemWidth)
							[
								SAssignNew(ColorWheel, SColorWheel)
								.SelectedColor(this, &SFlareDropList<OptionType>::GetCurrentColor)
								.OnValueChanged(this, &SFlareDropList<OptionType>::HandleColorSpectrumValueChanged)
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							MakeColorSlider()
						]
					]
				]
			]
		];

		// Default settings
		ItemArray->SetVisibility(EVisibility::Collapsed);
		ItemScrollBox->SetVisibility(EVisibility::Collapsed);
		ColorWheel->SetVisibility(EVisibility::Collapsed);
		ColorPickerVisible = false;

		// Header
		HeaderButton->GetContainer()->SetHAlign(HAlign_Fill);
		HeaderButton->GetContainer()->SetVAlign(VAlign_Fill);
		HeaderButton->GetContainer()->SetContent(InArgs._Content.Widget);
	}


	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Force the selected index */
	void SetSelectedIndex(int32 ItemIndex)
	{
		ItemArray->SetSelectedIndex(ItemIndex);
		HeaderButton->GetContainer()->SetContent(ContentArray[ItemIndex]);
	}

	/** Get the selected index */
	int32 GetSelectedIndex() const
	{
		return ItemArray->GetSelectedIndex();
	}

	/** Select item */
	void SetSelectedItem(OptionType InSelectedItem)
	{
		int32 Index = OptionsSource->Find(InSelectedItem);
		if (Index != INDEX_NONE)
		{
			SetSelectedIndex(Index);
		}
	}

	/** Get the selected item */
	OptionType GetSelectedItem()
	{
		FCHECK(OptionsSource);
		return (*OptionsSource)[GetSelectedIndex()];
	}

	/** Refresh the list from the source */
	void RefreshOptions()
	{
		FCHECK(OptionsSource);
		FCHECK(OnGenerateWidget.IsBound());

		ClearItems();
		for (const OptionType& Option : *OptionsSource)
		{
			AddItem(OnGenerateWidget.Execute(Option));
		}
	}

	/** Add a new Slate widget */
	void AddItem(const TSharedRef< SWidget >& InContent)
	{
		ItemArray->AddItem(InContent);

		ContentArray.Add(InContent);
	}

	/** Delete all widgets */
	void ClearItems()
	{
		ContentArray.Empty();
		ItemArray->ClearItems();
	}

	/** Get the widget content */
	TSharedRef<SWidget> GetItemContent(int32 ItemIndex) const
	{
		return ItemArray->GetItemContent(ItemIndex);
	}

	/** Make the header a solid color block */
	void SetColor(FLinearColor Color)
	{
		CurrentColorHSV = Color.LinearRGBToHSV();
		CurrentColorRGB = Color;

		HeaderButton->GetContainer()->SetContent(SNew(SColorBlock).Color(CurrentColorRGB));
	}


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	void OnHeaderClicked()
	{
		ItemArray->SetVisibility(IsDropped ? EVisibility::Collapsed : EVisibility::Visible);
		ItemScrollBox->SetVisibility(IsDropped ? EVisibility::Collapsed : EVisibility::Visible);
		if (HasColorWheel)
		{
			ColorWheel->SetVisibility(IsDropped ? EVisibility::Collapsed : EVisibility::Visible);
			ColorPickerVisible = !IsDropped;
		}
		else
		{
			ColorWheel->SetVisibility(EVisibility::Collapsed);
		}
		IsDropped = !IsDropped;
	}

	void OnItemPicked(int32 ItemIndex)
	{
		HeaderButton->GetContainer()->SetContent(ContentArray[ItemIndex]);

		if (OnItemPickedCallback.IsBound())
		{
			OnItemPickedCallback.Execute(ItemIndex);
		}
		if (OnSelectionChanged.IsBound() && OptionsSource)
		{
			OnSelectionChanged.Execute((*OptionsSource)[ItemIndex], ESelectInfo::OnMouseClick);
		}

		IsDropped = false;
		ItemArray->SetVisibility(EVisibility::Collapsed);
		ItemScrollBox->SetVisibility(EVisibility::Collapsed);
		ColorWheel->SetVisibility(EVisibility::Collapsed);
		ColorPickerVisible = false;
	}


	/*----------------------------------------------------
		Color slider
	----------------------------------------------------*/

	void HandleColorSpectrumValueChanged( FLinearColor NewValue )
	{
		SetNewTargetColorHSV(NewValue);
	}

	FLinearColor HandleColorSliderEndColor() const
	{
		return FLinearColor(CurrentColorHSV.R, CurrentColorHSV.G, 1.0f, 1.0f).HSVToLinearRGB();
	}

	FLinearColor HandleColorSliderStartColor() const
	{
		return FLinearColor(CurrentColorHSV.R, CurrentColorHSV.G, 0.0f, 1.0f).HSVToLinearRGB();
	}

	bool SetNewTargetColorHSV( const FLinearColor& NewValue, bool bForceUpdate = false )
	{
		CurrentColorHSV = NewValue;
		CurrentColorRGB = NewValue.HSVToLinearRGB();
		CurrentColorRGB.A = 1;

		HeaderButton->GetContainer()->SetContent(SNew(SColorBlock).Color(CurrentColorRGB));

		if (OnColorPickedCallback.IsBound() == true)
		{
			OnColorPickedCallback.Execute(CurrentColorRGB);
		}


		return true;
	}

	float HandleColorSpinBoxValue() const
	{
		return CurrentColorHSV.B;
	}

	void HandleColorSpinBoxValueChanged( float NewValue)
	{
		int32 ComponentIndex;


		ComponentIndex = 2;

		FLinearColor& NewColor = CurrentColorHSV;

		if (FMath::IsNearlyEqual(NewValue, NewColor.B, KINDA_SMALL_NUMBER))
		{
			return;
		}

		NewColor.Component(ComponentIndex) = NewValue;

		// true as second param ?
		SetNewTargetColorHSV(NewColor);
	}

	EVisibility GetColorPickerVisibility() const
	{
		if (!ColorPickerVisible)
		{
			return EVisibility::Collapsed;
		}
		else
		{
			return EVisibility::Visible;
		}
	}

	FLinearColor GetCurrentColor() const
	{
		return CurrentColorHSV;
	}

	TSharedRef<SWidget> MakeColorSlider() const
	{
		return SNew(SOverlay)

		+ SOverlay::Slot()
		.Padding(FMargin(4.0f, 0.0f))
		[
			SNew(SSimpleGradient)
				.EndColor(this, &SFlareDropList<OptionType>::HandleColorSliderEndColor)
				.StartColor(this, &SFlareDropList<OptionType>::HandleColorSliderStartColor)
				.Orientation(Orient_Vertical)
				.UseSRGB(false)
		]

		+ SOverlay::Slot()
		[
			SNew(SSlider)
				.IndentHandle(false)
				.Orientation(Orient_Horizontal)
				.SliderBarColor(FLinearColor::Transparent)
				.Value(this, &SFlareDropList<OptionType>::HandleColorSpinBoxValue)
				.OnValueChanged(this, &SFlareDropList<OptionType>::HandleColorSpinBoxValueChanged)
		];
	}


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Parameters
	const TArray< OptionType >*   OptionsSource;
	FFlareOnSelectionChanged      OnSelectionChanged;
	FFlareOnGenerateWidget        OnGenerateWidget;
	FFlareItemPicked              OnItemPickedCallback;
	FFlareColorPicked             OnColorPickedCallback;
	int32                         LineSize;

	// Data
	bool                          IsDropped;
	bool                          HasColorWheel;
	bool                          ColorPickerVisible;
	
	// Slate data
	TSharedPtr<SFlareButton>      HeaderButton;
	TSharedPtr<SFlareItemArray>   ItemArray;
	TSharedPtr<SScrollBox>        ItemScrollBox;
	TSharedPtr<SColorWheel>       ColorWheel;
	TArray< TSharedRef<SWidget> > ContentArray;
	
	// Colors
	FLinearColor                  CurrentColorHSV;
	FLinearColor                  CurrentColorRGB;


};
