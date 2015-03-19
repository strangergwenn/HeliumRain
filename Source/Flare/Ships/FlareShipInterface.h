#pragma once

#include "../Game/FlareCompany.h"
#include "FlareShipInterface.generated.h"

class IFlareStationInterface;
struct FFlareShipModuleSave;

/** Game save data */

/** Ship save data */
USTRUCT()
struct FFlareShipSave
{
	GENERATED_USTRUCT_BODY()

	/** Ship location */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector Location;

	/** Ship rotation */
	UPROPERTY(EditAnywhere, Category = Save)
	FRotator Rotation;
	
	/** Ship linear velocity */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector LinearVelocity;

	/** Ship angular velocity */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector AngularVelocity;

	/** Ship name */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Name;

	/** Ship catalog identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	/** Ship company identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName CompanyIdentifier;


	/** Orbital engine catalog identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareShipModuleSave> Modules;

	/** We are docked at this station */
	UPROPERTY(EditAnywhere, Category = Save)
	FName DockedTo;

	/** We are docked at this specific dock */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 DockedAt;

};

/** Catalog binding between FFlareShipDescription and FFlareShipModuleDescription structure */
USTRUCT()
struct FFlareShipSlotDescription
{
	GENERATED_USTRUCT_BODY()
	
	/** Slot internal name */
	UPROPERTY(EditAnywhere, Category = Content) FName Identifier;
	
	/** Module description can be empty if configurable slot */
	UPROPERTY(EditAnywhere, Category = Content) FFlareShipModuleDescription Module;
};

/** Catalog data structure for a ship */
USTRUCT()
struct FFlareShipDescription
{
	GENERATED_USTRUCT_BODY()

	/** Ship internal name */
	UPROPERTY(EditAnywhere, Category = Content) FName Identifier;

	/** Ship name */
	UPROPERTY(EditAnywhere, Category = Content) FText Name;

	/** Ship description */
	UPROPERTY(EditAnywhere, Category = Content) FText Description;

	/** Ship cost */
	UPROPERTY(EditAnywhere, Category = Content) int32 Cost;

	/** Size of the ship components */
	UPROPERTY(EditAnywhere, Category = Save)
	TEnumAsByte<EFlarePartSize::Type> Size;

	/** Ship status */
	UPROPERTY(EditAnywhere, Category = Content) bool Military;

	/** Number of RCS */
	UPROPERTY(EditAnywhere, Category = Save) int32 RCSCount;

	/** Number of orbital engine */
	UPROPERTY(EditAnywhere, Category = Save) int32 OrbitalEngineCount;

	/** Gun slots */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareShipSlotDescription> GunSlots;

	/** Turret slots */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareShipSlotDescription> TurretSlots;

	/** Internal module slots */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareShipSlotDescription> InternalModuleSlots;

	/** Max angular velocity in degree/s */
	UPROPERTY(EditAnywhere, Category = Content)	float AngularMaxVelocity;

	/** Max linear velocity in m/s */
	UPROPERTY(EditAnywhere, Category = Content)	float LinearMaxVelocity;

	/** Ship mesh name */
	UPROPERTY(EditAnywhere, Category = Content) UBlueprint* Template;

	/** Ship mesh preview image */
	UPROPERTY(EditAnywhere, Category = Content) FSlateBrush MeshPreviewBrush;

};


/** Interface wrapper */
UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UFlareShipInterface  : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};


/** Actual interface */
class IFlareShipInterface
{
	GENERATED_IINTERFACE_BODY()

public:

	/*----------------------------------------------------
		Save
	----------------------------------------------------*/

	/** Load the ship from a save file */
	virtual void Load(const FFlareShipSave& Data) = 0;

	/** Save the ship to a save file */
	virtual FFlareShipSave* Save() = 0;

	/** Set the parent company */
	virtual void SetOwnerCompany(UFlareCompany* Company) = 0;

	/** Get the parent company */
	virtual UFlareCompany* GetCompany() = 0;


	/*----------------------------------------------------
		Navigation API
	----------------------------------------------------*/

	/** Navigate there (scene coordinates) */
	virtual bool NavigateTo(FVector TargetLocation) = 0;

	/** Check if the ship is manually flown */
	virtual bool IsManualPilot() = 0;

	/** Check if the gliding mode is activated */
	virtual bool IsGliding() = 0;

	/** Check if the autopilot is enabled */
	virtual bool IsAutoPilot() = 0;

	/** Check if the ship is currently docked */
	virtual bool IsDocked() = 0;

	/** Navigate to and dock at this station */
	virtual bool DockAt(IFlareStationInterface* TargetStation) = 0;

	/** Undock from the current station */
	virtual bool Undock() = 0;

	/** Get the station where we are docked to */
	virtual IFlareStationInterface* GetDockStation() = 0;


	/*----------------------------------------------------
		Content
	----------------------------------------------------*/

	/** Get a Slate brush */
	static const FSlateBrush* GetIcon(FFlareShipDescription* Characteristic);

};
