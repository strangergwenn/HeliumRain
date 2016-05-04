
#include "../../Flare.h"
#include "FlareCargoInfo.h"
#include "../../Spacecrafts/FlareSpacecraftInterface.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Economy/FlareCargoBay.h"

#define LOCTEXT_NAMESPACE "FlareCargoInfo"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareCargoInfo::Construct(const FArguments& InArgs)
{
	// Params
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	TargetSpacecraft = InArgs._Spacecraft;
	CargoIndex = InArgs._CargoIndex;
	OnClicked = InArgs._OnClicked;
	TSharedPtr<SButton> Button;
	
	// Layout
	ChildSlot
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Top)
	.Padding(FMargin(1))
	[
		SNew(SVerticalBox)

		// Main
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0))
		[
			// Button (behaviour only, no display)
			SAssignNew(Button, SButton)
			.OnClicked(this, &SFlareCargoInfo::OnButtonClicked)
			.ContentPadding(FMargin(0))
			.ButtonStyle(FCoreStyle::Get(), "NoBorder")
			[
				SNew(SBorder)
				.Padding(FMargin(0))
				.BorderImage(this, &SFlareCargoInfo::GetResourceIcon)
				[
					SNew(SBox)
					.WidthOverride(Theme.ResourceWidth)
					.HeightOverride(Theme.ResourceHeight)
					.Padding(FMargin(0))
					[
						SNew(SVerticalBox)
			
						// Resource name
						+ SVerticalBox::Slot()
						.Padding(Theme.SmallContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(this, &SFlareCargoInfo::GetResourceAcronym)
						]

						// Resource quantity
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.SmallContentPadding)
						.VAlign(VAlign_Bottom)
						.HAlign(HAlign_Right)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SmallFont)
							.Text(this, &SFlareCargoInfo::GetResourceQuantity)
						]
					]
				]
			]
		]

		// Dump
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0))
		[
			SAssignNew(DumpButton, SFlareButton)
			.Transparent(true)
			.Text(FText())
			.HelpText(LOCTEXT("DumpResourceHelp", "Dump this resource"))
			.Icon(FFlareStyleSet::GetIcon("Stop"))
			.OnClicked(this, &SFlareCargoInfo::OnDumpClicked)
			.Width(1)
		]
	];

	// Don't intercept clicks if it's not interactive
	if (!OnClicked.IsBound())
	{
		Button->SetVisibility(EVisibility::HitTestInvisible);
	}
	DumpButton->SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareCargoInfo::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SWidget::OnMouseEnter(MyGeometry, MouseEvent);

	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	FFlareCargo* Cargo = TargetSpacecraft->GetCargoBay()->GetSlot(CargoIndex);
	check(Cargo);

	// Tooltip
	if (MenuManager)
	{
		FText TitleText = Cargo->Resource ? Cargo->Resource->Name : LOCTEXT("EmptyTitle", "Empty bay");
		FText InfoText = Cargo->Resource ? Cargo->Resource->Description : LOCTEXT("EmptyInfo", "This cargo bay is empty.");
		MenuManager->ShowTooltip(this, FText::Format(LOCTEXT("CargoBayFormat", "Cargo bay : {0}"), TitleText), InfoText);
	}

	// Dump button
	bool CanDump = Cargo->Resource && Cargo->Quantity > 0 && TargetSpacecraft->GetCompany() == MenuManager->GetPC()->GetCompany();
	DumpButton->SetVisibility(CanDump ? EVisibility::Visible : EVisibility::Collapsed);
}

void SFlareCargoInfo::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SWidget::OnMouseLeave(MouseEvent);

	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	if (MenuManager)
	{
		MenuManager->HideTooltip(this);
	}

	DumpButton->SetVisibility(EVisibility::Collapsed);
}

const FSlateBrush* SFlareCargoInfo::GetResourceIcon() const
{
	FFlareCargo* Cargo = TargetSpacecraft->GetCargoBay()->GetSlot(CargoIndex);
	check(Cargo);

	if (Cargo->Resource)
	{
		return &Cargo->Resource->Icon;
	}
	else
	{
		const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
		return &Theme.ResourceBackground;
	}
}

FText SFlareCargoInfo::GetResourceAcronym() const
{
	FFlareCargo* Cargo = TargetSpacecraft->GetCargoBay()->GetSlot(CargoIndex);
	check(Cargo);

	if (Cargo->Resource)
	{
		return Cargo->Resource->Acronym;
	}
	else
	{
		return LOCTEXT("Empty", " ");
	}
}

FText SFlareCargoInfo::GetResourceQuantity() const
{
	FFlareCargo* Cargo = TargetSpacecraft->GetCargoBay()->GetSlot(CargoIndex);
	check(Cargo);
	
	// Print IO text if any
	FText LockText;
	if (Cargo->Locked)
	{
		if (TargetSpacecraft->IsOutputResource(Cargo->Resource))
		{
			LockText = FText::Format(LOCTEXT("OutputCargoFormat", "(Output)\n"), Cargo->Resource->Acronym);
		}
		else
		{
			LockText = FText::Format(LOCTEXT("InputCargoFormat", "(Input)\n"), Cargo->Resource->Acronym);
		}
	}

	// Format the current capacity info
	if (Cargo->Capacity > 999)
	{
		FNumberFormattingOptions CargoFormat;
		CargoFormat.MaximumFractionalDigits = 1;

		return FText::Format(FText::FromString("{0} {1}k/{2}k"),
			LockText,
			FText::AsNumber(Cargo->Quantity / 1000.0f, &CargoFormat),
			FText::AsNumber(Cargo->Capacity / 1000.0f, &CargoFormat));
	}
	else
	{
		return FText::FromString(FString::Printf(TEXT("%s %u/%u"), *LockText.ToString(), Cargo->Quantity, Cargo->Capacity));
	}
}

FReply SFlareCargoInfo::OnButtonClicked()
{
	FFlareCargo* Cargo = TargetSpacecraft->GetCargoBay()->GetSlot(CargoIndex);

	if (Cargo && Cargo->Resource && Cargo->Quantity > 0)
	{
		OnClicked.ExecuteIfBound();
	}

	return FReply::Handled();
}

void SFlareCargoInfo::OnDumpClicked()
{
	FFlareCargo* Cargo = TargetSpacecraft->GetCargoBay()->GetSlot(CargoIndex);

	if (Cargo && Cargo->Resource)
	{
		TargetSpacecraft->GetCargoBay()->DumpCargo(Cargo);
	}
}

#undef LOCTEXT_NAMESPACE
