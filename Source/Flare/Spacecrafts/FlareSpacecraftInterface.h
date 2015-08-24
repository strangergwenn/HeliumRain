#pragma once

#include "FlareShipPilot.h"
#include "FlareSpacecraftComponent.h"
#include "Subsystems/FlareSpacecraftDamageSystem.h"
#include "Subsystems/FlareSpacecraftNavigationSystem.h"
#include "Subsystems/FlareSpacecraftDockingSystem.h"
#include "Subsystems/FlareSpacecraftWeaponsSystem.h"
#include "FlareSpacecraftInterface.generated.h"

class UFlareCompany;
class IFlareSpacecraftDamageSystemInterface;
class IFlareSpacecraftNavigationSystemInterface;
class IFlareSpacecraftWeaponsSystemInterface;
class IFlareSpacecraftDockingSystemInterface;

/** Spacecraft save data */
USTRUCT()
struct FFlareSpacecraftSave
{
	GENERATED_USTRUCT_BODY()

	/** Ship location */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector Location;

	/** Ship rotation */
	UPROPERTY(EditAnywhere, Category = Save)
	FRotator Rotation;

	/** The current location of the ship is safe. */
	UPROPERTY(EditAnywhere, Category = Save)
	bool SafeLocation;

	/** Ship linear velocity */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector LinearVelocity;

	/** Ship angular velocity */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector AngularVelocity;

	/** Ship immatriculation. Readable for the player */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Immatriculation;

	/** Ship nickname. Readable for the player */
	UPROPERTY(EditAnywhere, Category = Save)
	FName NickName;

	/** Ship catalog identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	/** Ship company identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName CompanyIdentifier;

	/** Components list */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareSpacecraftComponentSave> Components;

	/** We are docked at this station */
	UPROPERTY(EditAnywhere, Category = Save)
	FName DockedTo;

	/** We are docked at this specific dock */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 DockedAt;

	/** Accululated heat in KJ */
	UPROPERTY(EditAnywhere, Category = Save)
	float Heat;

	/** Duration until the end of the power outage, in seconds */
	UPROPERTY(EditAnywhere, Category = Save)
	float PowerOutageDelay;

	/** Pending power outage downtime, in seconds */
	UPROPERTY(EditAnywhere, Category = Save)
	float PowerOutageAcculumator;



	/** Pilot */
	UPROPERTY(EditAnywhere, Category = Save)
	FFlareShipPilotSave Pilot;

};


/** Catalog binding between FFlareSpacecraftDescription and FFlareSpacecraftComponentDescription structure */
USTRUCT()
struct FFlareSpacecraftSlotDescription
{
	GENERATED_USTRUCT_BODY()

	/** Slot internal name */
	UPROPERTY(EditAnywhere, Category = Content) FName SlotIdentifier;

	/** Component description can be empty if configurable slot */
	UPROPERTY(EditAnywhere, Category = Content) FName ComponentIdentifier;

	/** Component slot name */
	UPROPERTY(EditAnywhere, Category = Content) FText SlotName;

	/** Size of the slot  */
	UPROPERTY(EditAnywhere, Category = Content)
	TEnumAsByte<EFlarePartSize::Type> Size;

	/** Turret angle limits. The number of value indicate indicate the angular between each limit. For exemple 4 value are for 0°, 90°, -90° and 180°? */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<float> TurretBarrelsAngleLimit;
};


/** Catalog data structure for a ship */
USTRUCT()
struct FFlareSpacecraftDescription
{
	GENERATED_USTRUCT_BODY()

	/** Ship internal name */
	UPROPERTY(EditAnywhere, Category = Content) FName Identifier;

	/** Ship name */
	UPROPERTY(EditAnywhere, Category = Content) FText Name;

	/** Ship description */
	UPROPERTY(EditAnywhere, Category = Content) FText Description;

	/** Ship description */
	UPROPERTY(EditAnywhere, Category = Content) FText ImmatriculationCode;

	/** Ship cost */
	UPROPERTY(EditAnywhere, Category = Content) int32 Cost;

	/** Size of the ship components */
	UPROPERTY(EditAnywhere, Category = Save)
	TEnumAsByte<EFlarePartSize::Type> Size;

	/** Number of RCS */
	UPROPERTY(EditAnywhere, Category = Save) int32 RCSCount;

	/** Number of orbital engine */
	UPROPERTY(EditAnywhere, Category = Save) int32 OrbitalEngineCount;

	/** Gun slots */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareSpacecraftSlotDescription> GunSlots;

	/** Turret slots */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareSpacecraftSlotDescription> TurretSlots;

	/** Internal component slots */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareSpacecraftSlotDescription> InternalComponentSlots;

	/** Max angular velocity in degree/s */
	UPROPERTY(EditAnywhere, Category = Content)	float AngularMaxVelocity;

	/** Max linear velocity in m/s */
	UPROPERTY(EditAnywhere, Category = Content)	float LinearMaxVelocity;

	/** Heat capacity un KJ/K */
	UPROPERTY(EditAnywhere, Category = Content) float HeatCapacity;

	/** Ship mesh name */
	UPROPERTY(EditAnywhere, Category = Content) UBlueprint* Template;

	/** Ship mesh preview image */
	UPROPERTY(EditAnywhere, Category = Content) FSlateBrush MeshPreviewBrush;

	/** Engine Power sound*/
	UPROPERTY(EditAnywhere, Category = Content) USoundCue* PowerSound;

};

/** Interface wrapper */
UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UFlareSpacecraftInterface  : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};


/** Actual interface */
class IFlareSpacecraftInterface
{
	GENERATED_IINTERFACE_BODY()

public:

	/*----------------------------------------------------
		Save
	----------------------------------------------------*/

	/** Load the ship from a save file */
	virtual void Load(const FFlareSpacecraftSave& Data) = 0;

	/** Save the ship to a save file */
	virtual FFlareSpacecraftSave* Save() = 0;

	/** Get the parent company */
	virtual UFlareCompany* GetCompany() = 0;

	/** Get the ship nick name */
	virtual FName GetNickName() const = 0;

	/** Get the ship description*/
	virtual FFlareSpacecraftDescription* GetDescription() const = 0;

	/** Get the ship size class */
	virtual EFlarePartSize::Type GetSize() = 0;

	virtual FName GetImmatriculation() const = 0;

	/** Check if this is a military ship */
	virtual bool IsMilitary() = 0;

	/** Check if this is a station ship */
	virtual bool IsStation() = 0;

	/*----------------------------------------------------
		Sub system
	----------------------------------------------------*/

	virtual IFlareSpacecraftDamageSystemInterface* GetDamageSystem() const = 0;

	virtual IFlareSpacecraftNavigationSystemInterface* GetNavigationSystem() const = 0;

	virtual IFlareSpacecraftDockingSystemInterface* GetDockingSystem() const = 0;

	virtual IFlareSpacecraftWeaponsSystemInterface* GetWeaponsSystem() const = 0;

	/*----------------------------------------------------
		Content
	----------------------------------------------------*/

	/** Get a Slate brush */
	static const FSlateBrush* GetIcon(FFlareSpacecraftDescription* Characteristic);

	static bool IsStation(FFlareSpacecraftDescription* SpacecraftDesc);

	static bool IsMilitary(FFlareSpacecraftDescription* SpacecraftDesc);
};
