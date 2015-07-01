
#include "../../Flare.h"
#include "FlareShipStatus.h"

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
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareShipStatus::SetTargetShip(IFlareSpacecraftInterface* Target)
{
	TargetShip = Target;

	if (TargetShip && TargetShip->IsMilitary())
	{
		WeaponIndicator->SetVisibility(EVisibility::Visible);
	}
	else
	{
		WeaponIndicator->SetVisibility(CenterIcons ? EVisibility::Collapsed : EVisibility::Hidden);
	}
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FSlateColor SFlareShipStatus::GetIconColor(EFlareSubsystem::Type Type) const
{
	if (TargetShip)
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
