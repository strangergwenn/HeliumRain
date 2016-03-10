#pragma once

#include "../Spacecrafts/FlareSpacecraftTypes.h"
#include "FlareAsteroid.generated.h"


class AFlareGame;
class UFlareSector;


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class AFlareAsteroid : public AStaticMeshActor
{
public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;


	/** Properties setup */
	virtual void Load(const FFlareAsteroidSave& Data);

	/** Save the asteroid to a save file */
	virtual FFlareAsteroidSave* Save();

	/** Set as paused */
	virtual void SetPause(bool Paused);

	/** Setup an asteroid mesh */
	static void SetupAsteroidMesh(AFlareGame* Game, UStaticMeshComponent* Component, const FFlareAsteroidSave& Data, bool IsIcy);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	FVector                                 SpawnLocation;
	FFlareAsteroidSave                      AsteroidData;
	bool                                    Paused;

};
