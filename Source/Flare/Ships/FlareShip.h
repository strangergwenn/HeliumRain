#pragma once

#include "FlareShipBase.h"
#include "FlareWeapon.h"
#include "../Stations/FlareStation.h"
#include "FlareShipInterface.h"
#include "FlareShip.generated.h"


/** Status of the ship */
UENUM()
namespace EFlareShipStatus
{
	enum Type
	{
		SS_Manual,
		SS_Gliding,
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
	FVector RotationTarget;
	FVector LocalShipAxis;

	UPROPERTY()
	AActor* ActionTarget;

	int32 ActionTargetParam;

};


/** Ship class */
UCLASS(Blueprintable, ClassGroup = (Flare, Ship))
class AFlareShip : public AFlareShipBase, public IFlareShipInterface
{

public:

	GENERATED_UCLASS_BODY()

	/*----------------------------------------------------
		Component data
	----------------------------------------------------*/

	/** Airframe component */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly)
	class UFlareAirframe* Airframe;


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void ReceiveHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	virtual void Destroyed() override;


	/*----------------------------------------------------
		Player interface
	----------------------------------------------------*/
	
	/** Activate or deactivate the exterbal camera */
	virtual void SetExternalCamera(bool NewState);


	/*----------------------------------------------------
		Ship interface
	----------------------------------------------------*/

	virtual void Load(const FFlareShipSave& Data) override;

	virtual FFlareShipSave* Save() override;

	virtual void SetOwnerCompany(UFlareCompany* Company) override;

	virtual UFlareCompany* GetCompany() override;

	virtual float GetSubsystemHealth(EFlareSubsystem::Type Type) override;

	virtual float GetTemperature() override;

	virtual float GetMaxTemperature() override;

	bool NavigateTo(FVector TargetLocation) override;

	virtual bool IsManualPilot() override;

	virtual bool IsGliding() override;

	virtual bool IsAutoPilot() override;

	virtual bool IsDocked() override;

	/*----------------------------------------------------
		Docking
	----------------------------------------------------*/

	virtual bool DockAt(IFlareStationInterface* TargetStation) override;

	/** Confirm that the docking sequence has completed */
	virtual void ConfirmDock(IFlareStationInterface* DockStation, int32 DockId);

	virtual bool Undock() override;

	virtual IFlareStationInterface* GetDockStation() override;


	/*----------------------------------------------------
		Navigation commands and helpers
	----------------------------------------------------*/

	/** Brake (linear) */
	void PushCommandLinearBrake();

	/** Brake (angular) */
	void PushCommandAngularBrake();

	/** Go there */
	void PushCommandLocation(const FVector& Location);

	/** Turn this way */
	void PushCommandRotation(const FVector& RotationTarget, const FVector& LocalShipAxis);

	/** Dock to this */
	void PushCommandDock(const FFlareDockingInfo& DockingInfo);

	/** Enqueue an autopilot command */
	void PushCommand(const FFlareShipCommandData& Command);

	/** Clear the current autopilot command */
	void ClearCurrentCommand();

	/** Get the dock offset from the origin of the ship */
	virtual FVector GetDockLocation();
	
	/** Compute the path from OriginLocation to TargetLocation */
	virtual bool ComputePath(TArray<FVector>& Commands, TArray<AActor*>& Colliders, FVector OriginLocation, FVector TargetLocation, float ShipSize);

	/** Update collision data for pathfinding */
	virtual void UpdateColliders();

	/** Make sure this point is not in a path collider */
	virtual bool IsPointColliding(FVector Candidate, AActor* Ignore);

	/*----------------------------------------------------
		Damage status
	----------------------------------------------------*/

	/**
	 * Apply damage to this ship.
	 * Location is the center of sphere where damages are applied.
	 */
	virtual void ApplyDamage(float Energy, float Radius, FVector Location) override;

	/**
	 * Return true if any lifesupport system is alive
	 */
	virtual bool IsAlive() override;

	/**
	 * Return true if the ship cockpit is powered
	 */
	virtual bool IsPowered() override;

	/**
	 * Return true if the ship is currently on power outage
	 */
	virtual bool HasPowerOutage() override;

	/**
	 * If on power outage, time until the end of the power outage. Else 0.
	 */
	virtual float GetPowerOutageDuration() override;

	/**
	 * Method call if a electric component had been damaged
	 */
	virtual void OnElectricDamage(float DamageRatio);

protected:

	/*----------------------------------------------------
		Internal attitude control
	----------------------------------------------------*/

	/** Manually update the current linear attitude */
	void UpdateLinearAttitudeManual(float DeltaSeconds);

	/** Automatically update the current linear attitude */
	void UpdateLinearAttitudeAuto(float DeltaSeconds);

	/** Brake */
	void UpdateLinearBraking(float DeltaSeconds);

	/** Manually update the current angular attitude */
	void UpdateAngularAttitudeManual(float DeltaSeconds);

	/** Automatically update the current angular attitude */
	void UpdateAngularAttitudeAuto(float DeltaSeconds);

	/** Brake */
	void UpdateAngularBraking(float DeltaSeconds);

	/*----------------------------------------------------
		Physics
	----------------------------------------------------*/

	/** Update the attitude control */
	void PhysicSubTick(float DeltaSeconds);
	
	/** Update the ship's center of mass */
	void UpdateCOM();

public:

	/*----------------------------------------------------
		Customization
	----------------------------------------------------*/

	/** Set he ship description to load data from */
	virtual void SetShipDescription(FFlareShipDescription* Description);

	/** Set the description to use for all orbital engines */
	virtual void SetOrbitalEngineDescription(FFlareShipComponentDescription* Description);

	/** Set the description to use for all RCS */
	virtual void SetRCSDescription(FFlareShipComponentDescription* Description);

	virtual void UpdateCustomization() override;

	virtual void StartPresentation() override;


public:

	/*----------------------------------------------------
		Input methods
	----------------------------------------------------*/

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	virtual void MousePositionInput(FVector2D Val);

	virtual void ThrustInput(float Val);

	virtual void MoveVerticalInput(float Val);

	virtual void MoveHorizontalInput(float Val);

	virtual void RollInput(float Val);

	virtual void PitchInput(float Val);

	virtual void YawInput(float Val);

	virtual void ZoomIn();

	virtual void ZoomOut();

	virtual void FaceForward();

	virtual void FaceBackward();

	virtual void BoostOn();

	virtual void BoostOff();

	virtual void ToggleGliding();

	virtual void StartFire();

	virtual void StopFire();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Component description
	FFlareShipSave                       ShipData;
	FFlareShipDescription*               ShipDescription;
	FFlareShipComponentDescription*      OrbitalEngineDescription;
	FFlareShipComponentDescription*      RCSDescription;

	// Weapons
	TArray <UFlareWeapon*>                   WeaponList;
	TArray <FFlareShipComponentDescription*> WeaponDescriptionList;

	// Configuration properties
	float                                AngularDeadAngle;
	float                                AngularInputDeadRatio;
	float                                AngularMaxVelocity; // degree/s
	float                                AngularAccelerationRate;
	float                                LinearDeadDistance;
	float                                LinearMaxVelocity; // m/s
	float                                LinearMaxDockingVelocity; // m/s
	float                                NegligibleSpeedRatio;

	// Dynamic gameplay data
	TEnumAsByte <EFlareShipStatus::Type> Status;
	bool                                 ExternalCamera;

	// Navigation
	TArray <AActor*>                     PathColliders;
	TQueue <FFlareShipCommandData>       CommandData;

	// Manual pilot
	FVector                              ManualAngularVelocity; // In local space
	FVector                              ManualLinearVelocity;
	bool                                 ManualOrbitalBoost;

	// Physics simulation
	FVector                              LinearTargetVelocity;
	FVector                              AngularTargetVelocity;
	
	// Temporary variable reset each tick
	FVector                              COM;

	// Damage status
	UFlareShipComponent*                 ShipCockit;

public:

	/*----------------------------------------------------
		Getters (Attitude)
	----------------------------------------------------*/

	inline float GetAttitudeCommandOrbitalThrust() const
	{
		return ManualOrbitalBoost;
	}

	/**
	 * Return linear velocity in meters
	 */
	FVector GetLinearVelocity() const;

	/**
	 * Return the maximum current (with damages) trust the ship can provide in a specific axis.
	 * Engines : List of engine to use to compute the max thrust
	 * Axis : Axis of the thurst
	 * WithObitalEngines : if false, ignore orbitals engines
	 */
	FVector GetTotalMaxThrustInAxis(TArray<UActorComponent*>& Engines, FVector Axis, bool WithOrbitalEngines) const;

	/**
	 * Return the maximum torque the ship can provide in a specific axis.
	 * Engines : List of engine to use to compute the max thrust
	 * TorqueDirection : Axis of the torque
	 * WithDamages : if true, use current thrust value and not theorical thrust value
	 */
	float GetTotalMaxTorqueInAxis(TArray<UActorComponent*>& Engines, FVector TorqueDirection, bool WithDamages) const;


	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/
	
	inline FFlareShipDescription* GetDescription() const
	{
		return ShipDescription;
	}

	inline FFlareShipComponentDescription* GetOrbitalEngineDescription() const
	{
		return OrbitalEngineDescription;
	}

	inline FFlareShipComponentDescription* GetRCSDescription() const
	{
		return RCSDescription;
	}

	inline FFlareShipComponentDescription* GetWeaponDescription(int32 Index) const
	{
		return WeaponDescriptionList[Index];
	}

	inline TArray<UFlareWeapon*>& GetWeaponList()
	{
		return WeaponList;
	}

	virtual UFlareShipComponent* GetCockpit() const override { return ShipCockit; }

};
