
#include "../Flare.h"
#include "FlareWeapon.h"
#include "FlareShip.h"
#include "FlareShell.h"


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
	HasLocalHeatEffect = true;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareWeapon::Initialize(const FFlareShipComponentSave* Data, UFlareCompany* Company, AFlareShipPawn* OwnerShip, bool IsInMenu)
{
	Super::Initialize(Data, Company, OwnerShip, IsInMenu);

	float FiredAmmo = 0;

	// Setup properties
	if (ComponentDescription)
	{
		FiringRate = ComponentDescription->GunCharacteristics.AmmoRate;
		MaxAmmo = ComponentDescription->GunCharacteristics.AmmoCapacity;
		AmmoVelocity = ComponentDescription->GunCharacteristics.AmmoVelocity;

		FiringSound = ComponentDescription->GunCharacteristics.FiringSound;
		FiringEffectTemplate = ComponentDescription->GunCharacteristics.FiringEffect;

		for (int32 i = 0; i < ShipComponentData.Attributes.Num(); i++)
		{
			FFlareShipComponentAttributeSave FiredAmmoAttribute;
			if(ShipComponentData.Attributes[i].AttributeIdentifier == FName("weapon-fired-ammo"))
			{
				FiredAmmo = ShipComponentData.Attributes[i].AttributeValue;
			}
		}
	}

	// Spawn properties
	ProjectileSpawnParams.Instigator = Ship;
	ProjectileSpawnParams.bNoFail = true;
	ProjectileSpawnParams.bNoCollisionFail = true;

	// Additional properties
	CurrentAmmo = MaxAmmo - FiredAmmo;
	FiringPeriod = 1 / (FiringRate / 60);

	if (FiringEffect == NULL && FiringEffectTemplate)
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

FFlareShipComponentSave* UFlareWeapon::Save()
{
	float FiredAmmo = MaxAmmo - CurrentAmmo;
	FFlareShipComponentAttributeSave FiredAmmoAttribute;
	FiredAmmoAttribute.AttributeIdentifier = FName("weapon-fired-ammo");
	FiredAmmoAttribute.AttributeValue = FiredAmmo;
	// TODO implement correct attibute insert method
	ShipComponentData.Attributes.Empty();
	ShipComponentData.Attributes.Add(FiredAmmoAttribute);

	return Super::Save();
}

void UFlareWeapon::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceLastShell += DeltaTime;

	if (Firing && CurrentAmmo > 0 && TimeSinceLastShell > FiringPeriod && GetDamageRatio() > 0.f && IsPowered() && !Ship->HasPowerOutage())
	{
		// Get firing data
		FVector FiringLocation = GetSocketLocation(FName("Muzzle"));

		float Vibration = (1.f- GetDamageRatio()) * 0.05;
		FVector Imprecision = FVector(0, FMath::FRandRange(0.f, Vibration), 0).RotateAngleAxis(FMath::FRandRange(0.f, 360), FVector(1, 0, 0));

		FVector FiringDirection = GetComponentRotation().RotateVector(FVector(1, 0, 0) + Imprecision);
		FVector FiringVelocity = GetPhysicsLinearVelocity();

		// Create a shell
		AFlareShell* Shell = GetWorld()->SpawnActor<AFlareShell>(
			AFlareShell::StaticClass(),
			FiringLocation,
			FRotator::ZeroRotator,
			ProjectileSpawnParams);

		// Fire it. Tracer ammo every 3 bullets
		Shell->Initialize(this, ComponentDescription, FiringDirection, FiringVelocity, (CurrentAmmo % 3 == 0));
		if(FiringEffect)
		{
			FiringEffect->ActivateSystem();
		}

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

void UFlareWeapon::SetupComponentMesh()
{
	Super::SetupComponentMesh();
}

void UFlareWeapon::StartFire()
{
	Firing = true;
}

void UFlareWeapon::StopFire()
{
	Firing = false;
}

float UFlareWeapon::GetHeatProduction() const
{
	// Produce heat if the player has fire recently, so can't fire due to cooldown
	return Super::GetHeatProduction() * (TimeSinceLastShell <= FiringPeriod ? 1.f : 0.f);
}

void UFlareWeapon::ApplyHeatDamage(float OverheatEnergy, float BurnEnergy)
{
	Super::ApplyHeatDamage(OverheatEnergy, BurnEnergy);
	// Apply damage only if the player has fire recently, so can't fire due to cooldown
	if(TimeSinceLastShell <= FiringPeriod)
	{
		ApplyDamage(OverheatEnergy);
	}
}

void UFlareWeapon::RefillAmmo()
{
	CurrentAmmo = MaxAmmo;
}

