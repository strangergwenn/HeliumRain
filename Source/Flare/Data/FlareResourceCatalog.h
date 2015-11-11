#pragma once

#include "../Economy/FlareFactory.h"
#include "FlareResourceCatalog.generated.h"


UCLASS()
class UFlareResourceCatalog : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Resources data */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareResourceCatalogEntry*> Resources;

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Get a resource from identifier */
	FFlareResourceDescription* Get(FName Identifier) const;

};
