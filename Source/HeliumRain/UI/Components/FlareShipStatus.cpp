
#include "../../Flare.h"
#include "FlareShipStatus.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Data/FlareSpacecraftComponentsCatalog.h"

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
	.Padding(FMargin(8))
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

		// Refilling icon
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(FMargin(5, 0, 5, 0))
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetIcon("Tank"))
			.Visibility(this, &SFlareShipStatus::GetRefillingVisibility)
		]

		// Reparing icon
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(FMargin(5, 0, 5, 0))
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetIcon("Repair"))
			.Visibility(this, &SFlareShipStatus::GetRepairingVisibility)
		]

		// Health
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(50)
			.VAlign(VAlign_Center)
			[
				SNew(SProgressBar)
				.Percent(this, &SFlareShipStatus::GetGlobalHealth)
				.BorderPadding( FVector2D(0,0))
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
	if (MenuManager && TargetShip && TargetShip->IsValidLowLevel())
	{
		FText Info;
		UFlareSimulatedSpacecraftDamageSystem* DamageSystem = TargetShip->GetDamageSystem();

		// Station-specific damage & info
		if (TargetShip->IsStation())
		{
			// Structure
			Info = FText::Format(LOCTEXT("StationHealthInfoFormat", "Structure : {0}%"),
				FText::AsNumber(FMath::RoundToInt(100 * DamageSystem->GetGlobalHealth())));
		}

		// Ship-specific damage
		else
		{
			// Status info
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
			if (TargetShip->GetRefillStock() > 0 && TargetShip->NeedRefill())
			{
				Info = FText::Format(LOCTEXT("ShipRefillingFormat", "{0}This ship is being refilled with ammunition and fuel.\n"), Info);
			}
			if (TargetShip->GetRepairStock() > 0 && TargetShip->GetDamageSystem()->GetGlobalHealth() < 1.f)
			{
				Info = FText::Format(LOCTEXT("ShipRepairormat", "{0}This ship is being repaired.\n"), Info);
			}

			if (TargetShip->IsMilitary())
			{
				int32 MaxAmmo = 0;
				int32 CurrentSpentAmmo = 0;

				UFlareSpacecraftComponentsCatalog* Catalog = TargetShip->GetGame()->GetShipPartsCatalog();
				for (int32 ComponentIndex = 0; ComponentIndex < TargetShip->GetData().Components.Num(); ComponentIndex++)
				{
					FFlareSpacecraftComponentSave* ComponentData = &TargetShip->GetData().Components[ComponentIndex];
					FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

					if (ComponentDescription->Type == EFlarePartType::Weapon)
					{
						MaxAmmo += ComponentDescription->WeaponCharacteristics.AmmoCapacity;
						CurrentSpentAmmo += ComponentData->Weapon.FiredAmmo;
					}
				}

				Info = FText::Format(LOCTEXT("AmmoInfoFormat", "{0}\nAmmo : {1} / {2}"), Info,
					FText::AsNumber(MaxAmmo - CurrentSpentAmmo), FText::AsNumber(MaxAmmo));
			}

			// Subsystems
			for (int32 Index = EFlareSubsystem::SYS_None + 1; Index <= EFlareSubsystem::SYS_Weapon; Index++)
			{
				if (Index == EFlareSubsystem::SYS_Weapon && !TargetShip->IsMilitary())
				{
					continue;
				}
				Info = FText::Format(LOCTEXT("ShipHealthInfoFormat", "{0}\n{1} : {2}%"),
					Info,
					UFlareSimulatedSpacecraftDamageSystem::GetSubsystemName((EFlareSubsystem::Type)Index),
					FText::AsNumber(FMath::RoundToInt(100 * DamageSystem->GetSubsystemHealth((EFlareSubsystem::Type)Index))));
			}
		}

		MenuManager->ShowTooltip(this, LOCTEXT("Status", "SPACECRAFT STATUS"), Info);
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
	if (TargetShip && TargetShip->IsValidLowLevel())
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
	if (TargetShip && TargetShip->IsValidLowLevel() && !TargetShip->IsStation())
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


EVisibility SFlareShipStatus::GetRefillingVisibility() const
{

	if (TargetShip && TargetShip->IsValidLowLevel())
	{
		//FLOGV("%s need refill ? %d stock : %f", *TargetShip->GetImmatriculation().ToString(), TargetShip->NeedRefill(), TargetShip->GetRefillStock());
		if (TargetShip->GetRefillStock() > 0 && TargetShip->NeedRefill())
		{
			return EVisibility::Visible;
		}
	}
	return EVisibility::Collapsed;
}

EVisibility SFlareShipStatus::GetRepairingVisibility() const
{

	if (TargetShip && TargetShip->IsValidLowLevel())
	{
		//FLOGV("%s need repair ? %f stock : %f", *TargetShip->GetImmatriculation().ToString(), TargetShip->GetDamageSystem()->GetGlobalHealth(), TargetShip->GetRepairStock());
		if (TargetShip->GetRepairStock() > 0 && TargetShip->GetDamageSystem()->GetGlobalHealth() < 1.f)
		{
			return EVisibility::Visible;
		}
	}

	return EVisibility::Collapsed;
}

#undef LOCTEXT_NAMESPACE
