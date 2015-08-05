#pragma once

#include "FlareCompany.h"
#include "FlareWorld.h"

#include "FlareSaveGame.generated.h"



/** Player objective data */
USTRUCT()
struct FFlarePlayerObjective
{
	GENERATED_USTRUCT_BODY()

	bool                        Set;
	FText                       Name;
	FText                       Info;
	AActor*                     Target;
	FVector                     Location;
	float                       Progress;
};

/** Game save data */
USTRUCT()
struct FFlarePlayerSave
{
    GENERATED_USTRUCT_BODY()

    /** UObject name of the currently possessed ship */
    UPROPERTY(EditAnywhere, Category = Save)
	FName CurrentShipName;

	/** Chosen scenario */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 ScenarioId;

    /** Identifier of the company */
    UPROPERTY(EditAnywhere, Category = Save)
    FName CompanyIdentifier;
};


UCLASS()
class UFlareSaveGame : public USaveGame
{
public:

	GENERATED_UCLASS_BODY()


	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	UPROPERTY(VisibleAnywhere, Category = Save)
	FFlarePlayerSave PlayerData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	FFlareCompanyDescription PlayerCompanyDescription;

    UPROPERTY(VisibleAnywhere, Category = Save)
    FFlareWorldSave WorldData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	int32 CurrentImmatriculationIndex;
};

