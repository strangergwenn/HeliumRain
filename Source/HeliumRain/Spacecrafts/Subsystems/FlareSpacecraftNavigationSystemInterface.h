#pragma once

#include "FlareSpacecraftNavigationSystemInterface.generated.h"

/** Status of the ship */
UENUM()
namespace EFlareShipStatus
{
	enum Type
	{
		SS_Manual,
		SS_AutoPilot,
		SS_Docked
	};
}
namespace EFlareShipStatus
{
	inline FString ToString(EFlareShipStatus::Type EnumValue)
	{
		const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EFlareShipStatus"), true);
		return EnumPtr->GetEnumName(EnumValue);
	}
}


/** Type of command */
UENUM()
namespace EFlareCommandDataType
{
	enum Type
	{
		CDT_None,
		CDT_Location,
		CDT_Rotation,
		CDT_BrakeLocation,
		CDT_BrakeRotation,
		CDT_Dock
	};
}
namespace EFlareCommandDataType
{
	inline FString ToString(EFlareCommandDataType::Type EnumValue)
	{
		const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EFlareCommandDataType"), true);
		return EnumPtr->GetEnumName(EnumValue);
	}
}


/** Structure holding all data for a single command */
struct FFlareShipCommandData
{
	TEnumAsByte <EFlareCommandDataType::Type> Type;

	bool PreciseApproach;
	FVector LocationTarget;
	FVector VelocityTarget;
	FVector RotationTarget;
	FVector LocalShipAxis;

	UPROPERTY()
	AActor* ActionTarget;

	int32 ActionTargetParam;

};

/** Interface wrapper */
UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UFlareSpacecraftNavigationSystemInterface  : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};


/** Actual interface */
class IFlareSpacecraftNavigationSystemInterface
{
	GENERATED_IINTERFACE_BODY()

public:

		/*----------------------------------------------------
			System Interface
		----------------------------------------------------*/

		virtual bool IsAutoPilot() = 0;

		virtual bool IsDocked() = 0;

		virtual bool Undock() = 0;

		/** Get the current command */
		virtual FFlareShipCommandData GetCurrentCommand() = 0;

};
