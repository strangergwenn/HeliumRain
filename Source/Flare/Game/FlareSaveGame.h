#pragma once

#include "FlareCompany.h"
#include "FlareWorld.h"
#include "../Quests/FlareQuestManager.h"

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

	/** Chosen scenario */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 ScenarioId;

    /** Identifier of the company */
    UPROPERTY(EditAnywhere, Category = Save)
    FName CompanyIdentifier;

	UPROPERTY(EditAnywhere, Category = Save)
	FFlareQuestSave		QuestData;
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

