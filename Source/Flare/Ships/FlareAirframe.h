#pragma once

#include "FlareShipModule.h"
#include "FlareAirframe.generated.h"


USTRUCT()
struct FFlareAirframeDescription : public FFlareShipModuleDescription
{
	GENERATED_USTRUCT_BODY()


};


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class UFlareAirframe : public UFlareShipModule
{

public:

	GENERATED_UCLASS_BODY()


	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void OnRegister()  override;


};
