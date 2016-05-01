#pragma once
#include "FlareSpacecraftDockingSystem.h"
#include "FlareSpacecraftNavigationSystemInterface.h"
#include "FlareSpacecraftNavigationSystem.generated.h"

class AFlareSpacecraft;




/** Spacecraft navigation system class */
UCLASS()
class HELIUMRAIN_API UFlareSpacecraftNavigationSystem : public UObject, public IFlareSpacecraftNavigationSystemInterface
{

public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void TickSystem(float DeltaSeconds);

	/** Initialize this system */
	virtual void Initialize(AFlareSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData);

	virtual void Start();

public:

	/*----------------------------------------------------
		System API
	----------------------------------------------------*/

	virtual bool IsManualPilot();

	virtual bool IsAutoPilot();

	virtual bool IsDocked();

	/** Set the new flight status */
	virtual void SetStatus(EFlareShipStatus::Type NewStatus);

	virtual void SetAngularAccelerationRate(float Acceleration);

	/*----------------------------------------------------
		Docking
	----------------------------------------------------*/

	virtual bool DockAt(IFlareSpacecraftInterface* TargetStation);

	/** Continue docking sequence has completed until effectif docking */
	virtual void DockingAutopilot(IFlareSpacecraftInterface* DockStation, int32 DockId, float DeltaSeconds);

	/** Confirm that the docking sequence has completed */
	virtual void ConfirmDock(IFlareSpacecraftInterface* DockStation, int32 DockId);

	/** Check if the colliding spacecraft is not the station we want to dock to */
	virtual void CheckCollisionDocking(AFlareSpacecraft* DockingCandidate);

	virtual bool Undock() override;

	virtual IFlareSpacecraftInterface* GetDockStation();

	/*----------------------------------------------------
		Navigation commands and helpers
	----------------------------------------------------*/

	/** Brake (linear) */
	void PushCommandLinearBrake(const FVector& VelocityTarget = FVector::ZeroVector);

	/** Brake (angular) */
	void PushCommandAngularBrake();

	/** Go there */
	void PushCommandLocation(const FVector& Location, bool Precise = false);

	/** Turn this way */
	void PushCommandRotation(const FVector& RotationTarget, const FVector& LocalShipAxis);

	/** Dock to this */
	void PushCommandDock(const FFlareDockingInfo& DockingInfo);

	/** Enqueue an autopilot command */
	void PushCommand(const FFlareShipCommandData& Command);

	/** Clear the current autopilot command */
	void ClearCurrentCommand();

	/** Get the current command */
	FFlareShipCommandData GetCurrentCommand();

	/** Abort all the current pushed autopilot commands */
	void AbortAllCommands();

	/** Get the dock offset from the origin of the ship in local space */
	virtual FVector GetDockOffset();

	/** Get the dock world location */
	virtual FVector GetDockLocation();

	/** Make sure this point is not in a path collider */
	virtual bool IsPointColliding(FVector Candidate, AActor* Ignore);

	virtual AFlareSpacecraft* GetNearestShip(AFlareSpacecraft* DockingStation) const;

	/*----------------------------------------------------
		Internal attitude control
	----------------------------------------------------*/

	/** Automatically update the current linear attitude */
	bool UpdateLinearAttitudeAuto(float DeltaSeconds, FVector TargetLocation, FVector TargetVelocity, float MaxVelocity, float SecurityRatio);

	/** Brake */
	void UpdateLinearBraking(float DeltaSeconds, FVector TargetVelocity);

	/** Automatically update the current angular attitude */
	void UpdateAngularAttitudeAuto(float DeltaSeconds);

	/** Brake */
	void UpdateAngularBraking(float DeltaSeconds);

	/** Return the angular velocity need to align the local ship axis to the target axis */
	virtual FVector GetAngularVelocityToAlignAxis(FVector LocalShipAxis, FVector TargetAxis, FVector TargetAngularVelocity, float DeltaSeconds) const;


	/*----------------------------------------------------
		Physics
	----------------------------------------------------*/

	/** Update the attitude control */
	void PhysicSubTick(float DeltaSeconds);

	/** Update the ship's center of mass */
	void UpdateCOM();

protected:


	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UPROPERTY()
	AFlareSpacecraft*                               Spacecraft;
	FFlareSpacecraftSave*                           Data;
	FFlareSpacecraftDescription*                    Description;
	TArray<UActorComponent*>                        Components;

	TEnumAsByte <EFlareShipStatus::Type>     Status;

	// Configuration properties
	float                                    AngularDeadAngle;
	float                                    AngularMaxVelocity; // degree/s
	float                                    AngularAccelerationRate;
	float                                    LinearDeadDistance;
	float                                    LinearMaxVelocity; // m/s
	float                                    LinearMaxDockingVelocity; // m/s
	float                                    NegligibleSpeedRatio;

	// Navigation
	TArray <AActor*>                         PathColliders;
	TQueue <FFlareShipCommandData>           CommandData;
	float                                    AnticollisionAngle;

	// Physics simulation
	FVector                                  LinearTargetVelocity;
	FVector                                  AngularTargetVelocity;
	bool                                     UseOrbitalBoost;
	FVector                                  COM;


public:

	/*----------------------------------------------------
		Getters (Attitude)
	----------------------------------------------------*/

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

	inline float GetAngularAccelerationRate() const
	{
		return AngularAccelerationRate;
	}

	inline float GetAngularMaxVelocity() const
	{
		return AngularMaxVelocity;
	}

	inline float GetLinearMaxVelocity() const
	{
		return LinearMaxVelocity;
	}

	inline EFlareShipStatus::Type GetStatus() const
	{
		return Status;
	}
};
