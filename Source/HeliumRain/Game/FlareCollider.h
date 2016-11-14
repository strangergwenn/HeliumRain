#pragma once

#include "FlareCollider.generated.h"


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class AFlareCollider : public AActor
{
public:

	GENERATED_UCLASS_BODY()


protected:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	// Collider
	UPROPERTY(Category = Collider, EditAnywhere)
	UStaticMeshComponent*                           CollisionComponent;


};
