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

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Get a resource from identifier */
	FFlareResourceDescription* Get(FName Identifier) const;

};
