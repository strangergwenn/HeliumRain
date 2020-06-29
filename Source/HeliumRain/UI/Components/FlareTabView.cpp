
#include "HeliumRain/UI/Components/FlareTabView.h"
#include "../../Flare.h"
#include "../Style/FlareStyleSet.h"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareTabView::Construct(const FArguments& InArgs)
{
	// Data
	CurrentTabIndex = 0;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Structure
	ChildSlot
	[
		SNew(SVerticalBox)

		// Tabs
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Top)
		.AutoHeight()
		[
			SNew(SVerticalBox)
		
			// Header buttons
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(Header, SHorizontalBox)
				]

				+ SHorizontalBox::Slot()
				.Padding(FMargin(0, 0, 0, 1))
				[
					SNew(SImage)
					.Image(&Theme.BackgroundBrush)
				]
			]

			// Header border
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(1)
				[
					SNew(SImage)
					.Image(&Theme.NearInvisibleBrush)
				]
			]
		]

		// Main view
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Fill)
		[
			SNew(SScrollBox)
			.Style(&Theme.ScrollBoxStyle)
			.ScrollBarStyle(&Theme.ScrollBarStyle)

			+ SScrollBox::Slot()
			.HAlign(HAlign_Fill)
			[
				SAssignNew(Content, SWidgetSwitcher)
				.WidgetIndex(this, &SFlareTabView::GetCurrentTabIndex)
			]
		]
	];

	// Slot contents
	int32 Index = 0;
	for (SFlareTabView::FSlot* TabSlot : InArgs.Slots)
	{
		// Add header entry
		Header->AddSlot()
		.AutoWidth()
		[
			SNew(SFlareButton)
			.Text(TabSlot->HeaderText)
			.HelpText(TabSlot->HeaderHelpText)
			.OnClicked(this, &SFlareTabView::SetCurrentTabIndex, Index)
			.IsDisabled(this, &SFlareTabView::IsCurrentTab, Index)
			.Width(5)
		];
		
		// Add content
		Content->AddSlot()
		[
			TabSlot->GetWidget()
		];

		Index++;
	}
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

int32 SFlareTabView::GetCurrentTabIndex() const
{
	return CurrentTabIndex;
}

bool SFlareTabView::IsCurrentTab(int32 Index) const
{
	return (CurrentTabIndex == Index);
}

void SFlareTabView::SetCurrentTabIndex(int32 Index)
{
	CurrentTabIndex = Index;
}
