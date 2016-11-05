

#include "../Flare.h"
#include "FlareBattle.h"
#include "FlareWorld.h"
#include "FlareGame.h"
#include "FlareGameTools.h"

struct BattleTargetPreferences
{
        float IsLarge;
        float IsSmall;
        float IsStation;
        float IsNotStation;
        float IsMilitary;
        float IsNotMilitary;
        float IsDangerous;
        float IsNotDangerous;
        float IsStranded;
        float IsNotStranded;
		float IsUncontrollableCivil;
		float IsUncontrollableMilitary;
		float IsNotUncontrollable;
        float IsHarpooned;
        float TargetStateWeight;
};

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareBattle::UFlareBattle(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareBattle::Load(UFlareSimulatedSector* BattleSector)
{
    Game = Cast<UFlareWorld>(GetOuter())->GetGame();
    Sector = BattleSector;
    PlayerCompany = Game->GetPC()->GetCompany();
	Catalog = Game->GetShipPartsCatalog();

}

/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/


void UFlareBattle::Simulate()
{
    int32 BattleTurn = 0;

    FLOGV("Simulate battle in %s", *Sector->GetSectorName().ToString());

	CombatLog::AutomaticBattleStarted(Sector);

	while (HasBattle())
    {
        BattleTurn++;
		if(!SimulateTurn())
        {
            FLOG("Nobody can fight, end battle");
            break;
        }
        if (BattleTurn > 1000)
        {
            FLOG("ERROR: Battle too long, still not ended after 1000 turns");
            break;
        }
    }

	// Remove destroyed spacecraft
	TArray<UFlareSimulatedSpacecraft*> SpacecraftToRemove;

	for (int32 SpacecraftIndex = 0 ; SpacecraftIndex < Sector->GetSectorSpacecrafts().Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Sector->GetSectorSpacecrafts()[SpacecraftIndex];

		if(!Spacecraft->GetDamageSystem()->IsAlive())
		{
			SpacecraftToRemove.Add(Spacecraft);
		}
	}

	for (int SpacecraftIndex = 0; SpacecraftIndex < SpacecraftToRemove.Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = SpacecraftToRemove[SpacecraftIndex];
		Spacecraft->GetCompany()->DestroySpacecraft(Spacecraft);
	}


	CombatLog::AutomaticBattleEnded(Sector);
    FLOGV("Battle in %s finish after %d turns", *Sector->GetSectorName().ToString(), BattleTurn);
}

bool UFlareBattle::HasBattle()
{
    // Check if battle
    bool HasBattle = false;
    for (int CompanyIndex = 0; CompanyIndex < Game->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
    {
        UFlareCompany* Company = Game->GetGameWorld()->GetCompanies()[CompanyIndex];

        if (Company == PlayerCompany && Sector == GetGame()->GetPC()->GetPlayerShip()->GetCurrentSector())
        {
            // Local sector, don't check if the player want fight
            continue;
        }

        EFlareSectorBattleState::Type BattleState = Sector->GetSectorBattleState(Company);

        if(BattleState == EFlareSectorBattleState::NoBattle ||
            BattleState == EFlareSectorBattleState::BattleLost ||
            BattleState == EFlareSectorBattleState::BattleNoRetreat)
        {
            // Don't want fight
            continue;
        }

        return true;
    }
    return false;
}

bool UFlareBattle::SimulateTurn()
{
    bool HasFight = false;

    // List company in war
    TArray<UFlareCompany*> FightingCompanies;
    for (int CompanyIndex = 0; CompanyIndex < Game->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
    {
        UFlareCompany* Company = Game->GetGameWorld()->GetCompanies()[CompanyIndex];

        EFlareSectorBattleState::Type BattleState = Sector->GetSectorBattleState(Company);

        if(BattleState == EFlareSectorBattleState::NoBattle ||
            BattleState == EFlareSectorBattleState::BattleLost ||
            BattleState == EFlareSectorBattleState::BattleNoRetreat)
        {
            // Don't want fight
            continue;
        }

        FightingCompanies.Add(Company);
    }

    // List all fighting ships
    TArray<UFlareSimulatedSpacecraft*> ShipToSimulate;
    for (int32 ShipIndex = 0 ; ShipIndex < Sector->GetSectorShips().Num(); ShipIndex++)
    {
        UFlareSimulatedSpacecraft* Ship = Sector->GetSectorShips()[ShipIndex];

        if(!Ship->IsMilitary()  || Ship->GetDamageSystem()->IsDisarmed())
        {
            // No weapon
            continue;
        }

        if(!FightingCompanies.Contains(Ship->GetCompany()))
        {
            // Not in war
            continue;
        }

        ShipToSimulate.Add(Ship);
    }

    // Play fighting ship inthem in random order

    while(ShipToSimulate.Num())
    {
        int32 Index = FMath::RandRange(0, ShipToSimulate.Num() - 1);
        if(SimulateShipTurn(ShipToSimulate[Index]))
        {
            HasFight = true;
        }
        ShipToSimulate.RemoveAt(Index);
    }

    return HasFight;
}

bool UFlareBattle::SimulateShipTurn(UFlareSimulatedSpacecraft* Ship)
{
    if(Ship->GetSize() == EFlarePartSize::S)
    {
        return SimulateSmallShipTurn(Ship);
    }
    else if(Ship->GetSize() == EFlarePartSize::L)
    {
        return SimulateLargeShipTurn(Ship);
    }

    return false;
}

bool UFlareBattle::SimulateSmallShipTurn(UFlareSimulatedSpacecraft* Ship)
{
    //  - Find a target
    //  - Find a weapon
    //  - Apply damage


	UFlareSimulatedSpacecraft* Target = NULL;

    struct BattleTargetPreferences TargetPreferences;
    TargetPreferences.IsLarge = 1;
    TargetPreferences.IsSmall = 1;
    TargetPreferences.IsStation = 1;
    TargetPreferences.IsNotStation = 1;
    TargetPreferences.IsMilitary = 1;
    TargetPreferences.IsNotMilitary = 0.1;
    TargetPreferences.IsDangerous = 1;
    TargetPreferences.IsNotDangerous = 0.01;
    TargetPreferences.IsStranded = 1;
    TargetPreferences.IsNotStranded = 0.5;
	TargetPreferences.IsUncontrollableCivil = 0.0;
	TargetPreferences.IsUncontrollableMilitary = 0.01;
	TargetPreferences.IsNotUncontrollable = 1;
    TargetPreferences.IsHarpooned = 0;
    TargetPreferences.TargetStateWeight = 1;

	Ship->GetWeaponsSystem()->GetTargetPreference(&TargetPreferences.IsSmall, &TargetPreferences.IsLarge, &TargetPreferences.IsUncontrollableCivil, &TargetPreferences.IsUncontrollableMilitary, &TargetPreferences.IsNotUncontrollable, &TargetPreferences.IsStation, &TargetPreferences.IsHarpooned);

	Target = GetBestTarget(Ship, TargetPreferences);

	if (!Target)
    {
		return false;
	}

	// Find best weapon
	int32 WeaponGroupIndex = Ship->GetWeaponsSystem()->FindBestWeaponGroup(Target);

	if(WeaponGroupIndex == -1)
	{
		return false;
	}

	FLOGV("%s want to attack %s with %s",
		  *Ship->GetImmatriculation().ToString(),
		  *Target->GetImmatriculation().ToString(),
		  *Ship->GetWeaponsSystem()->GetWeaponGroup(WeaponGroupIndex)->Description->Identifier.ToString())


	return SimulateShipAttack(Ship, WeaponGroupIndex, Target);
}

bool UFlareBattle::SimulateLargeShipTurn(UFlareSimulatedSpacecraft* Ship)
{
	bool HasAttacked = false;

	// Fire each turret individualy
	for (int32 ComponentIndex = 0; ComponentIndex < Ship->GetData().Components.Num(); ComponentIndex++)
	{
		FFlareSpacecraftComponentSave* ComponentData = &Ship->GetData().Components[ComponentIndex];

		FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

		if(ComponentDescription->Type != EFlarePartType::Weapon || !ComponentDescription->WeaponCharacteristics.TurretCharacteristics.IsTurret)
		{
			// Ignore if not a turret
			continue;
		}

		if(Ship->GetDamageSystem()->GetUsableRatio(ComponentDescription, ComponentData) <= 0)
		{
			// Not usable
			continue;
		}


		UFlareSimulatedSpacecraft* Target = NULL;


		struct BattleTargetPreferences TargetPreferences;
		TargetPreferences.IsLarge = 1;
		TargetPreferences.IsSmall = 1;
		TargetPreferences.IsStation = 1;
		TargetPreferences.IsNotStation = 1;
		TargetPreferences.IsMilitary = 1;
		TargetPreferences.IsNotMilitary = 0.1;
		TargetPreferences.IsDangerous = 1;
		TargetPreferences.IsNotDangerous = 0.01;
		TargetPreferences.IsStranded = 1;
		TargetPreferences.IsNotStranded = 0.5;
		TargetPreferences.IsUncontrollableCivil = 0.0;
		TargetPreferences.IsUncontrollableMilitary = 0.01;
		TargetPreferences.IsNotUncontrollable = 1;
		TargetPreferences.IsHarpooned = 0;
		TargetPreferences.TargetStateWeight = 1;

		if (ComponentDescription->WeaponCharacteristics.DamageType == EFlareShellDamageType::HEAT)
		{
			TargetPreferences.IsLarge = 1.0f;
			TargetPreferences.IsSmall = 0.0f;
		}
		else
		{
			TargetPreferences.IsLarge = 0.0f;
			TargetPreferences.IsSmall = 1.0f;
		}


		Target = GetBestTarget(Ship, TargetPreferences);

		if (!Target)
		{
			return false;
		}

		FLOGV("%s want to attack %s with %s",
			  *Ship->GetImmatriculation().ToString(),
			  *Target->GetImmatriculation().ToString(),
			  *ComponentData->ShipSlotIdentifier.ToString())


		if (SimulateShipWeaponAttack(Ship, ComponentDescription, ComponentData, Target))
		{
			HasAttacked = true;
		}
	}
	return HasAttacked;
}

UFlareSimulatedSpacecraft* UFlareBattle::GetBestTarget(UFlareSimulatedSpacecraft* Ship, struct BattleTargetPreferences Preferences)
{
	UFlareSimulatedSpacecraft* BestTarget = NULL;
	float BestScore = 0;

	//FLOGV("GetBestTarget for %s", *Ship->GetImmatriculation().ToString());

	for (int32 SpacecraftIndex = 0 ; SpacecraftIndex < Sector->GetSectorSpacecrafts().Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* ShipCandidate = Sector->GetSectorSpacecrafts()[SpacecraftIndex];


		if (Ship->GetCompany()->GetWarState(ShipCandidate->GetCompany()) != EFlareHostility::Hostile)
		{
			// Ignore not hostile ships
			continue;
		}

		if (!ShipCandidate->GetDamageSystem()->IsAlive())
		{
			// Ignore destroyed ships
			continue;
		}

		float Score;
		float StateScore;
		float DistanceScore;

		StateScore = Preferences.TargetStateWeight;

		if (ShipCandidate->GetSize() == EFlarePartSize::L)
		{
			StateScore *= Preferences.IsLarge;
		}

		if (ShipCandidate->GetSize() == EFlarePartSize::S)
		{
			StateScore *= Preferences.IsSmall;
		}

		if (ShipCandidate->IsStation())
		{
			StateScore *= Preferences.IsStation;
		}
		else
		{
			StateScore *= Preferences.IsNotStation;
		}

		if (ShipCandidate->IsMilitary())
		{
			StateScore *= Preferences.IsMilitary;
		}
		else
		{
			StateScore *= Preferences.IsNotMilitary;
		}

		if(ShipCandidate->IsMilitary()  && !ShipCandidate->GetDamageSystem()->IsDisarmed())
		{
			StateScore *= Preferences.IsDangerous;
		}
		else
		{
			StateScore *= Preferences.IsNotDangerous;
		}

		if (ShipCandidate->GetDamageSystem()->IsStranded())
		{
			StateScore *= Preferences.IsStranded;
		}
		else
		{
			StateScore *= Preferences.IsNotStranded;
		}

		if (ShipCandidate->GetDamageSystem()->IsUncontrollable())
		{
			if(ShipCandidate->IsMilitary())
			{
				StateScore *= Preferences.IsUncontrollableMilitary;
			}
			else
			{
				StateScore *= Preferences.IsUncontrollableCivil;
			}
		}
		else
		{
			StateScore *= Preferences.IsNotUncontrollable;
		}

		if(ShipCandidate->IsHarpooned()) {
			if(ShipCandidate->GetDamageSystem()->IsUncontrollable())
			{
				// Never target harponned uncontrollable ships
				continue;
			}
			StateScore *=  Preferences.IsHarpooned;
		}

		DistanceScore = FMath::FRand();

		Score = StateScore * (DistanceScore);

		if (Score > 0)
		{
			if (BestTarget == NULL || Score > BestScore)
			{
				BestTarget = ShipCandidate;
				BestScore = Score;
			}
		}
	}

	return BestTarget;
}


bool UFlareBattle::SimulateShipAttack(UFlareSimulatedSpacecraft* Ship, int32 WeaponGroupIndex, UFlareSimulatedSpacecraft* Target)
{
	FFlareSimulatedWeaponGroup* WeaponGroup = Ship->GetWeaponsSystem()->GetWeaponGroup(WeaponGroupIndex);

	bool HasAttacked = false;

	// TODO configure Fire probability
	float FireProbability = 0.8f;
	if(FMath::FRand() < FireProbability)
	{
		// Fire with all weapon
		for (int32 WeaponIndex = 0; WeaponIndex <  WeaponGroup->Weapons.Num(); WeaponIndex++)
		{
			if(Ship->GetDamageSystem()->GetUsableRatio(WeaponGroup->Description, WeaponGroup->Weapons[WeaponIndex]) <= 0)
			{
				continue;
			}

			if (SimulateShipWeaponAttack(Ship, WeaponGroup->Description, WeaponGroup->Weapons[WeaponIndex], Target))
			{
				HasAttacked = true;
			}
		}
	}
	else
	{
		// Not really fire but not because its impossible
		HasAttacked = true;
	}

	return HasAttacked;
}

bool UFlareBattle::SimulateShipWeaponAttack(UFlareSimulatedSpacecraft* Ship, FFlareSpacecraftComponentDescription* WeaponDescription, FFlareSpacecraftComponentSave* Weapon, UFlareSimulatedSpacecraft* Target)
{
	float UsageRatio = Ship->GetDamageSystem()->GetUsableRatio(WeaponDescription, Weapon);
	int32 MaxAmmo = WeaponDescription->WeaponCharacteristics.AmmoCapacity;
	int32 CurrentAmmo = MaxAmmo - Weapon->Weapon.FiredAmmo;



	if(WeaponDescription->WeaponCharacteristics.GunCharacteristics.IsGun)
	{
		// Fire 5 s of ammo with a hit probability of 10% + precision * usage ratio
		float FiringPeriod = 1.f / (WeaponDescription->WeaponCharacteristics.GunCharacteristics.AmmoRate / 60.f);
		float DamageDelay = FMath::Square(1.f- UsageRatio) * 10 * FiringPeriod * FMath::FRandRange(0.f, 1.f);
		float Delay = DamageDelay + FiringPeriod;


		int32 AmmoToFire = FMath::Max(1, (int32) (5.f * (1.f/Delay)));

		AmmoToFire = FMath::Min(CurrentAmmo, AmmoToFire);

		float TargetCoef = 1.1;

		if(Target->GetSize() == EFlarePartSize::S)
		{
			TargetCoef *= 50;
		}

		if(Target->GetDamageSystem()->IsStranded())
		{
			TargetCoef /= 2;
		}

		if(Target->GetDamageSystem()->IsUncontrollable())
		{
			TargetCoef /= 10;
		}

		if(WeaponDescription->WeaponCharacteristics.FuzeType == EFlareShellFuzeType::Proximity)
		{
			TargetCoef /= 100;
		}

		float Precision = UsageRatio * FMath::Max(0.01f, 1.f-(WeaponDescription->WeaponCharacteristics.GunCharacteristics.AmmoPrecision * TargetCoef));

		FLOGV("Fire %d ammo with a hit probability of %f", AmmoToFire, Precision);
		for (int32 BulletIndex = 0; BulletIndex <  AmmoToFire; BulletIndex++)
		{
			if(FMath::FRand() < Precision)
			{
				// Apply bullet damage
				SimulateBulletDamage(WeaponDescription, Target, Ship->GetCompany());
			}
		}

		Weapon->Weapon.FiredAmmo += AmmoToFire;
		Target->GetDamageSystem()->SetAmmoDirty();
	}
	else if(WeaponDescription->WeaponCharacteristics.BombCharacteristics.IsBomb && CurrentAmmo > 0)
	{
		// Drop one bomb with a hit probabiliy of (1 + usable ratio + isUncontrollable)/3

		if (FMath::FRand() < (1+UsageRatio+(Target->GetDamageSystem()->IsUncontrollable() ? 1.f:0.f)))
		{
			// Apply bullet damage
			SimulateBombDamage(WeaponDescription, Target, Ship->GetCompany());
		}

		Weapon->Weapon.FiredAmmo++;
		Target->GetDamageSystem()->SetAmmoDirty();
	}
	else
	{
		FLOGV("Not supported weapon %s", *WeaponDescription->Identifier.ToString());
		return false;
	}

	return true;
}

void UFlareBattle::SimulateBulletDamage(FFlareSpacecraftComponentDescription* WeaponDescription, UFlareSimulatedSpacecraft* Target, UFlareCompany* DamageSource)
{
	if(WeaponDescription->WeaponCharacteristics.DamageType == EFlareShellDamageType::ArmorPiercing)
	{
		ApplyDamage(Target, WeaponDescription->WeaponCharacteristics.GunCharacteristics.KineticEnergy, EFlareDamage::DAM_ArmorPiercing, DamageSource);
	}
	else if(WeaponDescription->WeaponCharacteristics.DamageType == EFlareShellDamageType::HEAT)
	{
		ApplyDamage(Target, WeaponDescription->WeaponCharacteristics.ExplosionPower, EFlareDamage::DAM_HEAT, DamageSource);
	}
	else if(WeaponDescription->WeaponCharacteristics.DamageType == EFlareShellDamageType::HighExplosive)
	{
		// Generate fragments
		float FragmentHitRatio = FMath::FRandRange(0.01f, 0.1f);
		int32 FragmentCount = WeaponDescription->WeaponCharacteristics.AmmoFragmentCount * FragmentHitRatio;


		for(int FragmentIndex = 0; FragmentIndex < FragmentCount; FragmentIndex++)
		{
			float FragmentPowerEffet = FMath::FRandRange(0.f, 2.f);
			ApplyDamage(Target, FragmentPowerEffet * WeaponDescription->WeaponCharacteristics.ExplosionPower, EFlareDamage::DAM_HighExplosive, DamageSource);
		}
	}
}

void UFlareBattle::SimulateBombDamage(FFlareSpacecraftComponentDescription* WeaponDescription, UFlareSimulatedSpacecraft* Target, UFlareCompany* DamageSource)
{
	// Apply damage
	ApplyDamage(Target, WeaponDescription->WeaponCharacteristics.ExplosionPower,
		SpacecraftHelper::GetWeaponDamageType(WeaponDescription->WeaponCharacteristics.DamageType),
		DamageSource);

	// Ship salvage
	if (!Target->IsStation() &&
		((WeaponDescription->WeaponCharacteristics.DamageType == EFlareShellDamageType::LightSalvage && Target->GetDescription()->Size == EFlarePartSize::S)
	 || (WeaponDescription->WeaponCharacteristics.DamageType == EFlareShellDamageType::HeavySalvage && Target->GetDescription()->Size == EFlarePartSize::L)))
	{
		FLOGV("UFlareBattle::SimulateBombDamage : salvaging %s for %s", *Target->GetImmatriculation().ToString(), *DamageSource->GetCompanyName().ToString());
		Target->SetHarpooned(DamageSource);
	}
}

void UFlareBattle::ApplyDamage(UFlareSimulatedSpacecraft* Target, float Energy, EFlareDamage::Type DamageType, UFlareCompany* DamageSource)
{

	// Find a component and apply damages

	int32 ComponentIndex;
	if(DamageType == EFlareDamage::DAM_HighExplosive)
	{
		ComponentIndex = FMath::RandRange(0,  Target->GetData().Components.Num()-1);
	}
	else
	{
		ComponentIndex = GetBestTargetComponent(Target);
	}


	FFlareSpacecraftComponentSave* TargetComponent = &Target->GetData().Components[ComponentIndex];

	FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(TargetComponent->ComponentIdentifier);

	CombatLog::SpacecraftDamaged(Target, Energy, 0, FVector::ZeroVector, DamageType, DamageSource);
	float DamageRatio = Target->GetDamageSystem()->ApplyDamage(ComponentDescription, TargetComponent, Energy, DamageType, DamageSource);
}


int32 UFlareBattle::GetBestTargetComponent(UFlareSimulatedSpacecraft* TargetSpacecraft)
{
	// Is armed, target the gun
	// Else if not stranger target the orbital
	// else target the rsc

	float WeaponWeight = 1;
	float PodWeight = 1;
	float RCSWeight = 1;
	float HeatSinkWeight = 1;

	if (!TargetSpacecraft->GetDamageSystem()->IsDisarmed())
	{
		WeaponWeight = 20;
		PodWeight = 8;
		RCSWeight = 1;
		HeatSinkWeight = 1;
	}
	else if (!TargetSpacecraft->GetDamageSystem()->IsStranded())
	{
		PodWeight = 8;
		RCSWeight = 1;
		HeatSinkWeight = 1;
	}
	else
	{
		RCSWeight = 1;
		HeatSinkWeight = 1;
	}

	TArray<int32> ComponentSelection;

	for (int32 ComponentIndex = 0; ComponentIndex < TargetSpacecraft->GetData().Components.Num(); ComponentIndex++)
	{
		FFlareSpacecraftComponentSave* TargetComponent = &TargetSpacecraft->GetData().Components[ComponentIndex];

		FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(TargetComponent->ComponentIdentifier);

		float UsageRatio = TargetSpacecraft->GetDamageSystem()->GetUsableRatio(ComponentDescription, TargetComponent);
		float DamageRatio = TargetSpacecraft->GetDamageSystem()->GetDamageRatio(ComponentDescription, TargetComponent);


		if (ComponentDescription && UsageRatio > 0)
		{
			if (ComponentDescription->Type == EFlarePartType::RCS)
			{
				for (int32 i = 0; i < RCSWeight; i++)
				{
					ComponentSelection.Add(ComponentIndex);
				}
			}

			if (ComponentDescription->Type == EFlarePartType::OrbitalEngine)
			{
				for (int32 i = 0; i < PodWeight; i++)
				{
					ComponentSelection.Add(ComponentIndex);
				}
			}

			if (ComponentDescription->Type == EFlarePartType::Weapon)
			{
				for (int32 i = 0; i < WeaponWeight; i++)
				{
					ComponentSelection.Add(ComponentIndex);
				}
			}

			if (ComponentDescription->Type == EFlarePartType::InternalComponent)
			{
				for (int32 i = 0; i < HeatSinkWeight; i++)
				{
					ComponentSelection.Add(ComponentIndex);
				}
			}
		}
		else if (ComponentDescription && DamageRatio > 0)
		{
			ComponentSelection.Add(ComponentIndex);
		}
	}

	if(ComponentSelection.Num() == 0)
	{
		return 0;
	}

	int32 ComponentIndex = FMath::RandRange(0, ComponentSelection.Num() - 1);
	return ComponentSelection[ComponentIndex];
}


#undef LOCTEXT_NAMESPACE
