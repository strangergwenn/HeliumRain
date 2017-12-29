
#include "FlareSpacecraftWeaponsSystem.h"
#include "../../Flare.h"

#include "../FlareTurret.h"
#include "../../Game/FlareGame.h"
#include "../../Player/FlarePlayerController.h"
#include "../FlareSpacecraft.h"

DECLARE_CYCLE_STAT(TEXT("FlareWeaponsSystem Tick"), STAT_FlareWeaponsSystem_Tick, STATGROUP_Flare);

#define LOCTEXT_NAMESPACE "FlareSpacecraftWeaponsSystem"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftWeaponsSystem::UFlareSpacecraftWeaponsSystem(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, Spacecraft(NULL)
{
}

UFlareSpacecraftWeaponsSystem::~UFlareSpacecraftWeaponsSystem()
{
	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{
		delete WeaponGroupList[GroupIndex];
	}
}

/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void UFlareSpacecraftWeaponsSystem::TickSystem(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareWeaponsSystem_Tick);
	if (!ActiveWeaponGroup)
	{
		return;
	}

	// Set current gun target for proximity weapons on S ships
	if (Spacecraft->GetNavigationSystem()->IsManualPilot() && Spacecraft->GetDescription()->Size == EFlarePartSize::S)
	{
		FVector CameraAimDirection = Spacecraft->GetCamera()->GetComponentRotation().Vector();

		for (UFlareWeapon* Weapon : Spacecraft->GetWeaponsSystem()->GetActiveWeaponGroup()->Weapons)
		{
			// Try getting a target
			AActor* HitTarget = NULL;
			Weapon->IsSafeToFire(0, HitTarget);
			if (!HitTarget || HitTarget == Weapon->GetSpacecraft())
			{
				HitTarget = Weapon->GetSpacecraft()->GetCurrentTarget();
			}

			// Aim the turret toward the target or a distant point
			if (HitTarget && Cast<UPrimitiveComponent>(HitTarget->GetRootComponent()))
			{
				FVector Location = HitTarget->GetActorLocation();
				FVector Velocity = Cast<UPrimitiveComponent>(HitTarget->GetRootComponent())->GetPhysicsLinearVelocity() / 100;
				Weapon->SetTarget(Location, Velocity);
			}
			else
			{
				FVector FireTargetLocation = Weapon->GetMuzzleLocation(0) + CameraAimDirection * 100000;
				Weapon->SetTarget(FireTargetLocation, FVector::ZeroVector);
			}
		}
	}

	// Are we firing ?
	bool WantFire = false;
	if (Spacecraft->GetNavigationSystem()->IsManualPilot() && Spacecraft->GetParent()->GetDamageSystem()->IsAlive())
	{
		WantFire = Spacecraft->GetStateManager()->IsWantFire();
	}

	switch (ActiveWeaponGroup->Type)
	{
		// Guns are controlled directly
		case EFlareWeaponGroupType::WG_GUN:
			for (int32 i = 0; i < ActiveWeaponGroup->Weapons.Num(); i++)
			{
				if (WantFire)
				{
					ActiveWeaponGroup->Weapons[i]->StartFire();
				}
				else
				{
					ActiveWeaponGroup->Weapons[i]->StopFire();
				}
			}
			break;

		// Turrets follow the player aim
		case EFlareWeaponGroupType::WG_TURRET:
			{
				for (int32 i = 0; i < ActiveWeaponGroup->Weapons.Num(); i++)
				{
					UFlareTurret* Turret = Cast<UFlareTurret>(ActiveWeaponGroup->Weapons[i]);
					UFlareTurretPilot* Pilot = Turret->GetTurretPilot();
					FVector CameraLocation = Spacecraft->GetCamera()->GetComponentLocation();
					FVector CameraAimDirection = Spacecraft->GetCamera()->GetComponentRotation().Vector();

					FVector AimDirection;
					float AimDistance;

					if(Spacecraft->GetCurrentTarget())
					{

						FVector AmmoIntersectionLocation;
						float AmmoVelocity = Turret->GetAmmoVelocity();
						float InterceptTime = Spacecraft->GetCurrentTarget()->GetAimPosition(Spacecraft, AmmoVelocity, 0.0, &AmmoIntersectionLocation);

						FVector MuzzleLocation = Turret->GetMuzzleLocation(0);

						FVector TargetOffset = AmmoIntersectionLocation - MuzzleLocation;
						FVector CameraOffset = CameraLocation - MuzzleLocation;

						float InterceptDistance = TargetOffset.Size();
						float CameraOffsetSize = CameraOffset.Size();
						float Cos = FVector::DotProduct(TargetOffset.GetUnsafeNormal(), CameraOffset.GetUnsafeNormal());

						// Law of cosines c^2 = a^2 + b^2 - 2 ab cos alpha
						float CameraDistance = FMath::Sqrt(CameraOffsetSize*CameraOffsetSize // a^2
														   + InterceptDistance*InterceptDistance
														   - CameraOffsetSize*InterceptDistance*Cos); // b^2

						FVector AimLocation = CameraLocation + CameraAimDirection * CameraDistance;

						AimDirection = (AimLocation - MuzzleLocation).GetUnsafeNormal();
						AimDistance = InterceptDistance;
					}
					else
					{
						AimDirection = CameraAimDirection;
						AimDistance = 10000000;
					}

					Pilot->PlayerSetAim(AimDirection, AimDistance);

					if (WantFire)
					{
						Pilot->PlayerStartFire();
					}
					else
					{
						Pilot->PlayerStopFire();
					}
				}
			}
			break;

		// Bombs are like guns, without fire stopping
		case EFlareWeaponGroupType::WG_BOMB:
		case EFlareWeaponGroupType::WG_MISSILE:
			if (Armed && WantFire)
			{
				Armed = false;
				int32 FireWeaponIndex = (ActiveWeaponGroup->LastFiredWeaponIndex+1) % ActiveWeaponGroup->Weapons.Num();
				ActiveWeaponGroup->Weapons[FireWeaponIndex]->StartFire();
				ActiveWeaponGroup->LastFiredWeaponIndex = FireWeaponIndex;
			}
			else if (!WantFire)
			{
				Armed = true;
			}
			break;

		case EFlareWeaponGroupType::WG_NONE:
		default:
			break;
	}
}

void UFlareSpacecraftWeaponsSystem::Initialize(AFlareSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData)
{
	Spacecraft = OwnerSpacecraft;
	Components = Spacecraft->GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	Description = Spacecraft->GetParent()->GetDescription();
	Data = OwnerData;
}

inline static bool ConstPredicate (const FFlareWeaponGroup& ip1, const FFlareWeaponGroup& ip2)
 {
	 return (ip1.Description->WeaponCharacteristics.Order < ip2.Description->WeaponCharacteristics.Order);
 }

void UFlareSpacecraftWeaponsSystem::Start()
{
	// Clear previous data
	WeaponList.Empty();
	WeaponDescriptionList.Empty();
	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{
		delete WeaponGroupList[GroupIndex];
	}
	WeaponGroupList.Empty();

	TArray<UActorComponent*> Weapons = Spacecraft->GetComponentsByClass(UFlareWeapon::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Weapons.Num(); ComponentIndex++)
	{
		UFlareWeapon* Weapon = Cast<UFlareWeapon>(Weapons[ComponentIndex]);
		if(Weapon->GetDescription() == NULL)
		{
			FLOGV("ERROR: Weapon %s has no description", *Weapon->GetName());
			continue;
		}
		WeaponList.Add(Weapon);
		WeaponDescriptionList.Add(Weapon->GetDescription());
		int32 GroupIndex = GetGroupByWeaponIdentifer(Weapon->GetDescription()->Identifier);
		if (GroupIndex < 0)
		{
			// No existing group yet
			FFlareWeaponGroup* WeaponGroup = new FFlareWeaponGroup();

			WeaponGroup->Description = Weapon->GetDescription();

			if (WeaponGroup->Description->WeaponCharacteristics.TurretCharacteristics.IsTurret)
			{
				WeaponGroup->Type = EFlareWeaponGroupType::WG_TURRET;
			}
			else if (WeaponGroup->Description->WeaponCharacteristics.BombCharacteristics.IsBomb)
			{
				if(WeaponGroup->Description->WeaponCharacteristics.BombCharacteristics.MaxBurnDuration > 0)
				{
					WeaponGroup->Type = EFlareWeaponGroupType::WG_MISSILE;
				}
				else
				{
					WeaponGroup->Type = EFlareWeaponGroupType::WG_BOMB;
				}
			}
			else
			{
				WeaponGroup->Type = EFlareWeaponGroupType::WG_GUN;
			}
			WeaponGroup->LastFiredWeaponIndex = 0;
			WeaponGroup->Weapons.Add(Weapon);
			WeaponGroup->Target = NULL;

			Weapon->SetWeaponGroup(WeaponGroup);
			WeaponGroupList.Add(WeaponGroup);
		}
		else
		{
			FFlareWeaponGroup* WeaponGroup = WeaponGroupList[GroupIndex];
			WeaponGroup->Weapons.Add(Weapon);
			Weapon->SetWeaponGroup(WeaponGroup);
		}
	}

	// init last fired ammo with the one before the one with the most ammo count
	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{

		FFlareWeaponGroup* WeaponGroup = WeaponGroupList[GroupIndex];

		//FLOGV("Group %d : component=%s", GroupIndex, *WeaponGroup->Description->Identifier.ToString())
		//FLOGV("Group %d : type=%d", GroupIndex, (int32)WeaponGroup->Type.GetValue());
		//FLOGV("Group %d : count=%d", GroupIndex, WeaponGroup->Weapons.Num());
		int MaxAmmo = 0;
		int MinAmmo = -1;
		int MaxAmmoIndex = -1;

		for (int32 WeaponIndex = 0; WeaponIndex < WeaponGroup->Weapons.Num(); WeaponIndex++)
		{
			int32 CurrentAmmo = WeaponGroup->Weapons[WeaponIndex]->GetCurrentAmmo();
			//FLOGV("WeaponIndex %d", WeaponIndex);
			//FLOGV("CurrentAmmo %d", CurrentAmmo);

			if (MaxAmmoIndex == -1 || CurrentAmmo > MaxAmmo)
			{
				MaxAmmo = CurrentAmmo;
				MaxAmmoIndex = WeaponIndex;

			}

			if (MinAmmo == -1 || CurrentAmmo < MinAmmo)
			{
				MinAmmo = CurrentAmmo;
			}
		}

		//FLOGV("MinAmmo %d", MinAmmo);
		//FLOGV("MaxAmmo %d", MaxAmmo);
		//FLOGV("MaxAmmoIndex %d", MaxAmmoIndex);

		WeaponGroup->LastFiredWeaponIndex = MaxAmmoIndex - 1;
		if(WeaponGroup->LastFiredWeaponIndex < 0)
		{
			WeaponGroup->LastFiredWeaponIndex = WeaponGroup->Weapons.Num() - 1;
		}
		//FLOGV("LastFiredWeaponIndex %d", WeaponGroup->LastFiredWeaponIndex);

		if(MinAmmo != MaxAmmo)
		{
			// Find first max
			while (WeaponGroup->Weapons[WeaponGroup->LastFiredWeaponIndex]->GetCurrentAmmo() == MaxAmmo)
			{
				WeaponGroup->LastFiredWeaponIndex--;
				if(WeaponGroup->LastFiredWeaponIndex < 0)
				{
					WeaponGroup->LastFiredWeaponIndex = WeaponGroup->Weapons.Num() - 1;
				}
			}
			//FLOGV("First LastFiredWeaponIndex %d", WeaponGroup->LastFiredWeaponIndex);
		}
	}

	// Sort group
	WeaponGroupList.Sort(&ConstPredicate);

	// TODO save
	ActiveWeaponGroupIndex = -1;
	ActiveWeaponGroup = NULL;
	LastActiveWeaponGroupIndex = 0;
	StopAllWeapons();
}

void UFlareSpacecraftWeaponsSystem::SetActiveWeaponTarget(AFlareSpacecraft* Target)
{
	if(ActiveWeaponGroup)
	{
		ActiveWeaponGroup->Target = Target;
	}
}

AFlareSpacecraft* UFlareSpacecraftWeaponsSystem::GetActiveWeaponTarget()
{
	if(ActiveWeaponGroup)
	{
		return ActiveWeaponGroup->Target;
	}
	return NULL;
}

void UFlareSpacecraftWeaponsSystem::StopAllWeapons()
{
	for (int32 WeaponIndex = 0; WeaponIndex < WeaponList.Num(); WeaponIndex++)
	{
		WeaponList[WeaponIndex]->StopFire();
	}
}

void UFlareSpacecraftWeaponsSystem::ActivateWeaponGroup(int32 Index)
{

	if (Index >= 0 && Index < WeaponGroupList.Num() && ActiveWeaponGroupIndex != Index)
	{
		StopAllWeapons();
		ActiveWeaponGroupIndex = Index;
		ActiveWeaponGroup = WeaponGroupList[Index];
		LastActiveWeaponGroupIndex = ActiveWeaponGroupIndex;
		Armed = false;
	}
}

void UFlareSpacecraftWeaponsSystem::ActivateWeapons(bool Activate)
{
	if (Activate)
	{
		ActivateWeapons();
	}
	else
	{
		DeactivateWeapons();
	}
}

void UFlareSpacecraftWeaponsSystem::ActivateWeapons()
{
	ActivateWeaponGroup(LastActiveWeaponGroupIndex);
}

void UFlareSpacecraftWeaponsSystem::DeactivateWeapons()
{
	StopAllWeapons();
	ActiveWeaponGroup = NULL;
	ActiveWeaponGroupIndex = -1;
}

bool UFlareSpacecraftWeaponsSystem::IsInFireDirector()
{
	// The fire director mode was used to control turrets
	// It would be useful later on, for a command ship, but is now unused

	return false;
}

void UFlareSpacecraftWeaponsSystem::ToggleWeaponActivation()
{
	if (ActiveWeaponGroupIndex < 0)
	{
		ActivateWeapons();
		if (Spacecraft->GetParent() == Spacecraft->GetGame()->GetPC()->GetPlayerShip())
		{
			Spacecraft->GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("toggle-combat").PutInt32("new-state", 1));
		}
	}
	else
	{
		DeactivateWeapons();
		if (Spacecraft->GetParent() == Spacecraft->GetGame()->GetPC()->GetPlayerShip())
		{
			Spacecraft->GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("toggle-combat").PutInt32("new-state", 0));
		}
	}
}

int32 UFlareSpacecraftWeaponsSystem::GetGroupByWeaponIdentifer(FName Identifier) const
{
	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{
		if (WeaponGroupList[GroupIndex]->Description->Identifier == Identifier)
		{
			return GroupIndex;
		}
	}
	return -1;
}

EFlareWeaponGroupType::Type UFlareSpacecraftWeaponsSystem::GetActiveWeaponType() const
{
	if (ActiveWeaponGroup)
	{
		return ActiveWeaponGroup->Type;
	}
	return EFlareWeaponGroupType::WG_NONE;
}

bool UFlareSpacecraftWeaponsSystem::HasUsableWeaponType(EFlareWeaponGroupType::Type Type) const
{
	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{
		if (WeaponGroupList[GroupIndex]->Type == Type && Spacecraft->GetDamageSystem()->GetWeaponGroupHealth(GroupIndex, true))
		{
			return true;
		}
	}
	return false;
}

void UFlareSpacecraftWeaponsSystem::GetTargetPreference(float* IsSmall, float* IsLarge, float* IsUncontrollableCivil, float* IsUncontrollableSmallMilitary, float* IsUncontrollableLargeMilitary, float* IsNotUncontrollable, float* IsStation, float* IsHarpooned, FFlareWeaponGroup* RestrictGroup)
{
	float LargePool = 0;
	float SmallPool = 0;
	float StationPool = 0;
	float UncontrollableCivilPool = 0;
	float UncontrollableSmallMilitaryPool = 0;
	float UncontrollableLargeMilitaryPool = 0;
	float NotUncontrollablePool = 0;
	float HarpoonedPool = 0;


	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{
		EFlareShellDamageType::Type DamageType = WeaponGroupList[GroupIndex]->Description->WeaponCharacteristics.DamageType;

		if(RestrictGroup && WeaponGroupList[GroupIndex] != RestrictGroup)
		{
			continue;
		}

		if (Spacecraft->GetDamageSystem()->GetWeaponGroupHealth(GroupIndex, true) <= 0)
		{
			continue;
		}

		SmallPool += WeaponGroupList[GroupIndex]->Description->WeaponCharacteristics.AntiSmallShipValue;
		LargePool += WeaponGroupList[GroupIndex]->Description->WeaponCharacteristics.AntiLargeShipValue;
		StationPool += WeaponGroupList[GroupIndex]->Description->WeaponCharacteristics.AntiStationValue;

		if(DamageType == EFlareShellDamageType::LightSalvage)

		{
			UncontrollableCivilPool += 1.0;
			UncontrollableSmallMilitaryPool += 1.0;
		}
		else if(DamageType == EFlareShellDamageType::HeavySalvage)
		{
			UncontrollableCivilPool += 1.0;
			UncontrollableLargeMilitaryPool += 1.0;
		}
		else
		{
			NotUncontrollablePool += 1.0;
			HarpoonedPool += 1.0;
		}
	}

	if(LargePool > 0 || SmallPool > 0)
	{
		FVector2D PoolVector = FVector2D(LargePool, SmallPool);
		PoolVector.Normalize();
		LargePool = PoolVector.X;
		SmallPool = PoolVector.Y;
	}

	StationPool = FMath::Clamp(StationPool, 0.f, 0.1f);
	HarpoonedPool = FMath::Clamp(HarpoonedPool, 0.f, 0.1f);
	NotUncontrollablePool = FMath::Clamp(NotUncontrollablePool, 0.f, 0.1f);
	UncontrollableCivilPool = FMath::Clamp(UncontrollableCivilPool, 0.f, 0.1f);
	UncontrollableSmallMilitaryPool = FMath::Clamp(UncontrollableSmallMilitaryPool, 0.f, 0.1f);
	UncontrollableLargeMilitaryPool = FMath::Clamp(UncontrollableLargeMilitaryPool, 0.f, 0.1f);

	*IsLarge  = LargePool;
	*IsSmall  = SmallPool;
	*IsNotUncontrollable  = NotUncontrollablePool;
	*IsUncontrollableCivil  = UncontrollableCivilPool;
	*IsUncontrollableSmallMilitary = UncontrollableSmallMilitaryPool;
	*IsUncontrollableLargeMilitary = UncontrollableLargeMilitaryPool;
	*IsStation = StationPool;
	*IsHarpooned = HarpoonedPool;
}

int32 UFlareSpacecraftWeaponsSystem::FindBestWeaponGroup(AFlareSpacecraft* Target)
{
	int32 BestWeaponGroup = -1;
	float BestScore = 0;

	if (!Target) {
		return -1;
	}

	bool LargeTarget = (Target->GetSize() == EFlarePartSize::L);
	bool SmallTarget = (Target->GetSize() == EFlarePartSize::S);
	bool UncontrollableTarget = Target->GetParent()->GetDamageSystem()->IsUncontrollable();

	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{
		float Score = Spacecraft->GetDamageSystem()->GetWeaponGroupHealth(GroupIndex, true);


		EFlareShellDamageType::Type DamageType = WeaponGroupList[GroupIndex]->Description->WeaponCharacteristics.DamageType;

		if (SmallTarget)
		{
			Score *= WeaponGroupList[GroupIndex]->Description->WeaponCharacteristics.AntiSmallShipValue;
		}

		if (LargeTarget)
		{
			Score *= WeaponGroupList[GroupIndex]->Description->WeaponCharacteristics.AntiLargeShipValue;
		}


		if(DamageType == EFlareShellDamageType::LightSalvage || DamageType == EFlareShellDamageType::HeavySalvage)
		{
			Score *= (UncontrollableTarget ? 1.f : 0.f);
			Score *= (Target->GetParent()->IsHarpooned() ? 0.f: 1.f);
		}
		else
		{
			if(Target->IsMilitary())
			{
				Score *= (UncontrollableTarget ? 0.01f : 1.f);
			}
			else
			{
				Score *= (UncontrollableTarget ? 0.f : 1.f);
			}
		}

		if (Score > 0 && Score > BestScore)
		{
			BestWeaponGroup = GroupIndex;
			BestScore = Score;
		}
	}

	return BestWeaponGroup;
}

#undef LOCTEXT_NAMESPACE
