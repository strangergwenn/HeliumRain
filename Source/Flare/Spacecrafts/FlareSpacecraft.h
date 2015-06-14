#pragma once

#include "FlareSpacecraftPawn.h"
#include "FlareWeapon.h"
#include "FlareSpacecraftInterface.h"
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

	/*----------------------------------------------------
		Player interface
	----------------------------------------------------*/

	/** Extrapolate the position of a ship for a given targetting ship */
	virtual FVector GetAimPosition(AFlareSpacecraft* TargettingShip, float BulletSpeed, float PredictionDelay) const;

	/** Extrapolate the position of a ship for a given gun */
	FVector GetAimPosition(FVector GunLocation, FVector GunVelocity, float BulletSpeed, float PredictionDelay) const;

	/*----------------------------------------------------
		Ship interface
	----------------------------------------------------*/

	virtual void Load(const FFlareSpacecraftSave& Data) override;

	virtual FFlareSpacecraftSave* Save() override;

	virtual void SetOwnerCompany(UFlareCompany* Company) override;

	virtual UFlareCompany* GetCompany() override;

	virtual EFlarePartSize::Type GetSize() override;

	virtual bool IsMilitary() override;

	virtual bool IsStation() override;

	virtual UFlareInternalComponent* GetInternalComponentAtLocation(FVector Location) const override;

	virtual UFlareSpacecraftDamageSystem* GetDamageSystem() const;

	virtual UFlareSpacecraftNavigationSystem* GetNavigationSystem() const;

	virtual UFlareSpacecraftDockingSystem* GetDockingSystem() const;

	virtual UFlareSpacecraftWeaponsSystem* GetWeaponsSystem() const;

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


public:

	/*----------------------------------------------------
		Input methods
	----------------------------------------------------*/

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	virtual void LeftMousePress();

	virtual void LeftMouseRelease();

	virtual void ActivateWeaponGroup1();

	virtual void ActivateWeaponGroup2();

	virtual void ActivateWeaponGroup3();

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

	virtual void BoostOn();

	virtual void BoostOff();

	virtual void ForceManual();

protected:

	/*----------------------------------------------------
		Component data
	----------------------------------------------------*/

	// Component descriptions, save data
	FFlareSpacecraftSave                           ShipData;
	FFlareSpacecraftDescription*                   ShipDescription;
	FFlareSpacecraftComponentDescription*          OrbitalEngineDescription;
	FFlareSpacecraftComponentDescription*          RCSDescription;

	// Lifesupport status
	UPROPERTY()
	UFlareSpacecraftComponent*                     ShipCockit;

	// Pilot object
	UPROPERTY()
	UFlareShipPilot*                               Pilot;

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

	/*----------------------------------------------------
		Regular data
	----------------------------------------------------*/

public:

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	/** Return linear velocity in meters */
	FVector GetLinearVelocity() const;

	inline FFlareSpacecraftDescription* GetDescription() const
	{
		return ShipDescription;
	}

	inline FFlareSpacecraftComponentDescription* GetOrbitalEngineDescription() const
	{
		return OrbitalEngineDescription;
	}

	inline FFlareSpacecraftComponentDescription* GetRCSDescription() const
	{
		return RCSDescription;
	}

	virtual UFlareSpacecraftComponent* GetCockpit() const override
	{
		return ShipCockit;
	}

	inline UFlareSpacecraftStateManager* GetStateManager()
	{
		return StateManager;
	}

	inline UFlareShipPilot* GetPilot() const
	{
		return Pilot;
	}
};
