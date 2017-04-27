
#include "FlareDropList.h"
#include "../../Flare.h"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareDropList::Construct(const FArguments& InArgs)
{
	// Data
	IsDropped = false;
	HasColorWheel = InArgs._ShowColorWheel;
	LineSize = InArgs._LineSize;
	OnItemPickedCallback = InArgs._OnItemPicked;
	OnColorPickedCallback = InArgs._OnColorPicked;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	// Layout
	ChildSlot
	[
		SNew(SVerticalBox)

		// List header
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Left)
		[
			SAssignNew(HeaderButton, SFlareButton)
			.OnClicked(this, &SFlareDropList::OnHeaderClicked)
			.Width(InArgs._HeaderWidth)
			.Height(InArgs._HeaderHeight)
		]

		// Item picker below
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(ItemArray, SFlareItemArray)
			.OnItemPicked(this, &SFlareDropList::OnItemPicked)
			.LineSize(LineSize)
			.Width(InArgs._ItemWidth)
			.Height(InArgs._ItemHeight)
		]

		// Color picker
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(&Theme.BackgroundBrush)
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
						.SelectedColor(this, &SFlareDropList::GetCurrentColor)
						.OnValueChanged(this, &SFlareDropList::HandleColorSpectrumValueChanged)
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					// value slider
					MakeColorSlider()
				]
			]
		]
	];

	// Default settings
	ItemArray->SetVisibility(EVisibility::Collapsed);
	ColorWheel->SetVisibility(EVisibility::Collapsed);
	ColorPickerVisible = false;
	HeaderButton->GetContainer()->SetHAlign(HAlign_Fill);
	HeaderButton->GetContainer()->SetVAlign(VAlign_Fill);
}

void SFlareDropList::AddItem(const TSharedRef< SWidget >& InContent)
{
	ItemArray->AddItem(InContent);

	ContentArray.Add(InContent);
}

void SFlareDropList::ClearItems()
{
	ContentArray.Empty();
	ItemArray->ClearItems();
}

void SFlareDropList::SetSelectedIndex(int32 ItemIndex)
{
	ItemArray->SetSelectedIndex(ItemIndex);
	HeaderButton->GetContainer()->SetContent(ContentArray[ItemIndex]);
}

void SFlareDropList::SetColor(FLinearColor Color)
{
	CurrentColorHSV = Color.LinearRGBToHSV();
	CurrentColorRGB = Color;

	HeaderButton->GetContainer()->SetContent(SNew(SColorBlock).Color(CurrentColorRGB));
}

int32 SFlareDropList::GetSelectedIndex() const
{
	return ItemArray->GetSelectedIndex();
}

void SFlareDropList::HandleColorSpectrumValueChanged( FLinearColor NewValue )
{
	SetNewTargetColorHSV(NewValue);
}

TSharedRef<SWidget> SFlareDropList::GetItemContent(int32 ItemIndex) const
{
	return ItemArray->GetItemContent(ItemIndex);
}

void SFlareDropList::OnHeaderClicked()
{
	ItemArray->SetVisibility(IsDropped ? EVisibility::Collapsed : EVisibility::Visible);
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

void SFlareDropList::OnItemPicked(int32 ItemIndex)
{
	HeaderButton->GetContainer()->SetContent(ContentArray[ItemIndex]);

	if (OnItemPickedCallback.IsBound() == true)
	{
		OnItemPickedCallback.Execute(ItemIndex);
	}

	IsDropped = false;
	ItemArray->SetVisibility(EVisibility::Collapsed);
	ColorWheel->SetVisibility(EVisibility::Collapsed);
	ColorPickerVisible = false;
}

FLinearColor SFlareDropList::HandleColorSliderEndColor() const
{
	return FLinearColor(CurrentColorHSV.R, CurrentColorHSV.G, 1.0f, 1.0f).HSVToLinearRGB();
}

FLinearColor SFlareDropList::HandleColorSliderStartColor() const
{
	return FLinearColor(CurrentColorHSV.R, CurrentColorHSV.G, 0.0f, 1.0f).HSVToLinearRGB();
}

bool SFlareDropList::SetNewTargetColorHSV( const FLinearColor& NewValue, bool bForceUpdate /*= false*/ )
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

float SFlareDropList::HandleColorSpinBoxValue() const
{
	return CurrentColorHSV.B;
}

void SFlareDropList::HandleColorSpinBoxValueChanged( float NewValue)
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

EVisibility SFlareDropList::GetColorPickerVisibility() const
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

TSharedRef<SWidget> SFlareDropList::MakeColorSlider() const
{
	return SNew(SOverlay)

	+ SOverlay::Slot()
		.Padding(FMargin(4.0f, 0.0f))
		[
			SNew(SSimpleGradient)
				.EndColor(this, &SFlareDropList::HandleColorSliderEndColor)
				.StartColor(this, &SFlareDropList::HandleColorSliderStartColor)
				.Orientation(Orient_Vertical)
				.UseSRGB(false)
		]

	+ SOverlay::Slot()
		[
			SNew(SSlider)
				.IndentHandle(false)
				.Orientation(Orient_Horizontal)
				.Visibility(this, &SFlareDropList::GetColorPickerVisibility)
				.SliderBarColor(FLinearColor::Transparent)
				.Value(this, &SFlareDropList::HandleColorSpinBoxValue)
				.OnValueChanged(this, &SFlareDropList::HandleColorSpinBoxValueChanged)
		];
}

