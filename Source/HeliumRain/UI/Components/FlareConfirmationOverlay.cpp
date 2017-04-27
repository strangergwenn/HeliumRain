
#include "FlareConfirmationOverlay.h"
#include "../../Flare.h"
#include "../../Player/FlareMenuManager.h"
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
					]

					// Buttons
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
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

void SFlareConfirmationOverlay::Confirm(FText Title, FText Text, FSimpleDelegate OnConfirmed)
{
	InfoText = Text;
	InfoTitle = Title;
	OnConfirmedCB = OnConfirmed;

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
	SetVisibility(EVisibility::Collapsed);

	OnConfirmedCB.ExecuteIfBound();

	MenuManager->GetPC()->ClientPlaySound(MenuManager->GetPC()->GetSoundManager()->InfoSound);
}

void SFlareConfirmationOverlay::OnCancelled()
{
	MenuManager->HideTooltip(OKButton.Get());
	MenuManager->HideTooltip(CancelButton.Get());
	SetVisibility(EVisibility::Collapsed);

	MenuManager->GetPC()->ClientPlaySound(MenuManager->GetPC()->GetSoundManager()->NegativeClickSound);
}

#undef LOCTEXT_NAMESPACE
