#pragma once

#include "FlareStationCatalogEntry.h"
#include "../Stations/FlareStation.h"
#include "FlareStationCatalog.generated.h"


UCLASS()
class UFlareStationCatalog : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/
	
	/** Ships */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareStationCatalogEntry*> StationCatalog;


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/
	
	/** Get a station from identifier */
	FFlareStationDescription* Get(FName Identifier) const;


};
