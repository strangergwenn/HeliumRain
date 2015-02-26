
#include "../../Flare.h"
#include "FlareHeaderButton.h"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareHeaderButton::Construct(const FArguments& InArgs)
{
	// Data
	IsPressed = false;
	OnClicked = InArgs._OnClicked;
	ButtonStyle = InArgs._ButtonStyle;
	ContainerStyle = InArgs._ContainerStyle;

	ChildSlot
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Center)
	[
		// Button (behaviour only, no display)
		SNew(SButton)
		.OnClicked(this, &SFlareHeaderButton::OnButtonClicked)
		.ContentPadding(FMargin(0))
		.ButtonStyle(FCoreStyle::Get(), "NoBorder")
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0))
			[
				// Central content box
				SNew(SBox)
				.WidthOverride(ButtonStyle->Width)
				.HeightOverride(ButtonStyle->Height)
				.Padding(FMargin(0))
				[
					// Button background
					SNew(SBorder)
					.Padding(ContainerStyle->BorderPadding)
					.BorderImage(this, &SFlareHeaderButton::GetBackgroundBrush)
					[
						// Content box and inner container
						SAssignNew(InnerContainer, SBorder)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.Padding(ButtonStyle->ContentPadding)
						.BorderImage(new FSlateNoResource)
					]
				]
			]

			// Underline brush
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SImage).Image(this, &SFlareHeaderButton::GetDecoratorBrush)
			]
		]
	];

	// Construct text content
	InnerContainer->SetContent(
		SNew(STextBlock)
		.TextStyle(InArgs._TextStyle)
		.Text(InArgs._Text)
	);
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareHeaderButton::SetActive(bool State)
{
	IsPressed = State;
}

bool SFlareHeaderButton::IsActive() const
{
	return IsPressed;
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

const FSlateBrush* SFlareHeaderButton::GetDecoratorBrush() const
{
	return (IsPressed ? &ButtonStyle->ActiveDecoratorBrush : &ButtonStyle->DecoratorBrush);
}

const FSlateBrush* SFlareHeaderButton::GetBackgroundBrush() const
{
	return (IsHovered() ? &ContainerStyle->ActiveBackgroundBrush : &ContainerStyle->BackgroundBrush);
}

FReply SFlareHeaderButton::OnButtonClicked()
{
	IsPressed = !IsPressed;

	if (OnClicked.IsBound() == true)
	{
		OnClicked.Execute();
	}

	return FReply::Handled();
}
