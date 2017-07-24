
#include "FlareConfirmationOverlay.h"
#include "../../Flare.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "SBackgroundBlur.h"

#define LOCTEXT_NAMESPACE "FlareConfirmationOverlay"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareConfirmationOverlay::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	InfoTitle = LOCTEXT("AreYouSureTitleInfo", "ARE YOU SURE ?");
	InfoText = LOCTEXT("AreYouSureInfo", "Please confirm this action.");

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Fill)
	[
		SNew(SBox)
		[
			SNew(SBackgroundBlur)
			.BlurRadius(Theme.BlurRadius)
			.BlurStrength(Theme.BlurStrength)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(FMargin(0))
			[
				SNew(SBorder)
				.HAlign(HAlign_Center)
				.Padding(Theme.ContentPadding)
				.BorderImage(&Theme.BackgroundBrush)
				[
					SNew(SVerticalBox)

					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					[
						SNew(STextBlock)
						.Text(this, &SFlareConfirmationOverlay::GetTitle)
						.TextStyle(&Theme.TitleFont)
						.Justification(ETextJustify::Center)
					]

					// Info text
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					[
						SNew(STextBlock)
						.Text(this, &SFlareConfirmationOverlay::GetText)
						.TextStyle(&Theme.SubTitleFont)
						.Justification(ETextJustify::Center)
						.WrapTextAt(2 * Theme.ContentWidth)
					]

					// Buttons
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Center)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew(OKButton, SFlareButton)
							.Text(LOCTEXT("Confirm", "Confirm"))
							.HelpText(LOCTEXT("ConfirmInfo", "Confirm this action"))
							.Icon(FFlareStyleSet::GetIcon("OK"))
							.OnClicked(this, &SFlareConfirmationOverlay::OnConfirmed)
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew(CancelButton, SFlareButton)
							.Text(LOCTEXT("Cancel", "Cancel"))
							.HelpText(LOCTEXT("CancelInfo", "Cancel this action"))
							.Icon(FFlareStyleSet::GetIcon("Delete"))
							.OnClicked(this, &SFlareConfirmationOverlay::OnCancelled)
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew(IgnoreButton, SFlareButton)
							.Text(LOCTEXT("Ignore", "Ignore"))
							.HelpText(LOCTEXT("IgnoreInfo", "Ignore this message"))
							.Icon(FFlareStyleSet::GetIcon("Delete"))
							.OnClicked(this, &SFlareConfirmationOverlay::OnIgnored)
						]
					]
				]
			]
		]
	];

	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareConfirmationOverlay::Confirm(FText Title, FText Text, FSimpleDelegate OnConfirmed, FSimpleDelegate OnCancel, FSimpleDelegate OnIgnore)
{
	InfoText = Text;
	InfoTitle = Title;
	OnConfirmedCB = OnConfirmed;
	OnCancelCB = OnCancel;
	OnIgnoreCB = OnIgnore;

	if (OnIgnore.IsBound())
	{
		IgnoreButton->SetVisibility(EVisibility::Visible);
	}
	else
	{
		IgnoreButton->SetVisibility(EVisibility::Collapsed);
	}
	SetVisibility(EVisibility::Visible);

	MenuManager->GetPC()->ClientPlaySound(MenuManager->GetPC()->GetSoundManager()->BellSound);
}

bool SFlareConfirmationOverlay::IsOpen() const
{
	return (GetVisibility() == EVisibility::Visible);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FText SFlareConfirmationOverlay::GetTitle() const
{
	return InfoTitle;
}

FText SFlareConfirmationOverlay::GetText() const
{
	return InfoText;
}

void SFlareConfirmationOverlay::OnConfirmed()
{
	MenuManager->HideTooltip(OKButton.Get());
	MenuManager->HideTooltip(CancelButton.Get());
	MenuManager->HideTooltip(IgnoreButton.Get());
	SetVisibility(EVisibility::Collapsed);

	OnConfirmedCB.ExecuteIfBound();

	MenuManager->GetPC()->ClientPlaySound(MenuManager->GetPC()->GetSoundManager()->InfoSound);
}

void SFlareConfirmationOverlay::OnCancelled()
{
	MenuManager->HideTooltip(OKButton.Get());
	MenuManager->HideTooltip(CancelButton.Get());
	MenuManager->HideTooltip(IgnoreButton.Get());
	SetVisibility(EVisibility::Collapsed);

	OnCancelCB.ExecuteIfBound();

	MenuManager->GetPC()->ClientPlaySound(MenuManager->GetPC()->GetSoundManager()->NegativeClickSound);
}

void SFlareConfirmationOverlay::OnIgnored()
{
	MenuManager->HideTooltip(OKButton.Get());
	MenuManager->HideTooltip(CancelButton.Get());
	MenuManager->HideTooltip(IgnoreButton.Get());
	SetVisibility(EVisibility::Collapsed);

	OnIgnoreCB.ExecuteIfBound();

	MenuManager->GetPC()->ClientPlaySound(MenuManager->GetPC()->GetSoundManager()->NegativeClickSound);
}

#undef LOCTEXT_NAMESPACE
