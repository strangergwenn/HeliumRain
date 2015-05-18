#pragma once

#include "FlareSpacecraftComponent.h"
#include "FlareInternalComponent.generated.h"


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class FLARE_API UFlareInternalComponent : public UFlareSpacecraftComponent
{
public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/
	
	virtual void Initialize(const FFlareSpacecraftComponentSave* Data, UFlareCompany* Company, AFlareSpacecraftPawn* OwnerShip, bool IsInMenu) override;

	virtual FFlareSpacecraftComponentSave* Save() override;

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

