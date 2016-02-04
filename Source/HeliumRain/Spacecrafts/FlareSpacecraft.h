#pragma once

#include "FlareSpacecraftPawn.h"
#include "FlareWeapon.h"
#include "FlareSpacecraftInterface.h"
#include "FlareSpacecraftComponent.h"
#include "FlareSpacecraftSpinningComponent.h"
#include "Subsystems/FlareSpacecraftDamageSystem.h"
#include "Subsystems/FlareSpacecraftNavigationSystem.h"
#include "Subsystems/FlareSpacecraftDockingSystem.h"
#include "Subsystems/FlareSpacecraftWeaponsSystem.h"
#include "FlareSpacecraftStateManager.h"
#include "FlareSpacecraft.generated.h"

/** Ship class */
UCLASS(Blueprintable, ClassGroup = (Flare, Ship))
class AFlareSpacecraft : public AFlareSpacecraftPawn, public IFlareSpacecraftInterface
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

	virtual void OnDocked();

	virtual void OnUndocked();

	virtual void SetPause(bool Pause);

	virtual void Redock();

	/*----------------------------------------------------
		Player interface
	----------------------------------------------------*/

	/** Extrapolate the position of a ship for a given targetting ship. Return time before intersect. If time is negative, no intersection. */
	virtual float GetAimPosition(AFlareSpacecraft* TargettingShip, float BulletSpeed, float PredictionDelay, FVector* ResultPosition) const;

	/** Extrapolate the position of a ship for a given gun. Return time before intersect. If time is negative, no intersection. */
	virtual float GetAimPosition(FVector GunLocation, FVector GunVelocity, float BulletSpeed, float PredictionDelay, FVector* ResultPosition) const;

	/** Get the current target */
	AFlareSpacecraft* GetCurrentTarget() const;


	/*----------------------------------------------------
		Ship interface
	----------------------------------------------------*/

	virtual void Load(const FFlareSpacecraftSave& Data) override;

	virtual FFlareSpacecraftSave* Save() override;

	virtual void SetOwnerCompany(UFlareCompany* Company);

	virtual UFlareCompany* GetCompany() override;

	virtual EFlarePartSize::Type GetSize() override;

	virtual FName GetImmatriculation() const override;

	virtual bool IsMilitary() const override;

	virtual bool IsStation() const override;

	virtual UFlareInternalComponent* GetInternalComponentAtLocation(FVector Location) const;

	virtual UFlareSpacecraftDamageSystem* GetDamageSystem() const;

	virtual UFlareSpacecraftNavigationSystem* GetNavigationSystem() const;

	virtual UFlareSpacecraftDockingSystem* GetDockingSystem() const;

	virtual UFlareSpacecraftWeaponsSystem* GetWeaponsSystem() const;

	virtual bool CanBeFlown() const override
	{
		return !IsStation() && !IsAssignedToSector();
	}

	void AssignToSector(bool Assign) override
	{
		ShipData.IsAssigned = Assign;
	}

	virtual bool IsAssignedToSector() const override
	{
		return ShipData.IsAssigned;
	}

	/** Set asteroid data from an asteroid save */
	void SetAsteroidData(FFlareAsteroidSave* Data);

	/** Apply the current asteroid data */
	void ApplyAsteroidData();

	inline UFlareCargoBay* GetCargoBay() override
	{
		return CargoBay;
	}

	UFlareSectorInterface* GetCurrentSectorInterface() override;

protected:

	/*----------------------------------------------------
		Damage system
	----------------------------------------------------*/

	/** Our ship killed another ship */
	virtual void OnEnemyKilled(IFlareSpacecraftInterface* Enemy);


public:

	/*----------------------------------------------------
		Customization
	----------------------------------------------------*/

	/** Set he ship description to load data from */
	virtual void SetShipDescription(FFlareSpacecraftDescription* Description);

	/** Set the description to use for all orbital engines */
	virtual void SetOrbitalEngineDescription(FFlareSpacecraftComponentDescription* Description);

	/** Set the description to use for all RCS */
	virtual void SetRCSDescription(FFlareSpacecraftComponentDescription* Description);

	virtual void UpdateCustomization() override;

	virtual void StartPresentation() override;

	/** Set the cockpit mesh */
	void EnterCockpit(UStaticMesh* Mesh, UMaterialInstanceDynamic* Material, UMaterialInstanceDynamic* FrameMaterial, UCanvasRenderTarget2D* CameraTarget);

	/** Remove the cockpit mesh */
	void ExitCockpit();


public:

	/*----------------------------------------------------
		Input methods
	----------------------------------------------------*/

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	virtual void LeftMousePress();

	virtual void LeftMouseRelease();

	virtual void DeactivateWeapon();

	virtual void ActivateWeaponGroup1();

	virtual void ActivateWeaponGroup2();

	virtual void ActivateWeaponGroup3();

	virtual void NextWeapon();

	virtual void PreviousWeapon();

	virtual void NextTarget();

	virtual void PreviousTarget();

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

	virtual void Brake();

	virtual void BrakeToVelocity(const FVector& VelocityTarget = FVector::ZeroVector);

	virtual void BoostOn();

	virtual void BoostOff();

	virtual void ForceManual();


protected:

	/*----------------------------------------------------
		Internal data
	----------------------------------------------------*/

	// Component descriptions, save data
	FFlareSpacecraftSave                           ShipData;
	FFlareSpacecraftDescription*                   ShipDescription;
	FFlareSpacecraftComponentDescription*          OrbitalEngineDescription;
	FFlareSpacecraftComponentDescription*          RCSDescription;

	FVector                                        SmoothedVelocity;

	// Lifesupport status
	UPROPERTY()
	UFlareSpacecraftComponent*                     ShipCockit;

	// Pilot object
	UPROPERTY()
	UFlareShipPilot*                               Pilot;

	// Decal material
	UPROPERTY()
	UMaterialInstanceDynamic*                      DecalMaterial;

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

	bool                                           Paused;

	UPROPERTY()
	UFlareCargoBay*                                         CargoBay;

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


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline AFlareGame* GetGame() const override
	{
		return AFlareSpacecraftPawn::GetGame();
	}

	/** Return linear velocity in meters */
	FVector GetLinearVelocity() const;

	inline FFlareSpacecraftDescription* GetDescription() const
	{
		return ShipDescription;
	}

	inline FName GetNickName() const override
	{
		return ShipData.NickName;
	}

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

	inline UFlareSpacecraftStateManager* GetStateManager()
	{
		return StateManager;
	}

	inline UFlareShipPilot* GetPilot() const
	{
		return Pilot;
	}

	inline FVector GetSmoothedLinearVelocity() const
	{
		return SmoothedVelocity;
	}

	inline FVector GetFrontVector() const
	{
		return GetActorRotation().RotateVector(FVector(1,0,0));
	}

	inline bool IsPaused()
	{
		return Paused;
	}
};
