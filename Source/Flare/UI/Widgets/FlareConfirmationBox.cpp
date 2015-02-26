
#include "../../Flare.h"
#include "FlareConfirmationBox.h"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareConfirmationBox::Construct(const FArguments& InArgs)
{
	// Temporary data
	const FFlareContainerStyle* ContainerStyle = InArgs._ContainerStyle;

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)

		// Back button
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.AutoWidth()
		[
			SNew(SFlareButton)
			.Text(InArgs._CancelText)
			.ContainerStyle(ContainerStyle)
			.OnClicked(InArgs._OnCancelled)
		]

		// Buy button
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		[
			SAssignNew(ConfirmButton, SFlareButton)
			.ButtonStyle(FFlareStyleSet::Get(), "/Style/BuyButton")
			.ContainerStyle(ContainerStyle)
			.OnClicked(InArgs._OnConfirmed)
		]
	];

	// Buy button content
	ConfirmButton->GetContainer()->SetContent(
		SNew(SHorizontalBox)

		// Confirmation
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(InArgs._ConfirmText)
			.TextStyle(FFlareStyleSet::Get(), "Flare.Text")
		]

		// Cost icon
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(FMargin(5, 0))
		.AutoWidth()
		[
			SNew(SImage).Image(FFlareStyleSet::GetIcon("Cost"))
		]

		// Cost amount
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.AutoWidth()
		[
			SAssignNew(CostLabel, STextBlock)
			.TextStyle(FFlareStyleSet::Get(), "Flare.Text")
		]
	);

	// Default is hidden
	ConfirmButton->SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Content
----------------------------------------------------*/

void SFlareConfirmationBox::Show(float Amount)
{
	if (Amount > 0)
	{
		CostLabel->SetText("+" + FString::FromInt(Amount));
	}
	else
	{

		CostLabel->SetText(FString::FromInt(Amount));
	}
	ConfirmButton->SetVisibility(EVisibility::Visible);
}

void SFlareConfirmationBox::Hide()
{
	ConfirmButton->SetVisibility(EVisibility::Hidden);
}
