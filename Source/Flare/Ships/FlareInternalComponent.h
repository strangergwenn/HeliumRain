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

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;


	virtual void Initialize(const FFlareShipComponentSave* Data, UFlareCompany* Company, AFlareShipBase* OwnerShip, bool IsInMenu) override;

	virtual FFlareShipComponentSave* Save() override;

	virtual void StartDestroyedEffects() override;

	virtual void GetBoundingSphere(FVector& Location, float& Radius) override;


public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Ship rotation */
	UPROPERTY(EditAnywhere, Category = Content)
	float Radius;

};

