#pragma once

#include "../Ships/FlareShip.h"
#include "FlareShipCatalogEntry.generated.h"


UCLASS()
class UFlareShipCatalogEntry : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/
	
	/** Ship data */
	UPROPERTY(EditAnywhere, Category = Content)
	FFlareShipDescription Data;

};
