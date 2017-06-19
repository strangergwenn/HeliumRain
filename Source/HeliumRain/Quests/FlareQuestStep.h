#pragma once

#include "FlareQuestCondition.h"
#include "FlareQuestStep.generated.h"

class UFlareQuest;
class UFlareQuestAction;
class UFlareQuestCondition;
class UFlareQuestConditionGroup;
struct FFlarePlayerObjectiveData;

/** Quest action type */
UENUM()
namespace EFlareQuestStepStatus
{
	enum Type
	{
		PENDING,
		DISABLED,
		ENABLED, // Permanent after pending
		BLOCKED,
		COMPLETED, // Permanent
		FAILED // Permanent
	};
}

/** A quest Step */
UCLASS()
class HELIUMRAIN_API UFlareQuestStep: public UObject
{
	GENERATED_UCLASS_BODY()

public:

	static UFlareQuestStep* Create(UFlareQuest* Parent, FName Identifier, FText Description);


	void Init();

	/** Restore the quest step status from a save file */
	virtual void Restore(const TArray<FFlareQuestConditionSave>& Data);

	/** Save the quest step status to a save file */
	virtual void Save(TArray<FFlareQuestConditionSave>& Data);

	void UpdateState();

	void PerformInitActions();
	void PerformEndActions();

	void AddEndConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	void SetupStepIndexes(int32 Index);

	void SetStatus(EFlareQuestStepStatus::Type NewStatus)
	{
		Status = NewStatus;
	}

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	FName									Identifier;
	FText                                   StepDescription;
	int32                                   StepIndex;

	UPROPERTY()
	UFlareQuestCondition*           EnableCondition;
	UPROPERTY()
	UFlareQuestCondition*           BlockCondition;
	UPROPERTY()
	UFlareQuestCondition*           FailCondition;
	UPROPERTY()
	UFlareQuestCondition*           EndCondition;
	UPROPERTY()
	TArray<UFlareQuestAction*>				InitActions;
	UPROPERTY()
	TArray<UFlareQuestAction*>				EndActions;


	EFlareQuestStepStatus::Type					Status;

public:

	/*----------------------------------------------------
	 Getters
	----------------------------------------------------*/

	inline FName GetIdentifier() const
	{
		return Identifier;
	}

	FText GetStepDescription()
	{
		return StepDescription;
	}

	int32 GetStepIndex()
	{
		return StepIndex;
	}

	UFlareQuestCondition* GetEnableCondition()
	{
		return EnableCondition;
	}

	UFlareQuestCondition* GetBlockCondition()
	{
		return BlockCondition;
	}

	UFlareQuestCondition* GetFailCondition()
	{
		return FailCondition;
	}

	UFlareQuestCondition* GetEndCondition()
	{
		return EndCondition;
	}

	TArray<UFlareQuestAction*>& GetInitActions()
	{
		return InitActions;
	}

	TArray<UFlareQuestAction*>& GetEndActions()
	{
		return EndActions;
	}

	EFlareQuestStepStatus::Type GetStatus()
	{
		return Status;
	}

	bool IsFailed()
	{
		return Status == EFlareQuestStepStatus::FAILED;
	}

	bool IsCompleted()
	{
		return Status == EFlareQuestStepStatus::COMPLETED;
	}

	void SetEndCondition(UFlareQuestCondition* EndConditionParam)
	{
		EndCondition = EndConditionParam;
	}
};
