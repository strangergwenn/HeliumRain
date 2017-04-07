#pragma once

#include "FlareTechnologyCatalogEntry.h"
#include "FlareTechnologyCatalog.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareTechnologyCatalog : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/
	
	/** Tehcnologies */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareTechnologyCatalogEntry*> TechnologyCatalog;
	

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/
	
	/** Get a ship from identifier */
	FFlareTechnologyDescription* Get(FName Identifier) const;


};
