#pragma once

#include "../FlareQuest.h"
#include "FlareTutorialQuest.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareQuestTutorialFlying: public UFlareQuest
{
	GENERATED_UCLASS_BODY()

public:

	/** Load the quest from description file */
	virtual void Load(UFlareQuestManager* Parent);
	static UFlareQuest* Create(UFlareQuestManager* Parent);
};


UCLASS()
class HELIUMRAIN_API UFlareQuestTutorialNavigation: public UFlareQuest
{
	GENERATED_UCLASS_BODY()

public:

	/** Load the quest from description file */
	virtual void Load(UFlareQuestManager* Parent);
	static UFlareQuest* Create(UFlareQuestManager* Parent);
};

UCLASS()
class HELIUMRAIN_API UFlareQuestTutorialContracts: public UFlareQuest
{
	GENERATED_UCLASS_BODY()

public:

	/** Load the quest from description file */
	virtual void Load(UFlareQuestManager* Parent);
	static UFlareQuest* Create(UFlareQuestManager* Parent);
};

UCLASS()
class HELIUMRAIN_API UFlareQuestTutorialTechnology: public UFlareQuest
{
	GENERATED_UCLASS_BODY()

public:

	/** Load the quest from description file */
	virtual void Load(UFlareQuestManager* Parent);
	static UFlareQuest* Create(UFlareQuestManager* Parent);
};

UCLASS()
class HELIUMRAIN_API UFlareQuestTutorialBuildShip: public UFlareQuest
{
	GENERATED_UCLASS_BODY()

public:

	/** Load the quest from description file */
	virtual void Load(UFlareQuestManager* Parent);
	static UFlareQuest* Create(UFlareQuestManager* Parent);
};

UCLASS()
class HELIUMRAIN_API UFlareQuestTutorialBuildStation: public UFlareQuest
{
	GENERATED_UCLASS_BODY()

public:

	/** Load the quest from description file */
	virtual void Load(UFlareQuestManager* Parent);
	static UFlareQuest* Create(UFlareQuestManager* Parent);
};

UCLASS()
class HELIUMRAIN_API UFlareQuestTutorialResearchStation: public UFlareQuest
{
	GENERATED_UCLASS_BODY()

public:

	/** Load the quest from description file */
	virtual void Load(UFlareQuestManager* Parent);
	static UFlareQuest* Create(UFlareQuestManager* Parent);
};

// Conditions


//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialGetContrat: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionTutorialGetContrat* Create(UFlareQuest* ParentQuest);
	void Load(UFlareQuest* ParentQuest);

	virtual bool IsCompleted();
	virtual void OnEvent(FFlareBundle& Bundle);
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:
	bool Completed;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialOpenMenu: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionTutorialOpenMenu* Create(UFlareQuest* ParentQuest, EFlareMenu::Type Menu);
	void Load(UFlareQuest* ParentQuest, EFlareMenu::Type Menu);

	virtual bool IsCompleted();
	virtual void OnEvent(FFlareBundle& Bundle);
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:
	EFlareMenu::Type MenuType;
	bool Completed;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialAcceptQuest: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionTutorialAcceptQuest* Create(UFlareQuest* ParentQuest);
	void Load(UFlareQuest* ParentQuest);

	virtual bool IsCompleted();
	virtual void OnEvent(FFlareBundle& Bundle);
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:
	bool Completed;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialTrackQuest: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionTutorialTrackQuest* Create(UFlareQuest* ParentQuest);
	void Load(UFlareQuest* ParentQuest);

	virtual bool IsCompleted();
	virtual void OnEvent(FFlareBundle& Bundle);
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:
	bool Completed;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialFinishQuest: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionTutorialFinishQuest* Create(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, int32 Count);
	void Load(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, int32 Count);

	virtual void Restore(const FFlareBundle* Bundle);
	virtual void Save(FFlareBundle* Bundle);

	virtual bool IsCompleted();
	virtual void OnEvent(FFlareBundle& Bundle);
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:
	int32 QuestCount;
	int32 CurrentProgression;
};


//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialCommandDock : public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionTutorialCommandDock* Create(UFlareQuest* ParentQuest, UFlareSimulatedSpacecraft* TargetParam);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSpacecraft* TargetParam);

	virtual bool IsCompleted();
	virtual void OnEvent(FFlareBundle& Bundle);
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:
	UFlareSimulatedSpacecraft* Target;
	bool Completed;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialTargetSpacecraft : public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionTutorialTargetSpacecraft* Create(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, UFlareSimulatedSpacecraft* Spacecraft,  float Duration);
	void Load(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, UFlareSimulatedSpacecraft* Spacecraft, float Duration);

	virtual void Restore(const FFlareBundle* Bundle);
	virtual void Save(FFlareBundle* Bundle);

	virtual bool IsCompleted();
	virtual void OnEvent(FFlareBundle& Bundle);
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:
	float TargetDuration;
	UFlareSimulatedSpacecraft* TargetSpacecraft;
	float LastTargetChangeTimestamp;
	int32 CurrentProgression;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialResearchValue: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()

public:
	static UFlareQuestConditionTutorialResearchValue* Create(UFlareQuest* ParentQuest, int32 Count);
	void Load(UFlareQuest* ParentQuest,  int32 Count);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	int32 TargetResearchPoints;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialTechnologyLevel: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()

public:
	static UFlareQuestConditionTutorialTechnologyLevel* Create(UFlareQuest* ParentQuest, int32 Level);
	void Load(UFlareQuest* ParentQuest,  int32 Level);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	int32 TargetLevel;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialResearchTechnology : public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionTutorialResearchTechnology* Create(UFlareQuest* ParentQuest, int32 MinLevel);
	void Load(UFlareQuest* ParentQuest, int32 MinLevel);

	virtual bool IsCompleted();
	virtual void OnEvent(FFlareBundle& Bundle);
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:
	int32 TargetMinLevel;
	bool Completed;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialMoney: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()

public:
	static UFlareQuestConditionTutorialMoney* Create(UFlareQuest* ParentQuest, int32 Count);
	void Load(UFlareQuest* ParentQuest,  int32 Count);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	int32 TargetMoney;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialOrderShip : public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionTutorialOrderShip* Create(UFlareQuest* ParentQuest, EFlarePartSize::Type Size);
	void Load(UFlareQuest* ParentQuest, EFlarePartSize::Type Size);

	virtual bool IsCompleted();
	virtual void OnEvent(FFlareBundle& Bundle);
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:
	int32 TargetSize;
	bool Completed;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialBuildShip : public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionTutorialBuildShip* Create(UFlareQuest* ParentQuest, EFlarePartSize::Type Size);
	void Load(UFlareQuest* ParentQuest, EFlarePartSize::Type Size);

	virtual bool IsCompleted();
	virtual void OnEvent(FFlareBundle& Bundle);
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:
	int32 TargetSize;
	bool Completed;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialUnlockStation: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()

public:
	static UFlareQuestConditionTutorialUnlockStation* Create(UFlareQuest* ParentQuest);
	void Load(UFlareQuest* ParentQuest);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialStartStationConstruction : public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionTutorialStartStationConstruction* Create(UFlareQuest* ParentQuest, bool Upgrade);
	void Load(UFlareQuest* ParentQuest, bool Upgrade);

	virtual bool IsCompleted();
	virtual void OnEvent(FFlareBundle& Bundle);
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:
	bool TargetUpgrade;
	bool Completed;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialBuildStation : public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionTutorialBuildStation* Create(UFlareQuest* ParentQuest, bool Upgrade, FName StationIdentifier = NAME_None, UFlareSimulatedSector* Sector = NULL);
	void Load(UFlareQuest* ParentQuest, bool Upgrade, FName StationIdentifier, UFlareSimulatedSector* Sector);

	virtual bool IsCompleted();
	virtual void OnEvent(FFlareBundle& Bundle);
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:
	FName TargetStationIdentifier;
	bool TargetUpgrade;
	UFlareSimulatedSector* TargetSector;
	bool Completed;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialUnlockTechnology: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()

public:
	static UFlareQuestConditionTutorialUnlockTechnology* Create(UFlareQuest* ParentQuest, FName Identifier);
	void Load(UFlareQuest* ParentQuest, FName Identifier);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:
	FName TargetIdentifier;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialProduceResearch: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()

public:

	static UFlareQuestConditionTutorialProduceResearch* Create(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, int32 QuantityParam);
	void Load(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, int32 QuantityParam);

	virtual bool IsCompleted();
	virtual void Restore(const FFlareBundle* Bundle);
	virtual void Save(FFlareBundle* Bundle);

	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
	virtual void OnEvent(FFlareBundle& Bundle);

protected:

	int32 Quantity;
	int32 CurrentProgression;
};

