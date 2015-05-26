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

	float GetRemainingArmorAtLocation(FVector Location) override;

	protected:

		/*----------------------------------------------------
			Protected data
		----------------------------------------------------*/

		UPROPERTY()
		UFlareSpacecraftComponent*                               ParentComponent;
};
