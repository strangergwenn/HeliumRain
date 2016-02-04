
#include "../../Flare.h"
#include "FlareCargoInfo.h"
#include "../../Spacecrafts/FlareSimulatedSpacecraft.h"
#include "../../Player/FlareMenuManager.h"

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
	.VAlign(VAlign_Center)
	.Padding(FMargin(1))
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
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.SmallFont)
						.Text(this, &SFlareCargoInfo::GetResourceQuantity)
					]
				]
			]
		]
	];

	// Don't intercept clicks if it's not interactive
	if (!OnClicked.IsBound())
	{
		Button->SetVisibility(EVisibility::HitTestInvisible);
	}
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

	if (MenuManager)
	{
		FText InfoText = Cargo->Resource ? Cargo->Resource->Description : LOCTEXT("EmptyInfo", "Empty bay");
		MenuManager->ShowTooltip(this, LOCTEXT("CargoBayTitle", "Cargo bay"), InfoText);
	}
}

void SFlareCargoInfo::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SWidget::OnMouseLeave(MouseEvent);

	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	if (MenuManager)
	{
		MenuManager->HideTooltip(this);
	}
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

	return FText::FromString(FString::Printf(TEXT("%u/%u"), Cargo->Quantity, Cargo->Capacity)); //FString needed here
}

FReply SFlareCargoInfo::OnButtonClicked()
{
	FFlareCargo* Cargo = TargetSpacecraft->GetCargoBay()->GetSlot(CargoIndex);

	if (Cargo && Cargo->Resource)
	{
		OnClicked.ExecuteIfBound();
	}

	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE
