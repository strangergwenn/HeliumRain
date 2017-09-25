#pragma once

#include "FlareSpacecraftPawn.h"
#include "FlareWeapon.h"
#include "FlareSpacecraftComponent.h"
#include "FlareSpacecraftSpinningComponent.h"
#include "Subsystems/FlareSpacecraftDamageSystem.h"
#include "Subsystems/FlareSpacecraftNavigationSystem.h"
#include "Subsystems/FlareSpacecraftDockingSystem.h"
#include "Subsystems/FlareSpacecraftWeaponsSystem.h"
#include "FlareSpacecraftStateManager.h"
#include "FlareSpacecraft.generated.h"

class UFlareShipPilot;
class AFlareSpacecraft;

class UCanvasRenderTarget2D;


/** Target info */
USTRUCT()
struct FFlareScreenTarget
{
	GENERATED_USTRUCT_BODY()

	AFlareSpacecraft*      Spacecraft;

	float                  DistanceFromScreenCenter;

};


/** Ship class */
UCLASS(Blueprintable, ClassGroup = (Flare, Ship))
class AFlareSpacecraft : public AFlareSpacecraftPawn
{

public:

	GENERATED_UCLASS_BODY()

	/*----------------------------------------------------
		Component data
	----------------------------------------------------*/

	/** Airframe component */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly)
	class UFlareSpacecraftComponent* Airframe;


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	virtual void Destroyed() override;

	virtual void OnRepaired();

	virtual void OnRefilled();

	virtual void OnDocked(AFlareSpacecraft* DockStation, bool TellUser);

	virtual void OnUndocked(AFlareSpacecraft* DockStation);

	virtual void SetPause(bool Pause);

	virtual void Redock();

	virtual float GetSpacecraftMass() const;


	/*----------------------------------------------------
		Player interface
	----------------------------------------------------*/

	/** Extrapolate the position of a ship for a given targeting ship. Return time before intersect. If time is negative, no intersection. */
	virtual float GetAimPosition(AFlareSpacecraft* TargettingShip, float BulletSpeed, float PredictionDelay, FVector* ResultPosition) const;

	/** Reset the target */
	void ResetCurrentTarget();

	/** Get the current target */
	AFlareSpacecraft* GetCurrentTarget() const;

	/** Are we scanning for a waypoint ? */
	bool IsInScanningMode();

	/** Get the progress of scanning */
	void GetScanningProgress(bool& AngularIsActive, bool& LinearIsActive, bool& ScanningIsActive,
		float& AngularProgress, float& LinearProgress, float& AnalyzisProgress, float& ScanningDistance, float& ScanningSpeed);

	/** Are we done with the scan ? */
	bool IsScanningFinished() const;

	/** Are we currently docking ? */
	bool GetManualDockingProgress(AFlareSpacecraft*& OutStation, FFlareDockingParameters& OutParameters, FText& OutInfo) const;


	/*----------------------------------------------------
		Ship interface
	----------------------------------------------------*/

	virtual void Load(UFlareSimulatedSpacecraft* Parent);

	virtual void Save();

	virtual void SetOwnerCompany(UFlareCompany* Company);
	
	virtual UFlareInternalComponent* GetInternalComponentAtLocation(FVector Location) const;
	
	virtual UFlareSpacecraftDamageSystem* GetDamageSystem() const;

	virtual UFlareSpacecraftNavigationSystem* GetNavigationSystem() const;
	
	virtual UFlareSpacecraftDockingSystem* GetDockingSystem() const;

	virtual UFlareSpacecraftWeaponsSystem* GetWeaponsSystem() const;

	/** Set asteroid data from an asteroid save */
	void SetAsteroidData(FFlareAsteroidSave* Data);

	/** Try to attach to the parent world object */
	void TryAttachParentActor();

	/** Try to attach to the parent complex station */
	void TryAttachParentComplex();

	/** Apply the current asteroid data */
	void ApplyAsteroidData();

	void UpdateDynamicComponents();
	
	UFlareSimulatedSector* GetOwnerSector();
	
	void SetCurrentTarget(AFlareSpacecraft* Target);

public:

	/*----------------------------------------------------
		Customization
	----------------------------------------------------*/

	/** Set the description to use for all orbital engines */
	virtual void SetOrbitalEngineDescription(FFlareSpacecraftComponentDescription* Description);

	/** Set the description to use for all RCS */
	virtual void SetRCSDescription(FFlareSpacecraftComponentDescription* Description);

	virtual void UpdateCustomization() override;

	virtual void StartPresentation() override;

	/** Canvas callback for the ship name */
	UFUNCTION()
	void DrawShipName(UCanvas* TargetCanvas, int32 Width, int32 Height);
	

public:

	/*----------------------------------------------------
		Input methods
	----------------------------------------------------*/

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	virtual void StartFire();

	virtual void StopFire();

	virtual void LeftMousePress();

	virtual void LeftMouseRelease();

	virtual void DeactivateWeapon();

	virtual void ActivateWeaponGroup1();

	virtual void ActivateWeaponGroup2();

	virtual void ActivateWeaponGroup3();

	virtual void ActivateWeaponGroupByIndex(int32 Index);

	virtual void NextWeapon();

	virtual void PreviousWeapon();

	virtual void NextTarget();

	virtual void PreviousTarget();

	virtual void AlternateNextTarget();

	virtual void AlternatePreviousTarget();


	virtual void YawInput(float Val);

	virtual void PitchInput(float Val);

	virtual void RollInput(float Val);

	virtual void ThrustInput(float Val);

	virtual void MoveVerticalInput(float Val);

	virtual void MoveHorizontalInput(float Val);



	virtual void GamepadMoveVerticalInput(float Val);

	virtual void GamepadMoveHorizontalInput(float Val);

	virtual void GamepadThrustInput(float Val);

	virtual void GamepadYawInput(float Val);

	virtual void GamepadPitchInput(float Val);


	virtual void JoystickYawInput(float Val);

	virtual void JoystickPitchInput(float Val);

	virtual void JoystickRollInput(float Val);

	virtual void JoystickThrustInput(float Val);

	virtual void JoystickMoveVerticalInput(float Val);

	virtual void JoystickMoveHorizontalInput(float Val);


	virtual void ZoomIn();

	virtual void ZoomOut();

	virtual void CombatZoomIn();

	virtual void CombatZoomOut();

	virtual void FaceForward();

	virtual void FaceBackward();

	virtual void Brake();

	virtual void BrakeToVelocity(const FVector& VelocityTarget = FVector::ZeroVector);

	virtual void LockDirectionOn();

	virtual void LockDirectionOff();

	virtual void FindTarget();

protected:

	/*----------------------------------------------------
		Internal data
	----------------------------------------------------*/

	// Component descriptions, save data
	UFlareSimulatedSpacecraft*	                   Parent;
	FFlareSpacecraftComponentDescription*          OrbitalEngineDescription;
	FFlareSpacecraftComponentDescription*          RCSDescription;
	FVector                                        SmoothedVelocity;
	
	// Idle shipyard dynamic component
	UPROPERTY()
	UClass*                                        IdleShipyardTemplate;

	// Weapon loaded
	UPROPERTY()
	USoundCue*                                     WeaponLoadedSound;

	// Weapon loaded
	UPROPERTY()
	USoundCue*                                     WeaponUnloadedSound;

	// Lifesupport status
	UPROPERTY()
	UFlareSpacecraftComponent*                     ShipCockit;

	// Pilot object
	UPROPERTY()
	UFlareShipPilot*                               Pilot;

	// Decal material
	UPROPERTY()
	UMaterialInstanceDynamic*                      DecalMaterial;

	// Decal material
	UPROPERTY()
	UMaterialInstanceDynamic*                      ShipNameDecalMaterial;

	// Ship name texture
	UPROPERTY()
	UCanvasRenderTarget2D*                         ShipNameTexture;

	// Ship name font
	UPROPERTY()
	UFont*                                         ShipNameFont;

	// Systems
	UPROPERTY()
	UFlareSpacecraftDamageSystem*                  DamageSystem;
	UPROPERTY()
	UFlareSpacecraftNavigationSystem*              NavigationSystem;
	UPROPERTY()
	UFlareSpacecraftDockingSystem*                 DockingSystem;
	UPROPERTY()
	UFlareSpacecraftWeaponsSystem*                 WeaponsSystem;
	UPROPERTY()
	UFlareSpacecraftStateManager*				   StateManager;

	bool                                           HasExitedSector;
	bool                                           Paused;
	bool                                           LoadedAndReady;
	
	bool                                           AttachedToParentActor;

	float                                          ScanningTimer;
	float                                          ScanningTimerDuration;
	
	// Docking info
	bool                                           IsManualDocking;
	FFlareDockingParameters                        ManualDockingStatus;
	FFlareDockingInfo                              ManualDockingInfo;
	AFlareSpacecraft*                              ManualDockingTarget;

	float										   TimeSinceUncontrollable;


	/*----------------------------------------------------
		Target selection
	----------------------------------------------------*/

	// Target spacecraft
	UPROPERTY()
	AFlareSpacecraft*                              CurrentTarget;

	// target spacecraft index in the list
	int32                                          TargetIndex;

	// Time elapsed since we last selected
	float                                          TimeSinceSelection;

	// Max time between target selections before "prev" or "next" resets to center
	float                                          MaxTimeBeforeSelectionReset;

	TArray<FFlareScreenTarget> Targets;

	TArray<FFlareScreenTarget>& GetCurrentTargets();

	mutable bool TimeToStopCached = false;
	mutable float TimeToStopCache;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	bool GetIsManualDocking() const
	{
		return IsManualDocking;
	}

	FText GetShipStatus() const;

	/** Return linear velocity in meters */
	FVector GetLinearVelocity() const;

	float GetTimeToStop() const;

	inline UFlareSimulatedSpacecraft* GetParent() const
	{
		return Parent;
	}

	bool IsOutsideSector() const;

	inline bool IsLoadedAndReady() const
	{
		return LoadedAndReady;
	}

	inline FFlareSpacecraftDescription* GetDescription() const
	{
		return Parent->GetDescription();
	}

	inline UFlareCompany* GetCompany()
	{
		return Parent->GetCompany();
	}

	inline FName GetImmatriculation() const
	{
		return Parent->GetImmatriculation();
	}

	inline EFlarePartSize::Type GetSize()
	{
		return Parent->GetSize();
	}

	inline bool IsMilitary()
	{
		return Parent->IsMilitary();
	}

	inline FFlareSpacecraftSave& GetData()
	{
		return Parent->GetData();
	}

	bool IsStation() const
	{
		return Parent->IsStation();
	}

	/*inline FText GetNickName() const override
	{
		return ShipData.NickName;
	}*/

	inline FFlareSpacecraftComponentDescription* GetOrbitalEngineDescription() const
	{
		return OrbitalEngineDescription;
	}

	inline FFlareSpacecraftComponentDescription* GetRCSDescription() const
	{
		return RCSDescription;
	}

	virtual UFlareSpacecraftComponent* GetCockpit() const
	{
		return ShipCockit;
	}

	virtual UCameraComponent* GetCamera() const
	{
		return Cast<UCameraComponent>(Camera);
	}

	inline UFlareSpacecraftStateManager* GetStateManager() const
	{
		return StateManager;
	}

	inline UFlareShipPilot* GetPilot() const
	{
		return Pilot;
	}

	inline bool IsMovingForward() const
	{
		return (FVector::DotProduct(GetSmoothedLinearVelocity(), GetFrontVector()) > 0);
	}

	inline FVector GetSmoothedLinearVelocity() const
	{
		return SmoothedVelocity;
	}

	inline FVector GetFrontVector() const
	{
		return GetActorRotation().RotateVector(FVector(1,0,0));
	}

	inline FVector GetUpVector() const
	{
		return GetActorRotation().RotateVector(FVector(0, 0, 1));
	}

	inline bool IsPaused()
	{
		return Paused;
	}

	float GetTimeSinceUncontrollable() const
	{
		return TimeSinceUncontrollable;
	}

	float GetPreferedAnticollisionTime() const;

	float GetAgressiveAnticollisionTime() const;

};
