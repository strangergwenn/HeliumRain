#pragma once
#include "Game/FlareGameTypes.h"
#include "FlareQuestManager.h"
#include "FlareQuestGenerator.generated.h"

class AFlareGame;
class UFlareQuestManager;
class UFlareQuest;
class UFlareSimulatedSector;
class UFlareQuestGenerated;

/** Quest generator */
UCLASS()
class HELIUMRAIN_API UFlareQuestGenerator: public UObject
{
	GENERATED_UCLASS_BODY()

public:

	virtual void Load(UFlareQuestManager* Parent, const FFlareQuestSave& Data);

	void LoadQuests(const FFlareQuestSave& Data);

	virtual void Save(FFlareQuestSave& Data);

	/*----------------------------------------------------
		Quest generation
	----------------------------------------------------*/

	static FText GeneratePersonName(TArray<FString> UsedNames);

	void GenerateIdentifer(FName QuestClass, FFlareBundle& Data);

	void GenerateSectorQuest(UFlareSimulatedSector* Sector);

	void GenerateMilitaryQuests();

	void GenerateAttackQuests(UFlareCompany* AttackCompany, int32 AttackCombatPoints, WarTarget& Target, int64 TravelDuration);

	void RegisterQuest(UFlareQuestGenerated* Quest);

	bool FindUniqueTag(FName Tag);

	float ComputeQuestProbability(UFlareCompany* Company);

	FName GenerateVipTag(UFlareSimulatedSpacecraft* SourceSpacecraft);

	FName GenerateTradeTag(UFlareSimulatedSpacecraft* SourceSpacecraft, FFlareResourceDescription* Resource);


	FName GenerateDefenseTag(UFlareSimulatedSector* Sector, UFlareCompany* HostileCompany);

	FName GenerateAttackTag(UFlareSimulatedSector* Sector, UFlareCompany* OwnerCompany);

	FName GenerateHarassTag(UFlareCompany* OwnerCompany, UFlareCompany* HostileCompany);

	bool IsGenerationEnabled();


protected:

   /*----------------------------------------------------
	   Protected data
   ----------------------------------------------------*/

	UFlareQuestManager*						QuestManager;

	AFlareGame*                             Game;

	int64                                   NextQuestIndex;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline AFlareGame* GetGame() const
	{
		return Game;
	}

	inline UFlareQuestManager* GetQuestManager() const
	{
		return QuestManager;
	}
protected:
	UPROPERTY()
	TArray<UFlareQuestGenerated*>	                 GeneratedQuests;
};


UCLASS()
class HELIUMRAIN_API UFlareQuestGenerated: public UFlareQuest
{
	GENERATED_UCLASS_BODY()

public:
	virtual void Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);

	void AddGlobalFailCondition(UFlareQuestCondition* Condition);
	void SetupQuestGiver(UFlareCompany* Company, bool AddWarCondition);
	void SetupGenericReward(const FFlareBundle& Data);

	static void CreateGenericReward(FFlareBundle& Data, int64 QuestValue, UFlareCompany* Client);

	FFlareBundle* GetInitData() {
		return &InitData;
	}

	FName GetQuestClass() {
		return QuestClass;
	}

protected:
	FName QuestClass;
	FFlareBundle InitData;
	UFlareQuestGenerator* QuestGenerator;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedVipTransport: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "vip-transport"; }

	/** Load the quest from description file */
	virtual void Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedResourceSale: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "resource-sale"; }

	/** Load the quest from description file */
	virtual void Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedResourcePurchase: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "resource-purchase"; }

	/** Load the quest from description file */
	virtual void Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedResourceTrade: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "resource-trade"; }

	/** Load the quest from description file */
	virtual void Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedStationDefense: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "station-defense"; }

	/** Load the quest from description file */
	virtual void Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company, UFlareCompany* HostileCompany);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedJoinAttack: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "join-attack"; }

	/** Load the quest from description file */
	virtual void Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareCompany* Company, int32 AttackCombatPoints, WarTarget& Target, int64 TravelDuration);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedSectorDefense: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "sector-defense"; }

	/** Load the quest from description file */
	virtual void Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareCompany* Company, UFlareCompany* HostileCompany, int32 AttackCombatPoints, WarTarget& Target, int64 TravelDuration);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedCargoHunt: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "cargo-hunt"; }

	/** Load the quest from description file */
	virtual void Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareCompany* Company, UFlareCompany* HostileCompany);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedMilitaryHunt: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "military-hunt"; }

	/** Load the quest from description file */
	virtual void Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareCompany* Company, UFlareCompany* HostileCompany);
};
