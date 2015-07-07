
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
	Icon = InArgs._Icon;
	IsToggle = InArgs._Toggle;
	OnClicked = InArgs._OnClicked;
	Color = InArgs._Color;
	int32 Width = InArgs._Width * Theme.ButtonWidth;
	int32 Height = InArgs._Height * Theme.ButtonHeight;

	// Text color for tooltips
	FLinearColor Color = Theme.NeutralColor;
	Color.A = Theme.DefaultAlpha;

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
		.ToolTip(

			// Tooltip overlay
			SNew(SToolTip)
			.BorderImage(&Theme.BackgroundBrush)
			[
				SNew(STextBlock)
				.Text(InArgs._ToolTipText)
				.Font(Theme.SmallFont.Font)
				.ColorAndOpacity(Color)
				.WrapTextAt(Theme.ContentWidth / 2.0f)
			]
		)
		[
			SNew(SVerticalBox)

			// Upper border
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SImage).Image(&Theme.ButtonBorderBrush)
			]

			// Main line
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				// Left border
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Fill)
				[
					SNew(SImage).Image(&Theme.ButtonBorderBrush)
				]

				// Main content box
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(Width)
					.HeightOverride(Height)
					.Padding(FMargin(0))
					[
						// Button background
						SNew(SBorder)
						.Padding(FMargin(0))
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

				// Right border
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Fill)
				[
					SNew(SImage).Image(&Theme.ButtonBorderBrush)
				]
			]

			// Lower border
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SImage).Image(&Theme.ButtonBorderBrush)
			]
		]
	];

	// Construct text content if we need to
	if (InArgs._Text.IsSet())
	{
		TSharedPtr<SVerticalBox> IconBox;

		InnerContainer->SetPadding(Theme.ButtonPadding);
		InnerContainer->SetContent(
			SNew(SHorizontalBox)

			// Icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(0, 0, 10, 0))
			[
				SAssignNew(IconBox, SVerticalBox)
			]

			// Text
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TextFont)
				.Text(InArgs._Text)
			]
		);

		if (Icon.IsSet())
		{
			IconBox->AddSlot()
				.VAlign(VAlign_Center)
				[
					SNew(SImage).Image(Icon)
				];
		}
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
