#pragma once

#include "../Ships/FlareShip.h"
#include "FlareSpacecraftCatalogEntry.generated.h"


UCLASS()
class UFlareSpacecraftCatalogEntry : public UDataAsset
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
