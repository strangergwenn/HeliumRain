
#include "../Flare.h"
#include "FlareWeapon.h"
#include "FlareSpacecraft.h"
#include "FlareShell.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareWeapon::UFlareWeapon(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, FiringEffect(NULL)
	, Target(NULL)
	, FiringRate(0)
	, MaxAmmo(0)
	, Firing(false)
{
	HasLocalHeatEffect = true;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareWeapon::Initialize(const FFlareSpacecraftComponentSave* Data, UFlareCompany* Company, AFlareSpacecraftPawn* OwnerShip, bool IsInMenu)
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
			FFlareSpacecraftComponentAttributeSave FiredAmmoAttribute;
			if(ShipComponentData.Attributes[i].AttributeIdentifier == FName("weapon-fired-ammo"))
			{
				FiredAmmo = ShipComponentData.Attributes[i].AttributeValue;
			}
		}
	}

	// Spawn properties
	ProjectileSpawnParams.Instigator = SpacecraftPawn;
	ProjectileSpawnParams.bNoFail = true;
	ProjectileSpawnParams.bNoCollisionFail = true;

	// Additional properties
	CurrentAmmo = MaxAmmo - FiredAmmo;
	FiringPeriod = 1 / (FiringRate / 60);
	LastFiredGun = -1;

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

FFlareSpacecraftComponentSave* UFlareWeapon::Save()
{
	float FiredAmmo = MaxAmmo - CurrentAmmo;
	FFlareSpacecraftComponentAttributeSave FiredAmmoAttribute;
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

	if (Firing && CurrentAmmo > 0 && TimeSinceLastShell > FiringPeriod && GetDamageRatio() > 0.f && IsPowered() && Spacecraft && !Spacecraft->GetDamageSystem()->HasPowerOutage())
	{
		if (ComponentDescription->GunCharacteristics.AlternedFire)
		{
			LastFiredGun = (LastFiredGun + 1 ) % ComponentDescription->GunCharacteristics.GunCount;
			FireGun(LastFiredGun);
		}
		else
		{
			for(int GunIndex = 0; GunIndex < ComponentDescription->GunCharacteristics.GunCount; GunIndex++)
			{
				FireGun(GunIndex);
			}
		}

		// If damage the firerate is randomly reduced to a min of 10 times normal value
		float DamageDelay = FMath::Square(1.f- GetDamageRatio()) * 10 * FiringPeriod * FMath::FRandRange(0.f, 1.f);
		TimeSinceLastShell = -DamageDelay;
	}
}


bool UFlareWeapon::FireGun(int GunIndex)
{
	if(!IsSafeToFire(GunIndex))
	{
		// Avoid to fire itself
		return false;
	}

	// Get firing data
	FVector FiringLocation = GetMuzzleLocation(GunIndex);
	float Imprecision  = FMath::DegreesToRadians(ComponentDescription->GunCharacteristics.AmmoPrecision  + 3.f *(1 - GetDamageRatio()));

	FVector FiringDirection = FMath::VRandCone(GetFireAxis(), Imprecision);
	FVector FiringVelocity = GetPhysicsLinearVelocity();

	// Create a shell
	AFlareShell* Shell = GetWorld()->SpawnActor<AFlareShell>(
		AFlareShell::StaticClass(),
		FiringLocation,
		FRotator::ZeroRotator,
		ProjectileSpawnParams);

	// Fire it. Tracer ammo every bullets
	Shell->Initialize(this, ComponentDescription, FiringDirection, FiringVelocity, true);

	//Configure fuze if needed
	ConfigureShellFuze(Shell);

	if(FiringEffect)
	{
		FiringEffect->ActivateSystem();
	}

	// Play sound
	if (SpacecraftPawn && SpacecraftPawn->IsLocallyControlled())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FiringSound, GetComponentLocation(), 1, 1);
	}

	// Update data
	CurrentAmmo--;
	return true;
}

void UFlareWeapon::ConfigureShellFuze(AFlareShell* Shell)
{
	if(ComponentDescription->GunCharacteristics.FuzeType == EFlareShellFuzeType::Proximity)
	{
		float SecurityRadius = 	ComponentDescription->GunCharacteristics.AmmoExplosionRadius + Spacecraft->GetMeshScale() / 100;
		float SecurityDelay = SecurityRadius / ComponentDescription->GunCharacteristics.AmmoVelocity;
		float ActiveTime = 10;

		if(Target)
		{
			FVector TargetOffset = Target->GetActorLocation() - Spacecraft->GetActorLocation();
			float EstimatedDistance = TargetOffset .Size() / 100;

			FVector FiringVelocity = Spacecraft->GetLinearVelocity();
			FVector TargetVelocity = FVector::ZeroVector;
			UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(Target->GetRootComponent());
			if(RootComponent)
			{
				TargetVelocity = RootComponent->GetPhysicsLinearVelocity() / 100;
			}

			FVector RelativeFiringVelocity = FiringVelocity - TargetVelocity;

			float RelativeFiringVelocityInAxis = FVector::DotProduct(RelativeFiringVelocity, TargetOffset.GetUnsafeNormal());

			float EstimatedRelativeVelocity = ComponentDescription->GunCharacteristics.AmmoVelocity + RelativeFiringVelocityInAxis;
			float EstimatedFlightTime = EstimatedDistance / EstimatedRelativeVelocity;

			float NeededSecurityDelay = EstimatedFlightTime * 0.5;
			SecurityDelay = FMath::Max(SecurityDelay, NeededSecurityDelay);
			ActiveTime = EstimatedFlightTime * 1.5 - SecurityDelay;
		}

		Shell->SetFuzeTimer(SecurityDelay, ActiveTime);
	}
}

void UFlareWeapon::SetTarget(AActor *NewTarget)
{
	Target = NewTarget;
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

FVector UFlareWeapon::GetFireAxis() const
{
	return GetComponentRotation().RotateVector(FVector(1, 0, 0));
}

FVector UFlareWeapon::GetMuzzleLocation(int muzzleIndex) const
{
	if (ComponentDescription->GunCharacteristics.GunCount == 1)
	{
		return GetSocketLocation(FName("Muzzle"));
	}
	else
	{
		return GetSocketLocation(FName(*(FString("Muzzle") + FString::FromInt(muzzleIndex))));
	}

}

int UFlareWeapon::GetGunCount() const
{
	return ComponentDescription->GunCharacteristics.GunCount;
}

bool UFlareWeapon::IsTurret() const
{
	return ComponentDescription->TurretCharacteristics.IsTurret;
}

bool UFlareWeapon::IsSafeToFire(int GunIndex) const
{
	// Only turret are unsafe
	return true;
}

float UFlareWeapon::GetAimRadius() const
{
	if(ComponentDescription->GunCharacteristics.FuzeType == EFlareShellFuzeType::Proximity)
	{
		return ComponentDescription->GunCharacteristics.FuzeMaxDistanceThresold;
	}
	return 0;
}

