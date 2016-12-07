#pragma once

#include "FlareQuestAction.generated.h"

class UFlareSimulatedSector;
class UFlareQuest;

/** A quest Step action */
UCLASS(abstract)
class HELIUMRAIN_API UFlareQuestAction: public UObject
{
	GENERATED_UCLASS_BODY()


public:
	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	virtual void Perform();

	static void PerformActions(const TArray<UFlareQuestAction*>& Actions);
protected:

	void LoadInternal(UFlareQuest* ParentQuest)
	{
		Quest = ParentQuest;
	}

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UFlareQuest* Quest;

public:
	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/




};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestActionDiscoverSector: public UFlareQuestAction
{
	GENERATED_UCLASS_BODY()


public:
	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/
	static UFlareQuestActionDiscoverSector* Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam);

	virtual void Perform();

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UFlareSimulatedSector* Sector;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestActionVisitSector: public UFlareQuestAction
{
	GENERATED_UCLASS_BODY()


public:
	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/
	static UFlareQuestActionVisitSector* Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam);

	virtual void Perform();

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UFlareSimulatedSector* Sector;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestActionPrintMessage: public UFlareQuestAction
{
	GENERATED_UCLASS_BODY()


public:
	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/
	static UFlareQuestActionPrintMessage* Create(UFlareQuest* ParentQuest, FText MessageParam);
	void Load(UFlareQuest* ParentQuest, FText MessageParam);

	virtual void Perform();

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	FText Message;
};
