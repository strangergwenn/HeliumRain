#pragma once

#include "../Spacecrafts/FlareSpacecraftTypes.h"
#include "FlareAsteroid.generated.h"


class AFlareGame;
class UFlareSimulatedSector;


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class AFlareAsteroid : public AStaticMeshActor
{
public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/
	
	/** Properties setup */
	virtual void Load(const FFlareAsteroidSave& Data);

	/** Save the asteroid to a save file */
	virtual FFlareAsteroidSave* Save();

	virtual void SetPause(bool Paused);

	/** Setup an asteroid mesh */
	static void SetupAsteroidMesh(AFlareGame* Game, UFlareSimulatedSector* Sector, UStaticMeshComponent* Component, const FFlareAsteroidSave& Data);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	FFlareAsteroidSave                      AsteroidData;
	bool                                    Paused;

};
