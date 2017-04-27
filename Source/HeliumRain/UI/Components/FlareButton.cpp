
#include "FlareButton.h"
#include "../../Flare.h"
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
	IsSmall = InArgs._Small;
	IsTransparent = InArgs._Transparent;
	OnClicked = InArgs._OnClicked;
	Color = InArgs._Color;
	Text = InArgs._Text;
	HelpText = InArgs._HelpText;
	IsDisabled = InArgs._IsDisabled;
	Width = InArgs._Width * Theme.ButtonWidth;
	Height = InArgs._Height * Theme.ButtonHeight;
	
	// Structure
	ChildSlot
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Center)
	[
		// Button (behaviour only, no display)
		SAssignNew(InternalButton, SButton)
		.OnClicked(this, &SFlareButton::OnButtonClicked)
		.ContentPadding(FMargin(0))
		.ButtonStyle(FFlareStyleSet::Get(), "Flare.CoreButton")
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
		
		InnerContainer->SetPadding(IsSmall ? FMargin(0) : Theme.ButtonPadding);
		InnerContainer->SetContent(
			SNew(SHorizontalBox)

			// Icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(3, 0))
			[
				SAssignNew(IconBox, SVerticalBox)
			]

			// Text
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SAssignNew(TextBlock, STextBlock)
				.TextStyle(&Theme.TextFont)
				.Font(this, &SFlareButton::GetTextStyle)
				.Text(InArgs._Text)
				.ColorAndOpacity(this, &SFlareButton::GetMainColor)
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
	IsDisabled.Set(State);
}

void SFlareButton::SetText(FText NewText)
{
	if (TextBlock.IsValid())
	{
		Text.Set(NewText);
		TextBlock->SetText(Text);
	}
}

void SFlareButton::SetHelpText(FText NewText)
{
	HelpText.Set(NewText);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareButton::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SWidget::OnMouseEnter(MyGeometry, MouseEvent);

	InternalButton->SetButtonStyle(&FFlareStyleSet::Get().GetWidgetStyle<FButtonStyle>("Flare.ActiveCoreButton"));

	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	if (MenuManager)
	{
		MenuManager->ShowTooltip(this, Text.Get(), HelpText.Get());
	}
}

void SFlareButton::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SWidget::OnMouseLeave(MouseEvent);

	InternalButton->SetButtonStyle(&FFlareStyleSet::Get().GetWidgetStyle<FButtonStyle>("Flare.CoreButton"));

	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	if (MenuManager)
	{
		MenuManager->HideTooltip(this);
	}
}

const FSlateBrush* SFlareButton::GetDecoratorBrush() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	bool WasDisabled = (IsDisabled.IsBound() || IsDisabled.IsSet()) && IsDisabled.Get();

	if (IsTransparent)
	{
		return &Theme.InvisibleBrush;
	}
	else if (WasDisabled)
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
		if (IsSmall)
		{
			return (IsPressed ? FFlareStyleSet::GetIcon("OK_Small") : FFlareStyleSet::GetIcon("Disabled_Small"));
		}
		else
		{
			return (IsPressed ? FFlareStyleSet::GetIcon("OK") : FFlareStyleSet::GetIcon("Disabled"));
		}
	}
	else
	{
		return NULL;
	}
}

const FSlateBrush* SFlareButton::GetBackgroundBrush() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	bool WasDisabled = (IsDisabled.IsBound() || IsDisabled.IsSet()) && IsDisabled.Get();

	if (WasDisabled)
	{
		return &Theme.ButtonDisabledBackground;
	}
	else if (IsTransparent)
	{
		return (IsHovered() ? &Theme.NearInvisibleBrush : &Theme.InvisibleBrush);
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

FSlateFontInfo SFlareButton::GetTextStyle() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	if (Text.IsSet() || Text.IsBound())
	{
		float TextLength = Text.Get().ToString().Len();
		float ButtonWidth = GetDesiredSize().X;

		if (TextLength > 0.09 * ButtonWidth || IsSmall)
		{
			return Theme.SmallFont.Font;
		}
	}

	return Theme.TextFont.Font;
}

FReply SFlareButton::OnButtonClicked()
{
	bool WasDisabled = (IsDisabled.IsBound() || IsDisabled.IsSet()) && IsDisabled.Get();
	if (!WasDisabled && IsHovered())
	{
		if (IsToggle)
		{
			IsPressed = !IsPressed;
		}

		AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
		if (MenuManager)
		{
			MenuManager->HideTooltip(this);
		}

		if (OnClicked.IsBound() == true)
		{
			OnClicked.Execute();
		}
	}

	return FReply::Handled();
}
