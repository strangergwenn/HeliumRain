#pragma once

#include "../Spacecrafts/FlareSpacecraftTypes.h"
#include "FlareAsteroidComponent.generated.h"


class AFlareGame;
class UFlareSector;


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class UFlareAsteroidComponent : public UStaticMeshComponent
{
public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void SetIcy(bool Icy);

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	bool                                    IsIcyAsteroid;

	UPROPERTY()
	UParticleSystem*                        IceEffectTemplate;

	UPROPERTY()
	UParticleSystem*                        DustEffectTemplate;

	int32                                   EffectsCount;
	float                                   EffectsScale;
	float                                   EffectsUpdatePeriod;
	float                                   EffectsUpdateTimer;
	TArray<FVector>                         EffectsKernels;
	TArray<UParticleSystemComponent*>       Effects;

};
