#pragma once

#include "FlareSpacecraftComponent.h"
#include "FlareWeapon.h"
#include "FlareTurretPilot.h"
#include "FlareTurret.generated.h"

class UFlareSpacecraftSubComponent;

UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class UFlareTurret : public UFlareWeapon
{
public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	void Initialize(const FFlareSpacecraftComponentSave* Data, UFlareCompany* Company, AFlareSpacecraftPawn* OwnerShip, bool IsInMenu) override;

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	void SetupComponentMesh() override;

	virtual FVector GetFireAxis() const;

	virtual FVector GetIdleAxis() const;

	virtual FVector GetMuzzleLocation(int GunIndex) const;

	virtual FVector GetTurretBaseLocation() const;

	virtual bool IsSafeToFire(int GunIndex) const;

	virtual bool IsReacheableAxis(FVector TargetAxis) const;

	virtual void GetBoundingSphere(FVector& Location, float& Radius) override;

	// TODO Put in help with FlareShell::Trace
	bool Trace(const FVector& Start, const FVector& End, FHitResult& HitOut) const;

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	/** Mesh component */
	UPROPERTY()
	UFlareSpacecraftSubComponent*                    TurretComponent;
	UPROPERTY()
	UFlareSpacecraftSubComponent*                    BarrelComponent;

	// Pilot object
	UPROPERTY()
	UFlareTurretPilot*                               Pilot;


	float                                    TurretAngle;
	float                                    BarrelAngle;
	FVector  								 AimDirection;
	// TODO Save

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

};
