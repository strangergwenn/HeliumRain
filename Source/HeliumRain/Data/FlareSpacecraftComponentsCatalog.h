#pragma once

#include "HeliumRain/Data/FlareSpacecraftComponentsCatalogEntry.h"
#include "FlareSpacecraftComponentsCatalog.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareSpacecraftComponentsCatalog : public UObject
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
	const void GetEngineList(TArray<FFlareSpacecraftComponentDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size, UFlareCompany* FilterCompany = NULL);

	/** Search all RCS and get one that fits */
	const void GetRCSList(TArray<FFlareSpacecraftComponentDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size, UFlareCompany* FilterCompany = NULL);

	/** Search all weapons and get one that fits */
	const void GetWeaponList(TArray<FFlareSpacecraftComponentDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size, UFlareCompany* FilterCompany = NULL);


};
