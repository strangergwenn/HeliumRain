#pragma once

#include "FlareQuestManager.h"

#include "FlareQuest.generated.h"

class UFlareQuestCondition;
class UFlareQuestStep;
class UFlareQuestAction;

/** Quest action type */
UENUM()
namespace EFlareQuestStatus
{
	enum Type
	{
		PENDING, // Wait available condition
		AVAILABLE, // Wait acceptation
		ACTIVE, // In progress
		SUCCESSFUL,
		ABANDONED,
		FAILED
	};
}



/** Quest category type */
UENUM()
namespace EFlareQuestCategory
{
	enum Type
	{
		TUTORIAL,
		HISTORY,
		ACHIEVEMENT,
		SECONDARY
	};
}


struct FFlarePlayerObjectiveData;

/** Quest */
UCLASS()
class HELIUMRAIN_API UFlareQuest: public UObject
{
	GENERATED_UCLASS_BODY()

public:

	 /** Load */
	 virtual void LoadInternal(UFlareQuestManager* Parent);

	/** Restore the quest status from a save file */
	virtual void Restore(const FFlareQuestProgressSave& Data);

	/** Save the quest status to a save file */
	virtual FFlareQuestProgressSave* Save();


	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	virtual void SetStatus(EFlareQuestStatus::Type Status);

	virtual void UpdateState();

	virtual void EndStep();

	virtual void NextStep();

	virtual void Success();

	virtual void Abandon();

	virtual void Fail();

	virtual void MakeAvailable();

	virtual void Accept();

	virtual void Activate();

	virtual void SendQuestNotification(FText Message, FName Tag, bool Pinned = true);

	virtual FText FormatTags(FText Message);

	/*----------------------------------------------------
		Objective tracking
	----------------------------------------------------*/

	virtual void StartObjectiveTracking();

	virtual void StopObjectiveTracking();

	virtual void UpdateObjectiveTracker();

	//virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData, const TArray<FFlareQuestConditionDescription>& Conditions);

	/*----------------------------------------------------
		Callback
	----------------------------------------------------*/

	virtual TArray<EFlareQuestCallback::Type> GetCurrentCallbacks();

	//  UFlareQuestCondition->GetCallbacks();

	virtual void OnFlyShip(AFlareSpacecraft* Ship);

	virtual void OnSectorActivation(UFlareSimulatedSector* Sector);

	virtual void OnSectorVisited(UFlareSimulatedSector* Sector);

	virtual void OnTick(float DeltaSeconds);

	virtual void OnQuestStatusChanged(UFlareQuest* Quest);

protected:

	/*----------------------------------------------------
		Objective tracking
	----------------------------------------------------*/



   /*----------------------------------------------------
	   Protected data
   ----------------------------------------------------*/

	FFlareQuestProgressSave					QuestData;
	EFlareQuestStatus::Type					QuestStatus;
	UFlareQuestStep*                        CurrentStep;
	UFlareQuestManager*						QuestManager;

	UPROPERTY()
	TArray<UFlareQuestStep*>				Steps;

	UPROPERTY()
	TArray<UFlareQuestStep*>				SuccessfullSteps;

	UPROPERTY()
	TArray<UFlareQuestCondition*>           TriggerConditions;

	UPROPERTY()
	TArray<UFlareQuestAction*>				SuccessActions;
	UPROPERTY()
	TArray<UFlareQuestAction*>				FailActions;

	bool									TrackObjectives;
	FName									Identifier;
	FText									QuestName;
	FText                                   QuestDescription;
	EFlareQuestCategory::Type               QuestCategory;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline FName GetIdentifier() const
	{
		return Identifier;
	}

	inline FText GetQuestName() const
	{
		return QuestName;
	}

	inline FText GetQuestDescription() const
	{
		return QuestDescription;
	}

	inline EFlareQuestCategory::Type GetQuestCategory() const
	{
		return QuestCategory;
	}

	inline EFlareQuestStatus::Type GetStatus() const
	{
		return QuestStatus;
	}

	FText GetStatusText() const;

	inline UFlareQuestStep* GetCurrentStep() const
	{
		return CurrentStep;
	}

	TArray<UFlareQuestStep*>& GetSteps()
	{
		return Steps;
	}

	int32 GetSuccessfullStepCount()
	{
		return SuccessfullSteps.Num();
	}

	int32 GetStepCount()
	{
		return Steps.Num();
	}

	bool IsAccepted()
	{
		return QuestData.Accepted;
	}

	bool HasAutoAccept();

	UFlareQuestManager*	GetQuestManager()
	{
		return QuestManager;
	}

	UFlareSimulatedSector* FindSector(FName SectorIdentifier);
};
