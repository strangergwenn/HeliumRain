#pragma once

#include "FlareScenarioTools.generated.h"

class UFlareCompany;
struct FFlarePlayerSave;

UCLASS()
class FLARE_API UFlareScenarioTools : public UObject
{
public:

	GENERATED_UCLASS_BODY()


	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	void Init(UFlareCompany* Company, FFlarePlayerSave* Player);

	void GenerateEmptyScenario();

	void GenerateFighterScenario();

	void GenerateDebugScenario();

protected:

	/*----------------------------------------------------
		Helpers
	----------------------------------------------------*/

	/** Create asteroid, artefact and common things */
	void SetupWorld();

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UFlareCompany*             PlayerCompany;

	FFlarePlayerSave*          PlayerData;

	AFlareGame*                                Game;
	UFlareWorld*                               World;
public:
	/*----------------------------------------------------
		Getter
	----------------------------------------------------*/

	/*AFlarePlayerController* GetPC() const;

	AFlareGame* GetGame() const;

	UFlareWorld* GetGameWorld() const;*/
};
