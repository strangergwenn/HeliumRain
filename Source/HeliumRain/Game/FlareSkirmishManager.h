#pragma once

#include "Object.h"
#include "FlareSkirmishManager.generated.h"


class AFlareGame;


/** Skirmish phase state */
UENUM()
namespace EFlareSkirmishPhase
{
	enum Type
	{
		Idle,
		Setup,
		Play,
		End
	};
}


/** Skirmish managing class */
UCLASS()
class HELIUMRAIN_API UFlareSkirmishManager : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	
	/** Setup the setup phase */
	void StartSetup();

	/** Setup the playing phase */
	void StartPlay();

	/** Start the endgame phase */
	void EndPlay();

	/** End the game */
	void EndSkirmish();
	
	/** Add a ship */
	void AddShip(FFlareSpacecraftDescription*, bool ForPlayer);

	/** Are we playing skirmish */
	bool IsPlaying() const;


protected:

	UPROPERTY()
	TEnumAsByte<EFlareSkirmishPhase::Type>           CurrentPhase;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const;


};
