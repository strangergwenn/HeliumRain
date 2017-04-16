
#include "../../Flare.h"
#include "FlareDropList.h"


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
						// TODO FRED
						// See UEDIR\4.15\Engine\Source\Runtime\AppFramework\Private\Widgets\Colors\SColorPicker.cpp :) 
						SAssignNew(ColorWheel, SColorWheel)
						//.OnValueChanged(this, &SFlareDropList::HandleColorSpectrumValueChanged)
						//.OnMouseCaptureBegin(this, &SFlareDropList::HandleInteractiveChangeBegin)
						//.OnMouseCaptureEnd(this, &SFlareDropList::HandleInteractiveChangeEnd)
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					// TODO FRED
					SAssignNew(ColorSlider, SSlider)
					.Style(&Theme.SliderStyle)
					.Value(0)
					//.OnValueChanged(this, &SFlareDropList::OnBrightnessChanged)    // void SFlareDropList::OnBrightnessChanged(float Value)
				]
			]
		]
	];

	// Default settings
	ItemArray->SetVisibility(EVisibility::Collapsed);
	ColorWheel->SetVisibility(EVisibility::Collapsed);
	ColorSlider->SetVisibility(EVisibility::Collapsed);
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

int32 SFlareDropList::GetSelectedIndex() const
{
	return ItemArray->GetSelectedIndex();
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
		ColorSlider->SetVisibility(IsDropped ? EVisibility::Collapsed : EVisibility::Visible);
	}
	else
	{
		ColorWheel->SetVisibility(EVisibility::Collapsed);
		ColorSlider->SetVisibility(EVisibility::Collapsed);
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
	ColorSlider->SetVisibility(EVisibility::Collapsed);
}
