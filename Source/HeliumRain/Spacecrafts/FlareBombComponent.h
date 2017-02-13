#pragma once

#include "FlareSpacecraftComponent.h"
#include "FlareBombComponent.generated.h"


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class HELIUMRAIN_API UFlareBombComponent : public UFlareSpacecraftComponent
{
public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/
	virtual UStaticMesh* GetMesh(bool PresentationMode) const override;

	virtual float GetDamageRatio() const override;

	/** Update the exhaust special effect */
	virtual void UpdateEffects(float Alpha);


public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Ship rotation */
	UPROPERTY(EditAnywhere, Category = Content)
	float Radius;

};

