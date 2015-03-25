
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
				SAssignNew(HullStatus, SFlareSubsystemStatus)
				.Subsystem(EFlareSubsystem::SYS_Temperature)
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
		+SHorizontalBox::Slot()
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
	HullStatus->SetTargetShip(Target);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/


#undef LOCTEXT_NAMESPACE
