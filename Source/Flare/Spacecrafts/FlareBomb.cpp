
#include "../Flare.h"
#include "FlareBomb.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareBomb::AFlareBomb(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	// Mesh data
	BombComp = PCIP.CreateDefaultSubobject<UFlareBombComponent>(this);
	BombComp->bTraceComplexOnMove = true;
	BombComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BombComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	BombComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	BombComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	BombComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	BombComp->LDMaxDrawDistance = 100000; // 1km
	RootComponent = BombComp;

	// Settings
	FlightEffects = NULL;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlareBomb::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	BombComp->OnProjectileBounce.AddDynamic(this, &AFlareProjectile::OnImpact);
}

void AFlareBomb::Initialize(UFlareWeapon* Weapon, const FFlareShipComponentDescription* Description, FVector ShootDirection, FVector ParentVelocity)
{
	ParentWeapon = Weapon;
	WeaponDescription = Description;

	// Get the power from description
	if (Description)
	{

		//void UFlareSpacecraftComponent::Initialize(const FFlareSpacecraftComponentSave* Data, UFlareCompany* Company, AFlareSpacecraftPawn* OwnerSpacecraftPawn, bool IsInMenu)
		{*/

		ShellComp->SetStaticMesh(Description->);
		
		for (int32 i = 0; i < ShellDescription->Characteristics.Num(); i++)
		{
			const FFlareShipComponentCharacteristic& Characteristic = ShellDescription->Characteristics[i];
			switch (Characteristic.CharacteristicType)
			{
				case EFlarePartCharacteristicType::AmmoPower:
					ShellPower = Characteristic.CharacteristicValue;
					break;
			}
		}
	}
}

void AFlareBomb::OnImpact(const FHitResult& HitResult, const FVector& HitVelocity)
{
	Destroy();
}
