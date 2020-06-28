#pragma once

#include "HeliumRain/Data/FlareSpacecraftCatalogEntry.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "FlareSpacecraftCatalog.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareSpacecraftCatalog : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/
	
	/** Ships */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareSpacecraftCatalogEntry*> ShipCatalog;

	/** Stations */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareSpacecraftCatalogEntry*> StationCatalog;

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/
	
	/** Get a ship from identifier */
	FFlareSpacecraftDescription* Get(FName Identifier) const;


};
