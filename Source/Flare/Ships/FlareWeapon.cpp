
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

void UFlareWeapon::Initialize(const FFlareShipComponentSave* Data, UFlareCompany* Company, AFlareShipBase* OwnerShip, bool IsInMenu)
{
	Super::Initialize(Data, Company, OwnerShip, IsInMenu);

	// Setup properties
	if (ComponentDescription)
	{
		ShellMesh = ComponentDescription->EffectMesh;

		for (int32 i = 0; i < ComponentDescription->Characteristics.Num(); i++)
		{
			const FFlareShipComponentCharacteristic& Characteristic = ComponentDescription->Characteristics[i];
			switch (Characteristic.CharacteristicType)
			{
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

	if (Firing && CurrentAmmo > 0 && TimeSinceLastShell > FiringPeriod && GetDamageRatio() > 0.f)
	{
		// Get firing data
		FVector FiringLocation = GetSocketLocation(FName("Muzzle"));

		float Vibration = (1.f- GetDamageRatio()) * 0.05;
		FVector Imprecision = FVector(0, FMath::FRandRange(0.f, Vibration), 0).RotateAngleAxis(FMath::FRandRange(0.f, 360), FVector(1, 0, 0));

		FVector FiringDirection = GetComponentRotation().RotateVector(FVector(1, 0, 0) + Imprecision);
		FVector FiringVelocity = GetPhysicsLinearVelocity();

		// Create a shell
		AFlareProjectile* Shell = GetWorld()->SpawnActor<AFlareProjectile>(
			AFlareProjectile::StaticClass(),
			FiringLocation,
			FRotator::ZeroRotator, 
			ProjectileSpawnParams);

		// Fire it
		Shell->Initialize(this, ComponentDescription, FiringDirection, FiringVelocity);
		FiringEffect->ActivateSystem();

		// Play sound
		if (Ship && Ship->IsLocallyControlled())
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), FiringSound, GetComponentLocation(), 1, 1);
		}

		// Update data
		// If damage the firerate is randomly reduced to a min of 10 times normal value
		float DamageDelay = FMath::Square(1.f- GetDamageRatio()) * 10 * FiringPeriod * FMath::FRandRange(0.f, 1.f);
		TimeSinceLastShell = -DamageDelay;
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
