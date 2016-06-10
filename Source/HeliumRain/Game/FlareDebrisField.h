#pragma once

#include "Object.h"
#include "FlareDebrisField.generated.h"


class AFlareGame;
class UFlareSimulatedSector;


UCLASS()
class HELIUMRAIN_API UFlareDebrisField : public UObject
{
    GENERATED_UCLASS_BODY()

public:

    /*----------------------------------------------------
        Public interface
    ----------------------------------------------------*/

	/** Load the debris field from a sector info */
	void Load(AFlareGame* GameMode, UFlareSimulatedSector* Sector);

	/** Cleanup the debris field */
	void Reset();

	/** Toggle the game pause */
	void SetWorldPause(bool Pause);


private:

	/*----------------------------------------------------
		Internals
	----------------------------------------------------*/

	/** Add debris */
	AStaticMeshActor* AddDebris(UFlareSimulatedSector* Sector, UStaticMesh* Mesh, float Debris, float SectorScale, FName Name);
	

protected:

    /*----------------------------------------------------
        Protected data
    ----------------------------------------------------*/

	/** Debris field */
	UPROPERTY()
	TArray<AStaticMeshActor*>                  DebrisField;
	
	/** Game reference */
	UPROPERTY()
	AFlareGame*                                Game;

};
