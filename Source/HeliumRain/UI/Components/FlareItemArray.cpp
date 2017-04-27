
#include "FlareItemArray.h"
#include "../../Flare.h"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareItemArray::Construct(const FArguments& InArgs)
{
	// Data
	CurrentIndex = 0;
	LineSize = InArgs._LineSize;
	Width = InArgs._Width;
	Height = InArgs._Height;
	OnItemPickedCallback = InArgs._OnItemPicked;
	
	// Grid object
	ChildSlot
	[
		SAssignNew(WidgetGrid, SGridPanel)
	];
}

void SFlareItemArray::AddItem(const TSharedRef< SWidget >& InContent)
{
	TSharedPtr<SFlareButton> Temp;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Create button
	WidgetGrid->AddSlot(CurrentIndex % LineSize, CurrentIndex / LineSize)
		[
			SAssignNew(Temp, SFlareButton)
			.OnClicked(this, &SFlareItemArray::OnItemPicked, TSharedPtr<int32>(new int32(CurrentIndex)))
			.Toggle(true)
			.Width(Width)
			.Height(Height)
		];

	// Fill content
	Temp->GetContainer()->SetHAlign(HAlign_Fill);
	Temp->GetContainer()->SetVAlign(VAlign_Fill);
	Temp->GetContainer()->SetContent(InContent);
	ContentArray.Add(Temp);

	CurrentIndex++;
}

void SFlareItemArray::ClearItems()
{
	CurrentIndex = 0;
	ContentArray.Empty();
	WidgetGrid->ClearChildren();
}

void SFlareItemArray::SetSelectedIndex(int32 ItemIndex)
{
	for (int32 i = 0; i < ContentArray.Num(); i++)
	{
		ContentArray[i]->SetActive(i == ItemIndex);
	}
}

int32 SFlareItemArray::GetSelectedIndex() const
{
	for (int32 i = 0; i < ContentArray.Num(); i++)
	{
		if (ContentArray[i]->IsActive())
		{
			return i;
		}
	}
	return 0;
}

TSharedRef<SWidget> SFlareItemArray::GetItemContent(int32 ItemIndex) const
{
	return ContentArray[ItemIndex]->GetContainer()->GetContent();
}

void SFlareItemArray::OnItemPicked(TSharedPtr<int32> ItemIndex)
{
	// Disable other items in the array
	for (int32 i = 0; i < ContentArray.Num(); i++)
	{
		ContentArray[i]->SetActive(i == *ItemIndex.Get());
	}

	// Pass it on !
	if (OnItemPickedCallback.IsBound() == true)
	{
		OnItemPickedCallback.Execute(*ItemIndex.Get());
	}
}
