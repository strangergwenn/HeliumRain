
#include "../../Flare.h"
#include "FlareShipStatus.h"

#define LOCTEXT_NAMESPACE "FlareShipStatus"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareShipStatus::Construct(const FArguments& InArgs)
{
	TargetShip = InArgs._Ship;
	FLinearColor Color = FFlareStyleSet::GetHeatColor();

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
			.ColorAndOpacity(Color)
			.Visibility(this, &SFlareShipStatus::IsVisible, EFlareSubsystem::SYS_Temperature)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetIcon("Power"))
			.ColorAndOpacity(Color)
			.Visibility(this, &SFlareShipStatus::IsVisible, EFlareSubsystem::SYS_Power)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetIcon("Propulsion"))
			.ColorAndOpacity(Color)
			.Visibility(this, &SFlareShipStatus::IsVisible, EFlareSubsystem::SYS_Propulsion)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetIcon("RCS"))
			.ColorAndOpacity(Color)
			.Visibility(this, &SFlareShipStatus::IsVisible, EFlareSubsystem::SYS_RCS)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetIcon("Shell"))
			.ColorAndOpacity(Color)
			.Visibility(this, &SFlareShipStatus::IsVisible, EFlareSubsystem::SYS_Weapon)
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareShipStatus::SetTargetShip(IFlareShipInterface* Target)
{
	TargetShip = Target;
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

EVisibility SFlareShipStatus::IsVisible(EFlareSubsystem::Type Type) const
{
	if (TargetShip)
	{
		return ((TargetShip->GetSubsystemHealth(Type) <= 0.5f) ? EVisibility::Visible : EVisibility::Collapsed);
	}
	{
		return EVisibility::Collapsed;
	}
}


#undef LOCTEXT_NAMESPACE
