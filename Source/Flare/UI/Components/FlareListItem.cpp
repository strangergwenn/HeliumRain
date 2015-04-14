
#include "../../Flare.h"
#include "FlareListItem.h"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareListItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	IsSelected = false;
	ButtonStyle = InArgs._ButtonStyle;
	ContainerStyle = InArgs._ContainerStyle;

	STableRow< TSharedPtr<FInterfaceContainer> >::Construct(
		STableRow< TSharedPtr<FInterfaceContainer> >::FArguments()
		.Style(FFlareStyleSet::Get(), "Flare.TableRow")
		.Content()
		[
			// Central content box
			SNew(SBox)
			.WidthOverride(ButtonStyle->Width)
			.MinDesiredHeight(ButtonStyle->Height)
			.Padding(FMargin(0))
			.VAlign(VAlign_Top)
			[
				// Button background
				SNew(SBorder)
				.Padding(ContainerStyle->BorderPadding)
				.BorderImage(this, &SFlareListItem::GetBackgroundBrush)
				[
					SNew(SHorizontalBox)

					// Content box and inner container
					+ SHorizontalBox::Slot()
					[
						SAssignNew(InnerContainer, SBorder)
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Top)
						.Padding(ButtonStyle->ContentPadding)
						.BorderImage(new FSlateNoResource)
						[
							InArgs._Content.Widget
						]
					]

					// Toggle light
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					[
						SNew(SBorder)
						.Padding(ContainerStyle->BorderPadding)
						.BorderImage(this, &SFlareListItem::GetBackgroundBrush)
						[
							SNew(SImage)
							.Image(this, &SFlareListItem::GetDecoratorBrush)
						]
					]
				]
			]
		],
		InOwnerTableView
	);
}

/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

const FSlateBrush* SFlareListItem::GetDecoratorBrush() const
{
	return (IsSelected ? &ButtonStyle->ActiveDecoratorBrush : &ButtonStyle->DecoratorBrush);
}

const FSlateBrush* SFlareListItem::GetBackgroundBrush() const
{
	return (IsHovered() ? &ContainerStyle->ActiveBackgroundBrush : &ContainerStyle->BackgroundBrush);
}

void SFlareListItem::SetSelected(bool Selected)
{
	IsSelected = Selected;
}
