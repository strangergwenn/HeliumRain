#pragma once

#include "FlareShipPartsCatalogEntry.h"
#include "FlareShipPartsCatalog.generated.h"


UCLASS()
class UFlareShipPartsCatalog : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/
	
	/** Orbital engines */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareShipPartsCatalogEntry*> EngineCatalog;

	/** RCS engines */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareShipPartsCatalogEntry*> RCSCatalog;

	/** Weapons */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareShipPartsCatalogEntry*> WeaponCatalog;

	/** Internal modules */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareShipPartsCatalogEntry*> InternalModulesCatalog;

	/** Meta objects */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareShipPartsCatalogEntry*> MetaCatalog;

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Get a part description */
	FFlareShipModuleDescription* Get(FName Identifier) const;

	/** Search all engines and get one that fits */
	const void GetEngineList(TArray<FFlareShipModuleDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size);

	/** Search all RCS and get one that fits */
	const void GetRCSList(TArray<FFlareShipModuleDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size);

	/** Search all weapons and get one that fits */
	const void GetWeaponList(TArray<FFlareShipModuleDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size);


};
