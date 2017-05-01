#pragma once

#include "../Flare.h"
#include "Engine/DataAsset.h"
#include "FlareCameraShakeCatalog.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareCameraShakeCatalog : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Fighter getting hit */
	UPROPERTY(EditAnywhere, Category = Content)
	TSubclassOf<UCameraShake> HitNormal;

	/** Capital getting hit  */
	UPROPERTY(EditAnywhere, Category = Content)
	TSubclassOf<UCameraShake> HitHeavy;
	
	/** Fighter impact */
	UPROPERTY(EditAnywhere, Category = Content)
	TSubclassOf<UCameraShake> ImpactS;
	
	/** Capital impact */
	UPROPERTY(EditAnywhere, Category = Content)
	TSubclassOf<UCameraShake> ImpactL;
	
	/** Exhaust */
	UPROPERTY(EditAnywhere, Category = Content)
	TSubclassOf<UCameraShake> Exhaust;
	
};
