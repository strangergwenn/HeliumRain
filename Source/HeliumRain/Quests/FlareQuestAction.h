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


UCLASS()
class HELIUMRAIN_API UFlareQuestActionDiscoverSector: public UFlareQuestAction
{
	GENERATED_UCLASS_BODY()


public:
	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam);

	virtual void Perform();

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UFlareSimulatedSector* Sector;
};
