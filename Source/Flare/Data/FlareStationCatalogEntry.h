#pragma once

#include "../Stations/FlareStation.h"
#include "FlareStationCatalogEntry.generated.h"


UCLASS()
class UFlareStationCatalogEntry : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Data */
	UPROPERTY(EditAnywhere, Category = Content)
	FFlareStationDescription Data;


};
