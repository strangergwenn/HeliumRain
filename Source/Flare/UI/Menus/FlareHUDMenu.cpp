
#include "../../Flare.h"
#include "FlareHUDMenu.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareHUDMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareHUDMenu::Construct(const FArguments& InArgs)
{
	// Data
	OwnerHUD = InArgs._OwnerHUD;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	
	// Sructure
	ChildSlot
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Bottom)
	.Padding(FMargin(20))
	[
		SNew(SHorizontalBox)

		// Static container
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(TemperatureStatus, SFlareSubsystemStatus)
				.Subsystem(EFlareSubsystem::SYS_Temperature)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(PowerStatus, SFlareSubsystemStatus)
				.Subsystem(EFlareSubsystem::SYS_Power)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(PropulsionStatus, SFlareSubsystemStatus)
				.Subsystem(EFlareSubsystem::SYS_Propulsion)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(RCSStatus, SFlareSubsystemStatus)
				.Subsystem(EFlareSubsystem::SYS_RCS)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(LifeSupportStatus, SFlareSubsystemStatus)
				.Subsystem(EFlareSubsystem::SYS_LifeSupport)
			]
		]

		// Weapon container
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SAssignNew(WeaponContainer, SHorizontalBox)
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareHUDMenu::SetTargetShip(AFlareShip* Target)
{
	// Set targets
	TemperatureStatus->SetTargetShip(Target);
	PowerStatus->SetTargetShip(Target);
	PropulsionStatus->SetTargetShip(Target);
	RCSStatus->SetTargetShip(Target);
	LifeSupportStatus->SetTargetShip(Target);

	// Cleanup old weapon indicators
	TSharedPtr<SFlareSubsystemStatus> Temp;
	WeaponContainer->ClearChildren();

	// Add weapon indicators
	for (int32 i = 0; i < 2; i++)
	{
		WeaponContainer->AddSlot()
			.AutoWidth()
			[
				SAssignNew(Temp, SFlareSubsystemStatus)
				.Subsystem(EFlareSubsystem::SYS_Weapon)
			];
		Temp->SetTargetShip(Target);
	}
	WeaponContainer->SetVisibility(EVisibility::Visible);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/


#undef LOCTEXT_NAMESPACE
