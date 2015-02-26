
#include "../../Flare.h"
#include "FlareButton.h"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareButton::Construct(const FArguments& InArgs)
{
	// Initial setup
	IsPressed = false;

	// Arguments
	IsToggle = InArgs._Toggle;
	OnClicked = InArgs._OnClicked;
	ButtonStyle = InArgs._ButtonStyle;
	ContainerStyle = InArgs._ContainerStyle;
	BackgroundImage = InArgs._BackgroundImage;
	DecoratorImage = InArgs._DecoratorImage;

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
			.WidthOverride(ButtonStyle->Width)
			.HeightOverride(ButtonStyle->Height)
			.Padding(FMargin(0))
			[
				// Button background
				SNew(SBorder)
				.Padding(ContainerStyle->BorderPadding)
				.BorderImage(this, &SFlareButton::GetBackgroundBrush)
				[
					SNew(SHorizontalBox)

					// Content box and inner container
					+ SHorizontalBox::Slot()
					[
						SAssignNew(InnerContainer, SBorder)
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Center)
						.Padding(ButtonStyle->ContentPadding)
						.BorderImage(new FSlateNoResource)
					]

					// Toggle light
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
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
			.TextStyle(InArgs._TextStyle)
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
	if (IsToggle)
	{
		return (IsPressed ? &ButtonStyle->ActiveDecoratorBrush : &ButtonStyle->DecoratorBrush);
	}
	else if (IsPressed)
	{
		if (DecoratorImage != NULL)
		{
			return DecoratorImage;
		}
		else
		{
			return &ButtonStyle->ActiveDecoratorBrush;
		}
	}
	else
	{
		return NULL;
	}
}

const FSlateBrush* SFlareButton::GetBackgroundBrush() const
{
	if (BackgroundImage != NULL)
	{
		return BackgroundImage;
	}
	else
	{
		return (IsHovered() ? &ContainerStyle->ActiveBackgroundBrush : &ContainerStyle->BackgroundBrush);
	}
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
