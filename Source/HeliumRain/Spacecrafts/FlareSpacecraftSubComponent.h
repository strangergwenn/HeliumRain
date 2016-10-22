#pragma once

#include "FlareSpacecraftComponent.h"
#include "FlareSpacecraftSubComponent.generated.h"

UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class UFlareSpacecraftSubComponent : public UFlareSpacecraftComponent
{

public:

	GENERATED_UCLASS_BODY()

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void SetParentSpacecraftComponent(UFlareSpacecraftComponent* Component);

	float GetArmorAtLocation(FVector Location) override;

	virtual float ApplyDamage(float Energy, EFlareDamage::Type DamageType, UFlareCompany* DamageSource) override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	protected:

		/*----------------------------------------------------
			Protected data
		----------------------------------------------------*/

		UPROPERTY()
		UFlareSpacecraftComponent*                               ParentComponent;
};
