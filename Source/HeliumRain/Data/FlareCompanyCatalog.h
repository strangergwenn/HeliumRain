#pragma once

#include "Engine/DataAsset.h"
#include "../Game/FlareCompany.h"
#include "FlareCompanyCatalog.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareCompanyCatalog : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/
	
	/** Company data */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareCompanyDescription> Companies;

};
