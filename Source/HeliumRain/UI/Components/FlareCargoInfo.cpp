
#include "FlareCargoInfo.h"

#include "../../Flare.h"
#include "../../Game/FlareGame.h"

#include "../../Economy/FlareCargoBay.h"

#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"

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

		// Trade permission
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0))
		[
			SAssignNew(PermissionButton, SFlareButton)
			.Transparent(true)
			.Toggle(true)
			.Small(true)
			.Text(LOCTEXT("PermissionButton", "Trade"))
			.HelpText(LOCTEXT("PermissionButtonHelp", "Set whether resources in this cargo slot can be traded with other companies"))
			.OnClicked(this, &SFlareCargoInfo::OnPermissionClicked)
			.Visibility(this, &SFlareCargoInfo::GetPermissionVisibility)
			.Width(1.5)
		]

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
			.Icon(FFlareStyleSet::GetIcon("Dump"))
			.OnClicked(this, &SFlareCargoInfo::OnDumpClicked)
			.Width(1)
		]
	];

	// Initial state of cargo slot
	if (TargetSpacecraft->GetActiveCargoBay()->GetSlotCount() <= CargoIndex)
	{
		return;
	}
	FFlareCargo* Cargo = TargetSpacecraft->GetActiveCargoBay()->GetSlot(CargoIndex);
	FCHECK(Cargo);
	PermissionButton->SetActive(Cargo->Restriction == EFlareResourceRestriction::Everybody);

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

	if (TargetSpacecraft->GetActiveCargoBay()->GetSlotCount() <= CargoIndex)
	{
		return;
	}

	FFlareCargo* Cargo = TargetSpacecraft->GetActiveCargoBay()->GetSlot(CargoIndex);
	FCHECK(Cargo);

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
	if (TargetSpacecraft->GetActiveCargoBay()->GetSlotCount() <= CargoIndex)
	{
		return NULL;
	}

	FFlareCargo* Cargo = TargetSpacecraft->GetActiveCargoBay()->GetSlot(CargoIndex);
	FCHECK(Cargo);

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
	if (TargetSpacecraft->GetActiveCargoBay()->GetSlotCount() <= CargoIndex)
	{
		return FText();
	}

	FFlareCargo* Cargo = TargetSpacecraft->GetActiveCargoBay()->GetSlot(CargoIndex);
	FCHECK(Cargo);

	if (Cargo->Resource)
	{
		return Cargo->Resource->Acronym;
	}
	else
	{
		return FText();
	}
}

FText SFlareCargoInfo::GetResourceQuantity() const
{
	if (TargetSpacecraft->GetActiveCargoBay()->GetSlotCount() <= CargoIndex)
	{
		return FText();
	}

	FFlareCargo* Cargo = TargetSpacecraft->GetActiveCargoBay()->GetSlot(CargoIndex);
	FCHECK(Cargo);
	
	// Print IO text if any
	FText LockText;
	if (Cargo->Lock == EFlareResourceLock::Output)
	{
		LockText = FText::Format(LOCTEXT("OutputCargoFormat", "(Output)\n"), Cargo->Resource->Acronym);
	}
	else if (Cargo->Lock == EFlareResourceLock::Input)
	{
		LockText = FText::Format(LOCTEXT("InputCargoFormat", "(Input)\n"), Cargo->Resource->Acronym);
	}
	else if (Cargo->Lock == EFlareResourceLock::Trade)
	{
		LockText = FText::Format(LOCTEXT("TradeCargoFormat", "(Trade)\n"), Cargo->Resource->Acronym);
	}
	else if (Cargo->Lock == EFlareResourceLock::Hidden)
	{
		LockText = FText::Format(LOCTEXT("HiddenCargoFormat", "(Hidden)\n"), Cargo->Resource->Acronym);
	}
	
	// Format the current capacity info
	int32 Capacity = TargetSpacecraft->GetActiveCargoBay()->GetSlotCapacity();

	if (Capacity > 999)
	{
		FNumberFormattingOptions CargoFormat;
		CargoFormat.MaximumFractionalDigits = 1;

		return FText::Format(FText::FromString("{0} {1}/{2}k"),
			LockText,
			FText::AsNumber(Cargo->Quantity, &CargoFormat),
			FText::AsNumber(Capacity / 1000.0f, &CargoFormat));
	}
	else
	{
		return FText::Format(FText::FromString("{0} {1}/{2}"),
			LockText,
			FText::AsNumber(Cargo->Quantity),
			FText::AsNumber(Capacity));
	}
}

FReply SFlareCargoInfo::OnButtonClicked()
{
	if (TargetSpacecraft->GetActiveCargoBay()->GetSlotCount() <= CargoIndex)
	{
		return FReply::Handled();
	}

	FFlareCargo* Cargo = TargetSpacecraft->GetActiveCargoBay()->GetSlot(CargoIndex);

	if (Cargo && Cargo->Resource && Cargo->Quantity > 0)
	{
		OnClicked.ExecuteIfBound();
	}

	return FReply::Handled();
}

void SFlareCargoInfo::OnDumpClicked()
{
	AFlareMenuManager::GetSingleton()->Confirm(LOCTEXT("ConfirmDump", "ARE YOU SURE ?"),
		LOCTEXT("ConfirmDumpInfo", "Do you really want to dump this cargo slot ?"),
		FSimpleDelegate::CreateSP(this, &SFlareCargoInfo::OnDumpConfirmed));
}

void SFlareCargoInfo::OnDumpConfirmed()
{
	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	if (TargetSpacecraft->GetActiveCargoBay()->GetSlotCount() <= CargoIndex)
	{
		return;
	}
	FFlareCargo* Cargo = TargetSpacecraft->GetActiveCargoBay()->GetSlot(CargoIndex);

	if (Cargo && Cargo->Resource)
	{
		MenuManager->GetPC()->ClientPlaySound(MenuManager->GetPC()->GetSoundManager()->DeleteSound);
		TargetSpacecraft->GetActiveCargoBay()->DumpCargo(Cargo);
	}
}

void SFlareCargoInfo::OnPermissionClicked()
{
	TargetSpacecraft->GetActiveCargoBay()->SetSlotRestriction(
		CargoIndex,
		PermissionButton->IsActive() ? EFlareResourceRestriction::Everybody : EFlareResourceRestriction::OwnerOnly);
}

EVisibility SFlareCargoInfo::GetPermissionVisibility() const
{
	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();

	if (IsEnabled() && IsValid(TargetSpacecraft)
	 && TargetSpacecraft->IsStation()
	 && TargetSpacecraft->GetCompany() == MenuManager->GetPC()->GetCompany()
	 && MenuManager->GetCurrentMenu() != EFlareMenu::MENU_Trade)
	{
		return EVisibility::Visible;
	}
	else
	{
		return EVisibility::Collapsed;
	}
}


#undef LOCTEXT_NAMESPACE
