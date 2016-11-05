
#include "../Flare.h"
#include "FlareSpacecraftTypes.h"
#include "FlareWeapon.h"
#include "FlareSpacecraft.h"
#include "FlareShell.h"
#include "FlareBomb.h"
#include "../Player/FlarePlayerController.h"

DECLARE_CYCLE_STAT(TEXT("FlareWeapon Firing"), STAT_Weapon_Firing, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareWeapon FireGun"), STAT_Weapon_FireGun, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareWeapon ConfigureShellFuze"), STAT_Weapon_ConfigureShellFuze, STATGROUP_Flare);


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareWeapon::UFlareWeapon(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, FiringEffect(NULL)
	, FiringRate(0)
	, Firing(false)
{
	static ConstructorHelpers::FObjectFinder<USoundCue> EmptySoundObj(TEXT("/Game/Master/Sound/Sounds/A_Empty"));
	static ConstructorHelpers::FObjectFinder<USoundCue> BombDroppedSoundObj(TEXT("/Game/Master/Sound/Sounds/A_Bomb_Dropped"));
	BombDroppedSound = BombDroppedSoundObj.Object;
	EmptySound = EmptySoundObj.Object;

	LocalHeatEffect = true;
	HasFlickeringLights = false;

	TargetLocation = FVector::ZeroVector;
	TargetVelocity = FVector::ZeroVector;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareWeapon::Initialize(FFlareSpacecraftComponentSave* Data, UFlareCompany* Company, AFlareSpacecraftPawn* OwnerShip, bool IsInMenu)
{
	Super::Initialize(Data, Company, OwnerShip, IsInMenu);
	
	// Destroy attached bombs
	ClearBombs();

	// Setup properties
	if (ComponentDescription && Spacecraft)
	{
		FiringRate = ComponentDescription->WeaponCharacteristics.GunCharacteristics.AmmoRate;

		FiringSound = ComponentDescription->WeaponCharacteristics.FiringSound;
		FiringEffectTemplate = ComponentDescription->WeaponCharacteristics.GunCharacteristics.FiringEffect;

		//FLOGV("UFlareWeapon::Initialize IsBomb ? %d", ComponentDescription->WeaponCharacteristics.BombCharacteristics.IsBomb);

		if (ComponentDescription->WeaponCharacteristics.BombCharacteristics.IsBomb)
		{
			AmmoVelocity = ComponentDescription->WeaponCharacteristics.BombCharacteristics.DropLinearVelocity;
			FiringPeriod =  0;
			FLOGV("IsBomb num = %d", ComponentDescription->WeaponCharacteristics.AmmoCapacity);
			FillBombs();
		}
		else
		{
			AmmoVelocity = ComponentDescription->WeaponCharacteristics.GunCharacteristics.AmmoVelocity;
			FiringPeriod =  1.f / (FiringRate / 60.f);
		}
	}

	// Spawn properties
	ProjectileSpawnParams.Instigator = SpacecraftPawn;
	ProjectileSpawnParams.bNoFail = true;
	ProjectileSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Additional properties
	LastFiredGun = -1;
	SetupFiringEffects();
}

void UFlareWeapon::SetupFiringEffects()
{
	// TODO Handle multiple muzzle has for turrets
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
	return Super::Save();
}

void UFlareWeapon::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceLastShell += DeltaTime;

	if (Firing && TimeSinceLastShell >= FiringPeriod && GetUsableRatio() > 0.f && Spacecraft->GetParent()->GetDamageSystem()->IsAlive())
	{
		if (GetCurrentAmmo() > 0)
		{
			SCOPE_CYCLE_COUNTER(STAT_Weapon_Firing);

			if (ComponentDescription->WeaponCharacteristics.GunCharacteristics.IsGun)
			{
				if (ComponentDescription->WeaponCharacteristics.GunCharacteristics.AlternedFire)
				{
					LastFiredGun = (LastFiredGun + 1 ) % ComponentDescription->WeaponCharacteristics.GunCharacteristics.GunCount;
					FireGun(LastFiredGun);
				}
				else
				{
					for (int GunIndex = 0; GunIndex < ComponentDescription->WeaponCharacteristics.GunCharacteristics.GunCount; GunIndex++)
					{
						FireGun(GunIndex);
					}
				}
			}
			else if (ComponentDescription->WeaponCharacteristics.BombCharacteristics.IsBomb)
			{
				FireBomb();
			}

			// If damage the firerate is randomly reduced to a min of 10 times normal value
			float DamageDelay = FMath::Square(1.f- GetUsableRatio()) * 10 * FiringPeriod * FMath::FRandRange(0.f, 1.f);
			TimeSinceLastShell = -DamageDelay;
		}

		// Empty
		else if (!IsTurret() && SpacecraftPawn && SpacecraftPawn->IsLocallyControlled())
		{
			Spacecraft->GetPC()->ClientPlaySound(EmptySound);
			TimeSinceLastShell = 0;
		}
	}

	if (FiringPeriod == 0)
	{
		Firing = false;
	}
}

void UFlareWeapon::SetTarget(FVector NewTargetLocation, FVector NewTargetVelocity)
{
	TargetLocation = NewTargetLocation;
	TargetVelocity = NewTargetVelocity;
}

bool UFlareWeapon::FireGun(int GunIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_Weapon_FireGun);

	// Avoid firing itself
	AActor* Unused;
	if (!IsSafeToFire(GunIndex, Unused))
	{
		FLOGV("%s Not secure", *GetReadableName());
		return false;
	}

	// Get firing data
	FVector FiringLocation = GetMuzzleLocation(GunIndex);
	float Imprecision  = FMath::DegreesToRadians(ComponentDescription->WeaponCharacteristics.GunCharacteristics.AmmoPrecision  + 3.f *(1 - GetUsableRatio()));
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
	ConfigureShellFuze(Shell);
	ShowFiringEffects(GunIndex);

	// Play sound
	if (SpacecraftPawn && SpacecraftPawn->IsLocallyControlled())
	{
		Spacecraft->GetPC()->ClientPlaySound(FiringSound);
	}

	// Update data
	ShipComponentData->Weapon.FiredAmmo++;
	Spacecraft->GetParent()->GetDamageSystem()->SetAmmoDirty();
	return true;
}

void UFlareWeapon::ShowFiringEffects(int GunIndex)
{
	if (FiringEffect)
	{
		FiringEffect->ActivateSystem();
	}
}

bool UFlareWeapon::FireBomb()
{
	// Play sound
	if (SpacecraftPawn && SpacecraftPawn->IsLocallyControlled())
	{
		Spacecraft->GetPC()->ClientPlaySound(BombDroppedSound);
	}

	// Drop bomb
	AFlareBomb* Bomb = Bombs.Pop();
	if (Bomb)
	{
		Bomb->OnLaunched();
		ShipComponentData->Weapon.FiredAmmo++;
		Spacecraft->GetParent()->GetDamageSystem()->SetAmmoDirty();
	}
	return true;
}

void UFlareWeapon::ConfigureShellFuze(AFlareShell* Shell)
{
	SCOPE_CYCLE_COUNTER(STAT_Weapon_ConfigureShellFuze);

	if (ComponentDescription->WeaponCharacteristics.FuzeType == EFlareShellFuzeType::Proximity)
	{
		float SecurityRadius = 	ComponentDescription->WeaponCharacteristics.AmmoExplosionRadius + Spacecraft->GetMeshScale() / 100;
		float SecurityDelay = SecurityRadius / ComponentDescription->WeaponCharacteristics.GunCharacteristics.AmmoVelocity;
		float ActiveTime = 10;

		FVector RelativeFiringVelocity = Spacecraft->GetLinearVelocity() - TargetVelocity;
		FVector TargetOffset = TargetLocation - Spacecraft->GetActorLocation();
		float EstimatedDistance = TargetOffset.Size() / 100;

		float RelativeFiringVelocityInAxis = FVector::DotProduct(RelativeFiringVelocity, TargetOffset.GetUnsafeNormal());
		float EstimatedRelativeVelocity = ComponentDescription->WeaponCharacteristics.GunCharacteristics.AmmoVelocity + RelativeFiringVelocityInAxis;
		float EstimatedFlightTime = EstimatedDistance / EstimatedRelativeVelocity;

		float NeededSecurityDelay = EstimatedFlightTime * 0.5;
		SecurityDelay = FMath::Max(SecurityDelay, NeededSecurityDelay);
		ActiveTime = EstimatedFlightTime * 1.5 - SecurityDelay;

		Shell->SetFuzeTimer(SecurityDelay, ActiveTime);
	}
}

void UFlareWeapon::SetVisibleInUpgrade(bool Visible)
{
	Super::SetVisibleInUpgrade(Visible);

	if (!Visible)
	{
		ClearBombs();
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

float UFlareWeapon::GetHeatProduction() const
{
	// Produce heat if the player has fire recently, so can't fire due to cooldown
	return Super::GetHeatProduction() * (TimeSinceLastShell <= FiringPeriod ? 1.f : 0.f);
}

void UFlareWeapon::OnRefilled()
{
	if (ComponentDescription->WeaponCharacteristics.BombCharacteristics.IsBomb)
	{
		FillBombs();
	}
}

void UFlareWeapon::FillBombs()
{
	UStaticMeshSocket* BombHardpoint = ComponentDescription->Mesh->FindSocket("Hardpoint");
	//FLOGV("BombHardpoint RelativeLocation=%s", *BombHardpoint->RelativeLocation.ToString());
	int CurrentBombCount = Bombs.Num();

	for (int BombIndex = CurrentBombCount; BombIndex < GetCurrentAmmo() ; BombIndex++)
	{
		// Get data
		FVector HardpointLocation;
		FRotator HardpointRotation;

		FName HardpointName = FName(*(FString("Hardpoint") + FString::FromInt(BombIndex)));

		FTransform HardPointWorldTransform = GetSocketTransform(HardpointName);

		GetSocketWorldLocationAndRotation(HardpointName, HardpointLocation, HardpointRotation);
		
		/*FLOGV("Bomb %d HardpointName=%s", BombIndex, *HardpointName.ToString());
		FLOGV("Bomb %d HardpointLocation=%s", BombIndex, *HardpointLocation.ToString());
		FLOGV("Bomb %d HardpointRotation=%s", BombIndex, *HardpointRotation.ToString());*/

		UStaticMeshSocket* Hardpoint = StaticMesh->FindSocket(HardpointName);
		FMatrix SocketMatrix;
		Hardpoint->GetSocketMatrix(SocketMatrix, this);

		/*FLOGV("Bomb %d RelativeLocation=%s", BombIndex, *(Hardpoint->RelativeLocation.ToString()));
		FLOGV("Bomb %d RelativeRotation=%s", BombIndex, *(Hardpoint->RelativeRotation.ToString()));*/
		
		FVector BombLocation = HardPointWorldTransform.TransformPosition(-BombHardpoint->RelativeLocation);

		//FLOGV("Bomb %d BombLocation=%s", BombIndex, *BombLocation.ToString());

		BombLocation = SocketMatrix.TransformPosition(-BombHardpoint->RelativeLocation);

		//FLOGV("Bomb %d BombLocation2=%s", BombIndex, *BombLocation.ToString());
		//FLOGV("Bomb %d HardPointWorldTransform.Rotator()=%s", BombIndex, *(HardPointWorldTransform.Rotator().ToString()));
		//FLOGV("Bomb %d SocketMatrix.Rotator()=%s", BombIndex, *(SocketMatrix.Rotator().ToString()));
		
		float Roll = 0;

		bool NegativeZScale = RelativeScale3D.Z < 0;

		//FLOGV("Bomb %d NegativeZScale=%d", BombIndex, NegativeZScale);
		//FLOGV("Bomb %d RelativeScale3D=%s", BombIndex, *RelativeScale3D.ToString());

		if (BombIndex == 0)
		{
			Roll = 90;
		}
		else if (BombIndex == 1)
		{
			Roll = -90;
		}
		else if (BombIndex == 2)
		{
			Roll = (NegativeZScale ? 180 : 0);
		}

		FTransform LocalRotation(FRotator(0,0,Roll));

		//FLOGV("Bomb %d Roll=%f", Roll);
		
		FTransform Rotation = LocalRotation * ComponentToWorld;
		
		//FLOGV("Bomb %d LocalRotation.Rotator()=%s", BombIndex, *(LocalRotation.Rotator().ToString()));
		//FLOGV("Bomb %d Rotation.Rotator()=%s", BombIndex, *(Rotation.Rotator().ToString()));

		// Spawn parameters
		FActorSpawnParameters Params;
		Params.bNoFail = true;

		AFlareBomb* Bomb = GetWorld()->SpawnActor<AFlareBomb>(AFlareBomb::StaticClass(), BombLocation, Rotation.Rotator(), Params);
		Bomb->AttachToActor(Spacecraft, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true), NAME_None);
		Bomb->Initialize(NULL, this);

		Cast<class UPrimitiveComponent>(Bomb->GetRootComponent())->IgnoreActorWhenMoving(GetSpacecraft(), true);
		GetSpacecraft()->Airframe->IgnoreActorWhenMoving(Bomb, true);
		for (int i = 0; i < Bombs.Num(); i++)
		{
			Cast<class UPrimitiveComponent>(Bombs[i]->GetRootComponent())->IgnoreActorWhenMoving(Bomb, true);
			Cast<class UPrimitiveComponent>(Bomb->GetRootComponent())->IgnoreActorWhenMoving(Bombs[i], true);
		}

		Bombs.Add(Bomb);
	}
}

void UFlareWeapon::ClearBombs()
{
	for (int i = 0; i < Bombs.Num(); i++)
	{
		Bombs[i]->Destroy();
	}
	Bombs.Empty();
}

FVector UFlareWeapon::GetFireAxis() const
{
	return GetComponentRotation().RotateVector(FVector(1, 0, 0));
}

FVector UFlareWeapon::GetMuzzleLocation(int muzzleIndex) const
{
	if (ComponentDescription->WeaponCharacteristics.GunCharacteristics.GunCount == 1)
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
	return ComponentDescription->WeaponCharacteristics.GunCharacteristics.GunCount;
}

bool UFlareWeapon::IsTurret() const
{
	return ComponentDescription->WeaponCharacteristics.TurretCharacteristics.IsTurret;
}

bool UFlareWeapon::IsSafeToFire(int GunIndex, AActor*& HitTarget) const
{
	HitTarget = NULL;
	return true;
}

float UFlareWeapon::GetAimRadius() const
{
	if (ComponentDescription->WeaponCharacteristics.FuzeType == EFlareShellFuzeType::Proximity)
	{
		return ComponentDescription->WeaponCharacteristics.FuzeMaxDistanceThresold;
	}
	return 0;
}

UStaticMesh* UFlareWeapon::GetMesh(bool PresentationMode) const
{
	if (!PresentationMode && ComponentDescription && ComponentDescription->WeaponCharacteristics.BombCharacteristics.IsBomb)
	{
		return ComponentDescription->WeaponCharacteristics.BombCharacteristics.BombMesh;
	}

	if (!PresentationMode && ComponentDescription && ComponentDescription->WeaponCharacteristics.TurretCharacteristics.IsTurret)
	{
		return NULL;
	}

	return Super::GetMesh(PresentationMode);
}

void UFlareWeapon::OnAttachmentChanged()
{
	Super::OnAttachmentChanged();

	if (!GetAttachParent())
	{
		ClearBombs();
	}
}

void UFlareWeapon::BeginDestroy()
{
	Super::BeginDestroy();
	ClearBombs();
}

FText UFlareWeapon::GetSlotName() const
{
	FFlareSpacecraftDescription* SpacecraftDesc = Spacecraft->GetParent()->GetDescription();

	//Find Local slot check
	if (ComponentDescription && ComponentDescription->WeaponCharacteristics.TurretCharacteristics.IsTurret)
	{
		for (int32 i = 0; i < SpacecraftDesc->TurretSlots.Num(); i++)
		{
				// TODO optimize and store that in cache
				if (Spacecraft->GetParent()->GetDescription()->TurretSlots[i].SlotIdentifier == ShipComponentData->ShipSlotIdentifier)
				{
					int32 GroupIndex = SpacecraftDesc->TurretSlots[i].GroupIndex;
					FCHECK(GroupIndex >= 0 && GroupIndex < SpacecraftDesc->WeaponGroups.Num());
					return SpacecraftDesc->WeaponGroups[GroupIndex].GroupName;
				}
		}
	}
	else
	{
		for (int32 i = 0; i < Spacecraft->GetParent()->GetDescription()->GunSlots.Num(); i++)
		{
			// TODO optimize and store that in cache
			if (SpacecraftDesc->GunSlots[i].SlotIdentifier == ShipComponentData->ShipSlotIdentifier)
			{
				int32 GroupIndex = SpacecraftDesc->GunSlots[i].GroupIndex;
				FCHECK(GroupIndex >= 0 && GroupIndex < SpacecraftDesc->WeaponGroups.Num());
				return SpacecraftDesc->WeaponGroups[GroupIndex].GroupName;
			}
		}
	}

	return FText();
}

bool UFlareWeapon::IsDestroyedEffectRelevant()
{
	// Always smoke
	return true;
}
