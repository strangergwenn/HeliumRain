
#include "FlareCollider.h"
#include "../Flare.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareCollider::AFlareCollider(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ColliderTemplateObj(TEXT("/Game/Master/Default/SM_Collider.SM_Collider"));

	CollisionComponent = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("CollisionComponent"));
	CollisionComponent->SetCollisionProfileName("NoCollision");
	CollisionComponent->SetHiddenInGame(true);
	CollisionComponent->SetStaticMesh(ColliderTemplateObj.Object);
	RootComponent = CollisionComponent;
}

