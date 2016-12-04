
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
		/*case EFlareQuestCondition::SECTOR_ACTIVE:
			if (QuestManager->GetGame()->GetActiveSector() && QuestManager->GetGame()->GetActiveSector()->GetSimulatedSector()->GetIdentifier() == Condition->Identifier1)
			{
					Status = true;
			}
			break;
		case EFlareQuestCondition::SECTOR_VISITED:
			if (QuestManager->GetGame()->GetPC()->GetCompany()->HasVisitedSector(QuestManager->GetGame()->GetGameWorld()->FindSector(Condition->Identifier1)))
			{
					Status = true;
			}
			break;
		case EFlareQuestCondition::SHIP_MIN_COLLINEAR_VELOCITY:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
				float CollinearVelocity = FVector::DotProduct(Spacecraft->GetLinearVelocity(), Spacecraft->GetFrontVector());

				FFlareQuestStepProgressSave* ProgressSave = GetCurrentStepProgressSave(Condition);
				if (!ProgressSave)
				{
					ProgressSave = CreateStepProgressSave(Condition);
					ProgressSave->CurrentProgression = 0;
					ProgressSave->InitialVelocity = CollinearVelocity;
				}

				Status = CollinearVelocity > Condition->FloatParam1;
			}
			break;
		case EFlareQuestCondition::SHIP_MAX_COLLINEAR_VELOCITY:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
				float CollinearVelocity = FVector::DotProduct(Spacecraft->GetLinearVelocity(), Spacecraft->GetFrontVector());

				FFlareQuestStepProgressSave* ProgressSave = GetCurrentStepProgressSave(Condition);
				if (!ProgressSave)
				{
					ProgressSave = CreateStepProgressSave(Condition);
					ProgressSave->CurrentProgression = 0;
					ProgressSave->InitialVelocity = CollinearVelocity;
				}

				Status = CollinearVelocity < Condition->FloatParam1;
			}
			break;
		case EFlareQuestCondition::SHIP_MIN_COLLINEARITY:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
				if (Spacecraft->GetLinearVelocity().IsNearlyZero())
				{
					Status = false;
				}
				else
				{
					Status = (FVector::DotProduct(Spacecraft->GetLinearVelocity().GetUnsafeNormal(), Spacecraft->GetFrontVector()) > Condition->FloatParam1);
				}
			}
			break;
		case EFlareQuestCondition::SHIP_MAX_COLLINEARITY:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
				if (Spacecraft->GetLinearVelocity().IsNearlyZero())
				{
					Status = false;
				}
				else
				{
					Status = (FVector::DotProduct(Spacecraft->GetLinearVelocity().GetUnsafeNormal(), Spacecraft->GetFrontVector()) < Condition->FloatParam1);
				}
			}
			break;
		case EFlareQuestCondition::SHIP_MIN_PITCH_VELOCITY:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
				FVector WorldAngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocity();
				FVector LocalAngularVelocity = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(WorldAngularVelocity);
				Status = (LocalAngularVelocity.Y > Condition->FloatParam1);
			}
			break;
		case EFlareQuestCondition::SHIP_MAX_PITCH_VELOCITY:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
				FVector WorldAngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocity();
				FVector LocalAngularVelocity = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(WorldAngularVelocity);
				Status = (LocalAngularVelocity.Y < Condition->FloatParam1);
			}
			break;
		case EFlareQuestCondition::SHIP_MIN_YAW_VELOCITY:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
				FVector WorldAngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocity();
				FVector LocalAngularVelocity = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(WorldAngularVelocity);
				Status = (LocalAngularVelocity.Z > Condition->FloatParam1);
			}
			break;
		case EFlareQuestCondition::SHIP_MAX_YAW_VELOCITY:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
				FVector WorldAngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocity();
				FVector LocalAngularVelocity = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(WorldAngularVelocity);
				Status = (LocalAngularVelocity.Z < Condition->FloatParam1);
			}
			break;
		case EFlareQuestCondition::SHIP_MIN_ROLL_VELOCITY:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
				FVector WorldAngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocity();
				FVector LocalAngularVelocity = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(WorldAngularVelocity);
				Status = (LocalAngularVelocity.X > Condition->FloatParam1);
			}
			break;
		case EFlareQuestCondition::SHIP_MAX_ROLL_VELOCITY:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
				FVector WorldAngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocity();
				FVector LocalAngularVelocity = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(WorldAngularVelocity);
				Status = (LocalAngularVelocity.X < Condition->FloatParam1);
			}
			break;
		case EFlareQuestCondition::SHIP_FOLLOW_RELATIVE_WAYPOINTS:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();

				FFlareQuestStepProgressSave* ProgressSave = GetCurrentStepProgressSave(Condition);

				if (!ProgressSave)
				{
					ProgressSave = CreateStepProgressSave(Condition);
					ProgressSave->CurrentProgression = 0;
					ProgressSave->InitialTransform = Spacecraft->Airframe->GetComponentTransform();
				}

				FVector InitialLocation = ProgressSave->InitialTransform.GetTranslation();
				FVector RelativeTargetLocation = Condition->VectorListParam[ProgressSave->CurrentProgression] * 100;
				FVector WorldTargetLocation = InitialLocation + ProgressSave->InitialTransform.GetRotation().RotateVector(RelativeTargetLocation);


				float MaxDistance = Condition->FloatListParam[ProgressSave->CurrentProgression] * 100;


				if (FVector::Dist(Spacecraft->GetActorLocation(), WorldTargetLocation) < MaxDistance)
				{
					// Nearing the target
					if (ProgressSave->CurrentProgression + 2 <= Condition->VectorListParam.Num())
					{
						// Progress.
						ProgressSave->CurrentProgression++;

						FText WaypointText = LOCTEXT("WaypointProgress", "Waypoint reached, {0} left");

						SendQuestNotification(FText::Format(WaypointText, FText::AsNumber(Condition->VectorListParam.Num() - ProgressSave->CurrentProgression)),
											  FName(*(FString("quest-")+GetIdentifier().ToString()+"-step-progress")));
					}
					else
					{
						// All waypoint reach
						Status = true;
					}

				}
			}
			break;
		case EFlareQuestCondition::SHIP_ALIVE:
			if (QuestManager->GetGame()->GetPC()->GetPlayerShip())
			{
				Status = QuestManager->GetGame()->GetPC()->GetPlayerShip()->GetDamageSystem()->IsAlive();
			}
			break;
		case EFlareQuestCondition::QUEST_SUCCESSFUL:
			Status = QuestManager->IsQuestSuccesfull(Condition->Identifier1);
			break;
		case EFlareQuestCondition::QUEST_FAILED:
			Status = QuestManager->IsQuestFailed(Condition->Identifier1);
			break;*/
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
