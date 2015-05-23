#pragma once

#include "FlareSpacecraftComponent.h"
#include "FlareWeapon.h"
#include "FlareTurret.generated.h"


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

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	/** Mesh component */
	UPROPERTY()

	UStaticMeshComponent*                    TurretComponent;
	UStaticMeshComponent*                    BarrelComponent;


	float                                    TurretAngle;
	float                                    BarrelAngle;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

};
