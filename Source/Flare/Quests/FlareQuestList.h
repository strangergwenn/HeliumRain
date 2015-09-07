#pragma once

#include "FlareQuestList.generated.h"

class UFlareQuestProgress;
class UFlareQuest;

/** Quest list for a company */
UCLASS()
class FLARE_API UFlareQuestList: public UObject
{
	GENERATED_UCLASS_BODY()

public:

protected:

   /*----------------------------------------------------
	   Protected data
   ----------------------------------------------------*/

	UPROPERTY()
	TArray<UFlareQuestProgress*> QuestProgresses;

	TArray<UFlareQuest*> KnownQuests;
	TArray<UFlareQuest*> AcceptedQuests;
	TArray<UFlareQuest*> RefusedQuests;
	UFlareQuest* SelectedQuest;

};
