#pragma once

#include "../Ships/FlareShipComponent.h"
#include "FlareShipPartsCatalogEntry.generated.h"


UCLASS()
class UFlareShipPartsCatalogEntry : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Data */
	UPROPERTY(EditAnywhere, Category = Content)
	FFlareShipModuleDescription Data;


};
