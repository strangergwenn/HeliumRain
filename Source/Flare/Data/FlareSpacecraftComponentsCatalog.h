#pragma once

#include "FlareSpacecraftComponentsCatalogEntry.h"
#include "FlareSpacecraftComponentsCatalog.generated.h"


UCLASS()
class UFlareSpacecraftComponentsCatalog : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/
	
	/** Orbital engines */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareSpacecraftComponentsCatalogEntry*> EngineCatalog;

	/** RCS engines */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareSpacecraftComponentsCatalogEntry*> RCSCatalog;

	/** Weapons */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareSpacecraftComponentsCatalogEntry*> WeaponCatalog;

	/** Internal modules */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareSpacecraftComponentsCatalogEntry*> InternalComponentsCatalog;

	/** Meta objects */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareSpacecraftComponentsCatalogEntry*> MetaCatalog;

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Get a part description */
	FFlareSpacecraftComponentDescription* Get(FName Identifier) const;

	/** Search all engines and get one that fits */
	const void GetEngineList(TArray<FFlareSpacecraftComponentDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size);

	/** Search all RCS and get one that fits */
	const void GetRCSList(TArray<FFlareSpacecraftComponentDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size);

	/** Search all weapons and get one that fits */
	const void GetWeaponList(TArray<FFlareSpacecraftComponentDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size);


};
