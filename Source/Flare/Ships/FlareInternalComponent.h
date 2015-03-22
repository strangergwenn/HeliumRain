#pragma once

#include "FlareShipComponent.h"
#include "FlareInternalComponent.generated.h"


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class FLARE_API UFlareInternalComponent : public UFlareShipComponent
{
public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	void Initialize(const FFlareShipComponentSave* Data, UFlareCompany* Company, AFlareShipBase* OwnerShip, bool IsInMenu) override;

	virtual FFlareShipComponentSave* Save() override;

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;


	/** Ship rotation */
	UPROPERTY(EditAnywhere, Category = Content)
	float Radius;

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

};
