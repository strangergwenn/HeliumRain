
#include "../../Flare.h"
#include "FlareButton.h"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareButton::Construct(const FArguments& InArgs)
{
	// Initial setup
	IsPressed = false;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Arguments
	IsToggle = InArgs._Toggle;
	OnClicked = InArgs._OnClicked;
	Color = InArgs._Color;
	int32 Width = InArgs._Width * Theme.ButtonWidth;
	int32 Height = InArgs._Height * Theme.ButtonHeight;

	// Structure
	ChildSlot
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Center)
	[
		// Button (behaviour only, no display)
		SNew(SButton)
		.OnClicked(this, &SFlareButton::OnButtonClicked)
		.ContentPadding(FMargin(0))
		.ButtonStyle(FCoreStyle::Get(), "NoBorder")
		[
			// Central content box
			SNew(SBox)
			.WidthOverride(Width)
			.HeightOverride(Height)
			.Padding(FMargin(0))
			[
				// Button background
				SNew(SBorder)
				.Padding(Theme.ButtonBorderPadding)
				.BorderImage(this, &SFlareButton::GetBackgroundBrush)
				.BorderBackgroundColor(this, &SFlareButton::GetMainColor)
				[
					SNew(SHorizontalBox)

					// Content box and inner container
					+ SHorizontalBox::Slot()
					[
						SAssignNew(InnerContainer, SBorder)
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Center)
						.Padding(Theme.ButtonPadding)
						.BorderImage(new FSlateNoResource)
					]

					// Toggle light
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Fill)
					[
						SNew(SImage)
						.Image(this, &SFlareButton::GetDecoratorBrush)
					]
				]
			]
		]
	];

	// Construct text content if we need to
	if (!InArgs._Text.IsEmpty())
	{
		InnerContainer->SetContent(
			SNew(STextBlock)
			.TextStyle(&Theme.TextFont)
			.Text(InArgs._Text)
		);
	}
	else
	{
		InnerContainer->SetHAlign(HAlign_Fill);
		InnerContainer->SetVAlign(VAlign_Fill);
	}
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareButton::SetActive(bool State)
{
	IsPressed = State;
}

bool SFlareButton::IsActive() const
{
	return IsPressed;
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

const FSlateBrush* SFlareButton::GetDecoratorBrush() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	if (IsToggle)
	{
		return (IsPressed ? &Theme.ButtonActiveDecorator : &Theme.ButtonDecorator);
	}
	else if (IsPressed)
	{
		return &Theme.ButtonActiveDecorator;
	}
	else
	{
		return NULL;
	}
}

const FSlateBrush* SFlareButton::GetBackgroundBrush() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	return (IsHovered() ? &Theme.ButtonActiveBackground : &Theme.ButtonBackground);
}

FSlateColor SFlareButton::GetMainColor() const
{
	return Color.Get();
}

FReply SFlareButton::OnButtonClicked()
{
	if (IsToggle)
	{
		IsPressed = !IsPressed;
	}

	if (OnClicked.IsBound() == true)
	{
		OnClicked.Execute();
	}

	return FReply::Handled();
}
