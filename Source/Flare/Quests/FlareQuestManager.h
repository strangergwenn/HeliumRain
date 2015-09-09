#pragma once

#include "FlareQuestManager.generated.h"

class UFlareQuest;
class AFlareGame;
class UFlareSimulatedSpacecraft;


/** Quest current step status save data */
USTRUCT()
struct FFlareQuestStepProgressSave
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, Category = Save)
	FName ConditionIdentifier;

	int32 CurrentProgression;
};

/** Quest progress status save data */
USTRUCT()
struct FFlareQuestProgressSave
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, Category = Save)
	FName QuestIdentifier;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FName> SuccessfullSteps;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareQuestStepProgressSave> CurrentStepProgress;
};

/** Quest status save data */
USTRUCT()
struct FFlareQuestSave
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, Category = Save)
	FName SelectedQuest;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareQuestProgressSave> QuestProgresses;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FName> SuccessfulQuests;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FName> AbandonnedQuests;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FName> FailedQuests;
};


/** Quest system manager */
UCLASS()
class FLARE_API UFlareQuestManager: public UObject
{
	GENERATED_UCLASS_BODY()

public:
   /*----------------------------------------------------
	   Save
   ----------------------------------------------------*/

	/** Load the quests status from a save file */
	virtual void Load(const FFlareQuestSave& Data);

	/** Save the quests status to a save file */
	virtual FFlareQuestSave* Save();

   /*----------------------------------------------------
	   Callback
   ----------------------------------------------------*/

	void LoadCallbacks(UFlareQuest* Quest);

	void OnFlyShip(UFlareSimulatedSpacecraft* Ship);

protected:

   /*----------------------------------------------------
	   Protected data
   ----------------------------------------------------*/

	UPROPERTY()
	TArray<UFlareQuest*>	AvailableQuests;

	UPROPERTY()
	TArray<UFlareQuest*>	CurrentQuests;

	UPROPERTY()
	TArray<UFlareQuest*>	OldQuests;

	UFlareQuest*			SelectedQuest;
	TArray<UFlareQuest*>	FlyShipCallback;

	FFlareQuestSave			QuestData;

	AFlareGame*             Game;

	public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const
	{
		return Game;
	}

};
