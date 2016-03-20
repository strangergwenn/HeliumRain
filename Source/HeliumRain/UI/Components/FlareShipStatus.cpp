
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
	CenterIcons = InArgs._Center;

	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	.Padding(FMargin(8))
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetIcon("Temperature"))
			.ColorAndOpacity(this, &SFlareShipStatus::GetIconColor, EFlareSubsystem::SYS_Temperature)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetIcon("Power"))
			.ColorAndOpacity(this, &SFlareShipStatus::GetIconColor, EFlareSubsystem::SYS_Power)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetIcon("Propulsion"))
			.ColorAndOpacity(this, &SFlareShipStatus::GetIconColor, EFlareSubsystem::SYS_Propulsion)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetIcon("RCS"))
			.ColorAndOpacity(this, &SFlareShipStatus::GetIconColor, EFlareSubsystem::SYS_RCS)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetIcon("LifeSupport"))
			.ColorAndOpacity(this, &SFlareShipStatus::GetIconColor, EFlareSubsystem::SYS_LifeSupport)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SAssignNew(WeaponIndicator, SImage)
			.Image(FFlareStyleSet::GetIcon("Shell"))
			.ColorAndOpacity(this, &SFlareShipStatus::GetIconColor, EFlareSubsystem::SYS_Weapon)
		]
	];

	// Set visibility for the weapon indicator
	if (TargetShip && !TargetShip->IsMilitary())
	{
		WeaponIndicator->SetVisibility(CenterIcons ? EVisibility::Collapsed : EVisibility::Hidden);
	}

	if (TargetShip)
	{
		SetVisibility(Cast<AFlareSpacecraft>(TargetShip) ? EVisibility::Visible : EVisibility::Collapsed);
	}
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareShipStatus::SetTargetShip(IFlareSpacecraftInterface* Target)
{
	TargetShip = Target;

	if (TargetShip)
	{
		SetVisibility(Cast<AFlareSpacecraft>(TargetShip) ? EVisibility::Visible : EVisibility::Collapsed);

		if (TargetShip->IsMilitary())
		{
			WeaponIndicator->SetVisibility(EVisibility::Visible);
		}
		else
		{
			WeaponIndicator->SetVisibility(CenterIcons ? EVisibility::Collapsed : EVisibility::Hidden);
		}
	}
	else
	{
		SetVisibility(EVisibility::Collapsed);
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

		for (int32 Index = EFlareSubsystem::SYS_None + 1; Index <= EFlareSubsystem::SYS_Weapon; Index++)
		{
			if (Index == EFlareSubsystem::SYS_Weapon && !TargetShip->IsMilitary())
			{
				continue;
			}
			Info = FText::Format(LOCTEXT("HealthInfoFormat", "{0}{1} : {2}%\n"),
				Info,
				IFlareSpacecraftDamageSystemInterface::GetSubsystemName((EFlareSubsystem::Type)Index),
				FText::AsNumber(100 * TargetShip->GetDamageSystem()->GetSubsystemHealth((EFlareSubsystem::Type)Index, false, true))
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

FSlateColor SFlareShipStatus::GetIconColor(EFlareSubsystem::Type Type) const
{
	if (TargetShip && !TargetShip->IsStation())
	{
		float Health = TargetShip->GetDamageSystem()->GetSubsystemHealth(Type, false, true);
		return FFlareStyleSet::GetHealthColor(Health, false);
	}
	else
	{
		FLinearColor Result = FLinearColor::Black;
		Result.A = 0;
		return Result;
	}
}


#undef LOCTEXT_NAMESPACE
