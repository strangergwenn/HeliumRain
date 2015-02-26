#pragma once

#include "FlareShipCatalogEntry.h"
#include "../Ships/FlareShip.h"
#include "FlareShipCatalog.generated.h"


UCLASS()
class UFlareShipCatalog : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/
	
	/** Ships */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareShipCatalogEntry*> ShipCatalog;


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/
	
	/** Get a ship from identifier */
	FFlareShipDescription* Get(FName Identifier) const;


};
