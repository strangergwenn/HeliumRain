#pragma once

#include "FlareCollider.generated.h"


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class AFlareCollider : public AActor
{
public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;


protected:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	// Collider
	UPROPERTY(Category = Collider, EditAnywhere)
	UStaticMeshComponent*                           CollisionComponent;
	

};
