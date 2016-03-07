
#include "../../Flare.h"
#include "FlareButton.h"
#include "../../Player/FlareMenuManager.h"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareButton::Construct(const FArguments& InArgs)
{
	// Setup
	IsPressed = false;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Arguments
	Icon = InArgs._Icon;
	IsToggle = InArgs._Toggle;
	IsTransparent = InArgs._Transparent;
	OnClicked = InArgs._OnClicked;
	Color = InArgs._Color;
	Text = InArgs._Text;
	HelpText = InArgs._HelpText;
	IsDisabled = InArgs._IsDisabled;
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

							// Toggle light
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.HAlign(HAlign_Right)
							.VAlign(VAlign_Fill)
							[
								SNew(SImage)
								.Image(this, &SFlareButton::GetDecoratorBrush)
							]

							// Content box and inner container
							+ SHorizontalBox::Slot()
							[
								SAssignNew(InnerContainer, SBorder)
								.HAlign(HAlign_Left)
								.VAlign(VAlign_Center)
								.BorderImage(new FSlateNoResource)
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

		InnerContainer->SetPadding(IsTransparent ? FMargin(0) : Theme.ButtonPadding);
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

		if (Icon.IsSet() || IsToggle)
		{
			IconBox->AddSlot()
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.Image(this, &SFlareButton::GetIconBrush)
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

void SFlareButton::SetDisabled(bool State)
{
	IsDisabled = State;
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareButton::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SWidget::OnMouseEnter(MyGeometry, MouseEvent);

	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	if (MenuManager)
	{
		MenuManager->ShowTooltip(this, Text.Get(), HelpText.Get());
	}
}

void SFlareButton::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SWidget::OnMouseLeave(MouseEvent);

	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	if (MenuManager)
	{
		MenuManager->HideTooltip(this);
	}
}

const FSlateBrush* SFlareButton::GetDecoratorBrush() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	if (IsTransparent)
	{
		return &Theme.InvisibleBrush;
	}
	else if (IsDisabled.IsBound() && IsDisabled.Get())
	{
		return &Theme.ButtonDisabledDecorator;
	}
	else if (IsToggle)
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

const FSlateBrush* SFlareButton::GetIconBrush() const
{
	if (Icon.IsSet())
	{
		return Icon.Get();
	}
	else if (IsToggle)
	{
		return (IsPressed ? FFlareStyleSet::GetIcon("OK") : FFlareStyleSet::GetIcon("Disabled"));
	}
	else
	{
		return NULL;
	}
}

const FSlateBrush* SFlareButton::GetBackgroundBrush() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	if (IsTransparent)
	{
		return (IsHovered() ? &Theme.NearInvisibleBrush : &Theme.InvisibleBrush);
	}
	else if (IsDisabled.IsBound() && IsDisabled.Get())
	{
		return &Theme.ButtonDisabledBackground;
	}
	else
	{
		return (IsHovered() ? &Theme.ButtonActiveBackground : &Theme.ButtonBackground);
	}
}

FSlateColor SFlareButton::GetMainColor() const
{
	return Color.Get();
}

FReply SFlareButton::OnButtonClicked()
{
	if (!IsDisabled.IsBound() || !IsDisabled.Get())
	{
		if (IsToggle)
		{
			IsPressed = !IsPressed;
		}

		if (OnClicked.IsBound() == true)
		{
			OnClicked.Execute();
		}
	}

	return FReply::Handled();
}
