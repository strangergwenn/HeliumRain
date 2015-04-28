#pragma once

#include "FlareShipComponent.h"
#include "FlareAirframe.generated.h"


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class UFlareAirframe : public UFlareShipComponent
{

public:

	GENERATED_UCLASS_BODY()

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void OnRegister()  override;

	float GetRemainingArmorAtLocation(FVector Location) override;

	float GetAvailablePower() const override;

};
