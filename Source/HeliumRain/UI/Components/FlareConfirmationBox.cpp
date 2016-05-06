
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
	TargetCompany = NULL;

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SVerticalBox)
		
		// Help text
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.ContentPadding)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TextFont)
			.Text(this, &SFlareConfirmationBox::GetWalletText)
		]
		
		// Buttons
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.TitlePadding)
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
	if (Amount != 0 && TargetCompany)
	{
		if (Amount <= static_cast<int64>(TargetCompany->GetMoney()))
		{
			return FText::Format(LOCTEXT("ConfirmTextFormat", "{0} ({1} credits)"),
				ConfirmText,
				FText::AsNumber(UFlareGameTools::DisplayMoney(Amount)));
		}
		else
		{
			return FText::Format(LOCTEXT("DeniedTextFormat", "Company only has {0} credits"),
				FText::AsNumber(UFlareGameTools::DisplayMoney(TargetCompany->GetMoney())));
		}
	}
	else
	{
		return ConfirmText;
	}
}

FText SFlareConfirmationBox::GetWalletText() const
{
	if (PC)
	{
		return FText::Format(LOCTEXT("CompanyCurrentWallet", "You have {0} credits available."),
			FText::AsNumber(PC->GetCompany()->GetMoney() / 100));
	}

	return FText();
}

void SFlareConfirmationBox::Show(int64 NewAmount, UFlareCompany* NewTargetCompany)
{
	Amount = NewAmount;
	TargetCompany = NewTargetCompany;
	ConfirmButton->SetVisibility(EVisibility::Visible);
	CancelButton->SetVisibility(EVisibility::Visible);

	if (NewAmount > static_cast<int64>(TargetCompany->GetMoney()))
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
	Amount = 0;
	TargetCompany = NULL;
	ConfirmButton->SetDisabled(true);

	if (FullHide)
	{
		ConfirmButton->SetVisibility(EVisibility::Collapsed);
		CancelButton->SetVisibility(EVisibility::Collapsed);
	}
}


#undef LOCTEXT_NAMESPACE
