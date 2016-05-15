#pragma once

#include "../Economy/FlareFactory.h"
#include "FlareResourceCatalog.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareResourceCatalog : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Resources data */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareResourceCatalogEntry*> Resources;
	
	/** Resources data */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareResourceCatalogEntry*> ConsumerResources;

	/** Resources data */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareResourceCatalogEntry*> MaintenanceResources;

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Get a resource from identifier */
	FFlareResourceDescription* Get(FName Identifier) const;

	/** Get a resource from identifier */
	UFlareResourceCatalogEntry* GetEntry(FFlareResourceDescription*) const;

	/** Get all resources */
	TArray<UFlareResourceCatalogEntry*>& GetResourceList()
	{
		return Resources;
	}

	bool IsCustomerResource(FFlareResourceDescription* Resource) const;

	bool IsMaintenanceResource(FFlareResourceDescription* Resource) const;

};


inline static bool SortByResourceType(const UFlareResourceCatalogEntry& ResourceA, const UFlareResourceCatalogEntry& ResourceB)
{
	return (ResourceA.Data.Icon.GetResourceName() < ResourceB.Data.Icon.GetResourceName());
}
