
#include "../Flare.h"

#include "FlareSpacecraftDamageSystem.h"
#include "FlareSpacecraft.h"

#define LOCTEXT_NAMESPACE "FlareSpacecraftDamageSystem"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftDamageSystem::UFlareSpacecraftDamageSystem(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, Spacecraft(NULL)
{
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void UFlareSpacecraftDamageSystem::TickSystem(float DeltaSeconds)
{
	// Apply heat variation : add producted heat then substract radiated heat.

	// Get the to heat production and heat sink surface
	float HeatProduction = 0.f;
	float HeatSinkSurface = 0.f;

	for (int32 i = 0; i < Components.Num(); i++)
	{
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[i]);
		HeatProduction += Component->GetHeatProduction();
		HeatSinkSurface += Component->GetHeatSinkSurface();
	}

	// Add a part of sun radiation to ship heat production
	// Sun flow is 3.094KW/m^2 and keep only half.
	HeatProduction += HeatSinkSurface * 3.094 * 0.5;

	// Heat up
	Data->Heat += HeatProduction * DeltaSeconds;
	// Radiate: Stefan-Boltzmann constant=5.670373e-8
	float Temperature = Data->Heat / Description->HeatCapacity;
	float HeatRadiation = 0.f;
	if (Temperature > 0)
	{
		HeatRadiation = HeatSinkSurface * 5.670373e-8 * FMath::Pow(Temperature, 4) / 1000;
	}
	// Don't radiate too much energy : negative temperature is not possible
	Data->Heat -= FMath::Min(HeatRadiation * DeltaSeconds, Data->Heat);

	// Overheat after 800°K, compute heat damage from temperature beyond 800°K : 0.005%/(°K*s)
	float OverheatDamage = (Temperature - GetOverheatTemperature()) * DeltaSeconds * 0.00005;
	float BurningDamage = FMath::Max((Temperature - GetBurnTemperature()) * DeltaSeconds * 0.0001, 0.0);

	// Update component temperature and apply heat damage
	for (int32 i = 0; i < Components.Num(); i++)
	{
		// Apply temperature
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[i]);

		// Overheat apply damage is necessary
		if (OverheatDamage > 0)
		{
			Component->ApplyHeatDamage(Component->GetTotalHitPoints() * OverheatDamage, Component->GetTotalHitPoints() * BurningDamage);
		}
	}

	// If damage have been applied, power production may have change
	if (OverheatDamage > 0)
	{
		UpdatePower();
	}

	// Power outage
	if (Data->PowerOutageDelay > 0)
	{
		Data->PowerOutageDelay -=  DeltaSeconds;
		if (Data->PowerOutageDelay <=0)
		{
			Data->PowerOutageDelay = 0;
			UpdatePower(); // To update light
		}
	}

	// Update Alive status
	if (WasAlive && !IsAlive())
	{
		WasAlive = false;
		OnControlLost();
	}
}

void UFlareSpacecraftDamageSystem::Initialize(AFlareSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData)
{
	Spacecraft = OwnerSpacecraft;
	Components = Spacecraft->GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	Description = Spacecraft->GetDescription();
	Data = OwnerData;

}

void UFlareSpacecraftDamageSystem::Start()
{
	// Init alive status
	WasAlive = IsAlive();

	TArray<UFlareSpacecraftComponent*> PowerSources;
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);
		// Fill power sources
		if (Component->IsGenerator())
		{
			PowerSources.Add(Component);
		}
	}

	// Second pass, update component power sources and update power
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);
		Component->UpdatePowerSources(&PowerSources);
	}
	UpdatePower();
}


void UFlareSpacecraftDamageSystem::UpdatePower()
{
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);
		Component->UpdatePower();
	}
}

void UFlareSpacecraftDamageSystem::OnControlLost()
{
	AFlarePlayerController* PC = Spacecraft->GetPC();
	if (PC)
	{
		PC->Notify(LOCTEXT("ShipDestroyed", "Your ship has been destroyed !"), EFlareNotification::NT_Military, EFlareMenu::MENU_Company);
	}
}

void UFlareSpacecraftDamageSystem::OnCollision(class AActor* Other, FVector HitLocation, FVector HitNormal)
{
	// If receive hit from over actor, like a ship we must apply collision damages.
	// The applied damage energy is 0.2% of the kinetic energy of the other actor. The kinetic
	// energy is calculated from the relative speed between the 2 actors, and only with the relative
	// speed projected in the axis of the collision normal: if 2 very fast ship only slightly touch,
	// only few energy will be decipated by the impact.
	//
	// The damages are applied only to the current actor, the ReceiveHit method of the other actor
	// will also call an it will apply its collision damages itself.

	// If the other actor is a projectile, specific weapon damage code is done in the projectile hit
	// handler: in this case we ignore the collision
	AFlareShell* OtherProjectile = Cast<AFlareShell>(Other);
	if (OtherProjectile) {
		return;
	}

	UPrimitiveComponent* OtherRoot = Cast<UPrimitiveComponent>(Other->GetRootComponent());

	// Relative velocity
	FVector DeltaVelocity = ((OtherRoot->GetPhysicsLinearVelocity() - Spacecraft->Airframe->GetPhysicsLinearVelocity()) / 100);

	// Compute the relative velocity in the impact axis then compute kinetic energy
	float ImpactSpeed = FVector::DotProduct(DeltaVelocity, HitNormal);
	float ImpactMass = OtherRoot->GetMass();
	float ImpactEnergy = 0.5 * ImpactMass * FMath::Square(ImpactSpeed) * 0.001; // In KJ

	// Convert only 0.2% of the energy as damages and make vary de damage radius with the damage:
	// minimum 20cm and about 1 meter for 20KJ of damage (about the damages provide by a bullet)
	float Energy = ImpactEnergy / 500;
	float  Radius = 0.2 + FMath::Sqrt(Energy) * 0.22;

	ApplyDamage(Energy, Radius, HitLocation);
}

void UFlareSpacecraftDamageSystem::ApplyDamage(float Energy, float Radius, FVector Location)
{
	// The damages are applied to all component touching the sphere defined by the radius and the
	// location in parameter.
	// The maximum damage are applied to a component only if its bounding sphere touch the center of
	// the damage sphere. There is a linear decrease of damage with a minumum of 0 if the 2 sphere
	// only touch.

	//FLOGV("Apply %f damages to %s with radius %f at %s", Energy, *GetHumanReadableName(), Radius, *Location.ToString());
	//DrawDebugSphere(GetWorld(), Location, Radius * 100, 12, FColor::Red, true);

	bool IsAliveBeforeDamage = IsAlive();

	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);

		float ComponentSize;
		FVector ComponentLocation;
		Component->GetBoundingSphere(ComponentLocation, ComponentSize);

		float Distance = (ComponentLocation - Location).Size() / 100.0f;
		float IntersectDistance =  Radius + ComponentSize/100 - Distance;

		// Hit this component
		if (IntersectDistance > 0)
		{
			//FLOGV("Component %s. ComponentSize=%f, Distance=%f, IntersectDistance=%f", *(Component->GetReadableName()), ComponentSize, Distance, IntersectDistance);
			float Efficiency = FMath::Clamp(IntersectDistance / Radius , 0.0f, 1.0f);
			Component->ApplyDamage(Energy * Efficiency);
		}
	}

	// Update power
	UpdatePower();

	// Heat the ship
	Data->Heat += Energy;
}

void UFlareSpacecraftDamageSystem::OnElectricDamage(float DamageRatio)
{
	float MaxPower = 0.f;
	float AvailablePower = 0.f;

	float Total = 0.f;
	float GeneratorCount = 0;
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);
		MaxPower += Component->GetMaxGeneratedPower();
		AvailablePower += Component->GetGeneratedPower();
	}

	float PowerRatio = AvailablePower/MaxPower;


	//FLOGV("OnElectricDamage initial PowerOutageDelay=%f, DamageRatio=%f, PowerRatio=%f", ShipData.PowerOutageDelay, DamageRatio, PowerRatio);

	// The outage probability depend on global available power ratio
	if (FMath::FRand() > PowerRatio)
	{
		// The outage duration depend on the relative amount of damage the component just receive
		// This avoid very long outage if multiple small collision.
		// Between 10 and 20s of outage if component one shot
		Data->PowerOutageDelay += DamageRatio *  FMath::FRandRange(10, 20 * (1.f - PowerRatio));
		UpdatePower();
	}
}

float UFlareSpacecraftDamageSystem::GetSubsystemHealth(EFlareSubsystem::Type Type, bool WithArmor) const
{
	float Health = 0.f;

	switch(Type)
	{
		case EFlareSubsystem::SYS_Propulsion:
		{
			TArray<UActorComponent*> Engines = Spacecraft->GetComponentsByClass(UFlareEngine::StaticClass());
			float Total = 0.f;
			float EngineCount = 0;
			for (int32 ComponentIndex = 0; ComponentIndex < Engines.Num(); ComponentIndex++)
			{
				UFlareEngine* Engine = Cast<UFlareEngine>(Engines[ComponentIndex]);
				if (Engine->IsA(UFlareOrbitalEngine::StaticClass()))
				{
					EngineCount+=1.f;
					Total+=Engine->GetDamageRatio(WithArmor)*(Engine->IsPowered() ? 1 : 0);
				}
			}
			Health = Total/EngineCount;
		}
		break;
		case EFlareSubsystem::SYS_RCS:
		{
			TArray<UActorComponent*> Engines = Spacecraft->GetComponentsByClass(UFlareEngine::StaticClass());
			float Total = 0.f;
			float EngineCount = 0;
			for (int32 ComponentIndex = 0; ComponentIndex < Engines.Num(); ComponentIndex++)
			{
				UFlareEngine* Engine = Cast<UFlareEngine>(Engines[ComponentIndex]);
				if (!Engine->IsA(UFlareOrbitalEngine::StaticClass()))
				{
					EngineCount+=1.f;
					Total+=Engine->GetDamageRatio(WithArmor)*(Engine->IsPowered() ? 1 : 0);
				}
			}
			Health = Total/EngineCount;
		}
		break;
		case EFlareSubsystem::SYS_LifeSupport:
		{
			if (Spacecraft->GetCockpit())
			{
				Health = Spacecraft->GetCockpit()->GetDamageRatio(WithArmor) * (Spacecraft->GetCockpit()->IsPowered() ? 1 : 0);
			}
			else
			{
				// No cockpit mean no destructible
				Health = 1.0f;
			}
		}
		break;
		case EFlareSubsystem::SYS_Power:
		{
			float Total = 0.f;
			float GeneratorCount = 0;
			for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
			{
				UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);
				if (Component->IsGenerator())
				{
					GeneratorCount+=1.f;
					Total+=Component->GetDamageRatio(WithArmor);
				}
			}
			Health = Total/GeneratorCount;
		}
		break;
		case EFlareSubsystem::SYS_Weapon:
		{
			TArray<UActorComponent*> Weapons = Spacecraft->GetComponentsByClass(UFlareWeapon::StaticClass());
			float Total = 0.f;
			for (int32 ComponentIndex = 0; ComponentIndex < Weapons.Num(); ComponentIndex++)
			{
				UFlareWeapon* Weapon = Cast<UFlareWeapon>(Weapons[ComponentIndex]);
				Total += Weapon->GetDamageRatio(WithArmor)*(Weapon->IsPowered() ? 1 : 0)*(Weapon->GetCurrentAmmo() > 0 ? 1 : 0);
			}
			Health = Total/Weapons.Num();
		}
		break;
		case EFlareSubsystem::SYS_Temperature:
		{
			float Total = 0.f;
			float HeatSinkCount = 0;
			for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
			{
				UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);
				if (Component->IsHeatSink())
				{
					HeatSinkCount+=1.f;
					Total+=Component->GetDamageRatio(WithArmor) * (Spacecraft->GetCockpit() && Spacecraft->GetCockpit()->IsPowered() ? 1 : 0);
				}
			}
			Health = Total/HeatSinkCount;
		}
		break;
	}

	return Health;
}

float UFlareSpacecraftDamageSystem::GetTemperature() const
{
	return Data->Heat / Description->HeatCapacity;
}


bool UFlareSpacecraftDamageSystem::IsAlive() const
{
	return GetSubsystemHealth(EFlareSubsystem::SYS_LifeSupport) > 0;
}

bool UFlareSpacecraftDamageSystem::IsPowered() const
{
	if (Spacecraft->GetCockpit())
	{
		return Spacecraft->GetCockpit()->IsPowered();
	}
	else
	{
		return false;
	}
}

bool UFlareSpacecraftDamageSystem::HasPowerOutage() const
{
	return GetPowerOutageDuration() > 0.f;
}

float UFlareSpacecraftDamageSystem::GetPowerOutageDuration() const
{
	return Data->PowerOutageDelay;
}

#undef LOCTEXT_NAMESPACE
