
#include "Flare.h"
#include "FlareQuest.h"

#define LOCTEXT_NAMESPACE "FlareQuest"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareQuest::UFlareQuest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


/*----------------------------------------------------
	Save
----------------------------------------------------*/


void UFlareQuest::Load(const FFlareQuestDescription* Description)
{
	QuestDescription = Description;
	QuestData.QuestIdentifier = QuestDescription->Identifier;
	QuestStatus = EFlareQuestStatus::AVAILABLE;
}

void UFlareQuest::Restore(const FFlareQuestProgressSave& Data)
{
	QuestData = Data;
	QuestStatus = EFlareQuestStatus::ACTIVE;
}

FFlareQuestProgressSave* UFlareQuest::Save()
{
	// TODO Save
	return &QuestData;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareQuest::SetStatus(EFlareQuestStatus::Type Status)
{
	QuestStatus = Status;
}

/*----------------------------------------------------
	Callback
----------------------------------------------------*/


TArray<EFlareQuestCallback::Type> UFlareQuest::GetCurrentCallbacks()
{
	TArray<EFlareQuestCallback::Type> Callbacks;

	switch(QuestStatus)
	{
		case EFlareQuestStatus::AVAILABLE:
			// Use trigger conditions
			for (int TriggerIndex = 0; TriggerIndex < QuestDescription->Triggers.Num(); TriggerIndex++)
			{
				TArray<EFlareQuestCallback::Type> ConditionCallbacks = GetConditionCallbacks(&QuestDescription->Triggers[TriggerIndex]);
				for (int CallbackIndex = 0; CallbackIndex < ConditionCallbacks.Num(); CallbackIndex++)
				{
					Callbacks.AddUnique(ConditionCallbacks[CallbackIndex]);
				}
			}

			if(Callbacks.Contains(EFlareQuestCallback::TICK))
			{
				FLOGV("WARNING: The quest %s need a TICK callback as trigger", *GetIdentifier().ToString());
			}
			break;
		case EFlareQuestStatus::ACTIVE:
			// Use current step conditions
			// TODO
			break;
		default:
			// Don't add callback in others cases
			break;
	}

	return Callbacks;
}

TArray<EFlareQuestCallback::Type> UFlareQuest::GetConditionCallbacks(const FFlareQuestConditionDescription* Condition)
{
	TArray<EFlareQuestCallback::Type> Callbacks;

	switch(Condition->Type)
	{
		case EFlareQuestCondition::SHARED_CONDITION:
		{
			const FFlareSharedQuestCondition* SharedCondition = FindSharedCondition(Condition->ReferenceIdentifier);
			if(SharedCondition)
			{
				for (int ConditionIndex = 0; ConditionIndex < SharedCondition->Conditions.Num(); ConditionIndex++)
				{
					TArray<EFlareQuestCallback::Type> ConditionCallbacks = GetConditionCallbacks(&SharedCondition->Conditions[ConditionIndex]);
					for (int CallbackIndex = 0; CallbackIndex < ConditionCallbacks.Num(); CallbackIndex++)
					{
						Callbacks.AddUnique(ConditionCallbacks[CallbackIndex]);
					}
				}
			}
			break;
		}
		case EFlareQuestCondition::FLYING_SHIP:
			Callbacks.AddUnique(EFlareQuestCallback::FLY_SHIP);
			break;
		case EFlareQuestCondition::SHIP_MIN_COLLINEAR_VELOCITY:
		case EFlareQuestCondition::SHIP_MAX_COLLINEAR_VELOCITY:
		case EFlareQuestCondition::SHIP_ALIVE:
			Callbacks.AddUnique(EFlareQuestCallback::TICK);
			break;
		case EFlareQuestCondition::VALIDATE:
			// No callback
			break;
		default:
			FLOGV("ERROR: GetConditionCallbacks not implemented for condition type %d", (int)(Condition->Type +0));
			break;
	}

	return Callbacks;
}

void UFlareQuest::OnTick(float DeltaSeconds)
{
	// TODO
	FLOGV("TODO: %s OnTick", *QuestData.QuestIdentifier.ToString());
}

void UFlareQuest::OnFlyShip(AFlareSpacecraft* Ship)
{
	//TODO
	FLOGV("TODO: %s OnFlyShip", *QuestData.QuestIdentifier.ToString());
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

const FFlareSharedQuestCondition* UFlareQuest::FindSharedCondition(FName SharedConditionIdentifier)
{
	for (int SharedConditionIndex = 0; SharedConditionIndex < QuestDescription->SharedCondition.Num(); SharedConditionIndex++)
	{
		if(QuestDescription->SharedCondition[SharedConditionIndex].Identifier == SharedConditionIdentifier)
		{
			return &QuestDescription->SharedCondition[SharedConditionIndex];
		}
	}

	FLOGV("ERROT: The quest %s don't have shared condition named %s", *GetIdentifier().ToString(), *SharedConditionIdentifier.ToString());

	return NULL;
}


#undef LOCTEXT_NAMESPACE
