#pragma once

#include "FlareSpacecraftPawn.h"
#include "FlareWeapon.h"
#include "FlareSpacecraftInterface.h"
#include "FlareSpacecraftDamageSystem.h"
#include "FlareSpacecraftNavigationSystem.h"
#include "FlareSpacecraftDockingSystem.h"
#include "FlareSpacecraftWeaponsSystem.h"
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
	class UFlareAirframe* Airframe;


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

	/** Activate or deactivate the exterbal camera */
	virtual void SetExternalCamera(bool NewState);

	/** Switch to combat mode */
	virtual void SetCombatMode(bool NewState);

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

	virtual bool IsMilitary() override;

	virtual bool IsStation() override;

	virtual UFlareInternalComponent* GetInternalComponentAtLocation(FVector Location) const override;

	virtual UFlareSpacecraftDamageSystem* GetDamageSystem() const;

	virtual UFlareSpacecraftNavigationSystem* GetNavigationSystem() const;

	virtual UFlareSpacecraftDockingSystem* GetDockingSystem() const;

	virtual UFlareSpacecraftWeaponsSystem* GetWeaponsSystem() const;



	/*----------------------------------------------------
		Pilot
	----------------------------------------------------*/

	virtual void EnablePilot(bool EnablePilot);

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

	virtual void FirePress();

	virtual void FireRelease();

	virtual void MousePositionInput(FVector2D Val);

	virtual void ThrustInput(float Val);

	virtual void MoveVerticalInput(float Val);

	virtual void MoveHorizontalInput(float Val);

	virtual void RollInput(float Val);

	virtual void PitchInput(float Val);

	virtual void YawInput(float Val);

	virtual void ZoomIn();

	virtual void ZoomOut();

	virtual void StartFire();

	virtual void StopFire();

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

	// Weapon components and descriptions
	TArray <UFlareWeapon*>                   WeaponList;
	TArray <FFlareSpacecraftComponentDescription*> WeaponDescriptionList;

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

	/*----------------------------------------------------
		Regular data
	----------------------------------------------------*/

	// Dynamic gameplay data

	bool                                     ExternalCamera;
	bool                                     FiringPressed;
	bool                                     CombatMode;
	bool                                     IsPiloted;

public:

	// Manual player pilot
	FVector2D                                PlayerMouseOffset;
	FVector                                  PlayerManualAngularVelocity; // In local space
	FVector                                  PlayerManualLinearVelocity;
	bool                                     PlayerManualOrbitalBoost;
	float                                    AngularInputDeadRatio;

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

	inline FFlareSpacecraftComponentDescription* GetWeaponDescription(int32 Index) const
	{
		return WeaponDescriptionList[Index];
	}

	inline TArray<UFlareWeapon*>& GetWeaponList()
	{
		return WeaponList;
	}

	virtual UFlareSpacecraftComponent* GetCockpit() const override
	{
		return ShipCockit;
	}

	inline bool IsExternalCamera() const
	{
		return ExternalCamera;
	}

	inline bool IsCombatMode() const
	{
		return CombatMode;
	}

	inline bool IsPilotMode() const
	{
		return IsPiloted;
	}

	inline UFlareShipPilot* GetPilot() const
	{
		return Pilot;
	}

	inline FVector2D GetMouseOffset() const
	{
		return PlayerMouseOffset;
	}
};
