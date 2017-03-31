#pragma once

#include "FlareQuestManager.h"

#include "FlareQuest.generated.h"

class UFlareQuestCondition;
class UFlareQuestConditionGroup;
class UFlareQuestStep;
class UFlareQuestAction;
class UFlareCompany;





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

	void SetupIndexes();

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	virtual void SetStatus(EFlareQuestStatus::Type Status);

	virtual void UpdateState();

	virtual void EndStep();

	virtual void NextStep(bool Silent = false);

	virtual void Success();

	virtual void Abandon(bool Expired);

	virtual void Fail();

	virtual void MakeAvailable();

	virtual void Accept();

	virtual void MakeOngoing();

	virtual void SendQuestNotification(FText Message, FName Tag, bool Pinned = true);

	virtual FText FormatTags(FText Message);

	/*----------------------------------------------------
		Objective tracking
	----------------------------------------------------*/

	virtual void StartObjectiveTracking();

	virtual void StopObjectiveTracking();

	virtual void UpdateObjectiveTracker();

	/*----------------------------------------------------
		Callback
	----------------------------------------------------*/

	virtual TArray<UFlareQuestCondition*> GetCurrentConditions();

	virtual TArray<EFlareQuestCallback::Type> GetCurrentCallbacks();

	virtual void OnTradeDone(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource, int32 Quantity);

	virtual void OnSpacecraftCaptured(UFlareSimulatedSpacecraft* CapturedSpacecraftBefore, UFlareSimulatedSpacecraft* CapturedSpacecraftAfter);

	virtual void OnTravelStarted(UFlareTravel* Travel);


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
	UFlareQuestConditionGroup*           TriggerCondition;

	UPROPERTY()
	UFlareQuestConditionGroup*           ExpirationCondition;

	UPROPERTY()
	TArray<UFlareQuestAction*>				SuccessActions;
	UPROPERTY()
	TArray<UFlareQuestAction*>				FailActions;

	bool									TrackObjectives;
	FName									Identifier;
	FText									QuestName;
	FText                                   QuestDescription;
	EFlareQuestCategory::Type               QuestCategory;
	UFlareCompany*							Client;
	bool									Accepted;

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

	FText GetQuestReward();

	FText GetQuestPenalty();

	FText GetQuestExpiration();

	inline EFlareQuestCategory::Type GetQuestCategory() const
	{
		return QuestCategory;
	}

	inline EFlareQuestStatus::Type GetStatus() const
	{
		return QuestStatus;
	}

	FName GetQuestNotificationTag() const;

	FText GetStatusText() const;

	inline UFlareQuestStep* GetCurrentStep() const
	{
		return CurrentStep;
	}

	TArray<UFlareQuestStep*>& GetSteps()
	{
		return Steps;
	}

	TArray<UFlareQuestAction*>&	GetSuccessActions()
	{
		return SuccessActions;
	}

	TArray<UFlareQuestAction*>&	GetFailActions()
	{
		return FailActions;
	}

	int32 GetSuccessfullStepCount() const
	{
		return SuccessfullSteps.Num();
	}

	int32 GetStepCount() const
	{
		return Steps.Num();
	}

	bool IsAccepted() const
	{
		return Accepted;
	}

	bool HasAutoAccept();

	UFlareQuestManager*	GetQuestManager()
	{
		return QuestManager;
	}

	FFlareBundle& GetSaveBundle()
	{
		return QuestData.Data;
	}

	int64 GetAvailableDate()
	{
		return QuestData.AvailableDate;
	}

	UFlareCompany* GetClient()
	{
		return Client;
	}

	bool IsActive();

	UFlareSimulatedSector* FindSector(FName SectorIdentifier);
};
