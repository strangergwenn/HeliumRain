
#include "../../Flare.h"
#include "FlareShipStatus.h"
#include "../../Player/FlareMenuManager.h"

#define LOCTEXT_NAMESPACE "FlareShipStatus"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareShipStatus::Construct(const FArguments& InArgs)
{
	TargetShip = InArgs._Ship;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	.Padding(FMargin(8, 8, 32, 8))
	[
		SNew(SHorizontalBox)

		// Power icon
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetIcon("Propulsion"))
			.ColorAndOpacity(this, &SFlareShipStatus::GetIconColor, EFlareSubsystem::SYS_Propulsion)
		]

		// RCS icon
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetIcon("RCS"))
			.ColorAndOpacity(this, &SFlareShipStatus::GetIconColor, EFlareSubsystem::SYS_RCS)
		]

		// Weapon icon
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SAssignNew(WeaponIndicator, SImage)
			.Image(FFlareStyleSet::GetIcon("Shell"))
			.ColorAndOpacity(this, &SFlareShipStatus::GetIconColor, EFlareSubsystem::SYS_Weapon)
		]

		// Health
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(60)
			.VAlign(VAlign_Center)
			[
				SNew(SProgressBar)
				.Percent(this, &SFlareShipStatus::GetGlobalHealth)
				.Style(&Theme.ProgressBarStyle)
			]
		]
	];

	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareShipStatus::SetTargetShip(UFlareSimulatedSpacecraft* Target)
{
	TargetShip = Target;

	if (TargetShip)
	{
		SetVisibility(EVisibility::Visible);
		if (TargetShip->IsMilitary())
		{
			WeaponIndicator->SetVisibility(EVisibility::Visible);
		}
		else
		{
			WeaponIndicator->SetVisibility(EVisibility::Hidden);
		}
	}
	else
	{
		SetVisibility(EVisibility::Hidden);
	}
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareShipStatus::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SWidget::OnMouseEnter(MyGeometry, MouseEvent);

	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	if (MenuManager && TargetShip)
	{
		FText Info;
		UFlareSimulatedSpacecraftDamageSystem* DamageSystem = TargetShip->GetDamageSystem();

		if (DamageSystem->IsStranded())
		{
			Info = FText::Format(LOCTEXT("ShipStrandedFormat", "{0}This ship is stranded and can't exit the sector !\n"), Info);
		}

		if (DamageSystem->IsUncontrollable())
		{
			Info = FText::Format(LOCTEXT("ShipUncontrollableFormat", "{0}This ship is uncontrollable and can't move in the local space !\n"), Info);
		}

		if (TargetShip->IsMilitary() && DamageSystem->IsDisarmed())
		{
			Info = FText::Format(LOCTEXT("ShipDisarmedFormat", "{0}This ship is disarmed and unable to fight back !\n"), Info);
		}

		for (int32 Index = EFlareSubsystem::SYS_None + 1; Index <= EFlareSubsystem::SYS_Weapon; Index++)
		{
			if (Index == EFlareSubsystem::SYS_Weapon && !TargetShip->IsMilitary())
			{
				continue;
			}
			Info = FText::Format(LOCTEXT("HealthInfoFormat", "{0}\n{1} : {2}%"),
				Info,
				UFlareSimulatedSpacecraftDamageSystem::GetSubsystemName((EFlareSubsystem::Type)Index),
				FText::AsNumber(100 * FMath::RoundToInt(DamageSystem->GetSubsystemHealth((EFlareSubsystem::Type)Index, false)))
				);
		}

		MenuManager->ShowTooltip(this, LOCTEXT("Status", "SHIP STATUS"), Info);
	}
}

void SFlareShipStatus::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SWidget::OnMouseLeave(MouseEvent);

	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	if (MenuManager)
	{
		MenuManager->HideTooltip(this);
	}
}

TOptional<float> SFlareShipStatus::GetGlobalHealth() const
{
	if (TargetShip)
	{
		return TargetShip->GetDamageSystem()->GetGlobalHealth();
	}
	else
	{
		return 0;
	}
}

FSlateColor SFlareShipStatus::GetIconColor(EFlareSubsystem::Type Type) const
{
	FLinearColor Result = FLinearColor::Black;
	Result.A = 0.0f;

	// Ignore stations
	if (TargetShip && !TargetShip->IsStation())
	{
		bool IsIncapacitated = false;
		UFlareSimulatedSpacecraftDamageSystem* DamageSystem = TargetShip->GetDamageSystem();

		// Only those are used (see above)
		switch (Type)
		{
			case EFlareSubsystem::SYS_Propulsion:    IsIncapacitated = DamageSystem->IsStranded();         break;
			case EFlareSubsystem::SYS_RCS:           IsIncapacitated = DamageSystem->IsUncontrollable();   break;
			case EFlareSubsystem::SYS_Weapon:        IsIncapacitated = DamageSystem->IsDisarmed();         break;
			default: break;
		}

		// Show in red when disabled
		if (IsIncapacitated)
		{
			const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
			Result = Theme.DamageColor;
			Result.A = 1.0f;
		}
	}

	return Result;
}


#undef LOCTEXT_NAMESPACE
