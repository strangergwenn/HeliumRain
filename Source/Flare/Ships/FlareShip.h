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
		Physics
	----------------------------------------------------*/
	void AddForceAtLocation(FVector LinearForce, FVector AngularForce, FVector ApplicationPoint);

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

	/** Configure all engine from commands*/
	void LowLevelAutoPilotSubTick(float DeltaSeconds);

	void PhysicSubTick(float DeltaSeconds);
	
	/** Linear velocity in meters */
	FVector GetLinearVelocity() const;
	
	FVector GetTotalMaxThrustInAxis(TArray<UActorComponent*>& Engines, FVector Axis, float ThurstAngleLimit, bool WithOrbitalEngines) const;
	
	FVector GetTotalMaxTorqueInAxis(TArray<UActorComponent*>& Engines, FVector TorqueDirection, FVector COM, float ThurstAngleLimit, bool WithDamages, bool WithOrbitalEngines) const;

	void UpdateCOM();
	
	/*----------------------------------------------------
		Autopilot
	----------------------------------------------------*/
	
	float* ComputeLinearVelocityStabilisation(float DeltaSeconds, TArray<UActorComponent*>& Engines, FVector WorldTargetSpeed, float ThrustAngleLimit) const;
	
	float* ComputeAngularVelocityStabilisation(float DeltaSeconds, TArray<UActorComponent*>& Engines, FVector LocalTargetSpeed) const;

public:

	/*----------------------------------------------------
		Customization
	----------------------------------------------------*/

	/** Set he ship description to load data from */
	virtual void SetShipDescription(FFlareShipDescription* Description);

	/** Set the description to use for all orbital engines */
	virtual void SetOrbitalEngineDescription(FFlareShipModuleDescription* Description);

	/** Set the description to use for all RCS */
	virtual void SetRCSDescription(FFlareShipModuleDescription* Description);

	/** Set the description for a specific weapon slot */
	virtual void SetWeaponDescription(int32 Index, FFlareShipModuleDescription* Description);

	/** Setup for menu display */
	virtual void StartPresentation();

	virtual void UpdateCustomization() override;


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
	FFlareShipSave                ShipData;
	FFlareShipDescription*        ShipDescription;
	FFlareShipModuleDescription*  OrbitalEngineDescription;
	FFlareShipModuleDescription*  RCSDescription;

	// Weapons
	TArray <UFlareWeapon*>                 WeaponList;
	TArray <FFlareShipModuleDescription*>  WeaponDescriptionList;

	// Configuration properties
	float                         AngularDeadAngle;
	float                         AngularInputDeadRatio;
	float                         AngularMaxVelocity; // degree/s
	float                         AngularAccelerationRate;
	float                         LinearDeadDistance;
	float                         LinearMaxVelocity; // m/s
	float                         LinearThrust;
	float                         NegligibleSpeedRatio;

	// Dynamic gameplay data
	TEnumAsByte <EFlareShipStatus::Type> Status;
	bool                          FakeThrust;
	bool                          CanMoveVertical;
	bool                          ExternalCamera;

	// Navigation
	TArray <AActor*>              PathColliders;
	TQueue <FFlareShipCommandData> CommandData;

	// Manual pilot
	FVector                       ManualAngularVelocity; // In local space
	FVector                       ManualLinearVelocity;
	bool                          ManualOrbitalBoost;

	// Physics simulation
	FVector                       LinearTargetVelocity;
	FVector                       LinearThrustDirection;
	FVector                       LinearVelocity;
	float                         LinearStopDistance;

	// Physics simulation
	FVector                       AngularTargetVelocity;
	//FQuat                         AngularVelocity;
	FQuat                         AngularVelocityDelta;
	//float                         AngularStopDistance;
	FVector                       LocalInertiaTensor;

	// Temporary variable reset each tich
	FVector TickSumForce;
	FVector TickSumTorque;
	FVector COM;

public:

	/*----------------------------------------------------
		Getters (Attitude)
	----------------------------------------------------*/

	inline float GetAttitudeCommandOrbitalThrust() const
	{
		return ManualOrbitalBoost;
	}

	inline float GetAttitudeCommandThrust() const
	{
		return LinearThrustDirection.X;
	}

	inline float GetAttitudeCommandHorizontal() const
	{
		return LinearThrustDirection.Y;
	}

	inline float GetAttitudeCommandVertical() const
	{
		return LinearThrustDirection.Z;
	}

	inline float GetAttitudeCommandPitch() const
	{
		return AngularVelocityDelta.Rotator().Pitch / AngularAccelerationRate;
	}

	inline float GetAttitudeCommandYaw() const
	{
		return AngularVelocityDelta.Rotator().Yaw / AngularAccelerationRate;
	}

	inline float GetAttitudeCommandRoll() const
	{
		return AngularVelocityDelta.Rotator().Roll / AngularAccelerationRate;
	}


	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/
	
	inline FFlareShipDescription* GetDescription() const
	{
		return ShipDescription;
	}

	inline FFlareShipModuleDescription* GetOrbitalEngineDescription() const
	{
		return OrbitalEngineDescription;
	}

	inline FFlareShipModuleDescription* GetRCSDescription() const
	{
		return RCSDescription;
	}

	inline FFlareShipModuleDescription* GetWeaponDescription(int32 Index) const
	{
		return WeaponDescriptionList[Index];
	}

	inline TArray<UFlareWeapon*>& GetWeaponList()
	{
		return WeaponList;
	}

	inline bool IsFakeThrust() const
	{
		return FakeThrust;
	}

};
