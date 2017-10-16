
#include "FlareQuestCondition.h"
#include "Flare.h"
#include "FlareQuest.h"

#include "../Data/FlareSpacecraftCatalog.h"

#include "../Game/FlareGame.h"
#include "../Game/FlareGameTools.h"
#include "../Game/FlareSectorHelper.h"

#include "../Player/FlarePlayerController.h"

#include "../Spacecrafts/FlareSpacecraft.h"


#define LOCTEXT_NAMESPACE "FlareQuestCondition"

static FName INITIAL_VELOCITY_TAG("initial-velocity");
static FName INITIAL_CAPTURING_STATIONS_TAG("initial-capturing-stations");
static FName CURRENT_PROGRESSION_TAG("current-progression");
static FName INITIAL_TRANSFORM_TAG("initial-transform");
static FName WAYPOINTS_TAG("waypoints");


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareQuestCondition::UFlareQuestCondition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	  Quest(NULL)
{
}

void UFlareQuestCondition::AddSave(TArray<FFlareQuestConditionSave>& Data)
{
	if(Identifier != NAME_None)
	{
		FFlareQuestConditionSave ConditionData;
		ConditionData.ConditionIdentifier = GetIdentifier();
		Save(&ConditionData.Data);
		Data.Add(ConditionData);
	}
}

TArray<UFlareQuestCondition*> UFlareQuestCondition::GetAllConditions(bool OnlyLeaf)
{
	TArray<UFlareQuestCondition*> Conditions;

	Conditions.Add(this);

	return Conditions;
}

const FFlareBundle* UFlareQuestCondition::GetStepConditionBundle(UFlareQuestCondition* Condition, const TArray<FFlareQuestConditionSave>& Data)
{
	if (Condition && Condition->GetIdentifier() != NAME_None)
	{
		for (const FFlareQuestConditionSave& ConditionSave : Data)
		{
			if (ConditionSave.ConditionIdentifier == Condition->GetIdentifier())
			{
				return &ConditionSave.Data;
			}
		}
	}

	return NULL;
}


void UFlareQuestCondition::AddConditionCallbacks(TArray<EFlareQuestCallback::Type>& Callbacks, const TArray<UFlareQuestCondition*>& Conditions)
{
	for (UFlareQuestCondition* Condition : Conditions)
	{
		TArray<EFlareQuestCallback::Type> ConditionCallbacks = Condition->GetConditionCallbacks();
		for (int CallbackIndex = 0; CallbackIndex < ConditionCallbacks.Num(); CallbackIndex++)
		{
			Callbacks.AddUnique(ConditionCallbacks[CallbackIndex]);
		}
	}
}

void UFlareQuestCondition::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FLOG("ERROR: Not implemented AddConditionObjectives")
}

bool UFlareQuestCondition::IsCompleted()
{
	FLOG("ERROR: Not implemented IsCompleted");
	return false;
}

AFlareGame* UFlareQuestCondition::GetGame()
{
	return Quest->GetQuestManager()->GetGame();
}

AFlarePlayerController* UFlareQuestCondition::GetPC()
{
	return Quest->GetQuestManager()->GetGame()->GetPC();
}


/*----------------------------------------------------
	Group condition
----------------------------------------------------*/
UFlareQuestConditionGroup::UFlareQuestConditionGroup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareQuestConditionGroup::AddChildCondition(UFlareQuestCondition* Condition)
{
	Conditions.Add(Condition);
}

TArray<UFlareQuestCondition*> UFlareQuestConditionGroup::GetAllConditions(bool OnlyLeaf)
{
	TArray<UFlareQuestCondition*> AllConditions;

	if (!OnlyLeaf)
	{
		AllConditions.Add(this);
	}

	for(UFlareQuestCondition* Condition: Conditions)
	{
		AllConditions += Condition->GetAllConditions();
	}
	return AllConditions;
}

/*----------------------------------------------------
	And condition
----------------------------------------------------*/
UFlareQuestConditionAndGroup::UFlareQuestConditionAndGroup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


UFlareQuestConditionAndGroup* UFlareQuestConditionAndGroup::Create(UFlareQuest* ParentQuest, bool EmptyValueParam)
{
	UFlareQuestConditionAndGroup*Condition = NewObject<UFlareQuestConditionAndGroup>(ParentQuest, UFlareQuestConditionAndGroup::StaticClass());
	Condition->Load(ParentQuest, EmptyValueParam);
	return Condition;
}

void UFlareQuestConditionAndGroup::Load(UFlareQuest* ParentQuest, bool EmptyValueParam)
{
	LoadInternal(ParentQuest);
	EmptyValue = EmptyValueParam;
}

bool UFlareQuestConditionAndGroup::IsCompleted()
{
	if(Conditions.Num() == 0)
	{
		return EmptyValue;
	}

	for(UFlareQuestCondition* Condition: Conditions)
	{
		if (!Condition->IsCompleted())
		{
			return false;
		}
	}

	return true;
}

void UFlareQuestConditionAndGroup::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	ObjectiveData->IsAndCondition = true;
}

/*----------------------------------------------------
	Or condition
----------------------------------------------------*/
UFlareQuestConditionOrGroup::UFlareQuestConditionOrGroup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


UFlareQuestConditionOrGroup* UFlareQuestConditionOrGroup::Create(UFlareQuest* ParentQuest, bool EmptyValueParam)
{
	UFlareQuestConditionOrGroup*Condition = NewObject<UFlareQuestConditionOrGroup>(ParentQuest, UFlareQuestConditionOrGroup::StaticClass());
	Condition->Load(ParentQuest, EmptyValueParam);
	return Condition;
}

void UFlareQuestConditionOrGroup::Load(UFlareQuest* ParentQuest, bool EmptyValueParam)
{
	LoadInternal(ParentQuest);
	EmptyValue = EmptyValueParam;
}

bool UFlareQuestConditionOrGroup::IsCompleted()
{
	if(Conditions.Num() == 0)
	{
		return EmptyValue;
	}

	for(UFlareQuestCondition* Condition: Conditions)
	{
		if (Condition->IsCompleted())
		{
			return true;
		}
	}

	return false;
}

void UFlareQuestConditionOrGroup::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	ObjectiveData->IsAndCondition = false;
}

/*----------------------------------------------------
	Flying ship class condition
----------------------------------------------------*/
UFlareQuestConditionFlyingShipClass::UFlareQuestConditionFlyingShipClass(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionFlyingShipClass* UFlareQuestConditionFlyingShipClass::Create(UFlareQuest* ParentQuest, FName ShipClassParam)
{
	UFlareQuestConditionFlyingShipClass*Condition = NewObject<UFlareQuestConditionFlyingShipClass>(ParentQuest, UFlareQuestConditionFlyingShipClass::StaticClass());
	Condition->Load(ParentQuest, ShipClassParam);
	return Condition;
}

void UFlareQuestConditionFlyingShipClass::Load(UFlareQuest* ParentQuest, FName ShipClassParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::FLY_SHIP);
	ShipClass = ShipClassParam;

	FFlareSpacecraftDescription* SpacecraftDesc = GetGame()->GetSpacecraftCatalog()->Get(ShipClass);
	if (SpacecraftDesc)
	{
		InitialLabel = FText::Format(LOCTEXT("FlyShipFormat", "Fly a {0}-class ship"), SpacecraftDesc->Name);
	}
}

bool UFlareQuestConditionFlyingShipClass::IsCompleted()
{
	if (GetPC()->GetShipPawn())
	{
		if (ShipClass == NAME_None)
		{
			// No specific ship required
			return true;
		}
		else if (ShipClass == GetPC()->GetPlayerShip()->GetDescription()->Identifier)
		{
			// The flyed ship is the right kind of ship
			return true;
		}
	}
	return false;
}

void UFlareQuestConditionFlyingShipClass::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlareSpacecraftDescription* SpacecraftDesc = GetGame()->GetSpacecraftCatalog()->Get(ShipClass);
	if (SpacecraftDesc)
	{
		AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();

		FFlarePlayerObjectiveCondition ObjectiveCondition;
		ObjectiveCondition.InitialLabel = InitialLabel;
		ObjectiveCondition.TerminalLabel = FText();
		ObjectiveCondition.Progress = 0;
		ObjectiveCondition.MaxProgress = 0;
		ObjectiveCondition.Counter = (Spacecraft && Spacecraft->GetDescription()->Identifier == ShipClass) ? 1 : 0;
		ObjectiveCondition.MaxCounter = 1;

		ObjectiveData->ConditionList.Add(ObjectiveCondition);
	}
}

/*----------------------------------------------------
	Sector active condition
----------------------------------------------------*/
UFlareQuestConditionSectorActive::UFlareQuestConditionSectorActive(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionSectorActive* UFlareQuestConditionSectorActive::Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam)
{
	UFlareQuestConditionSectorActive*Condition = NewObject<UFlareQuestConditionSectorActive>(ParentQuest, UFlareQuestConditionSectorActive::StaticClass());
	Condition->Load(ParentQuest, SectorParam);
	return Condition;
}

void UFlareQuestConditionSectorActive::Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::SECTOR_ACTIVE);
	Sector = SectorParam;
	if (Sector)
	{
		InitialLabel = FText::Format(LOCTEXT("BeInSectorFormat", "Fly in the sector \"{0}\""), Sector->GetSectorName());
	}
}

bool UFlareQuestConditionSectorActive::IsCompleted()
{
	return (GetGame()->GetActiveSector() && (GetGame()->GetActiveSector()->GetSimulatedSector() == Sector));
}

void UFlareQuestConditionSectorActive::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	if (Sector)
	{
		FFlarePlayerObjectiveCondition ObjectiveCondition;
		ObjectiveCondition.InitialLabel = InitialLabel;
		ObjectiveCondition.TerminalLabel = FText();
		ObjectiveCondition.Progress = 0;
		ObjectiveCondition.MaxProgress = 0;
		ObjectiveCondition.Counter = (IsCompleted()) ? 1 : 0;
		ObjectiveCondition.MaxCounter = 1;

		ObjectiveData->TargetSectors.Add(Sector);
		ObjectiveData->ConditionList.Add(ObjectiveCondition);
	}
}

/*----------------------------------------------------
	Sector visited condition
----------------------------------------------------*/
UFlareQuestConditionSectorVisited::UFlareQuestConditionSectorVisited(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionSectorVisited* UFlareQuestConditionSectorVisited::Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam)
{
	UFlareQuestConditionSectorVisited*Condition = NewObject<UFlareQuestConditionSectorVisited>(ParentQuest, UFlareQuestConditionSectorVisited::StaticClass());
	Condition->Load(ParentQuest, SectorParam);
	return Condition;
}

void UFlareQuestConditionSectorVisited::Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::SECTOR_VISITED);
	Sector = SectorParam;
	if (Sector)
	{
		InitialLabel = FText::Format(LOCTEXT("VisitSectorFormat", "Visit the sector \"{0}\""), Sector->GetSectorName());
	}
}

bool UFlareQuestConditionSectorVisited::IsCompleted()
{
	return GetPC()->GetCompany()->HasVisitedSector(Sector);
}

void UFlareQuestConditionSectorVisited::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	if (Sector)
	{
		FFlarePlayerObjectiveCondition ObjectiveCondition;
		ObjectiveCondition.InitialLabel = InitialLabel;
		ObjectiveCondition.TerminalLabel = FText();
		ObjectiveCondition.Progress = 0;
		ObjectiveCondition.MaxProgress = 0;
		ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
		ObjectiveCondition.MaxCounter = 0;

		ObjectiveData->TargetSectors.Add(Sector);
		ObjectiveData->ConditionList.Add(ObjectiveCondition);
	}
}

/*----------------------------------------------------
	Min vertical velocity condition
----------------------------------------------------*/
UFlareQuestConditionMinVerticalVelocity::UFlareQuestConditionMinVerticalVelocity(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionMinVerticalVelocity* UFlareQuestConditionMinVerticalVelocity::Create(UFlareQuest* ParentQuest, FName ConditionIdentifier, float VelocityLimitParam)
{
	UFlareQuestConditionMinVerticalVelocity* Condition = NewObject<UFlareQuestConditionMinVerticalVelocity>(ParentQuest, UFlareQuestConditionMinVerticalVelocity::StaticClass());
	Condition->Load(ParentQuest, ConditionIdentifier, VelocityLimitParam);
	return Condition;
}

void UFlareQuestConditionMinVerticalVelocity::Load(UFlareQuest* ParentQuest, FName ConditionIdentifier, float VelocityLimitParam)
{
	if (ConditionIdentifier == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionMinVerticalVelocity need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifier);
	Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
	VelocityLimit = VelocityLimitParam;

	FText ReachSpeedText;
	if (VelocityLimit > 0)
	{
		ReachSpeedText = LOCTEXT("ReachMinUpSpeedFormat", "Reach at least {0} m/s up");
	}
	else
	{
		ReachSpeedText = LOCTEXT("ReachMinDownSpeedFormat", "Reach at most {0} m/s down");
	}
	InitialLabel = FText::Format(ReachSpeedText, FText::AsNumber((int)(FMath::Abs(VelocityLimit))));
}

void UFlareQuestConditionMinVerticalVelocity::Restore(const FFlareBundle* Bundle)
{
	if (Bundle)
	{
		HasInitialVelocity = Bundle->HasFloat(INITIAL_VELOCITY_TAG);
		InitialVelocity = Bundle->GetFloat(INITIAL_VELOCITY_TAG);
	}
	else
	{
		HasInitialVelocity = false;
	}
}

void UFlareQuestConditionMinVerticalVelocity::Save(FFlareBundle* Bundle)
{
	if (HasInitialVelocity)
	{
		Bundle->PutFloat(INITIAL_VELOCITY_TAG, InitialVelocity);
	}
}

float UFlareQuestConditionMinVerticalVelocity::GetVerticalVelocity()
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();
	if (Spacecraft)
	{
		float Velocity = FVector::DotProduct(Spacecraft->GetLinearVelocity(), Spacecraft->GetUpVector());
		if(!HasInitialVelocity)
		{
			HasInitialVelocity = true;
			InitialVelocity = Velocity;
		}

		return Velocity;
	}

	return 0;
}

bool UFlareQuestConditionMinVerticalVelocity::IsCompleted()
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();
	if (Spacecraft)
	{
		return GetVerticalVelocity() > VelocityLimit;
	}
	return false;
}

void UFlareQuestConditionMinVerticalVelocity::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();
	if (Spacecraft)
	{
		float Velocity = GetVerticalVelocity();

		FText ReachSpeedShortText = LOCTEXT("ReachMinVerticalSpeedShortFormat", "{0} m/s");

		FFlarePlayerObjectiveCondition ObjectiveCondition;
		ObjectiveCondition.InitialLabel = InitialLabel;
		ObjectiveCondition.TerminalLabel = FText::Format(ReachSpeedShortText, FText::AsNumber((int)(Velocity)));
		ObjectiveCondition.Counter = 0;
		ObjectiveCondition.MaxCounter = 0;
		ObjectiveCondition.MaxProgress = FMath::Abs(InitialVelocity - VelocityLimit);
		ObjectiveCondition.Progress = ObjectiveCondition.MaxProgress - FMath::Abs(Velocity - VelocityLimit);

		if (Velocity > VelocityLimit)
		{
			ObjectiveCondition.Progress = ObjectiveCondition.MaxProgress;
		}

		ObjectiveData->ConditionList.Add(ObjectiveCondition);
	}
}


/*----------------------------------------------------
	Max vertical velocity condition
----------------------------------------------------*/
UFlareQuestConditionMaxVerticalVelocity::UFlareQuestConditionMaxVerticalVelocity(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionMaxVerticalVelocity* UFlareQuestConditionMaxVerticalVelocity::Create(UFlareQuest* ParentQuest, FName ConditionIdentifier, float VelocityLimitParam)
{
	UFlareQuestConditionMaxVerticalVelocity* Condition = NewObject<UFlareQuestConditionMaxVerticalVelocity>(ParentQuest, UFlareQuestConditionMaxVerticalVelocity::StaticClass());
	Condition->Load(ParentQuest, ConditionIdentifier, VelocityLimitParam);
	return Condition;
}

void UFlareQuestConditionMaxVerticalVelocity::Load(UFlareQuest* ParentQuest, FName ConditionIdentifier, float VelocityLimitParam)
{
	if (ConditionIdentifier == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionMaxVerticalVelocity need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifier);
	Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
	VelocityLimit = VelocityLimitParam;

	FText ReachSpeedText;
	if (VelocityLimit > 0)
	{
		ReachSpeedText = LOCTEXT("ReachMaxUpSpeedFormat", "Reach at most {0} m/s up");
	}
	else
	{
		ReachSpeedText = LOCTEXT("ReachMaxDownSpeedFormat", "Reach at least {0} m/s down");
	}
	InitialLabel = FText::Format(ReachSpeedText, FText::AsNumber((int)(FMath::Abs(VelocityLimit))));
}

void UFlareQuestConditionMaxVerticalVelocity::Restore(const FFlareBundle* Bundle)
{
	if (Bundle)
	{
		HasInitialVelocity = Bundle->HasFloat(INITIAL_VELOCITY_TAG);
		InitialVelocity = Bundle->GetFloat(INITIAL_VELOCITY_TAG);
	}
	else
	{
		HasInitialVelocity = false;
	}
}

void UFlareQuestConditionMaxVerticalVelocity::Save(FFlareBundle* Bundle)
{
	if (HasInitialVelocity)
	{
		Bundle->PutFloat(INITIAL_VELOCITY_TAG, InitialVelocity);
	}
}

float UFlareQuestConditionMaxVerticalVelocity::GetVerticalVelocity()
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();
	if (Spacecraft)
	{
		float Velocity = FVector::DotProduct(Spacecraft->GetLinearVelocity(), Spacecraft->GetUpVector());
		if(!HasInitialVelocity)
		{
			HasInitialVelocity = true;
			InitialVelocity = Velocity;
		}

		return Velocity;
	}

	return 0;
}

bool UFlareQuestConditionMaxVerticalVelocity::IsCompleted()
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();
	if (Spacecraft)
	{
		return GetVerticalVelocity() < VelocityLimit;
	}
	return false;
}

void UFlareQuestConditionMaxVerticalVelocity::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();
	if (Spacecraft)
	{
		float Velocity = GetVerticalVelocity();

		FText ReachSpeedShortText = LOCTEXT("ReachMaxVerticalSpeedShortFormat", "{0} m/s");

		FFlarePlayerObjectiveCondition ObjectiveCondition;
		ObjectiveCondition.InitialLabel = InitialLabel;
		ObjectiveCondition.TerminalLabel = FText::Format(ReachSpeedShortText, FText::AsNumber((int)(Velocity)));
		ObjectiveCondition.Counter = 0;
		ObjectiveCondition.MaxCounter = 0;
		ObjectiveCondition.MaxProgress = FMath::Abs(InitialVelocity - VelocityLimit);
		ObjectiveCondition.Progress = ObjectiveCondition.MaxProgress - FMath::Abs(Velocity - VelocityLimit);

		if (Velocity < VelocityLimit)
		{
			ObjectiveCondition.Progress = ObjectiveCondition.MaxProgress;
		}

		ObjectiveData->ConditionList.Add(ObjectiveCondition);
	}
}


/*----------------------------------------------------
	Min collinear velocity condition
----------------------------------------------------*/
UFlareQuestConditionMinCollinearVelocity::UFlareQuestConditionMinCollinearVelocity(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionMinCollinearVelocity* UFlareQuestConditionMinCollinearVelocity::Create(UFlareQuest* ParentQuest, FName ConditionIdentifier, float VelocityLimitParam)
{
	UFlareQuestConditionMinCollinearVelocity*Condition = NewObject<UFlareQuestConditionMinCollinearVelocity>(ParentQuest, UFlareQuestConditionMinCollinearVelocity::StaticClass());
	Condition->Load(ParentQuest, ConditionIdentifier, VelocityLimitParam);
	return Condition;
}

void UFlareQuestConditionMinCollinearVelocity::Load(UFlareQuest* ParentQuest, FName ConditionIdentifier, float VelocityLimitParam)
{
	if (ConditionIdentifier == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionMinCollinearVelocity need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifier);
	Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
	VelocityLimit = VelocityLimitParam;

	FText ReachSpeedText = LOCTEXT("ReachMinSpeedForwardFormat", "Reach at least {0} m/s forward");
	InitialLabel = FText::Format(ReachSpeedText, FText::AsNumber((int)(VelocityLimit)));
}

void UFlareQuestConditionMinCollinearVelocity::Restore(const FFlareBundle* Bundle)
{
	if (Bundle)
	{
		HasInitialVelocity = Bundle->HasFloat(INITIAL_VELOCITY_TAG);
		InitialVelocity = Bundle->GetFloat(INITIAL_VELOCITY_TAG);
	}
	else
	{
		HasInitialVelocity = false;
	}
}

void UFlareQuestConditionMinCollinearVelocity::Save(FFlareBundle* Bundle)
{
	if (HasInitialVelocity)
	{
		Bundle->PutFloat(INITIAL_VELOCITY_TAG, InitialVelocity);
	}
}


float UFlareQuestConditionMinCollinearVelocity::GetCollinearVelocity()
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();
	if (Spacecraft)
	{
		float Velocity = FVector::DotProduct(Spacecraft->GetLinearVelocity(), Spacecraft->GetFrontVector());
		if(!HasInitialVelocity)
		{
			HasInitialVelocity = true;
			InitialVelocity = Velocity;
		}

		return Velocity;
	}

	return 0;
}

bool UFlareQuestConditionMinCollinearVelocity::IsCompleted()
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();
	if (Spacecraft)
	{
		return GetCollinearVelocity() > VelocityLimit;
	}
	return false;
}

void UFlareQuestConditionMinCollinearVelocity::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();
	if (Spacecraft)
	{
		float Velocity = GetCollinearVelocity();

		FText ReachSpeedShortText = LOCTEXT("ReachMinSpeedShortFormat", "{0} m/s");

		FFlarePlayerObjectiveCondition ObjectiveCondition;
		ObjectiveCondition.InitialLabel = InitialLabel;
		ObjectiveCondition.TerminalLabel = FText::Format(ReachSpeedShortText, FText::AsNumber((int)(Velocity)));
		ObjectiveCondition.Counter = 0;
		ObjectiveCondition.MaxCounter = 0;
		ObjectiveCondition.MaxProgress = FMath::Abs(InitialVelocity - VelocityLimit);
		ObjectiveCondition.Progress = ObjectiveCondition.MaxProgress - FMath::Abs(Velocity - VelocityLimit);

		if (Velocity > VelocityLimit)
		{
			ObjectiveCondition.Progress = ObjectiveCondition.MaxProgress;
		}

		ObjectiveData->ConditionList.Add(ObjectiveCondition);
	}
}

/*----------------------------------------------------
	Max collinear velocity condition
----------------------------------------------------*/
UFlareQuestConditionMaxCollinearVelocity::UFlareQuestConditionMaxCollinearVelocity(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionMaxCollinearVelocity* UFlareQuestConditionMaxCollinearVelocity::Create(UFlareQuest* ParentQuest, FName ConditionIdentifier, float VelocityLimitParam)
{
	UFlareQuestConditionMaxCollinearVelocity*Condition = NewObject<UFlareQuestConditionMaxCollinearVelocity>(ParentQuest, UFlareQuestConditionMaxCollinearVelocity::StaticClass());
	Condition->Load(ParentQuest, ConditionIdentifier, VelocityLimitParam);
	return Condition;
}

void UFlareQuestConditionMaxCollinearVelocity::Load(UFlareQuest* ParentQuest, FName ConditionIdentifier, float VelocityLimitParam)
{
	if (ConditionIdentifier == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionMaxCollinearVelocity need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifier);
	Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
	VelocityLimit = VelocityLimitParam;

	FText ReachSpeedText = LOCTEXT("ReachMinSpeedBackwardFormat", "Reach at least {0} m/s backward");
	InitialLabel = FText::Format(ReachSpeedText, FText::AsNumber((int)(-VelocityLimit)));
}

void UFlareQuestConditionMaxCollinearVelocity::Restore(const FFlareBundle* Bundle)
{
	if (Bundle)
	{
		HasInitialVelocity = Bundle->HasFloat(INITIAL_VELOCITY_TAG);
		InitialVelocity = Bundle->GetFloat(INITIAL_VELOCITY_TAG);
	}
	else
	{
		HasInitialVelocity = false;
	}
}

void UFlareQuestConditionMaxCollinearVelocity::Save(FFlareBundle* Bundle)
{
	if (HasInitialVelocity)
	{
		Bundle->PutFloat(INITIAL_VELOCITY_TAG, InitialVelocity);
	}
}

float UFlareQuestConditionMaxCollinearVelocity::GetCollinearVelocity()
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();
	if (Spacecraft)
	{
		float Velocity = FVector::DotProduct(Spacecraft->GetLinearVelocity(), Spacecraft->GetFrontVector());
		if(!HasInitialVelocity)
		{
			HasInitialVelocity = true;
			InitialVelocity = Velocity;
		}

		return Velocity;
	}

	return 0;
}

bool UFlareQuestConditionMaxCollinearVelocity::IsCompleted()
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();
	if (Spacecraft)
	{
		return GetCollinearVelocity() < VelocityLimit;
	}
	return false;
}

void UFlareQuestConditionMaxCollinearVelocity::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();
	if (Spacecraft)
	{
		float Velocity = GetCollinearVelocity();

		FText ReachSpeedShortText = LOCTEXT("ReachMinSpeedShortFormat", "{0} m/s");

		FFlarePlayerObjectiveCondition ObjectiveCondition;
		ObjectiveCondition.InitialLabel = InitialLabel;
		ObjectiveCondition.TerminalLabel = FText::Format(ReachSpeedShortText, FText::AsNumber((int)(Velocity)));
		ObjectiveCondition.Counter = 0;
		ObjectiveCondition.MaxCounter = 0;
		ObjectiveCondition.MaxProgress = FMath::Abs(InitialVelocity - VelocityLimit);
		ObjectiveCondition.Progress = ObjectiveCondition.MaxProgress - FMath::Abs(Velocity - VelocityLimit);

		if (Velocity < VelocityLimit)
		{
			ObjectiveCondition.Progress = ObjectiveCondition.MaxProgress;
		}

		ObjectiveData->ConditionList.Add(ObjectiveCondition);
	}
}

// Static helper
static FText GenerateConditionCollinearityInitialLabel(bool MaxLimit, float TargetCollinearity)
{
	float TargetAlignmentAngle = FMath::RadiansToDegrees(FMath::Acos(TargetCollinearity));

	FText MostOrLeast;
	if (MaxLimit)
	{
		MostOrLeast = LOCTEXT("Least", "least");
	}
	else
	{
		MostOrLeast = LOCTEXT("Most", "most");
	}

	return FText::Format(LOCTEXT("MisAlignFormat", "Put at {0} {1}\u00B0 between your nose and your prograde vector"),
		MostOrLeast, FText::AsNumber((int)(TargetAlignmentAngle)));
}

static void GenerateConditionCollinearityObjective(UFlareQuestCondition* Condition, AFlareSpacecraft* Spacecraft, FFlarePlayerObjectiveData* ObjectiveData, bool MaxLimit, float TargetCollinearity)
{
	if (Spacecraft)
	{
		float Alignment = 1;
		if (!Spacecraft->GetLinearVelocity().IsNearlyZero())
		{
			Alignment = FVector::DotProduct(Spacecraft->GetLinearVelocity().GetUnsafeNormal(), Spacecraft->GetFrontVector());
		}

		float AlignmentAngle = FMath::RadiansToDegrees(FMath::Acos(Alignment));
		float TargetAlignmentAngle = FMath::RadiansToDegrees(FMath::Acos(TargetCollinearity));

		FFlarePlayerObjectiveCondition ObjectiveCondition;
		ObjectiveCondition.MaxProgress = 1.0f;
		if (MaxLimit)
		{
			if (AlignmentAngle > TargetAlignmentAngle)
			{
				ObjectiveCondition.Progress = 1.0;
			}
			else
			{
				ObjectiveCondition.Progress = AlignmentAngle/TargetAlignmentAngle;
			}
		}
		else
		{
			if (AlignmentAngle < TargetAlignmentAngle)
			{
				ObjectiveCondition.Progress = 1.0;
			}
			else
			{
				ObjectiveCondition.Progress = (180 - AlignmentAngle) / (180 - TargetAlignmentAngle);
			}
		}

		ObjectiveCondition.InitialLabel = Condition->GetInitialLabel();
		ObjectiveCondition.TerminalLabel = FText::Format(LOCTEXT("MisAlignShortFormat", "{0}\u00B0"),
			FText::AsNumber((int)(AlignmentAngle)));
		ObjectiveCondition.Counter = 0;
		ObjectiveCondition.MaxCounter = 0;
		ObjectiveData->ConditionList.Add(ObjectiveCondition);
	}
}

/*----------------------------------------------------
	Min collinear condition
----------------------------------------------------*/
UFlareQuestConditionMinCollinear::UFlareQuestConditionMinCollinear(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionMinCollinear* UFlareQuestConditionMinCollinear::Create(UFlareQuest* ParentQuest, float CollinearLimitParam)
{
	UFlareQuestConditionMinCollinear*Condition = NewObject<UFlareQuestConditionMinCollinear>(ParentQuest, UFlareQuestConditionMinCollinear::StaticClass());
	Condition->Load(ParentQuest, CollinearLimitParam);
	return Condition;
}

void UFlareQuestConditionMinCollinear::Load(UFlareQuest* ParentQuest, float CollinearLimitParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::FLY_SHIP);
	CollinearLimit = CollinearLimitParam;

	InitialLabel = GenerateConditionCollinearityInitialLabel(false, CollinearLimit);
}

float UFlareQuestConditionMinCollinear::GetCollinear()
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();
	if (Spacecraft)
	{
		if (Spacecraft->GetLinearVelocity().IsNearlyZero())
		{
			return 0;
		}
		else
		{
			return FVector::DotProduct(Spacecraft->GetLinearVelocity().GetUnsafeNormal(), Spacecraft->GetFrontVector());
		}
	}

	return 0;
}

bool UFlareQuestConditionMinCollinear::IsCompleted()
{
	return GetCollinear() > CollinearLimit;
}

void UFlareQuestConditionMinCollinear::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();
	GenerateConditionCollinearityObjective(this, Spacecraft, ObjectiveData, false, CollinearLimit);
}

/*----------------------------------------------------
	Max collinear condition
----------------------------------------------------*/
UFlareQuestConditionMaxCollinear::UFlareQuestConditionMaxCollinear(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionMaxCollinear* UFlareQuestConditionMaxCollinear::Create(UFlareQuest* ParentQuest, float CollinearLimitParam)
{
	UFlareQuestConditionMaxCollinear*Condition = NewObject<UFlareQuestConditionMaxCollinear>(ParentQuest, UFlareQuestConditionMaxCollinear::StaticClass());
	Condition->Load(ParentQuest, CollinearLimitParam);
	return Condition;
}

void UFlareQuestConditionMaxCollinear::Load(UFlareQuest* ParentQuest, float CollinearLimitParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::FLY_SHIP);
	CollinearLimit = CollinearLimitParam;
	InitialLabel = GenerateConditionCollinearityInitialLabel(true, CollinearLimit);
}

float UFlareQuestConditionMaxCollinear::GetCollinear()
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();
	if (Spacecraft)
	{
		if (Spacecraft->GetLinearVelocity().IsNearlyZero())
		{
			return 0;
		}
		else
		{
			return FVector::DotProduct(Spacecraft->GetLinearVelocity().GetUnsafeNormal(), Spacecraft->GetFrontVector());
		}
	}

	return 0;
}

bool UFlareQuestConditionMaxCollinear::IsCompleted()
{
	return GetCollinear() < CollinearLimit;
}

void UFlareQuestConditionMaxCollinear::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();
	GenerateConditionCollinearityObjective(this, Spacecraft, ObjectiveData, true, CollinearLimit);
}

// Helper
static FText GenerateMaxRotationVelocityConditionInitialLabel(FVector LocalAxis, float AngularVelocityLimit)
{
	FText Direction;
	FText ReachSpeedText;
	float Sign = FMath::Sign(AngularVelocityLimit);

	if(LocalAxis.Y > 0)
	{
		Direction = (AngularVelocityLimit > 0) ? LOCTEXT("Down", "down") : LOCTEXT("Up", "up");
		ReachSpeedText = LOCTEXT("ReachPitchFormat", "Reach a pitch rate of {0}\u00B0/s {1}");
	}
	else if(LocalAxis.Z > 0)
	{
		Direction = (AngularVelocityLimit > 0) ? LOCTEXT("Right", "right") : LOCTEXT("Left", "left");
		ReachSpeedText = LOCTEXT("ReachYawFormat", "Reach a yaw rate of {0} \u00B0/s {1}");
	}
	else if(LocalAxis.X > 0)
	{
		Direction = (AngularVelocityLimit < 0) ? LOCTEXT("Right", "right") : LOCTEXT("Left", "left");
		ReachSpeedText = LOCTEXT("ReachRollFormat", "Reach a roll rate of {0} \u00B0/s {1}");
	}

	return FText::Format(ReachSpeedText, FText::AsNumber((int)(Sign * AngularVelocityLimit)), Direction);
}

// Helper
static void GenerateMaxRotationVelocityConditionObjectives(UFlareQuestCondition* Condition, FFlarePlayerObjectiveData* ObjectiveData, float Velocity, float AngularVelocityLimit)
{
	FText ReachSpeedShortText = LOCTEXT("ReachPitchShortFormat", "{0}\u00B0/s");
	float Sign = FMath::Sign(AngularVelocityLimit);

	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = Condition->GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText::Format(ReachSpeedShortText, FText::AsNumber((int)(Sign * Velocity)));
	ObjectiveCondition.Counter = 0;
	ObjectiveCondition.MaxCounter = 0;

	ObjectiveCondition.Progress = FMath::Clamp(Velocity * Sign , 0.0f, AngularVelocityLimit * Sign);
	ObjectiveCondition.MaxProgress = AngularVelocityLimit * Sign;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	Min rotation velocity condition
----------------------------------------------------*/
UFlareQuestConditionMinRotationVelocity::UFlareQuestConditionMinRotationVelocity(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionMinRotationVelocity* UFlareQuestConditionMinRotationVelocity::Create(UFlareQuest* ParentQuest, FVector LocalAxisParam, float AngularVelocityLimitParam)
{
	UFlareQuestConditionMinRotationVelocity*Condition = NewObject<UFlareQuestConditionMinRotationVelocity>(ParentQuest, UFlareQuestConditionMinRotationVelocity::StaticClass());
	Condition->Load(ParentQuest, LocalAxisParam, AngularVelocityLimitParam);
	return Condition;
}

void UFlareQuestConditionMinRotationVelocity::Load(UFlareQuest* ParentQuest, FVector LocalAxisParam, float AngularVelocityLimitParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
	AngularVelocityLimit = AngularVelocityLimitParam;
	LocalAxis = LocalAxisParam;
	InitialLabel = GenerateMaxRotationVelocityConditionInitialLabel (LocalAxis, AngularVelocityLimit);
}

float UFlareQuestConditionMinRotationVelocity::GetVelocityInAxis()
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();
	if (Spacecraft)
	{
		FVector WorldAngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocityInDegrees();
		FVector LocalAngularVelocity = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(WorldAngularVelocity);
		return FVector::DotProduct(LocalAngularVelocity, LocalAxis);
	}

	return 0;
}

bool UFlareQuestConditionMinRotationVelocity::IsCompleted()
{
	return GetVelocityInAxis() > AngularVelocityLimit;
}

void UFlareQuestConditionMinRotationVelocity::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	GenerateMaxRotationVelocityConditionObjectives(this, ObjectiveData, GetVelocityInAxis(), AngularVelocityLimit);
}

/*----------------------------------------------------
	Max rotation velocity condition
----------------------------------------------------*/
UFlareQuestConditionMaxRotationVelocity::UFlareQuestConditionMaxRotationVelocity(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionMaxRotationVelocity* UFlareQuestConditionMaxRotationVelocity::Create(UFlareQuest* ParentQuest, FVector LocalAxisParam, float AngularVelocityLimitParam)
{
	UFlareQuestConditionMaxRotationVelocity*Condition = NewObject<UFlareQuestConditionMaxRotationVelocity>(ParentQuest, UFlareQuestConditionMaxRotationVelocity::StaticClass());
	Condition->Load(ParentQuest, LocalAxisParam, AngularVelocityLimitParam);
	return Condition;
}

void UFlareQuestConditionMaxRotationVelocity::Load(UFlareQuest* ParentQuest, FVector LocalAxisParam, float AngularVelocityLimitParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
	AngularVelocityLimit = AngularVelocityLimitParam;
	LocalAxis = LocalAxisParam;
	InitialLabel = GenerateMaxRotationVelocityConditionInitialLabel (LocalAxis, AngularVelocityLimit);
}

float UFlareQuestConditionMaxRotationVelocity::GetVelocityInAxis()
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();
	if (Spacecraft)
	{
		FVector WorldAngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocityInDegrees();
		FVector LocalAngularVelocity = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(WorldAngularVelocity);
		return FVector::DotProduct(LocalAngularVelocity, LocalAxis);
	}

	return 0;
}

bool UFlareQuestConditionMaxRotationVelocity::IsCompleted()
{
	return GetVelocityInAxis() < AngularVelocityLimit;
}

void UFlareQuestConditionMaxRotationVelocity::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	GenerateMaxRotationVelocityConditionObjectives(this, ObjectiveData, GetVelocityInAxis(), AngularVelocityLimit);
}

/*----------------------------------------------------
	Ship alive condition
----------------------------------------------------*/
UFlareQuestConditionShipAlive::UFlareQuestConditionShipAlive(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionShipAlive* UFlareQuestConditionShipAlive::Create(UFlareQuest* ParentQuest, FName ShipIdentifierParam)
{
	UFlareQuestConditionShipAlive*Condition = NewObject<UFlareQuestConditionShipAlive>(ParentQuest, UFlareQuestConditionShipAlive::StaticClass());
	Condition->Load(ParentQuest, ShipIdentifierParam);
	return Condition;
}

void UFlareQuestConditionShipAlive::Load(UFlareQuest* ParentQuest, FName ShipIdentifierParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::FLY_SHIP);
	ShipIdentifier = ShipIdentifierParam;
	InitialLabel = FText::Format(LOCTEXT("ShipAliveFormat", "{0} must stay alive"), FText::FromName(ShipIdentifier));
}

bool UFlareQuestConditionShipAlive::IsShipAlive()
{
	UFlareSimulatedSpacecraft* TargetSpacecraft = GetGame()->GetGameWorld()->FindSpacecraft(ShipIdentifier);
	return TargetSpacecraft && TargetSpacecraft->GetDamageSystem()->IsAlive();
}

bool UFlareQuestConditionShipAlive::IsCompleted()
{
	return IsShipAlive();
}

void UFlareQuestConditionShipAlive::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = 0;
	ObjectiveCondition.Counter = IsShipAlive() ? 1 : 0;
	ObjectiveCondition.MaxCounter = 1;

	UFlareSimulatedSpacecraft* TargetSpacecraft = GetGame()->GetGameWorld()->FindSpacecraft(ShipIdentifier);
	if(TargetSpacecraft)
	{
		ObjectiveData->AddTargetSpacecraft(TargetSpacecraft);
	}

	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	Quest succesfull condition
----------------------------------------------------*/
UFlareQuestConditionQuestSuccessful::UFlareQuestConditionQuestSuccessful(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionQuestSuccessful* UFlareQuestConditionQuestSuccessful::Create(UFlareQuest* ParentQuest, FName



																				 QuestParam)
{
	UFlareQuestConditionQuestSuccessful*Condition = NewObject<UFlareQuestConditionQuestSuccessful>(ParentQuest, UFlareQuestConditionQuestSuccessful::StaticClass());
	Condition->Load(ParentQuest, QuestParam);
	return Condition;
}

void UFlareQuestConditionQuestSuccessful::Load(UFlareQuest* ParentQuest, FName QuestParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_CHANGED);
	TargetQuest = QuestParam;
}

bool UFlareQuestConditionQuestSuccessful::IsCompleted()
{
	UFlareQuest* Target = Quest->GetQuestManager()->FindQuest(TargetQuest);

	if (Target)
	{
		return Quest->GetQuestManager()->IsQuestSuccessfull(Target);
	}
	else
	{
			FLOGV("ERROR: UFlareQuestConditionQuestSuccessful fail to find quest '%s' for quest '%s'",
				  *TargetQuest.ToString(),
				  *Quest->GetIdentifier().ToString());
		return true;
	}
}

void UFlareQuestConditionQuestSuccessful::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
}

/*----------------------------------------------------
	Quest failed condition
----------------------------------------------------*/
UFlareQuestConditionQuestFailed::UFlareQuestConditionQuestFailed(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionQuestFailed* UFlareQuestConditionQuestFailed::Create(UFlareQuest* ParentQuest, FName QuestParam)
{
	UFlareQuestConditionQuestFailed*Condition = NewObject<UFlareQuestConditionQuestFailed>(ParentQuest, UFlareQuestConditionQuestFailed::StaticClass());
	Condition->Load(ParentQuest, QuestParam);
	return Condition;
}

void UFlareQuestConditionQuestFailed::Load(UFlareQuest* ParentQuest, FName QuestParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_CHANGED);
	TargetQuest = QuestParam;
}

bool UFlareQuestConditionQuestFailed::IsCompleted()
{
	UFlareQuest* Target = Quest->GetQuestManager()->FindQuest(TargetQuest);

	if (Target)
	{
		return Quest->GetQuestManager()->IsQuestFailed(Target);
	}
	else
	{
			FLOGV("ERROR: UFlareQuestConditionQuestSuccessful fail to find quest '%s' for quest '%s'",
				  *TargetQuest.ToString(),
				  *Quest->GetIdentifier().ToString());
		return true;
	}
}

void UFlareQuestConditionQuestFailed::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
}

/*----------------------------------------------------
	Follow relative waypoints condition
----------------------------------------------------*/

#define WAYPOINTS_RADIUS 5000
#define SCANNER_RADIUS 10000

UFlareQuestConditionFollowRelativeWaypoints::UFlareQuestConditionFollowRelativeWaypoints(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionFollowRelativeWaypoints* UFlareQuestConditionFollowRelativeWaypoints::Create(UFlareQuest* ParentQuest, FName ConditionIdentifier, TArray<FVector> VectorListParam, bool RequiresScan)
{
	UFlareQuestConditionFollowRelativeWaypoints*Condition = NewObject<UFlareQuestConditionFollowRelativeWaypoints>(ParentQuest, UFlareQuestConditionFollowRelativeWaypoints::StaticClass());
	Condition->Load(ParentQuest, ConditionIdentifier, VectorListParam, RequiresScan);
	return Condition;
}

void UFlareQuestConditionFollowRelativeWaypoints::Load(UFlareQuest* ParentQuest, FName ConditionIdentifier, TArray<FVector> VectorListParam, bool RequiresScan)
{
	if (ConditionIdentifier == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionFollowRelativeWaypoints need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifier);
	Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
	VectorList = VectorListParam;
	TargetRequiresScan = RequiresScan;

	if (RequiresScan)
	{
		InitialLabel = LOCTEXT("ScanWaypoints", "Analyze signals");
	}
	else
	{
		InitialLabel = LOCTEXT("FollowWaypoints", "Fly to waypoints");
	}
}

void UFlareQuestConditionFollowRelativeWaypoints::Restore(const FFlareBundle* Bundle)
{
	bool HasSave = true;
	if(Bundle)
	{
		HasSave &= Bundle->HasInt32(CURRENT_PROGRESSION_TAG);
		HasSave &= Bundle->HasTransform(INITIAL_TRANSFORM_TAG);
	}
	else
	{
		HasSave = false;
	}

	if(HasSave)
	{
		IsInit = true;
		CurrentProgression = Bundle->GetInt32(CURRENT_PROGRESSION_TAG);
		InitialTransform = Bundle->GetTransform(INITIAL_TRANSFORM_TAG);
	}
	else
	{
		IsInit = false;
	}

}

void UFlareQuestConditionFollowRelativeWaypoints::Save(FFlareBundle* Bundle)
{
	if (IsInit)
	{
		Bundle->PutInt32(CURRENT_PROGRESSION_TAG, CurrentProgression);
		Bundle->PutTransform(INITIAL_TRANSFORM_TAG, InitialTransform);
	}
}

void UFlareQuestConditionFollowRelativeWaypoints::Init()
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
		InitialTransform = Spacecraft->Airframe->GetComponentTransform();
	}
}

bool UFlareQuestConditionFollowRelativeWaypoints::IsCompleted()
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();

	if (Spacecraft)
	{
		Init();
		FVector InitialLocation = InitialTransform.GetTranslation();
		FVector RelativeTargetLocation = VectorList[CurrentProgression] * 100;
		FVector WorldTargetLocation = InitialLocation + InitialTransform.GetRotation().RotateVector(RelativeTargetLocation);

		float MaxDistance = TargetRequiresScan ? SCANNER_RADIUS : WAYPOINTS_RADIUS;

		// Waypoint completion logic
		bool HasCompletedWaypoint = false;
		if (TargetRequiresScan)
		{
			if (Spacecraft->IsScanningFinished())
			{
				HasCompletedWaypoint = true;
			}
		}
		else if (FVector::Dist(Spacecraft->GetActorLocation(), WorldTargetLocation) < MaxDistance)
		{
			HasCompletedWaypoint = true;
		}

		if (HasCompletedWaypoint)
		{
			// Nearing the target
			if (CurrentProgression + 2 <= VectorList.Num())
			{
				CurrentProgression++;

				FText WaypointText;
				if (TargetRequiresScan)
				{
					WaypointText = LOCTEXT("ScanProgress", "Signal analyzed, {0} left");
				}
				else
				{
					WaypointText = LOCTEXT("WaypointProgress", "Waypoint reached, {0} left");
				}

				Quest->SendQuestNotification(FText::Format(WaypointText, FText::AsNumber(VectorList.Num() - CurrentProgression)),
					FName(*(FString("quest-")+GetIdentifier().ToString()+"-step-progress")),
					false);
			}
			else
			{
				// All waypoints reached
				return true;
			}
		}
	}
	return false;
}

void UFlareQuestConditionFollowRelativeWaypoints::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
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
		ObjectiveTarget.RequiresScan = TargetRequiresScan;
		ObjectiveTarget.Actor = NULL;
		ObjectiveTarget.Active = (CurrentProgression == TargetIndex);
		ObjectiveTarget.Radius = TargetRequiresScan ? SCANNER_RADIUS : WAYPOINTS_RADIUS;

		FVector InitialLocation = InitialTransform.GetTranslation();
		FVector RelativeTargetLocation = VectorList[TargetIndex] * 100; // In cm
		FVector WorldTargetLocation = InitialLocation + InitialTransform.GetRotation().RotateVector(RelativeTargetLocation);

		ObjectiveTarget.Location = WorldTargetLocation;
		ObjectiveData->TargetList.Add(ObjectiveTarget);
	}
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	Follow random waypoints condition
----------------------------------------------------*/

UFlareQuestConditionFollowRandomWaypoints::UFlareQuestConditionFollowRandomWaypoints(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionFollowRandomWaypoints* UFlareQuestConditionFollowRandomWaypoints::Create(UFlareQuest* ParentQuest, FName ConditionIdentifier, bool RequiresScan)
{
	UFlareQuestConditionFollowRandomWaypoints*Condition = NewObject<UFlareQuestConditionFollowRandomWaypoints>(ParentQuest, UFlareQuestConditionFollowRandomWaypoints::StaticClass());
	Condition->Load(ParentQuest, ConditionIdentifier, RequiresScan);
	return Condition;
}

void UFlareQuestConditionFollowRandomWaypoints::Load(UFlareQuest* ParentQuest, FName ConditionIdentifier, bool RequiresScan)
{
	if (ConditionIdentifier == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionFollowWaypoints need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifier);
	Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
	TargetRequiresScan = RequiresScan;

	if (RequiresScan)
	{
		InitialLabel = LOCTEXT("ScanWaypoints", "Analyze signals");
	}
	else
	{
		InitialLabel = LOCTEXT("FollowWaypoints", "Fly to waypoints");
	}
}

void UFlareQuestConditionFollowRandomWaypoints::Restore(const FFlareBundle* Bundle)
{
	bool HasSave = true;
	if(Bundle)
	{
		HasSave &= Bundle->HasInt32(CURRENT_PROGRESSION_TAG);
		HasSave &= Bundle->HasVectorArray(WAYPOINTS_TAG);
	}
	else
	{
		HasSave = false;
	}

	if(HasSave)
	{
		IsInit = true;
		CurrentProgression = Bundle->GetInt32(CURRENT_PROGRESSION_TAG);
		Waypoints = Bundle->GetVectorArray(WAYPOINTS_TAG);
	}
	else
	{
		IsInit = false;
	}

}

void UFlareQuestConditionFollowRandomWaypoints::Save(FFlareBundle* Bundle)
{
	if (IsInit)
	{
		Bundle->PutInt32(CURRENT_PROGRESSION_TAG, CurrentProgression);
		Bundle->PutVectorArray(WAYPOINTS_TAG, Waypoints);
	}
}

void UFlareQuestConditionFollowRandomWaypoints::Init()
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
		Waypoints.Empty();

		FVector CurrentLocation = Spacecraft->GetActorLocation();
		FVector CurrentDirection= Spacecraft->GetFrontVector();

		FVector BaseDirection= CurrentDirection;
		FVector BaseLocation = (CurrentLocation / 100) + CurrentDirection * 200;
		float Distance = 0;
		float MaxDistance = 1500;
		float StepDistance = 150;

		while(Distance < MaxDistance)
		{
			FVector TargetLocation = BaseLocation + FMath::VRand() * FMath::FRandRange(1000, 2000);
			float TargetMaxTurnDistance = 20000;

			GenerateWaypointSegments(Waypoints, Distance, MaxDistance, StepDistance, BaseDirection, BaseLocation, TargetLocation, TargetMaxTurnDistance);
		}
	}
}

bool UFlareQuestConditionFollowRandomWaypoints::IsCompleted()
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();

	if (Spacecraft)
	{
		Init();
		if (Waypoints.Num() == 0)
		{
			return true;
		}

		FVector WorldTargetLocation = Waypoints[CurrentProgression] * 100;
		float MaxDistance = TargetRequiresScan ? SCANNER_RADIUS : WAYPOINTS_RADIUS;

		// Waypoint completion logic
		bool HasCompletedWaypoint = false;
		if (TargetRequiresScan)
		{
			if (Spacecraft->IsScanningFinished())
			{
				HasCompletedWaypoint = true;
			}
		}
		else if (FVector::Dist(Spacecraft->GetActorLocation(), WorldTargetLocation) < MaxDistance)
		{
			HasCompletedWaypoint = true;
		}

		if (HasCompletedWaypoint)
		{
			// Nearing the target
			if (CurrentProgression + 2 <= Waypoints.Num())
			{
				CurrentProgression++;

				FText WaypointText;
				if (TargetRequiresScan)
				{
					WaypointText = LOCTEXT("ScanProgress", "Signal analyzed, {0} left");
				}
				else
				{
					WaypointText = LOCTEXT("WaypointProgress", "Waypoint reached, {0} left");
				}

				Quest->SendQuestNotification(FText::Format(WaypointText, FText::AsNumber(Waypoints.Num() - CurrentProgression)),
					FName(*(FString("quest-")+GetIdentifier().ToString()+"-step-progress")),
					false);
			}
			else
			{
				// All waypoints reached
				return true;
			}
		}
	}
	return false;
}

void UFlareQuestConditionFollowRandomWaypoints::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	Init();
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.Counter = 0;
	ObjectiveCondition.MaxCounter = Waypoints.Num();
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = Waypoints.Num();
	ObjectiveCondition.Counter = CurrentProgression;
	ObjectiveCondition.Progress = CurrentProgression;
	for (int TargetIndex = 0; TargetIndex < Waypoints.Num(); TargetIndex++)
	{
		if (TargetIndex < CurrentProgression)
		{
			// Don't show old target
			continue;
		}
		FFlarePlayerObjectiveTarget ObjectiveTarget;
		ObjectiveTarget.RequiresScan = TargetRequiresScan;
		ObjectiveTarget.Actor = NULL;
		ObjectiveTarget.Active = (CurrentProgression == TargetIndex);
		ObjectiveTarget.Radius = TargetRequiresScan ? SCANNER_RADIUS : WAYPOINTS_RADIUS;

		FVector WorldTargetLocation = Waypoints[TargetIndex] * 100; // In cm

		ObjectiveTarget.Location = WorldTargetLocation;
		ObjectiveData->TargetList.Add(ObjectiveTarget);
	}
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

void UFlareQuestConditionFollowRandomWaypoints::GenerateWaypointSegments(TArray<FVector>& WaypointList, float& Distance, float MaxDistance, float StepDistance,
														 FVector& BaseDirection, FVector& BaseLocation, FVector TargetLocation,
														 float TargetMaxTurnDistance)
{
	// Compute circle insertion point and direction
	float TotalDistance = (TargetLocation - BaseLocation).Size();
	float TurnDistance = FMath::Min(TargetMaxTurnDistance, TotalDistance);
	int32 MaxTurnStep = TurnDistance / StepDistance;
	FRotator InitialDirection = BaseDirection.Rotation();
	int32 StepIndex = 1;

	FLOGV("TotalDistance %f", TotalDistance);
	FLOGV("TurnDistance %f", TurnDistance);
	FLOGV("MaxTurnStep %d", MaxTurnStep);


	while (Distance < MaxDistance){
		float Alpha = FMath::Min(1.f, (float) StepIndex / (float) MaxTurnStep);

		FLOGV("%d Alpha %f", StepIndex, Alpha);


		FRotator TargetDirection = (TargetLocation - BaseLocation).Rotation();

		BaseDirection = FMath::Lerp(InitialDirection, TargetDirection, Alpha).RotateVector(FVector(1,0,0));
		BaseDirection.GetUnsafeNormal();
		BaseLocation += BaseDirection * StepDistance;
		Distance+= StepDistance;
		StepIndex++;

		WaypointList.Add(BaseLocation);


		float RemainingDistance = (TargetLocation - BaseLocation).Size();
		if(RemainingDistance < StepDistance)
		{
			break;
		}
	}
}

/*----------------------------------------------------
	Quest dock at condition
----------------------------------------------------*/
UFlareQuestConditionDockAt::UFlareQuestConditionDockAt(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionDockAt* UFlareQuestConditionDockAt::Create(UFlareQuest* ParentQuest, UFlareSimulatedSpacecraft* Station)
{
	UFlareQuestConditionDockAt* Condition = NewObject<UFlareQuestConditionDockAt>(ParentQuest, UFlareQuestConditionDockAt::StaticClass());
	Condition->Load(ParentQuest, Station);
	return Condition;
}

void UFlareQuestConditionDockAt::Load(UFlareQuest* ParentQuest, UFlareSimulatedSpacecraft* Station)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::SHIP_DOCKED);
	TargetStation = Station;
	TargetShipMatchId = NAME_None;
	TargetShipSaveId = NAME_None;
	Completed = false;
}

bool UFlareQuestConditionDockAt::IsCompleted()
{
	if (Completed)
	{
		return true;
	}

	if (!TargetStation->GetActive())
	{
		return false;
	}

	for (AFlareSpacecraft* Ship : TargetStation->GetActive()->GetDockingSystem()->GetDockedShips())
	{
		if (Ship->GetCompany() != GetGame()->GetPC()->GetCompany())
		{
			continue;
		}

		// 2 cases:
		// - no ship restriction
		// - restricted to a given ship id
		bool ValidShip = false;
		if (TargetShipMatchId == NAME_None)
		{
			// No restriction
			ValidShip = true;
		}
		else
		{
			FName ShipName = Quest->GetSaveBundle().GetName(TargetShipMatchId);
			UFlareSimulatedSpacecraft* TargetShip = GetGame()->GetGameWorld()->FindSpacecraft(ShipName);
			if(TargetShip == Ship->GetParent())
			{
				ValidShip = true;
			}
		}

		if(ValidShip)
		{
			Completed = true;
			if (TargetShipSaveId != NAME_None)
			{
				// Save docked ship
				Quest->GetSaveBundle().PutName(TargetShipSaveId, Ship->GetImmatriculation());
			}
			return true;
		}
	}

	return false;
}

FText UFlareQuestConditionDockAt::GetInitialLabel()
{
	if (TargetShipMatchId != NAME_None)
	{
		FName ShipName = Quest->GetSaveBundle().GetName(TargetShipMatchId);
		UFlareSimulatedSpacecraft* TargetShip = GetGame()->GetGameWorld()->FindSpacecraft(ShipName);

		if (TargetShip)
		{
			return FText::Format(LOCTEXT("DockShipAtFormat", "Dock at {0} in {1} with a ship {2}"),
															UFlareGameTools::DisplaySpacecraftName(TargetStation),
															TargetStation->GetCurrentSector()->GetSectorName(),
															UFlareGameTools::DisplaySpacecraftName(TargetShip));
		}
	}
	return FText::Format(LOCTEXT("DockAtFormat", "Dock at {0} in {1} with a ship"),
													UFlareGameTools::DisplaySpacecraftName(TargetStation),
													TargetStation->GetCurrentSector()->GetSectorName());
}


void UFlareQuestConditionDockAt::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Counter = 0;
	ObjectiveCondition.MaxCounter = 0;
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
	ObjectiveData->AddTargetSpacecraft(TargetStation);
}

/*----------------------------------------------------
	At war condition
----------------------------------------------------*/
UFlareQuestConditionAtWar::UFlareQuestConditionAtWar(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionAtWar* UFlareQuestConditionAtWar::Create(UFlareQuest* ParentQuest, UFlareCompany* Company1, UFlareCompany* Company2)
{
	UFlareQuestConditionAtWar* Condition = NewObject<UFlareQuestConditionAtWar>(ParentQuest, UFlareQuestConditionAtWar::StaticClass());
	Condition->Load(ParentQuest, Company1, Company2);
	return Condition;
}

void UFlareQuestConditionAtWar::Load(UFlareQuest* ParentQuest, UFlareCompany* Company1, UFlareCompany* Company2)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::WAR_STATE_CHANGED);
	TargetCompany1 = Company1;
	TargetCompany2 = Company2;

	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	if (TargetCompany1 == PlayerCompany)
	{
		InitialLabel = FText::Format(LOCTEXT("PlayerAtWar", "Declare war on {0}"),
								 TargetCompany2->GetCompanyName());
	}
	else if (TargetCompany2 == PlayerCompany)
	{
		InitialLabel = FText::Format(LOCTEXT("PlayerAtWar", "Declare war on {0}"),
								 TargetCompany1->GetCompanyName());
	}
	else
	{
		InitialLabel = FText::Format(LOCTEXT("OtherCompanyAtWar", "{0} and {1} are at war"),
								 TargetCompany1->GetCompanyName(), TargetCompany2->GetCompanyName());
	}
}

bool UFlareQuestConditionAtWar::IsCompleted()
{
	return TargetCompany1->GetWarState(TargetCompany2) == EFlareHostility::Hostile;
}

void UFlareQuestConditionAtWar::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress =IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	At peace condition
----------------------------------------------------*/
UFlareQuestConditionAtPeace::UFlareQuestConditionAtPeace(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionAtPeace* UFlareQuestConditionAtPeace::Create(UFlareQuest* ParentQuest, UFlareCompany* Company1, UFlareCompany* Company2)
{
	UFlareQuestConditionAtPeace* Condition = NewObject<UFlareQuestConditionAtPeace>(ParentQuest, UFlareQuestConditionAtPeace::StaticClass());
	Condition->Load(ParentQuest, Company1, Company2);
	return Condition;
}

void UFlareQuestConditionAtPeace::Load(UFlareQuest* ParentQuest, UFlareCompany* Company1, UFlareCompany* Company2)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::WAR_STATE_CHANGED);
	TargetCompany1 = Company1;
	TargetCompany2 = Company2;

	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	if (TargetCompany1 == PlayerCompany)
	{
		InitialLabel = FText::Format(LOCTEXT("PlayerAtPeace", "Make peace with {0}"),
								 TargetCompany2->GetCompanyName());
	}
	else if (TargetCompany2 == PlayerCompany)
	{
		InitialLabel = FText::Format(LOCTEXT("PlayerAtPeace", "Make peace with {0}"),
								 TargetCompany1->GetCompanyName());
	}
	else
	{
		InitialLabel = FText::Format(LOCTEXT("OtherCompanyAtPeace", "{0} and {1} are at peace"),
								 TargetCompany1->GetCompanyName(), TargetCompany2->GetCompanyName());
	}
}

bool UFlareQuestConditionAtPeace::IsCompleted()
{
	return TargetCompany1->GetWarState(TargetCompany2) != EFlareHostility::Hostile;
}

void UFlareQuestConditionAtPeace::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress =IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}



/*----------------------------------------------------
	Spacecraft no more exist condition
----------------------------------------------------*/
UFlareQuestConditionSpacecraftNoMoreExist::UFlareQuestConditionSpacecraftNoMoreExist(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionSpacecraftNoMoreExist* UFlareQuestConditionSpacecraftNoMoreExist::Create(UFlareQuest* ParentQuest, UFlareSimulatedSpacecraft* TargetSpacecraftParam, FName TargetSpacecraftIdParam)
{
	UFlareQuestConditionSpacecraftNoMoreExist* Condition = NewObject<UFlareQuestConditionSpacecraftNoMoreExist>(ParentQuest, UFlareQuestConditionSpacecraftNoMoreExist::StaticClass());
	Condition->Load(ParentQuest, TargetSpacecraftParam, TargetSpacecraftIdParam);
	return Condition;
}

void UFlareQuestConditionSpacecraftNoMoreExist::Load(UFlareQuest* ParentQuest, UFlareSimulatedSpacecraft* TargetSpacecraftParam, FName TargetSpacecraftIdParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::SPACECRAFT_DESTROYED);
	TargetSpacecraft = TargetSpacecraftParam;
	TargetSpacecraftId = TargetSpacecraftIdParam;
}

bool UFlareQuestConditionSpacecraftNoMoreExist::IsCompleted()
{
	if(TargetSpacecraftId != NAME_None)
	{
		FName SpacecraftName = Quest->GetSaveBundle().GetName(TargetSpacecraftId);
		TargetSpacecraft = GetGame()->GetGameWorld()->FindSpacecraft(SpacecraftName);
	}

	if (TargetSpacecraft)
	{
		return TargetSpacecraft->IsDestroyed() || !TargetSpacecraft->GetDamageSystem()->IsAlive();
	}
	else
	{
		return true;
	}
}


FText UFlareQuestConditionSpacecraftNoMoreExist::GetInitialLabel()
{
	if(TargetSpacecraftId != NAME_None)
	{
		FName SpacecraftName = Quest->GetSaveBundle().GetName(TargetSpacecraftId);
		TargetSpacecraft = GetGame()->GetGameWorld()->FindSpacecraft(SpacecraftName);
	}

	if (TargetSpacecraft)
	{
		return FText::Format(LOCTEXT("SpacecraftNoMoreExist", "{0} is destroyed"),
							 UFlareGameTools::DisplaySpacecraftName(TargetSpacecraft));
	}
	return FText();
}

void UFlareQuestConditionSpacecraftNoMoreExist::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
}

/*----------------------------------------------------
	Buy at station condition
----------------------------------------------------*/

UFlareQuestConditionBuyAtStation::UFlareQuestConditionBuyAtStation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionBuyAtStation* UFlareQuestConditionBuyAtStation::Create(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, UFlareSimulatedSpacecraft* StationParam, FFlareResourceDescription* ResourceParam, int32 QuantityParam)
{
	UFlareQuestConditionBuyAtStation*Condition = NewObject<UFlareQuestConditionBuyAtStation>(ParentQuest, UFlareQuestConditionBuyAtStation::StaticClass());
	Condition->Load(ParentQuest, ConditionIdentifierParam, StationParam, ResourceParam, QuantityParam);
	return Condition;
}

void UFlareQuestConditionBuyAtStation::Load(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, UFlareSimulatedSpacecraft* StationParam, FFlareResourceDescription* ResourceParam, int32 QuantityParam)
{
	if (ConditionIdentifierParam == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionBuyAtStation need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifierParam);
	Callbacks.AddUnique(EFlareQuestCallback::TRADE_DONE);
	TargetStation = StationParam;
	Resource = ResourceParam;
	Quantity = QuantityParam;


	if (TargetStation->IsDestroyed())
	{
		InitialLabel = FText::Format(LOCTEXT("BuyAtStationDestroyed", "Buy {0} {1} from {2} (destroyed)"),
								 FText::AsNumber(Quantity), Resource->Name, UFlareGameTools::DisplaySpacecraftName(TargetStation));
	}
	else
	{
		InitialLabel = FText::Format(LOCTEXT("BuyAtStation", "Buy {0} {1} from {2} at {3}"),
								 FText::AsNumber(Quantity), Resource->Name, UFlareGameTools::DisplaySpacecraftName(TargetStation), TargetStation->GetCurrentSector()->GetSectorName());
	}
}

void UFlareQuestConditionBuyAtStation::Restore(const FFlareBundle* Bundle)
{
	bool HasSave = true;
	if(Bundle)
	{
		HasSave &= Bundle->HasInt32(CURRENT_PROGRESSION_TAG);
	}
	else
	{
		HasSave = false;
	}

	if(HasSave)
	{
		CurrentProgression = Bundle->GetInt32(CURRENT_PROGRESSION_TAG);
	}
	else
	{
		CurrentProgression = 0;
	}

}

void UFlareQuestConditionBuyAtStation::Save(FFlareBundle* Bundle)
{
	Bundle->PutInt32(CURRENT_PROGRESSION_TAG, CurrentProgression);
}


bool UFlareQuestConditionBuyAtStation::IsCompleted()
{
	return CurrentProgression >= Quantity;
}

void UFlareQuestConditionBuyAtStation::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = Quantity;
	ObjectiveCondition.MaxProgress = Quantity;
	ObjectiveCondition.Counter = CurrentProgression;
	ObjectiveCondition.Progress = CurrentProgression;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
	ObjectiveData->AddTargetSpacecraft(TargetStation);
}

void UFlareQuestConditionBuyAtStation::OnTradeDone(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* TradeResource, int32 TradeQuantity)
{
	if(SourceSpacecraft != TargetStation)
	{
		return;
	}

	if(DestinationSpacecraft->GetCompany() != GetGame()->GetPC()->GetCompany())
	{
		return;
	}

	if(Resource != TradeResource)
	{
		return;
	}

	CurrentProgression+=TradeQuantity;
	if (CurrentProgression >= Quantity)
	{
		CurrentProgression = Quantity;
	}
}

/*----------------------------------------------------
	Sell at station condition
----------------------------------------------------*/

UFlareQuestConditionSellAtStation::UFlareQuestConditionSellAtStation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionSellAtStation* UFlareQuestConditionSellAtStation::Create(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, UFlareSimulatedSpacecraft* StationParam, FFlareResourceDescription* ResourceParam, int32 QuantityParam)
{
	UFlareQuestConditionSellAtStation*Condition = NewObject<UFlareQuestConditionSellAtStation>(ParentQuest, UFlareQuestConditionSellAtStation::StaticClass());
	Condition->Load(ParentQuest, ConditionIdentifierParam, StationParam, ResourceParam, QuantityParam);
	return Condition;
}

void UFlareQuestConditionSellAtStation::Load(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, UFlareSimulatedSpacecraft* StationParam, FFlareResourceDescription* ResourceParam, int32 QuantityParam)
{
	if (ConditionIdentifierParam == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionSellAtStation need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifierParam);
	Callbacks.AddUnique(EFlareQuestCallback::TRADE_DONE);
	TargetStation = StationParam;
	Resource = ResourceParam;
	Quantity = QuantityParam;

	if (TargetStation->IsDestroyed())
	{
		InitialLabel = FText::Format(LOCTEXT("SellAtStationDestroyed", "Sell {0} {1} to {2} (destroyed)"),
								 FText::AsNumber(Quantity), Resource->Name, UFlareGameTools::DisplaySpacecraftName(TargetStation));
	}
	else
	{
		InitialLabel = FText::Format(LOCTEXT("SellAtStation", "Sell {0} {1} to {2} at {3}"),
								 FText::AsNumber(Quantity), Resource->Name, UFlareGameTools::DisplaySpacecraftName(TargetStation), TargetStation->GetCurrentSector()->GetSectorName());
	}
}

void UFlareQuestConditionSellAtStation::Restore(const FFlareBundle* Bundle)
{
	bool HasSave = true;
	if(Bundle)
	{
		HasSave &= Bundle->HasInt32(CURRENT_PROGRESSION_TAG);
	}
	else
	{
		HasSave = false;
	}

	if(HasSave)
	{
		CurrentProgression = Bundle->GetInt32(CURRENT_PROGRESSION_TAG);
	}
	else
	{
		CurrentProgression = 0;
	}

}

void UFlareQuestConditionSellAtStation::Save(FFlareBundle* Bundle)
{
	Bundle->PutInt32(CURRENT_PROGRESSION_TAG, CurrentProgression);
}


bool UFlareQuestConditionSellAtStation::IsCompleted()
{
	return CurrentProgression >= Quantity;
}

void UFlareQuestConditionSellAtStation::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = Quantity;
	ObjectiveCondition.MaxProgress = Quantity;
	ObjectiveCondition.Counter = CurrentProgression;
	ObjectiveCondition.Progress = CurrentProgression;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
	ObjectiveData->AddTargetSpacecraft(TargetStation);
}

void UFlareQuestConditionSellAtStation::OnTradeDone(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* TradeResource, int32 TradeQuantity)
{
	if(DestinationSpacecraft != TargetStation)
	{
		return;
	}

	if(SourceSpacecraft->GetCompany() != GetGame()->GetPC()->GetCompany())
	{
		return;
	}

	if(Resource != TradeResource)
	{
		return;
	}

	CurrentProgression+=TradeQuantity;
	if (CurrentProgression >= Quantity)
	{
		CurrentProgression = Quantity;
	}
}

/*----------------------------------------------------
	Time after availability date condition
----------------------------------------------------*/
UFlareQuestConditionTimeAfterAvailableDate::UFlareQuestConditionTimeAfterAvailableDate(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTimeAfterAvailableDate* UFlareQuestConditionTimeAfterAvailableDate::Create(UFlareQuest* ParentQuest, int64 Duration)
{
	UFlareQuestConditionTimeAfterAvailableDate*Condition = NewObject<UFlareQuestConditionTimeAfterAvailableDate>(ParentQuest, UFlareQuestConditionTimeAfterAvailableDate::StaticClass());
	Condition->Load(ParentQuest, Duration);
	return Condition;
}

void UFlareQuestConditionTimeAfterAvailableDate::Load(UFlareQuest* ParentQuest, int64 Duration)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::NEXT_DAY);
	DurationLimit = Duration;
}

FText UFlareQuestConditionTimeAfterAvailableDate::GetInitialLabel()
{
	int64 AvailabilityDate = Quest->GetAvailableDate();
	int64 RemainingDuration = DurationLimit - (GetGame()->GetGameWorld()->GetDate()- AvailabilityDate);

	if (RemainingDuration > 0)
	{
		return FText::Format(LOCTEXT("TimeAfterAvailableRemainingDurationFormat", "{0} left"), UFlareGameTools::FormatDate(RemainingDuration, 2));
	}
	else if (RemainingDuration == 0)
	{
		return LOCTEXT("TimeAfterAvailableRemainingDurationZero", "Last day");
	}
	else
	{
		return LOCTEXT("TimeAfterAvailableRemainingDurationLateFormat", "Too late");
	}
}

bool UFlareQuestConditionTimeAfterAvailableDate::IsCompleted()
{
	float AvailabilityDate = Quest->GetAvailableDate();
	return GetGame()->GetGameWorld()->GetDate()- AvailabilityDate > DurationLimit;
}

void UFlareQuestConditionTimeAfterAvailableDate::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = 0;
	ObjectiveCondition.Counter = (IsCompleted()) ? 1 : 0;
	ObjectiveCondition.MaxCounter = 1;

	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	After date condition
----------------------------------------------------*/
UFlareQuestConditionAfterDate::UFlareQuestConditionAfterDate(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionAfterDate* UFlareQuestConditionAfterDate::Create(UFlareQuest* ParentQuest, int64 Date)
{
	UFlareQuestConditionAfterDate*Condition = NewObject<UFlareQuestConditionAfterDate>(ParentQuest, UFlareQuestConditionAfterDate::StaticClass());
	Condition->Load(ParentQuest, Date);
	return Condition;
}

void UFlareQuestConditionAfterDate::Load(UFlareQuest* ParentQuest, int64 Date)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::NEXT_DAY);
	DateLimit = Date;
}

FText UFlareQuestConditionAfterDate::GetInitialLabel()
{
	int64 RemainingDuration = DateLimit - GetGame()->GetGameWorld()->GetDate();

	return FText::Format(LOCTEXT("AfterDateRemainingDurationFormat", "{0} remaining"), UFlareGameTools::FormatDate(RemainingDuration+1, 2));
}

bool UFlareQuestConditionAfterDate::IsCompleted()
{
	float AvailabilityDate = Quest->GetAvailableDate();
	return GetGame()->GetGameWorld()->GetDate() > DateLimit;
}

void UFlareQuestConditionAfterDate::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = 0;
	ObjectiveCondition.Counter = (IsCompleted()) ? 1 : 0;
	ObjectiveCondition.MaxCounter = 1;

	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	Min army combat point in sector condition
----------------------------------------------------*/
UFlareQuestConditionMinArmyCombatPointsInSector::UFlareQuestConditionMinArmyCombatPointsInSector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionMinArmyCombatPointsInSector* UFlareQuestConditionMinArmyCombatPointsInSector::Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam, int32 TargetArmyPointsParam)
{
	UFlareQuestConditionMinArmyCombatPointsInSector* Condition = NewObject<UFlareQuestConditionMinArmyCombatPointsInSector>(ParentQuest, UFlareQuestConditionMinArmyCombatPointsInSector::StaticClass());
	Condition->Load(ParentQuest, TargetSectorParam, TargetCompanyParam, TargetArmyPointsParam);
	return Condition;
}

void UFlareQuestConditionMinArmyCombatPointsInSector::Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam, int32 TargetArmyPointsParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
	Callbacks.AddUnique(EFlareQuestCallback::NEXT_DAY);
	TargetSector = TargetSectorParam;
	TargetCompany = TargetCompanyParam;
	TargetArmyPoints = TargetArmyPointsParam;

	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	if (TargetCompany == PlayerCompany)
	{
		FText InitialLabelText = LOCTEXT("PlayerMinArmyCombatPoints", "Bring at least {0} combat value in {1}");
		InitialLabel = FText::Format(InitialLabelText, FText::AsNumber(TargetArmyPoints), TargetSector->GetSectorName());
	}
	else
	{
		FText InitialLabelText = LOCTEXT("CompanyMinArmyCombatPoints", "{0} must have at least {1} combat value in {2}");
		InitialLabel = FText::Format(InitialLabelText, TargetCompany->GetCompanyName(), FText::AsNumber(TargetArmyPoints), TargetSector->GetSectorName());
	}
}

bool UFlareQuestConditionMinArmyCombatPointsInSector::IsCompleted()
{
	int32 armyPoints = SectorHelper::GetCompanyArmyCombatPoints(TargetSector, TargetCompany, true);

	return armyPoints >= TargetArmyPoints;
}

void UFlareQuestConditionMinArmyCombatPointsInSector::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = 0;
	ObjectiveCondition.Counter = SectorHelper::GetCompanyArmyCombatPoints(TargetSector, TargetCompany, true);
	ObjectiveCondition.MaxCounter = TargetArmyPoints;

	ObjectiveData->ConditionList.Add(ObjectiveCondition);
	ObjectiveData->TargetSectors.Add(TargetSector);

}

/*----------------------------------------------------
	Max army combat point in sector condition
----------------------------------------------------*/
UFlareQuestConditionMaxArmyCombatPointsInSector::UFlareQuestConditionMaxArmyCombatPointsInSector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionMaxArmyCombatPointsInSector* UFlareQuestConditionMaxArmyCombatPointsInSector::Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam, int32 TargetArmyPointsParam)
{
	UFlareQuestConditionMaxArmyCombatPointsInSector* Condition = NewObject<UFlareQuestConditionMaxArmyCombatPointsInSector>(ParentQuest, UFlareQuestConditionMaxArmyCombatPointsInSector::StaticClass());
	Condition->Load(ParentQuest, TargetSectorParam, TargetCompanyParam, TargetArmyPointsParam);
	return Condition;
}

void UFlareQuestConditionMaxArmyCombatPointsInSector::Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam, int32 TargetArmyPointsParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
	Callbacks.AddUnique(EFlareQuestCallback::NEXT_DAY);
	TargetSector = TargetSectorParam;
	TargetCompany = TargetCompanyParam;
	TargetArmyPoints = TargetArmyPointsParam;

	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	if (TargetCompany == PlayerCompany)
	{
		if(TargetArmyPoints == 0)
		{
			FText InitialLabelText = LOCTEXT("PlayerMaxArmyCombatPointsZero", "Die fighting in {0}");
			InitialLabel = FText::Format(InitialLabelText, TargetSector->GetSectorName());

		}
		else
		{
			FText InitialLabelText = LOCTEXT("PlayerMaxArmyCombatPoints", "Bring at most {0} combat value in {1}");
			InitialLabel = FText::Format(InitialLabelText, FText::AsNumber(TargetArmyPoints), TargetSector->GetSectorName());
		}
	}
	else
	{
		FText InitialLabelText = LOCTEXT("CompanyMasArmyCombatPoints", "{0} must have at most {1} combat value in {2}");
		InitialLabel = FText::Format(InitialLabelText, TargetCompany->GetCompanyName(), FText::AsNumber(TargetArmyPoints), TargetSector->GetSectorName());
	}
}

bool UFlareQuestConditionMaxArmyCombatPointsInSector::IsCompleted()
{
	int32 armyPoints = SectorHelper::GetCompanyArmyCombatPoints(TargetSector, TargetCompany, true);

	return armyPoints <= TargetArmyPoints;
}

void UFlareQuestConditionMaxArmyCombatPointsInSector::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = 0;
	ObjectiveCondition.Counter = SectorHelper::GetCompanyArmyCombatPoints(TargetSector, TargetCompany, true);
	ObjectiveCondition.MaxCounter = TargetArmyPoints;

	ObjectiveData->ConditionList.Add(ObjectiveCondition);
	ObjectiveData->TargetSectors.Add(TargetSector);
}

/*----------------------------------------------------
	No battle in sector condition
----------------------------------------------------*/
UFlareQuestConditionNoBattleInSector::UFlareQuestConditionNoBattleInSector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionNoBattleInSector* UFlareQuestConditionNoBattleInSector::Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam)
{
	UFlareQuestConditionNoBattleInSector* Condition = NewObject<UFlareQuestConditionNoBattleInSector>(ParentQuest, UFlareQuestConditionNoBattleInSector::StaticClass());
	Condition->Load(ParentQuest, TargetSectorParam, TargetCompanyParam);
	return Condition;
}

void UFlareQuestConditionNoBattleInSector::Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
	Callbacks.AddUnique(EFlareQuestCallback::NEXT_DAY);
	TargetSector = TargetSectorParam;
	TargetCompany = TargetCompanyParam;

	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	if (TargetCompany == PlayerCompany)
	{
		FText InitialLabelText = LOCTEXT("PlayerNoBattle", "Battle finished in {0}");
		InitialLabel = FText::Format(InitialLabelText, TargetSector->GetSectorName());


	}
	else
	{
		FText InitialLabelText = LOCTEXT("CompanyNoBattle", "Battle finished for {0} in {1}");
		InitialLabel = FText::Format(InitialLabelText, TargetCompany->GetCompanyName(), TargetSector->GetSectorName());
	}
}

bool UFlareQuestConditionNoBattleInSector::IsCompleted()
{
	return !TargetSector->GetSectorBattleState(TargetCompany).InFight;
}

void UFlareQuestConditionNoBattleInSector::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = 0;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.MaxCounter = 1;

	ObjectiveData->ConditionList.Add(ObjectiveCondition);
	ObjectiveData->TargetSectors.Add(TargetSector);
}

/*----------------------------------------------------
	Station lost in secotr condition
----------------------------------------------------*/
UFlareQuestConditionStationLostInSector::UFlareQuestConditionStationLostInSector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionStationLostInSector* UFlareQuestConditionStationLostInSector::Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam)
{
	UFlareQuestConditionStationLostInSector* Condition = NewObject<UFlareQuestConditionStationLostInSector>(ParentQuest, UFlareQuestConditionStationLostInSector::StaticClass());
	Condition->Load(ParentQuest, TargetSectorParam, TargetCompanyParam);
	return Condition;
}

void UFlareQuestConditionStationLostInSector::Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::SPACECRAFT_CAPTURED);
	Completed = false;
	TargetSector = TargetSectorParam;
	TargetCompany = TargetCompanyParam;

	InitialLabel = FText::Format(LOCTEXT("StationLostInSector", "A station is lost by {0} in {1}"),
										 TargetCompany->GetCompanyName(), TargetSector->GetSectorName());
}

void UFlareQuestConditionStationLostInSector::OnSpacecraftCaptured(UFlareSimulatedSpacecraft* CapturedSpacecraftBefore, UFlareSimulatedSpacecraft* CapturedSpacecraftAfter)
{
	if(CapturedSpacecraftBefore->GetCompany() == TargetCompany && CapturedSpacecraftBefore->GetCurrentSector() == TargetSector)
	{
		Completed = true;
	}
}

bool UFlareQuestConditionStationLostInSector::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionStationLostInSector::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
}

/*----------------------------------------------------
	No capturing station condition
----------------------------------------------------*/
UFlareQuestConditionNoCapturingStationInSector::UFlareQuestConditionNoCapturingStationInSector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	HasInitialCapturingStations = false;
}

UFlareQuestConditionNoCapturingStationInSector* UFlareQuestConditionNoCapturingStationInSector::Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam, UFlareCompany* TargetEnemyCompanyParam)
{
	UFlareQuestConditionNoCapturingStationInSector* Condition = NewObject<UFlareQuestConditionNoCapturingStationInSector>(ParentQuest, UFlareQuestConditionNoCapturingStationInSector::StaticClass());
	Condition->Load(ParentQuest, TargetSectorParam, TargetCompanyParam, TargetEnemyCompanyParam);
	return Condition;
}

void UFlareQuestConditionNoCapturingStationInSector::Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam, UFlareCompany* TargetEnemyCompanyParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::SPACECRAFT_CAPTURED);
	TargetSector = TargetSectorParam;
	TargetCompany = TargetCompanyParam;
	TargetEnemyCompany = TargetEnemyCompanyParam;


	FText InitialLabelText = LOCTEXT("NoCapturingStation", "No station owned by {0} must be captured by {1} in {2}");
	InitialLabel = FText::Format(InitialLabelText, TargetCompany->GetCompanyName(), TargetEnemyCompany->GetCompanyName(), TargetSector->GetSectorName());
}


void UFlareQuestConditionNoCapturingStationInSector::Restore(const FFlareBundle* Bundle)
{
	if (Bundle)
	{
		HasInitialCapturingStations = Bundle->HasInt32(INITIAL_CAPTURING_STATIONS_TAG);
		InitialCapturingStations = Bundle->GetInt32(INITIAL_CAPTURING_STATIONS_TAG);
	}
	else
	{
		HasInitialCapturingStations = false;
	}
}

void UFlareQuestConditionNoCapturingStationInSector::Save(FFlareBundle* Bundle)
{
	if (HasInitialCapturingStations)
	{
		Bundle->PutFloat(INITIAL_CAPTURING_STATIONS_TAG, InitialCapturingStations);
	}
}


int32 UFlareQuestConditionNoCapturingStationInSector::GetCapturingStations()
{
	int32 CapturingStationCount = 0;
	for(UFlareSimulatedSpacecraft* Station : TargetSector->GetSectorStations())
	{

		if(Station->GetCompany() != TargetCompany)
		{
			continue;
		}

		if(Station->GetCapturePoint(TargetEnemyCompany) > 0)
		{
			CapturingStationCount++;
		}
	}
	return CapturingStationCount;
}

bool UFlareQuestConditionNoCapturingStationInSector::IsCompleted()
{

	if (!HasInitialCapturingStations)
	{
		InitialCapturingStations = GetCapturingStations();
	}

	for(UFlareSimulatedSpacecraft* Station : TargetSector->GetSectorStations())
	{

		if(Station->GetCompany() != TargetCompany)
		{
			continue;
		}

		if(Station->GetCapturePoint(TargetEnemyCompany) > 0)
		{
			return false;
		}
	}
	return true;
}

void UFlareQuestConditionNoCapturingStationInSector::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	if (!HasInitialCapturingStations)
	{
		InitialCapturingStations = GetCapturingStations();
	}

	int32 CurrentStations = GetCapturingStations();


	int32 Progress = InitialCapturingStations - CurrentStations;

	FText StationShortText;
	if(CurrentStations > 1)
	{
		StationShortText = LOCTEXT("StationToSecureFormatOne", "{0} stations left");
	}
	else
	{
		StationShortText = LOCTEXT("StationToSecureFormat", "{0} station left");
	}

	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText::Format(StationShortText, FText::AsNumber(CurrentStations));
	ObjectiveCondition.Progress = Progress;
	ObjectiveCondition.MaxProgress = InitialCapturingStations;
	ObjectiveCondition.Counter = 0;
	ObjectiveCondition.MaxCounter = 0;

	ObjectiveData->ConditionList.Add(ObjectiveCondition);
	ObjectiveData->TargetSectors.Add(TargetSector);
}


/*----------------------------------------------------
	Retreat dangerous ships condition
----------------------------------------------------*/
UFlareQuestConditionRetreatDangerousShip::UFlareQuestConditionRetreatDangerousShip(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionRetreatDangerousShip* UFlareQuestConditionRetreatDangerousShip::Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam)
{
	UFlareQuestConditionRetreatDangerousShip* Condition = NewObject<UFlareQuestConditionRetreatDangerousShip>(ParentQuest, UFlareQuestConditionRetreatDangerousShip::StaticClass());
	Condition->Load(ParentQuest, TargetSectorParam, TargetCompanyParam);
	return Condition;
}

void UFlareQuestConditionRetreatDangerousShip::Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::TRAVEL_STARTED);
	Completed = false;
	TargetSector = TargetSectorParam;
	TargetCompany = TargetCompanyParam;

	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	if (TargetCompany == PlayerCompany)
	{
		InitialLabel = FText::Format(LOCTEXT("PlayerRetreatDangerousShip", "Retreat combat-ready ship from {0}"), TargetSector->GetSectorName());
	}
	else
	{
		InitialLabel = FText::Format(LOCTEXT("RetreatDangerousShip", "{0} retreat combat-ready ship from {1}"),
									 TargetCompany->GetCompanyName(),
									 TargetSector->GetSectorName());
	}
}

void UFlareQuestConditionRetreatDangerousShip::OnTravelStarted(UFlareTravel* Travel)
{
	if(Completed)
	{
		return;
	}

	if (Travel->GetFleet()->GetFleetCompany() != TargetCompany)
	{
		return;
	}

	if (Travel->GetSourceSector() != TargetSector)
	{
		return;
	}


	for(UFlareSimulatedSpacecraft* Ship : Travel->GetFleet()->GetShips())
	{
		if(Ship->IsMilitary()  && !Ship->GetDamageSystem()->IsDisarmed())
		{
			Completed = true;
			break;
		}
	}
}

bool UFlareQuestConditionRetreatDangerousShip::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionRetreatDangerousShip::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
}


/*----------------------------------------------------
	Destroy spacecraft condition
----------------------------------------------------*/

UFlareQuestConditionDestroySpacecraft::UFlareQuestConditionDestroySpacecraft(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionDestroySpacecraft* UFlareQuestConditionDestroySpacecraft::Create(UFlareQuest* ParentQuest,
																					 FName ConditionIdentifierParam,
																					 UFlareCompany* HostileCompanyParam,
																					 int32 SpacecraftCountParam,
																					 bool MilitaryParam,
																					 EFlarePartSize::Type SizeParam,
																					 bool DestroyTargetParam)
{
	UFlareQuestConditionDestroySpacecraft*Condition = NewObject<UFlareQuestConditionDestroySpacecraft>(ParentQuest, UFlareQuestConditionDestroySpacecraft::StaticClass());
	Condition->Load(ParentQuest, ConditionIdentifierParam, HostileCompanyParam, SpacecraftCountParam, MilitaryParam, SizeParam, DestroyTargetParam);
	return Condition;
}

void UFlareQuestConditionDestroySpacecraft::Load(UFlareQuest* ParentQuest,
												 FName ConditionIdentifierParam,
												 UFlareCompany* HostileCompanyParam,
												 int32 SpacecraftCountParam,
												 bool MilitaryParam,
												 EFlarePartSize::Type SizeParam,
												 bool DestroyTargetParam)
{
	if (ConditionIdentifierParam == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionDestroySpacecraft need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifierParam);
	Callbacks.AddUnique(EFlareQuestCallback::SPACECRAFT_DESTROYED);


	TargetCompany = HostileCompanyParam;
	Quantity = SpacecraftCountParam;
	TargetSize = SizeParam;
	DestroyTarget = DestroyTargetParam;
	MilitaryTarget = MilitaryParam;


	if(DestroyTarget)
	{
		InitialLabel = FText::Format(LOCTEXT("DestroySpacecraftLabel","Destroy {0} of {3}'s {1} {2} ships"),
									 FText::AsNumber(Quantity),
									 TargetSize == EFlarePartSize::L? LOCTEXT("TargetLarge","large") : LOCTEXT("TargetSmall","small"),
									 MilitaryTarget? LOCTEXT("TargetMilitay","military") : LOCTEXT("TargetCargo","cargo"),
									 TargetCompany->GetCompanyName());
	}
	else
	{
		InitialLabel = FText::Format(LOCTEXT("UncontrollableSpacecraftLabel","Render {0} of {3}'s {1} {2} ships uncontrollable"),
										 FText::AsNumber(Quantity),
										 TargetSize == EFlarePartSize::L? LOCTEXT("TargetLarge","large") : LOCTEXT("TargetSmall","small"),
										 MilitaryTarget? LOCTEXT("TargetMilitay","military") : LOCTEXT("TargetCargo","cargo"),
										 TargetCompany->GetCompanyName());

	}
}

void UFlareQuestConditionDestroySpacecraft::Restore(const FFlareBundle* Bundle)
{
	bool HasSave = true;
	if(Bundle)
	{
		HasSave &= Bundle->HasInt32(CURRENT_PROGRESSION_TAG);
	}
	else
	{
		HasSave = false;
	}

	if(HasSave)
	{
		CurrentProgression = Bundle->GetInt32(CURRENT_PROGRESSION_TAG);
	}
	else
	{
		CurrentProgression = 0;
	}

}

void UFlareQuestConditionDestroySpacecraft::Save(FFlareBundle* Bundle)
{
	Bundle->PutInt32(CURRENT_PROGRESSION_TAG, CurrentProgression);
}


bool UFlareQuestConditionDestroySpacecraft::IsCompleted()
{
	return CurrentProgression >= Quantity;
}

void UFlareQuestConditionDestroySpacecraft::OnSpacecraftDestroyed(UFlareSimulatedSpacecraft *Spacecraft, bool Uncontrollable, DamageCause Cause)
{
	if(Spacecraft->GetCompany() != TargetCompany)
	{
		return;
	}

	if(DestroyTarget != (!Uncontrollable))
	{
		return;
	}

	if(Spacecraft->GetSize() != TargetSize)
	{
		return;
	}

	if(Spacecraft->IsMilitary() != MilitaryTarget)
	{
		return;
	}

	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();
	if(PlayerCompany != Cause.Company)
	{
		return;
	}

	CurrentProgression++;
}


void UFlareQuestConditionDestroySpacecraft::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = Quantity;
	ObjectiveCondition.MaxProgress = Quantity;
	ObjectiveCondition.Counter = CurrentProgression;
	ObjectiveCondition.Progress = CurrentProgression;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	Destroy combat value condition
----------------------------------------------------*/

UFlareQuestConditionDestroyCombatValue::UFlareQuestConditionDestroyCombatValue(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionDestroyCombatValue* UFlareQuestConditionDestroyCombatValue::Create(UFlareQuest* ParentQuest,
																					 FName ConditionIdentifierParam,
																					 UFlareCompany* HostileCompanyParam,
																					 int32 CombatValueParam,
																					 bool DestroyTargetParam)
{
	UFlareQuestConditionDestroyCombatValue*Condition = NewObject<UFlareQuestConditionDestroyCombatValue>(ParentQuest, UFlareQuestConditionDestroyCombatValue::StaticClass());
	Condition->Load(ParentQuest, ConditionIdentifierParam, HostileCompanyParam, CombatValueParam, DestroyTargetParam);
	return Condition;
}

void UFlareQuestConditionDestroyCombatValue::Load(UFlareQuest* ParentQuest,
												 FName ConditionIdentifierParam,
												 UFlareCompany* HostileCompanyParam,
												 int32 CombatValueParam,
												 bool DestroyTargetParam)
{
	if (ConditionIdentifierParam == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionDestroyCombatValue need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifierParam);
	Callbacks.AddUnique(EFlareQuestCallback::SPACECRAFT_DESTROYED);


	TargetCompany = HostileCompanyParam;
	Quantity = CombatValueParam;
	DestroyTarget = DestroyTargetParam;


	if(DestroyTarget)
	{
		InitialLabel = FText::Format(LOCTEXT("DestroyCombatValueLabel","Destroy some of {1}'s ships to lower its combat value by {0} combats points"),
									 FText::AsNumber(Quantity),
									 TargetCompany->GetCompanyName());
	}
	else
	{
		InitialLabel = FText::Format(LOCTEXT("UncontrollableCombatValueLabel","Render some of {1}'s ships uncontrollable to lower its combat value by {0} combats points"),
										 FText::AsNumber(Quantity),
										 TargetCompany->GetCompanyName());

	}
}

void UFlareQuestConditionDestroyCombatValue::Restore(const FFlareBundle* Bundle)
{
	bool HasSave = true;
	if(Bundle)
	{
		HasSave &= Bundle->HasInt32(CURRENT_PROGRESSION_TAG);
	}
	else
	{
		HasSave = false;
	}

	if(HasSave)
	{
		CurrentProgression = Bundle->GetInt32(CURRENT_PROGRESSION_TAG);
	}
	else
	{
		CurrentProgression = 0;
	}

}

void UFlareQuestConditionDestroyCombatValue::Save(FFlareBundle* Bundle)
{
	Bundle->PutInt32(CURRENT_PROGRESSION_TAG, CurrentProgression);
}


bool UFlareQuestConditionDestroyCombatValue::IsCompleted()
{
	return CurrentProgression >= Quantity;
}

void UFlareQuestConditionDestroyCombatValue::OnSpacecraftDestroyed(UFlareSimulatedSpacecraft *Spacecraft, bool Uncontrollable, DamageCause Cause)
{
	if(Spacecraft->GetCompany() != TargetCompany)
	{
		return;
	}

	if(DestroyTarget != (!Uncontrollable))
	{
		return;
	}

	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();
	if(PlayerCompany != Cause.Company)
	{
		return;
	}

	CurrentProgression+=Spacecraft->GetCombatPoints(false);
}

void UFlareQuestConditionDestroyCombatValue::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = Quantity;
	ObjectiveCondition.MaxProgress = Quantity;
	ObjectiveCondition.Counter = CurrentProgression;
	ObjectiveCondition.Progress = CurrentProgression;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}



#undef LOCTEXT_NAMESPACE
