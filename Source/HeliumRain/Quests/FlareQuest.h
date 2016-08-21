#pragma once

#include "FlareQuestManager.h"
#include "FlareQuest.generated.h"


/** Quest action type */
UENUM()
namespace EFlareQuestStatus
{
	enum Type
	{
		AVAILABLE,
		ACTIVE,
		SUCCESSFUL,
		ABANDONNED, // Use ReferenceIdentifier as sector identifier
		FAILED // Use ReferenceIdentifier as sector identifier
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

/** Quest action type */
UENUM()
namespace EFlareQuestAction
{
	enum Type
	{
		/** Use Identifier1  as sector identifier*/
		DISCOVER_SECTOR,
		/** Use Identifier1  as sector identifier*/
		VISIT_SECTOR,
		/** Use MessagesParameter  as text*/
		PRINT_MESSAGE
	};
}


/** Quest condition type */
UENUM()
namespace EFlareQuestCondition
{
	enum Type
	{
		/** Use Identifier1 as shared condition identifier*/
		SHARED_CONDITION,
		/** Use Identifier1 as the ship class if need to fly a specific ship class*/
		FLYING_SHIP,
		/** Use Identifier1 as the sector identifier*/
		SECTOR_VISITED,
		/** Use Identifier1 as the sector identifier*/
		SECTOR_ACTIVE,
		/** Use Identifier1 as the ship taf if need to track a specific ship*/
		SHIP_ALIVE,
		/** Use FloatParam1 as velocity */
		SHIP_MIN_COLLINEAR_VELOCITY,
		/** Use FloatParam1 as velocity */
		SHIP_MAX_COLLINEAR_VELOCITY,
		/** Use FloatParam1 as angular velocity*/
		SHIP_MIN_PITCH_VELOCITY,
		/** Use FloatParam1 as angular velocity*/
		SHIP_MAX_PITCH_VELOCITY,
		/** Use FloatParam1 as angular velocity*/
		SHIP_MIN_YAW_VELOCITY,
		/** Use FloatParam1 as angular velocity*/
		SHIP_MAX_YAW_VELOCITY,
		/** Use FloatParam1 as angular velocity*/
		SHIP_MIN_ROLL_VELOCITY,
		/** Use FloatParam1 as angular velocity*/
		SHIP_MAX_ROLL_VELOCITY,
		/** Use FloatParam1 as dot product value*/
		SHIP_MIN_COLLINEARITY,
		/** Use FloatParam1 as dot product value*/
		SHIP_MAX_COLLINEARITY,
		/** Use FloatListParam as radius in meter and VectorListParam as relative target location */
		SHIP_FOLLOW_RELATIVE_WAYPOINTS,
		/** Use Identifier1 as reference quest identifier */
		QUEST_SUCCESSFUL,
		/** Use Identifier1 as reference quest identifier */
		QUEST_FAILED
	};
}

/** Quest condition */
USTRUCT()
struct FFlareQuestConditionDescription
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Quest)
	TEnumAsByte<EFlareQuestCondition::Type> Type;

	/** The condition identifier is usefull only for non stateless condition*/
	UPROPERTY(EditAnywhere, Category = Quest)
	FName ConditionIdentifier;

	UPROPERTY(EditAnywhere, Category = Quest)
	FName Identifier1;

	UPROPERTY(EditAnywhere, Category = Quest)
	float FloatParam1;

	UPROPERTY(EditAnywhere, Category = Quest)
	TArray<float> FloatListParam;

	UPROPERTY(EditAnywhere, Category = Quest)
	TArray<FVector> VectorListParam;
};


/** Quest share condition. A list of condition used by mutiple steps */
USTRUCT()
struct FFlareSharedQuestCondition
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere, Category = Quest)
	FName Identifier;

	/** Shared conditions list*/
	UPROPERTY(EditAnywhere, Category = Quest)
	TArray<FFlareQuestConditionDescription> Conditions;

};

/** Quest condition */
USTRUCT()
struct FFlareQuestActionDescription
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere, Category = Quest)
	TEnumAsByte<EFlareQuestAction::Type> Type;

	UPROPERTY(EditAnywhere, Category = Quest)
	FName Identifier1;

	UPROPERTY(EditAnywhere, Category = Quest)
	FText MessagesParameter;
};

/** Quest step */
USTRUCT()
struct FFlareQuestStepDescription
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere, Category = Quest)
	FName Identifier;

	UPROPERTY(EditAnywhere, Category = Quest)
	FText StepDescription;

	/** End conditions */
	UPROPERTY(EditAnywhere, Category = Quest)
	TArray<FFlareQuestConditionDescription> EndConditions;

	/** Step preconditions */
	UPROPERTY(EditAnywhere, Category = Quest)
	TArray<FFlareQuestConditionDescription> EnabledConditions;

	/** Block end conditions */
	UPROPERTY(EditAnywhere, Category = Quest)
	TArray<FFlareQuestConditionDescription> BlockConditions;

	/** Fail conditions */
	UPROPERTY(EditAnywhere, Category = Quest)
	TArray<FFlareQuestConditionDescription> FailConditions;

	/** Init actions */
	UPROPERTY(EditAnywhere, Category = Quest)
	TArray<FFlareQuestActionDescription> InitActions;

	/** End actions */
	UPROPERTY(EditAnywhere, Category = Quest)
	TArray<FFlareQuestActionDescription> EndActions;

};

/** Quest description */
USTRUCT()
struct FFlareQuestDescription
{
	GENERATED_USTRUCT_BODY()
	/** Quest identifier */
	UPROPERTY(EditAnywhere, Category = Quest)
	FName Identifier;

	UPROPERTY(EditAnywhere, Category = Quest)
	FText QuestName;

	UPROPERTY(EditAnywhere, Category = Quest)
	FText QuestDescription;

	UPROPERTY(EditAnywhere, Category = Quest)
	FText QuestEndedDescription;

	/** Is tutorial quests. Will not be displayed if tutorial is disabled */
	UPROPERTY(EditAnywhere, Category = Quest)
	TEnumAsByte<EFlareQuestCategory::Type> Category;

	/** Can be reset by the player if he want to play again the quest from the begining */
	UPROPERTY(EditAnywhere, Category = Quest)
	bool Resettable;

	/** Quest availability condition */
	UPROPERTY(EditAnywhere, Category = Quest)
	TArray<FFlareQuestConditionDescription> Triggers;

	/** Quest step list */
	UPROPERTY(EditAnywhere, Category = Quest)
	TArray<FFlareQuestStepDescription> Steps;

	/** Quest of shard conditions */
	UPROPERTY(EditAnywhere, Category = Quest)
	TArray<FFlareSharedQuestCondition> SharedConditions;

	/** Fail actions */
	UPROPERTY(EditAnywhere, Category = Quest)
	TArray<FFlareQuestActionDescription> FailActions;

	/** Success actions (public) */
	UPROPERTY(EditAnywhere, Category = Quest)
	TArray<FFlareQuestActionDescription> SuccessActions;
};

struct FFlarePlayerObjectiveData;

/** Quest */
UCLASS()
class HELIUMRAIN_API UFlareQuest: public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Save
	----------------------------------------------------*/

	/** Load the quest from description file */
	virtual void Load(const FFlareQuestDescription* Description);

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

	virtual void Fail();

	virtual void Activate();

	virtual bool CheckConditions(const TArray<FFlareQuestConditionDescription>& Conditions, bool EmptyResult);

	virtual bool CheckCondition(const FFlareQuestConditionDescription* Condition, bool EmptyResult);

	virtual void PerformActions(const TArray<FFlareQuestActionDescription>& Actions);

	virtual void PerformAction(const FFlareQuestActionDescription* Action);

	virtual FText FormatTags(FText Message);

	virtual void SendQuestNotification(FText Message, FName Tag);

	virtual FFlareQuestStepProgressSave* CreateStepProgressSave(const FFlareQuestConditionDescription* Condition);

	/*----------------------------------------------------
		Objective tracking
	----------------------------------------------------*/

	virtual void StartObjectiveTracking();

	virtual void StopObjectiveTracking();

	virtual void UpdateObjectiveTracker();

	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData, const TArray<FFlareQuestConditionDescription>& Conditions);

	virtual void GenerateConditionCollinearityObjective(FFlarePlayerObjectiveData* ObjectiveData, EFlareQuestCondition::Type ConditionType, float TargetCollinearity);

	/*----------------------------------------------------
		Callback
	----------------------------------------------------*/

	virtual TArray<EFlareQuestCallback::Type> GetCurrentCallbacks();

	virtual TArray<EFlareQuestCallback::Type> GetConditionCallbacks(const FFlareQuestConditionDescription* Condition);

	virtual void AddConditionCallbacks(TArray<EFlareQuestCallback::Type>& Callbacks, const TArray<FFlareQuestConditionDescription>& Conditions);

	virtual void OnFlyShip(AFlareSpacecraft* Ship);

	virtual void OnSectorActivation(UFlareSimulatedSector* Sector);

	virtual void OnSectorVisited(UFlareSimulatedSector* Sector);

	virtual void OnTick(float DeltaSeconds);

	virtual void OnQuestStatusChanged(UFlareQuest* Quest);


protected:

   /*----------------------------------------------------
	   Protected data
   ----------------------------------------------------*/

	FFlareQuestProgressSave					QuestData;
	EFlareQuestStatus::Type					QuestStatus;

	const FFlareQuestDescription*			QuestDescription;
	const FFlareQuestStepDescription*		CurrentStepDescription;
	UFlareQuestManager*						QuestManager;

	bool									TrackObjectives;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline FName GetIdentifier() const
	{
		return QuestDescription->Identifier;
	}

	inline FText GetQuestName() const
	{
		return QuestDescription->QuestName;
	}

	inline const FFlareQuestDescription* GetQuestDescription() const
	{
		return QuestDescription;
	}

	inline EFlareQuestStatus::Type GetStatus() const
	{
		return QuestStatus;
	}

	FText GetStatusText() const;

	const FFlareSharedQuestCondition* FindSharedCondition(FName SharedConditionIdentifier);

	inline const FFlareQuestStepDescription* GetCurrentStepDescription() const
	{
		return CurrentStepDescription;
	}

	FFlareQuestStepProgressSave* GetCurrentStepProgressSave(const FFlareQuestConditionDescription* Condition);

};
