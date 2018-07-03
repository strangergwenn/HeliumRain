#pragma once

#include "FlareScannableCatalogEntry.h"
#include "FlareScannableCatalog.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareScannableCatalog : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/
	
	/** Scannables */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareScannableCatalogEntry*> ScannableCatalog;
	

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/
	
	/** Get a scannable from identifier */
	FFlareScannableDescription* Get(FName Identifier) const;
	
};
