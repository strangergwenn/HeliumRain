#pragma once

#include "FlareAsteroid.generated.h"


/** Asteroid save data */
USTRUCT()
struct FFlareAsteroidSave
{
	GENERATED_USTRUCT_BODY()

	/** Asteroid location */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector Location;

	/** Asteroid rotation */
	UPROPERTY(EditAnywhere, Category = Save)
	FRotator Rotation;

	/** Asteroid linear velocity */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector LinearVelocity;

	/** Asteroid angular velocity */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector AngularVelocity;

	/** Content identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 AsteroidMeshID;
};


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

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	FFlareAsteroidSave                          AsteroidData;
	bool                                    Paused;
};
