
#include "../../Flare.h"
#include "FlareDropList.h"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareDropList::Construct(const FArguments& InArgs)
{
	// Data
	IsDropped = false;
	LineSize = InArgs._LineSize;
	OnItemPickedCallback = InArgs._OnItemPicked;
	
	// Layout
	ChildSlot
	[
		SNew(SVerticalBox)

		// List header
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(HeaderButton, SFlareButton)
			.OnClicked(this, &SFlareDropList::OnHeaderClicked)
			.Width(InArgs._HeaderWidth)
			.Height(InArgs._HeaderHeight)
		]

		// Item picker below
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(ItemArray, SFlareItemArray)
			.OnItemPicked(this, &SFlareDropList::OnItemPicked)
			.LineSize(LineSize)
			.Width(InArgs._ItemWidth)
			.Height(InArgs._ItemHeight)
		]
	];

	// Default settings
	ItemArray->SetVisibility(EVisibility::Collapsed);
	HeaderButton->GetContainer()->SetHAlign(HAlign_Fill);
	HeaderButton->GetContainer()->SetVAlign(VAlign_Fill);
}

void SFlareDropList::AddItem(const TSharedRef< SWidget >& InContent)
{
	ItemArray->AddItem(InContent);

	ContentArray.Add(InContent);
}

void SFlareDropList::ClearItems()
{
	ContentArray.Empty();
	ItemArray->ClearItems();
}

void SFlareDropList::SetSelectedIndex(int32 ItemIndex)
{
	ItemArray->SetSelectedIndex(ItemIndex);
	HeaderButton->GetContainer()->SetContent(ContentArray[ItemIndex]);
}

TSharedRef<SWidget> SFlareDropList::GetItemContent(int32 ItemIndex) const
{
	return ItemArray->GetItemContent(ItemIndex);
}

void SFlareDropList::OnHeaderClicked()
{
	ItemArray->SetVisibility(IsDropped ? EVisibility::Collapsed : EVisibility::Visible);
	IsDropped = !IsDropped;
}

void SFlareDropList::OnItemPicked(int32 ItemIndex)
{
	HeaderButton->GetContainer()->SetContent(ContentArray[ItemIndex]);

	if (OnItemPickedCallback.IsBound() == true)
	{
		OnItemPickedCallback.Execute(ItemIndex);
	}
}

void SFlareDropList::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	IsDropped = false;
	ItemArray->SetVisibility(EVisibility::Collapsed);
}
