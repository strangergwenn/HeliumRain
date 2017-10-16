#pragma once

#include "../Flare.h"
#include "Engine/DataAsset.h"
#include "FlareMeteoriteCatalog.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareMeteoriteCatalog : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:
	
	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/
	
	/** Meteorite data */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<class UDestructibleMesh*> RockMeteorites;
	
	/** Meteorite data */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<class UDestructibleMesh*> MetalMeteorites;

};
