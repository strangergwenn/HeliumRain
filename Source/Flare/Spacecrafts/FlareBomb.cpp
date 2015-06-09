
#include "../Flare.h"
#include "FlareBombComponent.h"
#include "FlareBomb.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareBomb::AFlareBomb(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	FLOG("AFlareBomb");
	// Mesh data
	BombComp = PCIP.CreateDefaultSubobject<UFlareBombComponent>(this, TEXT("Root"));
	BombComp->bTraceComplexOnMove = true;
	//BombComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	/*BombComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	BombComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
	BombComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
	BombComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);*/
	BombComp->LDMaxDrawDistance = 100000; // 1km*/
	BombComp->SetSimulatePhysics(true);
	RootComponent = BombComp;

	SetActorEnableCollision(false);
	// Settings
	PrimaryActorTick.bCanEverTick = true;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlareBomb::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AFlareBomb::Initialize(UFlareWeapon* Weapon, const FFlareSpacecraftComponentDescription* Description)
{
	ParentWeapon = Weapon;
	WeaponDescription = Description;

	//BombComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Get the power from description
	if (Description)
	{
		FFlareSpacecraftComponentSave ComponentData;
		ComponentData.ComponentIdentifier = Description->Identifier;
		BombComp->Initialize(&ComponentData, Weapon->GetSpacecraft()->GetCompany(), Weapon->GetSpacecraft(), false);
	}
}

void AFlareBomb::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	FLOG("AFlareBomb Hit");
	//Destroy();
}

void AFlareBomb::Drop()
{
	FLOG("AFlareBomb Drop");
	DetachRootComponentFromParent(true);

	FVector FrontVector = BombComp->ComponentToWorld.TransformVector(FVector(120,0,0));

	// Spin to stabilize
	BombComp->SetPhysicsAngularVelocity(FrontVector);
	BombComp->SetPhysicsLinearVelocity(ParentWeapon->GetSpacecraft()->Airframe->GetPhysicsLinearVelocity());
	//BombComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FLOGV("AFlareBomb FrontVector=%s", *FrontVector.ToString());

	/*BombComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BombComp->SetCollisionResponseToAllChannels(ECR_Block);
	BombComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	BombComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	BombComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);*/
	//BombComp->SetSimulatePhysics(true);
}


void AFlareBomb::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	//FLOG("AFlareBomb Tick");
}
