#pragma once

#include "../Flare.h"
#include "../Quests/FlareQuest.h"
#include "FlareQuestCatalogEntry.generated.h"

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

UCLASS()
class HELIUMRAIN_API UFlareQuestCatalogEntry : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Quest data */
	UPROPERTY(EditAnywhere, Category = Content)
	FFlareQuestDescription Data;

};
