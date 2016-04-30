
#include "../../Flare.h"
#include "FlareConfirmationBox.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareConfirmationBox"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareConfirmationBox::Construct(const FArguments& InArgs)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	ConfirmText = InArgs._ConfirmText;
	FullHide = InArgs._FullHide;
	PC = InArgs._PC;
	Amount = 0;

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
			SAssignNew(CancelButton, SFlareButton)
			.HelpText(LOCTEXT("Cancel", "Cancel"))
			.Text(InArgs._CancelText)
			.OnClicked(InArgs._OnCancelled)
		]

		// Buy button
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		[
			SAssignNew(ConfirmButton, SFlareButton)
			.Icon(FFlareStyleSet::GetIcon("Cost"))
			.Text(this, &SFlareConfirmationBox::GetBuyText)
			.HelpText(LOCTEXT("Confirm", "Confirm"))
			.OnClicked(InArgs._OnConfirmed)
			.Width(10)
			.Height(1)
		]
	];

	// Default is hidden
	Hide();
}


/*----------------------------------------------------
	Content
----------------------------------------------------*/

FText SFlareConfirmationBox::GetBuyText() const
{
	if (Amount != 0)
	{
		return FText::Format(LOCTEXT("ConfirmTextFormat", "{0} ({1} credits)"), ConfirmText, FText::AsNumber(UFlareGameTools::DisplayMoney(Amount)));
	}
	else
	{
		return ConfirmText;
	}
}

void SFlareConfirmationBox::Show(float NewAmount, UFlareCompany* TargetCompany)
{
	Amount = NewAmount;
	ConfirmButton->SetVisibility(EVisibility::Visible);
	CancelButton->SetVisibility(EVisibility::Visible);

	if (NewAmount > TargetCompany->GetMoney())
	{
		ConfirmButton->SetDisabled(true);
	}
	else
	{
		ConfirmButton->SetDisabled(false);
	}
}

void SFlareConfirmationBox::Hide()
{
	ConfirmButton->SetVisibility(EVisibility::Collapsed);

	if (FullHide)
	{
		CancelButton->SetVisibility(EVisibility::Collapsed);
	}
}


#undef LOCTEXT_NAMESPACE
