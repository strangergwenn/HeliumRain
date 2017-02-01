#include "Flare.h"
#include "FlareQuestCondition.h"
#include "FlareQuest.h"
#include "../Player/FlarePlayerController.h"

#define LOCTEXT_NAMESPACE "FlareQuestCondition"

static FName INITIAL_VELOCITY_TAG("initial-velocity");
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



bool UFlareQuestCondition::CheckConditions(TArray<UFlareQuestCondition*>& Conditions, bool EmptyResult)
{
	if (Conditions.Num() == 0)
	{
		return EmptyResult;
	}

	for(UFlareQuestCondition* Condition: Conditions)
	{
		if(!Condition->IsCompleted())
		{
			return false;
		}
	}

	return true;
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

	FText ReachSpeedText = LOCTEXT("ReachMinSpeedFormat", "Reach at least {0} m/s forward");
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

	FText ReachSpeedText = LOCTEXT("ReachMinSpeedFormat", "Reach at least {0} m/s backward");
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
		FVector WorldAngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocity();
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
		FVector WorldAngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocity();
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
	Callbacks.AddUnique(EFlareQuestCallback::QUEST);
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
	Callbacks.AddUnique(EFlareQuestCallback::QUEST);
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

UFlareQuestConditionFollowRelativeWaypoints::UFlareQuestConditionFollowRelativeWaypoints(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionFollowRelativeWaypoints* UFlareQuestConditionFollowRelativeWaypoints::Create(UFlareQuest* ParentQuest, FName ConditionIdentifier, TArray<FVector> VectorListParam)
{
	UFlareQuestConditionFollowRelativeWaypoints*Condition = NewObject<UFlareQuestConditionFollowRelativeWaypoints>(ParentQuest, UFlareQuestConditionFollowRelativeWaypoints::StaticClass());
	Condition->Load(ParentQuest, ConditionIdentifier, VectorListParam);
	return Condition;
}

void UFlareQuestConditionFollowRelativeWaypoints::Load(UFlareQuest* ParentQuest, FName ConditionIdentifier, TArray<FVector> VectorListParam)
{
	if (ConditionIdentifier == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionFollowRelativeWaypoints need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifier);
	Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
	VectorList = VectorListParam;
	InitialLabel = LOCTEXT("FollowWaypoints", "Fly to waypoints");
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

		float MaxDistance = WAYPOINTS_RADIUS;

		if (FVector::Dist(Spacecraft->GetActorLocation(), WorldTargetLocation) < MaxDistance)
		{
			// Nearing the target
			if (CurrentProgression + 2 <= VectorList.Num())
			{
				// Progress.
				CurrentProgression++;

				FText WaypointText = LOCTEXT("WaypointProgress", "Waypoint reached, {0} left");

				Quest->SendQuestNotification(FText::Format(WaypointText, FText::AsNumber(VectorList.Num() - CurrentProgression)),
									  FName(*(FString("quest-")+GetIdentifier().ToString()+"-step-progress")),
											 false);
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
		ObjectiveTarget.Actor = NULL;
		ObjectiveTarget.Active = (CurrentProgression == TargetIndex);
		ObjectiveTarget.Radius = WAYPOINTS_RADIUS;

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

UFlareQuestConditionFollowRandomWaypoints* UFlareQuestConditionFollowRandomWaypoints::Create(UFlareQuest* ParentQuest, FName ConditionIdentifier)
{
	UFlareQuestConditionFollowRandomWaypoints*Condition = NewObject<UFlareQuestConditionFollowRandomWaypoints>(ParentQuest, UFlareQuestConditionFollowRandomWaypoints::StaticClass());
	Condition->Load(ParentQuest, ConditionIdentifier);
	return Condition;
}

void UFlareQuestConditionFollowRandomWaypoints::Load(UFlareQuest* ParentQuest, FName ConditionIdentifier)
{
	if (ConditionIdentifier == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionFollowWaypoints need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifier);
	Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
	InitialLabel = LOCTEXT("FollowWaypoints", "Fly to waypoints");
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
		float MaxDistance = 10000;
		float StepDistance = 100;

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
		if(Waypoints.Num() == 0)
		{
			return true;
		}

		FVector WorldTargetLocation = Waypoints[CurrentProgression] * 100;

		float MaxDistance = WAYPOINTS_RADIUS;

		if (FVector::Dist(Spacecraft->GetActorLocation(), WorldTargetLocation) < MaxDistance)
		{
			// Nearing the target
			if (CurrentProgression + 2 <= Waypoints.Num())
			{
				// Progress.
				CurrentProgression++;

				FText WaypointText = LOCTEXT("WaypointProgress", "Waypoint reached, {0} left");

				Quest->SendQuestNotification(FText::Format(WaypointText, FText::AsNumber(Waypoints.Num() - CurrentProgression)),
									  FName(*(FString("quest-")+GetIdentifier().ToString()+"-step-progress")),
											 false);
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
		ObjectiveTarget.Actor = NULL;
		ObjectiveTarget.Active = (CurrentProgression == TargetIndex);
		ObjectiveTarget.Radius = WAYPOINTS_RADIUS;

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



#undef LOCTEXT_NAMESPACE
