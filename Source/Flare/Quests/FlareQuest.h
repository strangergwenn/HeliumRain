#pragma once

#include "FlareQuest.generated.h"

/** Quest action type */
UENUM()
namespace EFlareQuestAction
{
	enum Type
	{
		DISCOVER_SECTOR, // Use ReferenceIdentifier as sector identifier
		VISIT_SECTOR // Use ReferenceIdentifier as sector identifier
	};
}


/** Quest condition type */
UENUM()
namespace EFlareQuestCondition
{
	enum Type
	{
		SHARED_CONDITION, // Use ReferenceIdentifier as shared condition identifier
		VALIDATE, // Juste need to validate the step
		FLYING_SHIP, // Use ReferenceIdentifier as the ship class if need to fly a specific ship class
		SHIP_ALIVE,// Use ReferenceIdentifier as the ship taf if need to track a specific ship
		SHIP_MIN_COLLINEAR_VELOCITY, // Use velocity as min velocity
		SHIP_MAX_COLLINEAR_VELOCITY // Use velocity as max velocity
	};
}

/** Quest condition */
USTRUCT()
struct FFlareQuestConditionDescription
{
	GENERATED_USTRUCT_BODY()

	/** The condition identifier is usefull only for non stateless condition*/
	UPROPERTY(EditAnywhere, Category = Quest)
	FName Identifier;

	UPROPERTY(EditAnywhere, Category = Quest)
	TEnumAsByte<EFlareQuestCondition::Type> Type;

	UPROPERTY(EditAnywhere, Category = Quest)
	FName ReferenceIdentifier;

	UPROPERTY(EditAnywhere, Category = Quest)
	float Velocity;
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
	FName ReferenceIdentifier;
};


/** Quest step */
USTRUCT()
struct FFlareQuestStepDescription
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere, Category = Quest)
	FName Identifier;

	UPROPERTY(EditAnywhere, Category = Quest)
	FString StepDescription;

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
	FString Name;

	UPROPERTY(EditAnywhere, Category = Quest)
	FString Description;

	UPROPERTY(EditAnywhere, Category = Quest)
	FString EndedDescription;

	/** Is tutorial quests. Will not be displayed if tutorial is disabled */
	UPROPERTY(EditAnywhere, Category = Quest)
	bool IsTutorial;

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
	TArray<FFlareSharedQuestCondition> SharedCondition;
};


/** Quest */
UCLASS()
class FLARE_API UFlareQuest: public UObject
{
	GENERATED_UCLASS_BODY()

public:


};
