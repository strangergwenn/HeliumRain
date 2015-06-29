
#include "../../Flare.h"
#include "FlareListItem.h"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareListItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	// Args
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	int32 Width = InArgs._Width * Theme.ButtonWidth;
	int32 Height = InArgs._Height * Theme.ButtonHeight;
	IsSelected = false;

	// Structure
	STableRow< TSharedPtr<FInterfaceContainer> >::Construct(
		STableRow< TSharedPtr<FInterfaceContainer> >::FArguments()
		.Style(FFlareStyleSet::Get(), "Flare.TableRow")
		.Content()
		[
			// Central content box
			SNew(SBox)
			.WidthOverride(Width)
			.MinDesiredHeight(Height)
			.Padding(FMargin(0))
			.VAlign(VAlign_Top)
			[
				// Button background
				SNew(SBorder)
				.BorderImage(this, &SFlareListItem::GetBackgroundBrush)
				[
					SNew(SHorizontalBox)

					// Content box and inner container
					+ SHorizontalBox::Slot()
					[
						SAssignNew(InnerContainer, SBorder)
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Top)
						.Padding(Theme.ButtonPadding)
						.BorderImage(new FSlateNoResource)
						[
							InArgs._Content.Widget
						]
					]

					// Toggle light
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Fill)
					[
						SNew(SBorder)
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
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	return (IsSelected ? &Theme.ButtonActiveDecorator : &Theme.ButtonDecorator);
}

const FSlateBrush* SFlareListItem::GetBackgroundBrush() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	return (IsHovered() ? &Theme.ButtonActiveBackground : &Theme.ButtonBackground);
}

void SFlareListItem::SetSelected(bool Selected)
{
	IsSelected = Selected;
}
