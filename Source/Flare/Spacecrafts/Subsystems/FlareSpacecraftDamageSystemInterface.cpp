#include "../../Flare.h"
#include "FlareSpacecraftDamageSystemInterface.h"

#define LOCTEXT_NAMESPACE "FlareSpacecraftDamageSystemInterface"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftDamageSystemInterface::UFlareSpacecraftDamageSystemInterface(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}


/*----------------------------------------------------
	Tools
----------------------------------------------------*/

FText IFlareSpacecraftDamageSystemInterface::GetSubsystemName(EFlareSubsystem::Type SubsystemType) const
{
	FText Text;

	switch (SubsystemType)
	{
	case EFlareSubsystem::SYS_Temperature:   Text = LOCTEXT("SYS_Temperature", "Cooling");      break;
	case EFlareSubsystem::SYS_Propulsion:    Text = LOCTEXT("SYS_Propulsion", "Engines");       break;
	case EFlareSubsystem::SYS_RCS:           Text = LOCTEXT("SYS_RCS", "RCS");                  break;
	case EFlareSubsystem::SYS_LifeSupport:   Text = LOCTEXT("SYS_LifeSupport", "Crew");         break;
	case EFlareSubsystem::SYS_Power:         Text = LOCTEXT("SYS_Power", "Power");              break;
	case EFlareSubsystem::SYS_Weapon:        Text = LOCTEXT("SYS_Weapon", "Weapons");           break;
	}

	return Text;
}

#undef LOCTEXT_NAMESPACE