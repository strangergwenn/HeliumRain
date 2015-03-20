#pragma once

#include "FlareShipComponent.h"
#include "FlareAirframe.generated.h"


USTRUCT()
struct FFlareAirframeDescription : public FFlareShipModuleDescription
{
	GENERATED_USTRUCT_BODY()


};


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class UFlareAirframe : public UFlareShipComponent
{

public:

	GENERATED_UCLASS_BODY()


	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void OnRegister()  override;


};
