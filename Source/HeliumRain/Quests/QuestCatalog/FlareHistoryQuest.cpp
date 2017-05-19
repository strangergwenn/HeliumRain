
#include "FlareHistoryQuest.h"

#include "Flare.h"
#include "../FlareQuestCondition.h"
#include "Game/FlareGame.h"
#include "Game/FlareScenarioTools.h"
#include "Economy/FlareCargoBay.h"
#include "Data/FlareSpacecraftCatalog.h"
#include "Player/FlarePlayerController.h"

#define LOCTEXT_NAMESPACE "FlareHistotyQuest"

static FName HISTORY_CURRENT_PROGRESSION_TAG("current-progression");


/*----------------------------------------------------
	Quest pendulum
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "Pendulum"
UFlareQuestPendulum::UFlareQuestPendulum(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestPendulum::Create(UFlareQuestManager* Parent)
{
	UFlareQuestPendulum* Quest = NewObject<UFlareQuestPendulum>(Parent, UFlareQuestPendulum::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestPendulum::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "pendulum";
	QuestName = LOCTEXT(QUEST_TAG"Name","Pendulum");
	QuestDescription = LOCTEXT(QUEST_TAG"Description","Help has been requested to all companies around Nema.");
	QuestCategory = EFlareQuestCategory::TUTORIAL;

	UFlareSimulatedSector* Pendulum = FindSector("pendulum");
	if (!Pendulum)
	{
		return;
	}
	UFlareSimulatedSector* BlueHeart = FindSector("blue-heart");
	if (!BlueHeart)
	{
		return;
	}
	UFlareSimulatedSector* TheSpire = FindSector("the-spire");
	if (!TheSpire)
	{
		return;
	}


	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"DockAt"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description", "You are summoned by the Nema Colonial Administration to a meeting on the Pendulum project. Please dock to an habitation center in Blue Heart.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "meeting", Description);

		UFlareQuestConditionDockAtType* Condition = UFlareQuestConditionDockAtType::Create(this, BlueHeart, "station-bh-habitation");

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		Steps.Add(Step);
	}


	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"TravelToSpire"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Thanks for attending this meeting. As you may have observed, the Spire, our only source of gas, can't match our needs anymore."
									"\nWe need to build a new orbital extractor. However, the Spire was built before the war and all the knowledge disappeared when the Daedelus was shot to pieces. Your help is needed to build a new one."
									"\nPlease start reverse-engineering the Spire so that we can learn more.");

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "travel-to-spire", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionVisitSector::Create(this, TheSpire));
		Steps.Add(Step);
	}


	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Inspect"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Inspect The Spire in details to gather information about its materials and construction method.");

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "inspect-the-spire", Description);

		TArray<FVector> Waypoints;
		Waypoints.Add(FVector(0,0,-211084));
		Waypoints.Add(FVector(0,0,-98414));
		Waypoints.Add(FVector(-37257,28772,285917));
		Waypoints.Add(FVector(-7009,-47286,290234));
		Waypoints.Add(FVector(0,0,82801));
		Waypoints.Add(FVector(0,0,13138));
		Waypoints.Add(FVector(4176.675781,-486.620117,4294.422363));
		Waypoints.Add(FVector(1422.779175,449.966492,-12945.479492));

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionWaypoints::Create(this, QUEST_TAG"cond1", TheSpire, Waypoints));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"DataReturn"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description", "This is great data ! Come back to our scientists in Blue Heart to see what they make of it.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "data-return", Description);

		UFlareQuestConditionDockAtType* Condition = UFlareQuestConditionDockAtType::Create(this, BlueHeart, "station-bh-habitation");

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		Steps.Add(Step);
	}


	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"research"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","The data you brought back will help us understand how the Spire was built. We need you to transform this knowledge into a usable technology to build a new space elevevator.");

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "research", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialUnlockTechnology::Create(this, "orbital-pumps"));
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialUnlockTechnology::Create(this, "advanced-stations"));

		Steps.Add(Step);
	}


	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"telescope"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Congratulations, you now have the blueprints for a new orbital extractor. Two things are still needed : some construction resources, and a massive counterweight for the space elevator. Build a telescope station to help finding a good sector.");

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "telescope", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestStationCount::Create(this, "station-telescope", 1));

		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"resources"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Bring construction resources to Blue Heart.");

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "resources", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestBringResource::Create(this, BlueHeart, "steel", 1000));
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestBringResource::Create(this, BlueHeart, "plastics", 1000));
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestBringResource::Create(this, BlueHeart, "tech", 500));
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestBringResource::Create(this, BlueHeart, "tools", 500));

		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"peace"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","We've started construction for the Pendulum elevator. We need to be cautious of pirate activity, since we gathered a lot of resources here."
																"\nReduce pirates to silence for a while. If needed, use a telescope to locate their base in the Boneyard.");

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "pirates", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareCompanyMaxCombatValue::Create(this, GetQuestManager()->GetGame()->GetScenarioTools()->Pirates, 0));
		Steps.Add(Step);
	}


	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"TravelToPendulum"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","The construction of Pendulum construction is finished, come take a look !");

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "travel-to-pendulum", Description);

		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Pendulum));

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionSectorVisited::Create(this, Pendulum));
		Steps.Add(Step);
	}


	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"pumps"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Welcome to the Pendulum. Other companies will come here too, but you helped build this extractor. Build some gas terminals while the administrative fees are low !");

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "pumps", Description);
		Step->SetEndCondition(UFlareQuestConditionOrGroup::Create(this, true));

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialBuildStation::Create(this, false, "station-ch4-pump", Pendulum));
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialBuildStation::Create(this, false, "station-h2-pump", Pendulum));
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialBuildStation::Create(this, false, "station-he3-pump", Pendulum));

		Steps.Add(Step);
	}

}


/*----------------------------------------------------
	Quest dock at type condition
----------------------------------------------------*/
UFlareQuestConditionDockAtType::UFlareQuestConditionDockAtType(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionDockAtType* UFlareQuestConditionDockAtType::Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector, FName StationType)
{
	UFlareQuestConditionDockAtType* Condition = NewObject<UFlareQuestConditionDockAtType>(ParentQuest, UFlareQuestConditionDockAtType::StaticClass());
	Condition->Load(ParentQuest, Sector, StationType);
	return Condition;
}

void UFlareQuestConditionDockAtType::Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector, FName StationType)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::SHIP_DOCKED);
	TargetStationType = StationType;
	TargetSector = Sector;
	Completed = false;

	FFlareSpacecraftDescription* Desc = GetGame()->GetSpacecraftCatalog()->Get(TargetStationType);

	if(Desc)
	{
		InitialLabel = FText::Format(LOCTEXT("DockAtTypeFormat", "Dock at a {0} in {1}"),
													Desc->Name,
													TargetSector->GetSectorName());
	}

}

bool UFlareQuestConditionDockAtType::IsCompleted()
{
	if (Completed)
	{
		return true;
	}

	if (GetGame()->GetPC()->GetPlayerShip() && GetGame()->GetPC()->GetPlayerShip()->IsActive())
	{
		AFlareSpacecraft* PlayerShip = GetGame()->GetPC()->GetPlayerShip()->GetActive();

		if(PlayerShip->GetNavigationSystem()->GetDockStation())
		{
		if (PlayerShip->GetNavigationSystem()->GetDockStation()->GetDescription()->Identifier == TargetStationType)
			{
				Completed = true;
				return true;
			}
		}
	}

	return false;
}

void UFlareQuestConditionDockAtType::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Counter = 0;
	ObjectiveCondition.MaxCounter = 0;
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
	ObjectiveData->TargetSectors.Add(TargetSector);
}

/*----------------------------------------------------
	Visit sector condition
----------------------------------------------------*/
UFlareQuestConditionVisitSector::UFlareQuestConditionVisitSector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionVisitSector* UFlareQuestConditionVisitSector::Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector)
{
	UFlareQuestConditionVisitSector* Condition = NewObject<UFlareQuestConditionVisitSector>(ParentQuest, UFlareQuestConditionVisitSector::StaticClass());
	Condition->Load(ParentQuest, Sector);
	return Condition;
}

void UFlareQuestConditionVisitSector::Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	TargetSector = Sector;
	InitialLabel = FText::Format(LOCTEXT("VisitSector", "Visit {0}"), TargetSector->GetSectorName());
}

void UFlareQuestConditionVisitSector::OnEvent(FFlareBundle& Bundle)
{
	if(Completed)
	{
		return;
	}

	if (Bundle.HasTag("travel-end") && Bundle.GetName("sector") == TargetSector->GetIdentifier())
	{
		Completed = true;
	}
}

bool UFlareQuestConditionVisitSector::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionVisitSector::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress =IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
	ObjectiveData->TargetSectors.Add(TargetSector);
}

/*----------------------------------------------------
	Follow absolute waypoints condition
----------------------------------------------------*/

#define ABSOLUTE_WAYPOINTS_RADIUS 2000

UFlareQuestConditionWaypoints::UFlareQuestConditionWaypoints(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionWaypoints* UFlareQuestConditionWaypoints::Create(UFlareQuest* ParentQuest, FName ConditionIdentifier, UFlareSimulatedSector* Sector, TArray<FVector> VectorListParam)
{
	UFlareQuestConditionWaypoints*Condition = NewObject<UFlareQuestConditionWaypoints>(ParentQuest, UFlareQuestConditionWaypoints::StaticClass());
	Condition->Load(ParentQuest, ConditionIdentifier, Sector, VectorListParam);
	return Condition;
}

void UFlareQuestConditionWaypoints::Load(UFlareQuest* ParentQuest, FName ConditionIdentifier, UFlareSimulatedSector* Sector, TArray<FVector> VectorListParam)
{
	if (ConditionIdentifier == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionWaypoints need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifier);
	Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
	VectorList = VectorListParam;
	TargetSector = Sector;
	InitialLabel = FText::Format(LOCTEXT("FollowWaypointsInSector", "Fly to waypoints in {0}"), TargetSector->GetSectorName());
}

void UFlareQuestConditionWaypoints::Restore(const FFlareBundle* Bundle)
{
	bool HasSave = true;
	if(Bundle)
	{
		HasSave &= Bundle->HasInt32(HISTORY_CURRENT_PROGRESSION_TAG);
	}
	else
	{
		HasSave = false;
	}

	if(HasSave)
	{
		IsInit = true;
		CurrentProgression = Bundle->GetInt32(HISTORY_CURRENT_PROGRESSION_TAG);
	}
	else
	{
		IsInit = false;
	}

}

void UFlareQuestConditionWaypoints::Save(FFlareBundle* Bundle)
{
	if (IsInit)
	{
		Bundle->PutInt32(HISTORY_CURRENT_PROGRESSION_TAG, CurrentProgression);
	}
}

void UFlareQuestConditionWaypoints::Init()
{
	if(IsInit)
	{
		return;
	}

	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();

	if (Spacecraft)
	{
		IsInit = true;
		CurrentProgression = 0;
	}
}

bool UFlareQuestConditionWaypoints::IsCompleted()
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();

	if (Spacecraft)
	{
		Init();
		FVector WorldTargetLocation = VectorList[CurrentProgression];

		float MaxDistance = ABSOLUTE_WAYPOINTS_RADIUS;

		if (FVector::Dist(Spacecraft->GetActorLocation(), WorldTargetLocation) < MaxDistance)
		{
			// Nearing the target
			if (CurrentProgression + 2 <= VectorList.Num())
			{
				if(Spacecraft->GetParent()->GetCurrentSector() == TargetSector)
				{

				// Progress.
				CurrentProgression++;

				FText WaypointText = LOCTEXT("WaypointProgress", "Waypoint reached, {0} left");

				Quest->SendQuestNotification(FText::Format(WaypointText, FText::AsNumber(VectorList.Num() - CurrentProgression)),
									  FName(*(FString("quest-")+GetIdentifier().ToString()+"-step-progress")),
											 false);
				}
			}
			else
			{
				// All waypoint reach
				return true;
			}
		}
	}
	return false;
}

void UFlareQuestConditionWaypoints::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	Init();
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.Counter = 0;
	ObjectiveCondition.MaxCounter = VectorList.Num();
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = VectorList.Num();
	ObjectiveCondition.Counter = CurrentProgression;
	ObjectiveCondition.Progress = CurrentProgression;
	for (int TargetIndex = 0; TargetIndex < VectorList.Num(); TargetIndex++)
	{
		if (TargetIndex < CurrentProgression)
		{
			// Don't show old target
			continue;
		}
		FFlarePlayerObjectiveTarget ObjectiveTarget;
		ObjectiveTarget.Actor = NULL;
		ObjectiveTarget.Active = (CurrentProgression == TargetIndex);
		ObjectiveTarget.Radius = ABSOLUTE_WAYPOINTS_RADIUS;

		FVector WorldTargetLocation = VectorList[TargetIndex]; // In cm

		ObjectiveTarget.Location = WorldTargetLocation;
		ObjectiveData->TargetList.Add(ObjectiveTarget);
	}
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
	ObjectiveData->TargetSectors.Add(TargetSector);
}


/*----------------------------------------------------
Station count condition
----------------------------------------------------*/
UFlareQuestStationCount::UFlareQuestStationCount(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestStationCount* UFlareQuestStationCount::Create(UFlareQuest* ParentQuest, FName StationIdentifier, int32 StationCount)
{
	UFlareQuestStationCount* Condition = NewObject<UFlareQuestStationCount>(ParentQuest, UFlareQuestStationCount::StaticClass());
	Condition->Load(ParentQuest, StationIdentifier, StationCount);
	return Condition;
}

void UFlareQuestStationCount::Load(UFlareQuest* ParentQuest, FName StationIdentifier, int32 StationCount)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	TargetStationCount = StationCount;
	TargetStationIdentifier = StationIdentifier;

	FFlareSpacecraftDescription* Desc = GetGame()->GetSpacecraftCatalog()->Get(TargetStationIdentifier);



	if(TargetStationCount == 1)
	{
		InitialLabel = FText::Format(LOCTEXT("HaveOneSpecificStation", "Build a {0} "), Desc->Name);
	}
	else
	{
		InitialLabel = FText::Format(LOCTEXT("HaveMultipleSpecificStation", "Build {0} {1} "), FText::AsNumber(StationCount), Desc->Name);
	}
}

int32 UFlareQuestStationCount::GetStationCount()
{
	int StationCount = 0;
	for(UFlareSimulatedSpacecraft* Station : GetGame()->GetPC()->GetCompany()->GetCompanyStations())
	{
		if(Station->GetDescription()->Identifier == TargetStationIdentifier && !Station->IsUnderConstruction())
		{
			StationCount++;
		}
	}
	return StationCount;
}

bool UFlareQuestStationCount::IsCompleted()
{
	return GetStationCount() >= TargetStationCount;
}

void UFlareQuestStationCount::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = TargetStationCount;
	ObjectiveCondition.MaxProgress = TargetStationCount;
	ObjectiveCondition.Counter = GetStationCount();
	ObjectiveCondition.Progress = GetStationCount();
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
Bring resource condition
----------------------------------------------------*/
UFlareQuestBringResource::UFlareQuestBringResource(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestBringResource* UFlareQuestBringResource::Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector, FName ResourceIdentifier, int32 Count)
{
	UFlareQuestBringResource* Condition = NewObject<UFlareQuestBringResource>(ParentQuest, UFlareQuestBringResource::StaticClass());
	Condition->Load(ParentQuest, Sector, ResourceIdentifier, Count);
	return Condition;
}

void UFlareQuestBringResource::Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector, FName ResourceIdentifier, int32 Count)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	TargetResourceCount = Count;
	TargetResourceIdentifier = ResourceIdentifier;
	TargetSector = Sector;

	FFlareResourceDescription* Desc = GetGame()->GetResourceCatalog()->Get(TargetResourceIdentifier);


	if(Desc)
	{
		InitialLabel = FText::Format(LOCTEXT("BringResource", "Bring {0} {1} at {2}"),
									 FText::AsNumber(TargetResourceCount), Desc->Name, TargetSector->GetSectorName());
	}
}

int32 UFlareQuestBringResource::GetResourceCount()
{
	int ResourceCount = 0;

	FFlareResourceDescription* Resource = GetGame()->GetResourceCatalog()->Get(TargetResourceIdentifier);
	if(!Resource)
	{
		return 0;
	}

	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	for(UFlareSimulatedSpacecraft* Ship: TargetSector->GetSectorShips())
	{
		if(Ship->GetCompany() != PlayerCompany)
		{
			continue;
		}

		ResourceCount += Ship->GetCargoBay()->GetResourceQuantity(Resource, PlayerCompany);
	}

	return ResourceCount;
}

bool UFlareQuestBringResource::IsCompleted()
{
	return GetResourceCount() >= TargetResourceCount;
}

void UFlareQuestBringResource::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = TargetResourceCount;
	ObjectiveCondition.MaxProgress = TargetResourceCount;
	ObjectiveCondition.Counter = GetResourceCount();
	ObjectiveCondition.Progress = GetResourceCount();
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
	ObjectiveData->TargetSectors.Add(TargetSector);
}



/*----------------------------------------------------
	Max army in company condition
----------------------------------------------------*/
UFlareCompanyMaxCombatValue::UFlareCompanyMaxCombatValue(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareCompanyMaxCombatValue* UFlareCompanyMaxCombatValue::Create(UFlareQuest* ParentQuest, UFlareCompany* TargetCompanyParam, int32 TargetArmyPointsParam)
{
	UFlareCompanyMaxCombatValue* Condition = NewObject<UFlareCompanyMaxCombatValue>(ParentQuest, UFlareCompanyMaxCombatValue::StaticClass());
	Condition->Load(ParentQuest, TargetCompanyParam, TargetArmyPointsParam);
	return Condition;
}

void UFlareCompanyMaxCombatValue::Load(UFlareQuest* ParentQuest, UFlareCompany* TargetCompanyParam, int32 TargetArmyPointsParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
	Callbacks.AddUnique(EFlareQuestCallback::NEXT_DAY);
	TargetCompany = TargetCompanyParam;
	TargetArmyPoints = TargetArmyPointsParam;


	FText InitialLabelText = LOCTEXT("CompanyMaxArmyCombatPoints", "{0} must have at most {1} combat value");
	InitialLabel = FText::Format(InitialLabelText, TargetCompany->GetCompanyName(), FText::AsNumber(TargetArmyPoints));
}

bool UFlareCompanyMaxCombatValue::IsCompleted()
{
	return TargetCompany->GetCompanyValue().ArmyCurrentCombatPoints <= TargetArmyPoints;
}

void UFlareCompanyMaxCombatValue::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = 0;
	ObjectiveCondition.Counter = TargetCompany->GetCompanyValue().ArmyCurrentCombatPoints;
	ObjectiveCondition.MaxCounter = TargetArmyPoints;

	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}


#undef LOCTEXT_NAMESPACE
