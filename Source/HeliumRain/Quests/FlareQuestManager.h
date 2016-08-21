#pragma once

#include "FlareQuestManager.generated.h"

class UFlareQuest;
class AFlareGame;
class AFlareSpacecraft;
struct FFlareQuestDescription;


/** Quest callback type */
UENUM()
namespace EFlareQuestCallback
{
	enum Type
	{
		TICK_FLYING, // Trig the quest at each tick
		SECTOR_VISITED, // Trig when a sector is visited
		SECTOR_ACTIVE, // Trig when a sector is activated
		FLY_SHIP, // Trig the quest when a ship is flyed
		QUEST // Trig when a quest status change
	};
}

/** Quest current step status save data */
USTRUCT()
struct FFlareQuestStepProgressSave
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, Category = Save)
	FName ConditionIdentifier;

	int32 CurrentProgression;

	FTransform InitialTransform;
	float      InitialVelocity;
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

	UPROPERTY(VisibleAnywhere, Category = Save)
	bool PlayTutorial;
};


/** Quest system manager */
UCLASS()
class HELIUMRAIN_API UFlareQuestManager: public UObject
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
		Quest management
	----------------------------------------------------*/
		
	/** Select this quest */
	void SelectQuest(UFlareQuest* Quest);

	/** Auto select a quest */
	void AutoSelectQuest();


   /*----------------------------------------------------
	   Callback
   ----------------------------------------------------*/

	virtual void LoadCallbacks(UFlareQuest* Quest);

	virtual void ClearCallbacks(UFlareQuest* Quest);

	virtual void OnFlyShip(AFlareSpacecraft* Ship);

	virtual void OnSectorActivation(UFlareSimulatedSector* Sector);

	virtual void OnSectorVisited(UFlareSimulatedSector* Sector);

	virtual void OnTick(float DeltaSeconds);

	virtual void OnQuestStatusChanged(UFlareQuest* Quest);

	virtual void OnQuestSuccess(UFlareQuest* Quest);

	virtual void OnQuestFail(UFlareQuest* Quest);

	virtual void OnQuestActivation(UFlareQuest* Quest);


protected:

   /*----------------------------------------------------
	   Protected data
   ----------------------------------------------------*/

	UPROPERTY()
	TArray<UFlareQuest*>	                 AvailableQuests;

	UPROPERTY()
	TArray<UFlareQuest*>	                 ActiveQuests;

	UPROPERTY()
	TArray<UFlareQuest*>	                 OldQuests;
	
	UFlareQuest*			                 SelectedQuest;
	// TODO Use map structure
	TArray<UFlareQuest*>	                 FlyShipCallback;
	TArray<UFlareQuest*>	                 SectorVisitedCallback;
	TArray<UFlareQuest*>	                 SectorActiveCallback;
	TArray<UFlareQuest*>	                 TickFlyingCallback;
	TArray<UFlareQuest*>	                 QuestCallback;

	FFlareQuestSave			                 QuestData;

	AFlareGame*                              Game;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline AFlareGame* GetGame() const
	{
		return Game;
	}

	inline UFlareQuest* GetSelectedQuest()
	{
		return SelectedQuest;
	}

	inline TArray<UFlareQuest*>& GetActiveQuests()
	{
		return ActiveQuests;
	}

	inline TArray<UFlareQuest*>& GetPreviousQuests()
	{
		return OldQuests;
	}

	bool IsQuestActive(FName QuestIdentifier);

	bool IsQuestSuccesfull(FName QuestIdentifier);

	bool IsQuestFailed(FName QuestIdentifier);


};
