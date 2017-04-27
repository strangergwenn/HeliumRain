
#include "FlareListItem.h"
#include "../../Flare.h"


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
				.Padding(FMargin(0))
				.BorderImage(this, &SFlareListItem::GetBackgroundBrush)
				[
					SNew(SHorizontalBox)

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

					// Content box and inner container
					+ SHorizontalBox::Slot()
					[
						SAssignNew(InnerContainer, SBorder)
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						.BorderImage(new FSlateNoResource)
						[
							InArgs._Content.Widget
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
	return (IsSelected ? &Theme.ButtonActiveDecorator : &Theme.InvisibleBrush);
}

const FSlateBrush* SFlareListItem::GetBackgroundBrush() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	return (IsHovered() ? &Theme.ListActiveBackground : &Theme.ListBackground);
}

void SFlareListItem::SetSelected(bool Selected)
{
	IsSelected = Selected;
}
