#pragma once

#include "FlareQuestManager.generated.h"

class UFlareQuest;
class UFlareSimulatedSpacecraft;

/** World quest system manager */
UCLASS()
class FLARE_API UFlareQuestManager: public UObject
{
	GENERATED_UCLASS_BODY()

public:

	Load()

   /*----------------------------------------------------
	   Triggers
   ----------------------------------------------------*/

	void OnFlyShip(UFlareSimulatedSpacecraft* Ship);

protected:

   /*----------------------------------------------------
	   Protected data
   ----------------------------------------------------*/

	UPROPERTY()
	TArray<UFlareQuest*> AvailableQuest;

	UPROPERTY()
	TArray<UFlareQuest*> OldQuests;


	TArray<UFlareQuest*> FlyShipTriggerQuests;

};
