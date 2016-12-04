
#include "Flare.h"
#include "../Game/FlareGame.h"
#include "FlareCatalogQuest.h"
#include "FlareQuestCondition.h"

#define LOCTEXT_NAMESPACE "FlareCatalogQuest"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareCatalogQuest::UFlareCatalogQuest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareCatalogQuest::Load(UFlareQuestManager* Parent, const FFlareQuestDescription* Description)
{
	LoadInternal(Parent);

	CatalogDescription = Description;

	// Prefix all quest id to avoid collission
	Identifier = FName(*(FString("catalog-") + Description->Identifier.ToString()));
	QuestName = Description->QuestName;
	QuestDescription = Description->QuestDescription;
	QuestCategory = Description->Category;
	QuestDescription = Description->QuestDescription;

	for(const FFlareQuestConditionDescription& ConditionDescription : Description->Triggers)
	{
		TriggerConditions.Append(GenerateCatalogCondition(ConditionDescription));
	}

	for(const FFlareQuestActionDescription& ActionDescription : Description->SuccessActions)
	{
		SuccessActions.Add(GenerateCatalogAction(ActionDescription));
	}

	for(const FFlareQuestActionDescription& ActionDescription : Description->FailActions)
	{
		FailActions.Add(GenerateCatalogAction(ActionDescription));
	}

	for(const FFlareQuestStepDescription& StepDescription : Description->Steps)
	{
		Steps.Add(GenerateCatalogStep(StepDescription));
	}
}

TArray<UFlareQuestCondition*> UFlareCatalogQuest::GenerateCatalogCondition(const FFlareQuestConditionDescription& ConditionDescription)
{
	TArray<UFlareQuestCondition*> Conditions;

	switch(ConditionDescription.Type)
	{
		case EFlareQuestCondition::SHARED_CONDITION:
		{
			const FFlareSharedQuestCondition* SharedCondition = FindSharedCondition(ConditionDescription.Identifier1);
			if (SharedCondition)
			{
				for (const FFlareQuestConditionDescription& ChildConditionDescription : SharedCondition->Conditions)
				{
					Conditions.Append(GenerateCatalogCondition(ChildConditionDescription));
				}
			}
			else
			{
				FLOGV("ERROR: GenerateCatalogCondition fail to find shared condition '%s' for quest '%s'",
					  *ConditionDescription.Identifier1.ToString(),
					  *Identifier.ToString());
			}
			break;
		}
		case EFlareQuestCondition::FLYING_SHIP:
			Conditions.Add(UFlareQuestConditionFlyingShipClass::Create(this, ConditionDescription.Identifier1));
			break;
		case EFlareQuestCondition::SECTOR_ACTIVE:
		{
			UFlareSimulatedSector* Sector = QuestManager->GetGame()->GetGameWorld()->FindSector(ConditionDescription.Identifier1);
			if(Sector)
			{
				Conditions.Add(UFlareQuestConditionSectorActive::Create(this, Sector));
			}
			else
			{
				FLOGV("ERROR: GenerateCatalogCondition fail to find sector '%s' for quest '%s'",
					  *ConditionDescription.Identifier1.ToString(),
					  *Identifier.ToString());
			}
			break;
		}
		case EFlareQuestCondition::SECTOR_VISITED:
		{
			UFlareSimulatedSector* Sector = QuestManager->GetGame()->GetGameWorld()->FindSector(ConditionDescription.Identifier1);
			if(Sector)
			{
				Conditions.Add(UFlareQuestConditionSectorVisited::Create(this, Sector));
			}
			else
			{
				FLOGV("ERROR: GenerateCatalogCondition fail to find sector '%s' for quest '%s'",
					  *ConditionDescription.Identifier1.ToString(),
					  *Identifier.ToString());
			}
			break;
		}
		case EFlareQuestCondition::SHIP_MIN_COLLINEAR_VELOCITY:
			Conditions.Add(UFlareQuestConditionMinCollinearVelocity::Create(this, ConditionDescription.FloatParam1));
			break;
		case EFlareQuestCondition::SHIP_MAX_COLLINEAR_VELOCITY:
			Conditions.Add(UFlareQuestConditionMaxCollinearVelocity::Create(this, ConditionDescription.FloatParam1));
			break;
		case EFlareQuestCondition::SHIP_MIN_COLLINEARITY:
			Conditions.Add(UFlareQuestConditionMinCollinear::Create(this, ConditionDescription.FloatParam1));
			break;
		case EFlareQuestCondition::SHIP_MAX_COLLINEARITY:
			Conditions.Add(UFlareQuestConditionMaxCollinear::Create(this, ConditionDescription.FloatParam1));
			break;
		case EFlareQuestCondition::SHIP_MIN_PITCH_VELOCITY:
			Conditions.Add(UFlareQuestConditionMinRotationVelocity::Create(this, FVector(0,1,0), ConditionDescription.FloatParam1));
			break;
		case EFlareQuestCondition::SHIP_MAX_PITCH_VELOCITY:
			Conditions.Add(UFlareQuestConditionMaxRotationVelocity::Create(this, FVector(0,1,0), ConditionDescription.FloatParam1));
			break;
		case EFlareQuestCondition::SHIP_MIN_YAW_VELOCITY:
			Conditions.Add(UFlareQuestConditionMinRotationVelocity::Create(this, FVector(0,0,1), ConditionDescription.FloatParam1));
			break;
		case EFlareQuestCondition::SHIP_MAX_YAW_VELOCITY:
			Conditions.Add(UFlareQuestConditionMaxRotationVelocity::Create(this, FVector(0,0,1), ConditionDescription.FloatParam1));
			break;
		case EFlareQuestCondition::SHIP_MIN_ROLL_VELOCITY:
			Conditions.Add(UFlareQuestConditionMinRotationVelocity::Create(this, FVector(1,0,0), ConditionDescription.FloatParam1));
			break;
		case EFlareQuestCondition::SHIP_MAX_ROLL_VELOCITY:
			Conditions.Add(UFlareQuestConditionMaxRotationVelocity::Create(this, FVector(1,0,0), ConditionDescription.FloatParam1));
			break;
		case EFlareQuestCondition::SHIP_FOLLOW_RELATIVE_WAYPOINTS:
			Conditions.Add(UFlareQuestConditionFollowRelativeWaypoints::Create(this, FVector(1,0,0), ConditionDescription.VectorListParam));
			break;
		case EFlareQuestCondition::SHIP_ALIVE:
			FLOG("SHIP_ALIVE condition deprecated")
			break;
		case EFlareQuestCondition::QUEST_SUCCESSFUL:
		{
			UFlareQuest* Quest = QuestManager->FindQuest(ConditionDescription.Identifier1);
			if(Quest)
			{
				Conditions.Add(UFlareQuestConditionQuestSuccessful::Create(this, Quest));
			}
			else
			{
				FLOGV("ERROR: GenerateCatalogCondition fail to find quest '%s' for quest '%s'",
					  *ConditionDescription.Identifier1.ToString(),
					  *Identifier.ToString());
			}

			break;
		}
		case EFlareQuestCondition::QUEST_FAILED:
		{
			UFlareQuest* Quest = QuestManager->FindQuest(ConditionDescription.Identifier1);
			if(Quest)
			{
				Conditions.Add(UFlareQuestConditionQuestFailed::Create(this, Quest));
			}
			else
			{
				FLOGV("ERROR: GenerateCatalogCondition fail to find quest '%s' for quest '%s'",
					  *ConditionDescription.Identifier1.ToString(),
					  *Identifier.ToString());
			}

			break;
		}
		default:
			FLOGV("ERROR: CheckCondition not implemented for condition type %d", (int)(ConditionDescription.Type +0));
			break;
	}

	return Conditions;
}

UFlareQuestAction* UFlareCatalogQuest::GenerateCatalogAction(const FFlareQuestActionDescription& ActionDescription)
{
	// TODO
	return NULL;
}


UFlareQuestStep* UFlareCatalogQuest::GenerateCatalogStep(const FFlareQuestStepDescription& StepDescription)
{
	// TODO
	return NULL;
}


/*----------------------------------------------------
	Internal getters
----------------------------------------------------*/
const FFlareSharedQuestCondition* UFlareCatalogQuest::FindSharedCondition(FName SharedConditionIdentifier)
{
	for (int SharedConditionIndex = 0; SharedConditionIndex < CatalogDescription->SharedConditions.Num(); SharedConditionIndex++)
	{
		if (CatalogDescription->SharedConditions[SharedConditionIndex].Identifier == SharedConditionIdentifier)
		{
			return &CatalogDescription->SharedConditions[SharedConditionIndex];
		}
	}

	FLOGV("ERROR: The quest %s doesn't have shared condition named %s", *GetIdentifier().ToString(), *SharedConditionIdentifier.ToString());

	return NULL;
}
#undef LOCTEXT_NAMESPACE
