#pragma once

#include "Object.h"
#include "FlareSkirmishManager.generated.h"


class AFlareGame;
struct FFlareSpacecraftDescription;


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

/** Skirmish belligerent */
USTRUCT()
struct FFlareSkirmishPlayer
{
	GENERATED_USTRUCT_BODY()

	// Fleet building
	uint32                                           AllowedValue;
	TArray<FFlareSpacecraftDescription*>             OrderedSpacecrafts;

	// Defaults
	FFlareSkirmishPlayer()
		: AllowedValue(0)
	{
		OrderedSpacecrafts.Empty();
	}
};


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
	
	/** Set the combat value allowed */
	void SetAllowedValue(bool ForPlayer, uint32 Budget);

	/** Get the current combat value */
	inline uint32 GetCurrentCombatValue(bool ForPlayer) const;

	/** Add a ship */
	void AddShip(bool ForPlayer, FFlareSpacecraftDescription* Desc);

	/** Are we playing skirmish */
	bool IsPlaying() const;


protected:

	UPROPERTY()
	TEnumAsByte<EFlareSkirmishPhase::Type>           CurrentPhase;

	FFlareSkirmishPlayer                             Player;
	FFlareSkirmishPlayer                             Enemy;



public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const;

	inline uint32 GetAllowedCombatValue(bool ForPlayer) const
	{
		return ForPlayer ? Player.AllowedValue : Enemy.AllowedValue;
	}

};
