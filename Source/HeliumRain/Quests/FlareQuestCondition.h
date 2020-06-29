#pragma once

#include "HeliumRain/Quests/FlareQuestManager.h"
#include "FlareQuestCondition.generated.h"

class UFlareQuest;
class AFlarePlayerController;
class AFlareGame;
class UFlareCompany;
class UFlareSimulatedSector;
class UFlareSimulatedSpacecraft;
class UFlareTravel;
struct FFlarePlayerObjectiveData;
struct FFlareBundle;
struct FFlareResourceDescription;

/** A quest Step condition */
UCLASS(abstract)
class HELIUMRAIN_API UFlareQuestCondition: public UObject
{
	GENERATED_UCLASS_BODY()

public:
	virtual void Restore(const FFlareBundle* Bundle)
	{
	}

	virtual void Save(FFlareBundle* Bundle)
	{
	}

	virtual void AddSave(TArray<FFlareQuestConditionSave>& Data);

	virtual void OnTradeDone(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource, int32 Quantity) {}

	virtual void OnSpacecraftCaptured(UFlareSimulatedSpacecraft* CapturedSpacecraftBefore, UFlareSimulatedSpacecraft* CapturedSpacecraftAfter) {}

	virtual void OnSpacecraftDestroyed(UFlareSimulatedSpacecraft* Spacecraft, bool Uncontrollable, DamageCause Cause) {}

	virtual void OnTravelStarted(UFlareTravel* Travel) {}

	virtual void OnEvent(FFlareBundle& Bundle) {}

	virtual TArray<UFlareQuestCondition*> GetAllConditions(bool OnlyLeaf = true);

	virtual void SetConditionIndex(int32 Index)
	{
		ConditionIndex = Index;
	}

	TArray<EFlareQuestCallback::Type> Callbacks;
protected:

	  void LoadInternal(UFlareQuest* ParentQuest, FName ConditionIdentifier = NAME_None)
	  {
		  Identifier = ConditionIdentifier;
		  Quest = ParentQuest;
	  }

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	int32                                   ConditionIndex;
	FText                       InitialLabel;
	FText						TerminalLabel;
	FName                       Identifier;


	UFlareQuest* Quest;
public:

	/*----------------------------------------------------
	 Getters
	----------------------------------------------------*/

	virtual bool IsCompleted();

	int32 GetConditionIndex()
	{
		return ConditionIndex;
	}

	virtual FText GetInitialLabel()
	{
		/*if(InitialLabel.ToString() == "")
		{
			FLOGV("Empty label for %s", *this->GetClass()->GetName());
		}*/
		return InitialLabel;
	}

	FText GetTerminalLabel()
	{
		return TerminalLabel;
	}

	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	virtual TArray<EFlareQuestCallback::Type>& GetConditionCallbacks()
	{
		return Callbacks;
	}

	static void AddConditionCallbacks(TArray<EFlareQuestCallback::Type>& Callbacks, const TArray<UFlareQuestCondition*>& Conditions);

	static const FFlareBundle* GetStepConditionBundle(UFlareQuestCondition* Condition, const TArray<FFlareQuestConditionSave>& Data);

	FName GetIdentifier()
	{
		return Identifier;
	}

	AFlareGame* GetGame();
	AFlarePlayerController* GetPC();

};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionGroup: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	virtual void AddChildCondition(UFlareQuestCondition* Condition);

	virtual bool IsCompleted() { FCHECK(false); return false; }

	virtual TArray<UFlareQuestCondition*> GetAllConditions(bool OnlyLeaf = true);

protected:

	UPROPERTY()
	TArray<UFlareQuestCondition*>				Conditions;

};




//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionAndGroup: public UFlareQuestConditionGroup
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionAndGroup* Create(UFlareQuest* ParentQuest, bool EmptyValueParam);
	void Load(UFlareQuest* ParentQuest, bool EmptyValueParam);

	void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	virtual bool IsCompleted();

protected:

	bool EmptyValue;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionOrGroup: public UFlareQuestConditionGroup
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionOrGroup* Create(UFlareQuest* ParentQuest, bool EmptyValueParam);
	void Load(UFlareQuest* ParentQuest, bool EmptyValueParam);

	void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	virtual bool IsCompleted();

protected:

	bool EmptyValue;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionFlyingShipClass: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionFlyingShipClass* Create(UFlareQuest* ParentQuest, FName ShipClassParam);
	void Load(UFlareQuest* ParentQuest, FName ShipClassParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	FName ShipClass;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionSectorActive: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionSectorActive* Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	UFlareSimulatedSector* Sector;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionSectorVisited: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionSectorVisited* Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	UFlareSimulatedSector* Sector;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionMinVerticalVelocity: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionMinVerticalVelocity* Create(UFlareQuest* ParentQuest, FName ConditionIdentifier, float VelocityLimitParam);
	void Load(UFlareQuest* ParentQuest, FName ConditionIdentifier, float VelocityLimitParam);
	virtual void Restore(const FFlareBundle* Bundle);
	virtual void Save(FFlareBundle* Bundle);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	float GetVerticalVelocity();

protected:

	float VelocityLimit;
	bool HasInitialVelocity;
	float InitialVelocity;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionMaxVerticalVelocity: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionMaxVerticalVelocity* Create(UFlareQuest* ParentQuest, FName ConditionIdentifier, float VelocityLimitParam);
	void Load(UFlareQuest* ParentQuest, FName ConditionIdentifier, float VelocityLimitParam);
	virtual void Restore(const FFlareBundle* Bundle);
	virtual void Save(FFlareBundle* Bundle);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	float GetVerticalVelocity();

protected:

	float VelocityLimit;
	bool HasInitialVelocity;
	float InitialVelocity;
};


//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionMinCollinearVelocity: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionMinCollinearVelocity* Create(UFlareQuest* ParentQuest, FName ConditionIdentifier, float VelocityLimitParam);
	void Load(UFlareQuest* ParentQuest, FName ConditionIdentifier, float VelocityLimitParam);
	virtual void Restore(const FFlareBundle* Bundle);
	virtual void Save(FFlareBundle* Bundle);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	float GetCollinearVelocity();


protected:

	float VelocityLimit;
	bool HasInitialVelocity;
	float InitialVelocity;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionMaxCollinearVelocity: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionMaxCollinearVelocity* Create(UFlareQuest* ParentQuest, FName ConditionIdentifier, float VelocityLimitParam);
	void Load(UFlareQuest* ParentQuest, FName ConditionIdentifier, float VelocityLimitParam);
	virtual void Restore(const FFlareBundle* Bundle);
	virtual void Save(FFlareBundle* Bundle);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	float GetCollinearVelocity();


protected:

	float VelocityLimit;
	bool HasInitialVelocity;
	float InitialVelocity;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionMinCollinear: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionMinCollinear* Create(UFlareQuest* ParentQuest, float CollinearLimitParam);
	void Load(UFlareQuest* ParentQuest, float CollinearLimitParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	float GetCollinear();

protected:

	float CollinearLimit;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionMaxCollinear: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionMaxCollinear* Create(UFlareQuest* ParentQuest, float CollinearLimitParam);
	void Load(UFlareQuest* ParentQuest, float CollinearLimitParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	float GetCollinear();

protected:

	float CollinearLimit;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionMinRotationVelocity: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionMinRotationVelocity* Create(UFlareQuest* ParentQuest, FVector LocalAxisParam, float AngularVelocityLimitParam);
	void Load(UFlareQuest* ParentQuest, FVector LocalAxisParam, float AngularVelocityLimitParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	float GetVelocityInAxis();

protected:

	FVector LocalAxis;
	float AngularVelocityLimit;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionMaxRotationVelocity: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionMaxRotationVelocity* Create(UFlareQuest* ParentQuest, FVector LocalAxisParam, float AngularVelocityLimitParam);
	void Load(UFlareQuest* ParentQuest, FVector LocalAxisParam, float AngularVelocityLimitParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	float GetVelocityInAxis();

protected:

	FVector LocalAxis;
	float AngularVelocityLimit;
};


//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionShipAlive: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionShipAlive* Create(UFlareQuest* ParentQuest, FName ShipIdentifierParam);
	void Load(UFlareQuest* ParentQuest, FName ShipIdentifierParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	bool IsShipAlive();

protected:

	FName ShipIdentifier;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionQuestSuccessful: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionQuestSuccessful* Create(UFlareQuest* ParentQuest, FName QuestParam);
	void Load(UFlareQuest* ParentQuest, FName QuestParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	FName TargetQuest;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionQuestFailed: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionQuestFailed* Create(UFlareQuest* ParentQuest, FName QuestParam);
	void Load(UFlareQuest* ParentQuest, FName QuestParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	FName TargetQuest;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionFollowRelativeWaypoints: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionFollowRelativeWaypoints* Create(UFlareQuest* ParentQuest, FName ConditionIdentifier, TArray<FVector> VectorListParam, bool RequiresScan);
	void Load(UFlareQuest* ParentQuest, FName ConditionIdentifier, TArray<FVector> VectorListParam, bool RequiresScan);
	virtual void Restore(const FFlareBundle* Bundle);
	virtual void Save(FFlareBundle* Bundle);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	void Init();

	TArray<FVector> VectorList;

	bool IsInit;
	bool TargetRequiresScan;

	int32 CurrentProgression;
	FTransform InitialTransform;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionFollowRandomWaypoints: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionFollowRandomWaypoints* Create(UFlareQuest* ParentQuest, FName ConditionIdentifier, bool RequiresScan);
	void Load(UFlareQuest* ParentQuest, FName ConditionIdentifier, bool RequiresScan);
	virtual void Restore(const FFlareBundle* Bundle);
	virtual void Save(FFlareBundle* Bundle);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	void Init();

	void GenerateWaypointSegments(TArray<FVector>& WaypointList, float& Distance, float MaxDistance, float StepDistance,
								  FVector& BaseDirection, FVector& BaseLocation, FVector TargetLocation,
								  float TargetMaxTurnDistance);



	bool IsInit;
	bool TargetRequiresScan;

	TArray<FVector> Waypoints;
	int32 CurrentProgression;
};


//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionDockAt: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()

public:

	static UFlareQuestConditionDockAt* Create(UFlareQuest* ParentQuest, UFlareSimulatedSpacecraft* Station);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSpacecraft* Station);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
	virtual FText GetInitialLabel();

	FName TargetShipMatchId;
	FName TargetShipSaveId;

protected:

	UFlareSimulatedSpacecraft* TargetStation;
	bool Completed;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionAtWar: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionAtWar* Create(UFlareQuest* ParentQuest, UFlareCompany* Company1, UFlareCompany* Company2);
	void Load(UFlareQuest* ParentQuest, UFlareCompany* Company1, UFlareCompany* Company2);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	UFlareCompany* TargetCompany1;
	UFlareCompany* TargetCompany2;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionAtPeace: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionAtPeace* Create(UFlareQuest* ParentQuest, UFlareCompany* Company1, UFlareCompany* Company2);
	void Load(UFlareQuest* ParentQuest, UFlareCompany* Company1, UFlareCompany* Company2);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	UFlareCompany* TargetCompany1;
	UFlareCompany* TargetCompany2;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionSpacecraftNoMoreExist: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()

public:
	static UFlareQuestConditionSpacecraftNoMoreExist* Create(UFlareQuest* ParentQuest, UFlareSimulatedSpacecraft* TargetSpacecraftParam, FName TargetSpacecraftIdParam = NAME_None);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSpacecraft* TargetSpacecraftParam, FName TargetSpacecraftIdParam);

	virtual bool IsCompleted();
	virtual FText GetInitialLabel();

	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	UFlareSimulatedSpacecraft* TargetSpacecraft;
	FName TargetSpacecraftId;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionBuyAtStation: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()

public:

	static UFlareQuestConditionBuyAtStation* Create(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, UFlareSimulatedSpacecraft* StationParam, FFlareResourceDescription* ResourceParam, int32 QuantityParam);
	void Load(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, UFlareSimulatedSpacecraft* StationParam, FFlareResourceDescription* ResourceParam, int32 QuantityParam);

	virtual bool IsCompleted();
	virtual void Restore(const FFlareBundle* Bundle);
	virtual void Save(FFlareBundle* Bundle);

	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
	virtual void OnTradeDone(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* TradeResource, int32 TradeQuantity);

protected:

	UFlareSimulatedSpacecraft* TargetStation;
	FFlareResourceDescription* Resource;
	int32 Quantity;
	int32 CurrentProgression;
public:
	int32 GetTargetQuantity()
	{
		return Quantity;
	}

	int32 GetCurrentProgression()
	{
		return CurrentProgression;
	}

	FFlareResourceDescription* GetResource()
	{
		return Resource;
	}

	UFlareSimulatedSpacecraft* GetTargetStation()
	{
		return TargetStation;
	}
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionSellAtStation: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()

public:

	static UFlareQuestConditionSellAtStation* Create(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, UFlareSimulatedSpacecraft* StationParam, FFlareResourceDescription* ResourceParam, int32 QuantityParam);
	void Load(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, UFlareSimulatedSpacecraft* StationParam, FFlareResourceDescription* ResourceParam, int32 QuantityParam);

	virtual bool IsCompleted();
	virtual void Restore(const FFlareBundle* Bundle);
	virtual void Save(FFlareBundle* Bundle);

	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
	virtual void OnTradeDone(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* TradeResource, int32 TradeQuantity);

protected:

	UFlareSimulatedSpacecraft* TargetStation;
	FFlareResourceDescription* Resource;
	int32 Quantity;
	int32 CurrentProgression;

public:
	int32 GetTargetQuantity()
	{
		return Quantity;
	}

	int32 GetCurrentProgression()
	{
		return CurrentProgression;
	}

	FFlareResourceDescription* GetResource()
	{
		return Resource;
	}
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTimeAfterAvailableDate: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionTimeAfterAvailableDate* Create(UFlareQuest* ParentQuest, int64 Duration);
	void Load(UFlareQuest* ParentQuest, int64 Duration);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
	virtual FText GetInitialLabel();

protected:

	int64 DurationLimit;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionAfterDate: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionAfterDate* Create(UFlareQuest* ParentQuest, int64 Date);
	void Load(UFlareQuest* ParentQuest, int64 Date);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
	virtual FText GetInitialLabel();

protected:

	int64 DateLimit;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionPlayerTravelTooLong: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionPlayerTravelTooLong* Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, int64 Date);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, int64 Date);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
	virtual FText GetInitialLabel();

protected:
	UFlareSimulatedSector* TargetSector;
	int64 DateLimit;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionMinArmyCombatPointsInSector: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()

public:
	static UFlareQuestConditionMinArmyCombatPointsInSector* Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam, int32 TargetArmyPointsParam);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam, int32 TargetArmyPointsParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	UFlareSimulatedSector* TargetSector;
	UFlareCompany* TargetCompany;
	int32 TargetArmyPoints;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionMaxArmyCombatPointsInSector: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()

public:
	static UFlareQuestConditionMaxArmyCombatPointsInSector* Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam, int32 TargetArmyPointsParam);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam, int32 TargetArmyPointsParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	UFlareSimulatedSector* TargetSector;
	UFlareCompany* TargetCompany;
	int32 TargetArmyPoints;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionNoBattleInSector: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()

public:
	static UFlareQuestConditionNoBattleInSector* Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	UFlareSimulatedSector* TargetSector;
	UFlareCompany* TargetCompany;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionStationLostInSector: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()

public:
	static UFlareQuestConditionStationLostInSector* Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
	virtual void OnSpacecraftCaptured(UFlareSimulatedSpacecraft* CapturedSpacecraftBefore, UFlareSimulatedSpacecraft* CapturedSpacecraftAfter);

protected:

	UFlareSimulatedSector* TargetSector;
	UFlareCompany* TargetCompany;
	bool Completed;
};


//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionNoCapturingStationInSector: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()

public:
	static UFlareQuestConditionNoCapturingStationInSector* Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam, UFlareCompany* TargetEnemyCompanyParam);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam, UFlareCompany* TargetEnemyCompanyParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	virtual void Restore(const FFlareBundle* Bundle);
	virtual void Save(FFlareBundle* Bundle);

protected:
	int32 GetCapturingStations();

	UFlareSimulatedSector* TargetSector;
	UFlareCompany* TargetCompany;
	UFlareCompany* TargetEnemyCompany;
	int32 InitialCapturingStations;
	bool HasInitialCapturingStations;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionWorkFor: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()

public:
	static UFlareQuestConditionWorkFor* Create(UFlareQuest* ParentQuest, UFlareCompany* TargetCompanyParam);
	void Load(UFlareQuest* ParentQuest, UFlareCompany* TargetCompanyParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);


protected:
	UFlareCompany* TargetCompany;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionRetreatDangerousShip: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()

public:
	static UFlareQuestConditionRetreatDangerousShip* Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* TargetSectorParam, UFlareCompany* TargetCompanyParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
	virtual void OnTravelStarted(UFlareTravel* Travel);

protected:

	UFlareSimulatedSector* TargetSector;
	UFlareCompany* TargetCompany;
	bool Completed;

};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionDestroySpacecraft: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()

public:

	static UFlareQuestConditionDestroySpacecraft* Create(UFlareQuest* ParentQuest,
														 FName ConditionIdentifierParam,
														 UFlareCompany* HostileCompanyParam,
														 int32 SpacecraftCountParam,
														 bool MilitaryParam,
														 EFlarePartSize::Type SizeParam,
														 bool DestroyTargetParam);
	void Load(UFlareQuest* ParentQuest, FName ConditionIdentifierParam,
			  UFlareCompany* HostileCompanyParam,
			  int32 SpacecraftCountParam,
			  bool MilitaryParam,
			  EFlarePartSize::Type SizeParam,
			  bool DestroyTargetParam);

	virtual bool IsCompleted();
	virtual void Restore(const FFlareBundle* Bundle);
	virtual void Save(FFlareBundle* Bundle);

	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
	virtual void OnSpacecraftDestroyed(UFlareSimulatedSpacecraft *Spacecraft, bool Uncontrollable, DamageCause Cause);



protected:

	UFlareCompany* TargetCompany;
	int32 Quantity;
	int32 CurrentProgression;
	EFlarePartSize::Type TargetSize;
	bool DestroyTarget;
	bool MilitaryTarget;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionDestroyCombatValue: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()

public:

	static UFlareQuestConditionDestroyCombatValue* Create(UFlareQuest* ParentQuest,
														 FName ConditionIdentifierParam,
														 UFlareCompany* HostileCompanyParam,
														 int32 CombatValueParam,
														 bool DestroyTargetParam);
	void Load(UFlareQuest* ParentQuest, FName ConditionIdentifierParam,
			  UFlareCompany* HostileCompanyParam,
			  int32 CombatValueParam,
			  bool DestroyTargetParam);

	virtual bool IsCompleted();
	virtual void Restore(const FFlareBundle* Bundle);
	virtual void Save(FFlareBundle* Bundle);

	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
	virtual void OnSpacecraftDestroyed(UFlareSimulatedSpacecraft *Spacecraft, bool Uncontrollable, DamageCause Cause);



protected:

	UFlareCompany* TargetCompany;
	int32 Quantity;
	int32 CurrentProgression;
	bool DestroyTarget;
};
