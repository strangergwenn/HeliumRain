#pragma once

#include "../Spacecrafts/FlareSpacecraftTypes.h"
#include "FlareAsteroidComponent.h"
#include "FlareAsteroid.generated.h"


class AFlareGame;
class UFlareSector;
class UFlareAsteroidComponent;

UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class AFlareAsteroid : public AActor
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


public:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Mesh
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly)
	UFlareAsteroidComponent*                Asteroid;

	// Data
	FVector                                 SpawnLocation;
	FFlareAsteroidSave                      AsteroidData;
	bool                                    Paused;


public:

	/*----------------------------------------------------
		Getter
	----------------------------------------------------*/

	UFlareAsteroidComponent *GetAsteroidComponent()
	{
		return Asteroid;
	}

};
