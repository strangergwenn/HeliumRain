
#include "../Flare.h"
#include "FlareWeapon.h"
#include "FlareShip.h"
#include "FlareProjectile.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareWeapon::UFlareWeapon(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, FiringEffect(NULL)
	, FiringRate(0)
	, MaxAmmo(0)
	, Firing(false)
{
	// Firing sound
	static ConstructorHelpers::FObjectFinder<USoundCue> FiringSoundObject(TEXT("/Game/Master/Sound/A_Shot"));
	FiringSound = FiringSoundObject.Object;

	// Firing effects
	static ConstructorHelpers::FObjectFinder<UParticleSystem> FiringEffectObject(TEXT("/Game/Master/Particles/PS_FiringEffect"));
	FiringEffectTemplate = FiringEffectObject.Object;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareWeapon::Initialize(const FFlareShipModuleSave* Data, UFlareCompany* Company, AFlareShipBase* OwnerShip, bool IsInMenu)
{
	Super::Initialize(Data, Company, OwnerShip, IsInMenu);

	// Setup properties
	if (ModuleDescription)
	{
		ShellMesh = ModuleDescription->EffectMesh;

		for (int32 i = 0; i < ModuleDescription->Characteristics.Num(); i++)
		{
			const FFlarePartCharacteristic& Characteristic = ModuleDescription->Characteristics[i];
			switch (Characteristic.CharacteristicType)
			{
				case EFlarePartCharacteristicType::AmmoRange:
					// TODO
					break;

				case EFlarePartCharacteristicType::AmmoRate:
					FiringRate = Characteristic.CharacteristicValue;
					break;

				case EFlarePartCharacteristicType::AmmoPower:
					// TODO
					break;

				case EFlarePartCharacteristicType::AmmoCapacity:
					MaxAmmo = Characteristic.CharacteristicValue;
					break;
			}
		}
	}

	// Spawn properties
	ProjectileSpawnParams.Instigator = Ship;
	ProjectileSpawnParams.bNoFail = true;
	ProjectileSpawnParams.bNoCollisionFail = true;

	// Additional properties
	CurrentAmmo = MaxAmmo;
	FiringPeriod = 1 / (FiringRate / 60);
}

void UFlareWeapon::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceLastShell += DeltaTime;

	if (Firing && CurrentAmmo > 0 && TimeSinceLastShell > FiringPeriod)
	{
		// Get firing data
		FVector FiringLocation = GetSocketLocation(FName("Muzzle"));
		FVector FiringDirection = GetComponentRotation().RotateVector(FVector(1, 0, 0));
		FVector FiringVelocity = GetPhysicsLinearVelocity();

		// Create a shell
		AFlareProjectile* Shell = GetWorld()->SpawnActor<AFlareProjectile>(
			AFlareProjectile::StaticClass(),
			FiringLocation,
			FRotator::ZeroRotator, 
			ProjectileSpawnParams);

		// Fire it
		Shell->Initialize(this, ModuleDescription, FiringDirection, FiringVelocity);
		FiringEffect->ActivateSystem();

		// Play sound
		if (Ship && Ship->IsLocallyControlled())
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), FiringSound, GetComponentLocation(), 1, 1);
		}

		// Update data
		TimeSinceLastShell = 0;
		CurrentAmmo--;
	}
}

void UFlareWeapon::SetupEffectMesh()
{
	if (FiringEffect == NULL)
	{
		FiringEffect = UGameplayStatics::SpawnEmitterAttached(
			FiringEffectTemplate,
			this,
			NAME_None,
			GetSocketLocation(FName("Muzzle")),
			GetComponentRotation(),
			EAttachLocation::KeepWorldPosition,
			false);
		FiringEffect->DeactivateSystem();
		FiringEffect->SetTickGroup(ETickingGroup::TG_PostPhysics);
	}
}

void UFlareWeapon::StartFire()
{
	Firing = true;
}

void UFlareWeapon::StopFire()
{
	Firing = false;
}
