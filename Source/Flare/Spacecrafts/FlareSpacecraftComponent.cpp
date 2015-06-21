;
#include "../Flare.h"
#include "FlareSpacecraft.h"
#include "FlareSpacecraftComponent.h"
#include "../Player/FlareMenuPawn.h"
#include "../Player/FlarePlayerController.h"
#include "FlareOrbitalEngine.h"

#include "StaticMeshResources.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftComponent::UFlareSpacecraftComponent(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, SpacecraftPawn(NULL)
	, Spacecraft(NULL)
	, PlayerCompany(NULL)
	, DestroyedEffects(NULL)
	, ComponentMaterial(NULL)
	, ComponentDescription(NULL)
	, HasLocalHeatEffect(false)
	, LocalTemperature(0)
	, LightFlickeringStatus(EFlareLightStatus::Lit)
	, TimeLeftUntilFlicker(0)
	, TimeLeftInFlicker(0)
	, FlickerMaxOnPeriod(1)
	, FlickerMaxOffPeriod(3)
{
	// Physics setup
	PrimaryComponentTick.bCanEverTick = true;
	SetNotifyRigidBodyCollision(true);
	bGenerateOverlapEvents = false;
	bCanEverAffectNavigation = false;
	bTraceComplexOnMove = false;

	// Lighting settins
	bAffectDynamicIndirectLighting = false;
	bAffectDistanceFieldLighting = false;
	HasFlickeringLights = true;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareSpacecraftComponent::OnRegister()
{
	Super::OnRegister();

	Activate(true);
	SetCollisionProfileName("BlockAllDynamic");
}

void UFlareSpacecraftComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Visibility
	if (LDMaxDrawDistance)
	{
		AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
		if (PC)
		{
			FVector ViewLocation;
			FRotator ViewRotation;
			PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
			SetVisibility((ViewLocation - GetComponentLocation()).Size() < LDMaxDrawDistance);
		}
	}

	// Graphical updates
	if (ComponentMaterial && IsVisibleByPlayer())
	{
		// Update the light status
		if (HasFlickeringLights)
		{
			float GlowAlpha = 0;

			switch (LightFlickeringStatus)
			{
				// Flickering light
				case EFlareLightStatus::Flickering:
				{
					if (TimeLeftUntilFlicker > 0)
					{
						TimeLeftUntilFlicker -= DeltaTime;
						GlowAlpha = 0;
					}
					else
					{
						if (TimeLeftInFlicker > 0)
						{
							TimeLeftInFlicker -= DeltaTime;
							if (TimeLeftInFlicker > CurrentFlickerMaxPeriod / 2)
							{
								GlowAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, 2 * (TimeLeftInFlicker / CurrentFlickerMaxPeriod), 2);
							}
							else
							{
								GlowAlpha = FMath::InterpEaseInOut(1.0f, 0.0f, 2 * (TimeLeftInFlicker / CurrentFlickerMaxPeriod) - 1, 2);
							}
						}
						else
						{
							TimeLeftInFlicker = FMath::FRandRange(0, FlickerMaxOnPeriod);
							TimeLeftUntilFlicker = FMath::FRandRange(0, FlickerMaxOffPeriod);
							CurrentFlickerMaxPeriod = TimeLeftInFlicker;
							GlowAlpha = 0;
						}
					}
				}
				break;

				// Fully dark
				case EFlareLightStatus::Dark:
					GlowAlpha = 0;
					break;

				// Fully lit
				default:
				case EFlareLightStatus::Lit:
					GlowAlpha = 1;
					break;
			}
			ComponentMaterial->SetScalarParameterValue("GlowAlpha", GlowAlpha);
		}

		// Update health
		SetHealth(GetDamageRatio());
	}

	// Need even if no ComponentDescription to heat airframes
	if (Spacecraft)
	{
		if (HasLocalHeatEffect && HeatProduction > 0.f)
		{
			float Alpha = GetHeatProduction() / HeatProduction;
			float TargetTemperature = (1.f- Alpha) * (Spacecraft->GetDamageSystem()->GetTemperature() * 0.3f)
						+ Alpha * (Spacecraft->GetDamageSystem()->GetTemperature() * 1.8f);
			float HalfLife = 3;
			float Variation = DeltaTime / HalfLife;
			LocalTemperature = (LocalTemperature + (TargetTemperature * Variation)) / (1+Variation);
		}
		else
		{

			LocalTemperature = Spacecraft->GetDamageSystem()->GetTemperature();
		}
		SetTemperature(Spacecraft->IsPresentationMode() ? 290 : LocalTemperature);

	}
}

void UFlareSpacecraftComponent::Initialize(const FFlareSpacecraftComponentSave* Data, UFlareCompany* Company, AFlareSpacecraftPawn* OwnerSpacecraftPawn, bool IsInMenu)
{
	// Main data
	SpacecraftPawn = OwnerSpacecraftPawn;
	PlayerCompany = Company;

	Spacecraft = Cast<AFlareSpacecraft>(SpacecraftPawn);
	if (Spacecraft)
	{
		LocalTemperature = Spacecraft->GetDamageSystem()->GetTemperature();
	}

	// Setup properties
	if (Data)
	{
		ShipComponentData = *Data;
		ComponentDescription = OwnerSpacecraftPawn->GetGame()->GetShipPartsCatalog()->Get(Data->ComponentIdentifier);

		if (!ComponentDescription)
		{
			FLOGV("!!! Bad Component Identifier : %s", *Data->ComponentIdentifier.ToString());
		}
		else
		{
			LifeSupport = ComponentDescription->GeneralCharacteristics.LifeSupport;
			GeneratedPower = (ComponentDescription->GeneralCharacteristics.ElectricSystem ? 1.0 : 0.0);
			HeatProduction = ComponentDescription->GeneralCharacteristics.HeatProduction;
			HeatSinkSurface = ComponentDescription->GeneralCharacteristics.HeatSink;

			DestroyedEffectTemplate = ComponentDescription->DestroyedEffect;
		}

		// Destroyed component
		if (Spacecraft && GetDamageRatio() <= 0 && !Spacecraft->IsPresentationMode())
		{
			StartDestroyedEffects();
		}
	}

	// Mesh and material setup
	SetupComponentMesh();
	UpdateCustomization();
}

FFlareSpacecraftComponentSave* UFlareSpacecraftComponent::Save()
{
	if (ComponentDescription)
	{
		return &ShipComponentData;
	}
	else
	{
		return NULL;
	}
}

float UFlareSpacecraftComponent::GetMeshScale()
{
	FVector Extent = GetCollisionShape().GetExtent();
	return FMath::Max(Extent.Size(), 1.0f);
}

bool UFlareSpacecraftComponent::IsInitialized()
{
	return (SpacecraftPawn != NULL);
}

void UFlareSpacecraftComponent::SetTemperature(int32 TemperatureKelvin)
{
	if (ComponentMaterial)
	{
		ComponentMaterial->SetScalarParameterValue("Temperature", TemperatureKelvin);
	}
}

void UFlareSpacecraftComponent::SetHealth(float HealthRatio)
{
	if (ComponentMaterial)
	{
		ComponentMaterial->SetScalarParameterValue("Health", HealthRatio);
	}
}

void UFlareSpacecraftComponent::SetLightStatus(EFlareLightStatus::Type Status)
{
	LightFlickeringStatus = Status;
}

void UFlareSpacecraftComponent::GetBoundingSphere(FVector& Location, float& Radius)
{
	FVector Min;
	FVector Max;
	GetLocalBounds(Min,Max);

	FVector LocalBoxCenter = (Max + Min) /2;

	Radius = (Max - LocalBoxCenter).Size();
	Location = GetComponentToWorld().TransformPosition(LocalBoxCenter);
}

bool UFlareSpacecraftComponent::IsVisibleByPlayer()
{
	if (bVisible)
	{
		return (GetWorld()->TimeSeconds - LastRenderTime < 0.2f);
	}
	else
	{
		return false;
	}
}


/*----------------------------------------------------
	Customization
----------------------------------------------------*/

void UFlareSpacecraftComponent::SetupComponentMesh()
{
	UStaticMesh* Mesh = GetMesh(!(Spacecraft));

	// Set the mesh
	if (ComponentDescription && Mesh)
	{
		SetStaticMesh(Mesh);
		SetMaterial(0, Mesh->GetMaterial(0));
	}
	else if (ComponentDescription && !Mesh)
	{
		bVisible = false;
	}

	if (StaticMesh)
	{
		// Parse all LODs levels, then all elements
		for (int32 LODIndex = 0; LODIndex < StaticMesh->RenderData->LODResources.Num(); LODIndex++)
		{
			FStaticMeshLODResources& LOD = StaticMesh->RenderData->LODResources[LODIndex];
			for (int32 ElementIndex = 0; ElementIndex < LOD.Sections.Num(); ElementIndex++)
			{
				// Get base material from LOD element
				const FStaticMeshSection& Element = LOD.Sections[ElementIndex];
				UMaterialInterface* BaseMaterial = GetMaterial(Element.MaterialIndex);

				// Base material
				if (ElementIndex == 0)
				{
					// Generate MIDs from LOD 0 only, apply generated materials at each LOD
					if (LODIndex == 0 && BaseMaterial && !BaseMaterial->IsA(UMaterialInstanceDynamic::StaticClass()))
					{
						ComponentMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, GetWorld());
					}
					if (ComponentMaterial)
					{
						SetMaterial(Element.MaterialIndex, ComponentMaterial);
					}
				}

				// Effect material
				else if (ElementIndex == 1)
				{
					// Generate MIDs from LOD 0 only, apply generated materials at each LOD
					if (LODIndex == 0 && BaseMaterial && !BaseMaterial->IsA(UMaterialInstanceDynamic::StaticClass()))
					{
						EffectMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, GetWorld());
					}
					if (EffectMaterial)
					{
						SetMaterial(Element.MaterialIndex, EffectMaterial);
					}
				}
			}
		}
	}
}

void UFlareSpacecraftComponent::UpdateCustomization()
{
	if (PlayerCompany)
	{
		if (ComponentMaterial)
		{
			PlayerCompany->CustomizeComponentMaterial(ComponentMaterial);
		}
		if (EffectMaterial)
		{
			PlayerCompany->CustomizeEffectMaterial(EffectMaterial);
		}
	}
}


/*----------------------------------------------------
	Damages
----------------------------------------------------*/

float UFlareSpacecraftComponent::GetRemainingArmorAtLocation(FVector Location)
{
	if (!ComponentDescription)
	{
		if (Spacecraft)
		{
			UFlareInternalComponent* Component = Spacecraft->GetInternalComponentAtLocation(Location);
			if (Component == this)
			{
				FLOGV("!!! GetRemainingArmorAtLocation loop ! %s may not be correctly bind to its description", *Component->GetReadableName());
				return -1;
			}

			if (Component)
			{
				return Component->GetRemainingArmorAtLocation(Location);
			}
		}
	}
	else if (ComponentDescription->ArmorHitPoints != 0.0f || ComponentDescription->HitPoints != 0.0f)
	{
		return FMath::Max(0.0f, ComponentDescription->ArmorHitPoints - ShipComponentData.Damage);
	}

	// Not destructible
	return -1.0f;
}

void UFlareSpacecraftComponent::ApplyDamage(float Energy)
{
	if (ComponentDescription)
	{
		// Apply damage
		float StateBeforeDamage = GetDamageRatio();
		ShipComponentData.Damage += Energy;
		float StateAfterDamage = GetDamageRatio();

		//FLOGV("Component %s. Apply Energy=%f", *(GetReadableName()), Energy);

		// No more armor, power outage risk
		if (Spacecraft && IsGenerator() && StateAfterDamage < 1.0 && StateBeforeDamage > 0)
		{
			Spacecraft->GetDamageSystem()->OnElectricDamage(StateBeforeDamage - StateAfterDamage);
		}

		// Effects
		if (StateAfterDamage <= 0 && StateBeforeDamage > 0)
		{
			StartDestroyedEffects();
		}
		UpdateLight();
	}
}

void UFlareSpacecraftComponent::ApplyHeatDamage(float OverheatEnergy, float BurnEnergy)
{
	// Standard component have no overheat damage.
}

float UFlareSpacecraftComponent::GetDamageRatio(bool WithArmor) const
{
	if (ComponentDescription)
	{
		float RemainingHitPoints = ComponentDescription->ArmorHitPoints + ComponentDescription->HitPoints - ShipComponentData.Damage;
		return FMath::Clamp(RemainingHitPoints / (ComponentDescription->HitPoints + (WithArmor ? ComponentDescription->ArmorHitPoints : 0.f)), 0.f, 1.f);
	}
	else
	{
		return 1.f;
	}
}

bool UFlareSpacecraftComponent::IsDestroyed() const
{
	return (GetDamageRatio() <= 0);
}

bool UFlareSpacecraftComponent::IsPowered() const
{
	return (GetAvailablePower()> 0);
}

float UFlareSpacecraftComponent::GetGeneratedPower() const
{
	return GeneratedPower*GetDamageRatio();
}

float UFlareSpacecraftComponent::GetMaxGeneratedPower() const
{
	return GeneratedPower;
}

float UFlareSpacecraftComponent::GetAvailablePower() const
{
	if (!ComponentDescription && Spacecraft)
	{
		UFlareSpacecraftComponent* Cockpit = Spacecraft->GetCockpit();

		if (Cockpit)
		{
			Cockpit->UpdatePower();
			return Cockpit->GetAvailablePower();
		}
	}

	return Power*GetDamageRatio();
}

bool UFlareSpacecraftComponent::IsGenerator() const
{
	return GeneratedPower > 0;
}

void UFlareSpacecraftComponent::UpdatePower()
{
	Power = 0;
	for (int32 i = 0; i < PowerSources.Num(); i++)
	{
		Power += PowerSources[i]->GetGeneratedPower();
	}

	UpdateLight();
}

void UFlareSpacecraftComponent::UpdateLight()
{
	float AvailablePower = GetAvailablePower();
	if (AvailablePower <= 0)
	{
		SetLightStatus(EFlareLightStatus::Dark);
	}
	else if (AvailablePower < 0.5 || (Spacecraft && Spacecraft->GetDamageSystem()->HasPowerOutage()))
	{
		SetLightStatus(EFlareLightStatus::Flickering);
	}
	else
	{
		SetLightStatus(EFlareLightStatus::Lit);
	}
}

void UFlareSpacecraftComponent::UpdatePowerSources(TArray<UFlareSpacecraftComponent*>* AvailablePowerSources)
{
	PowerSources.Empty();

	float MinDistance = 100000; // 1km max attach MinDistance
	float DoubleConnectionThesold = 100; // 1m
	FVector Location = GetComponentLocation();

	// First pass, find the closest distance
	for (int32 i = 0; i < AvailablePowerSources->Num(); i++)
	{
		UFlareSpacecraftComponent* PowerSource = (*AvailablePowerSources)[i];

		FVector OtherLocation;
		float OtherRadius;
		PowerSource->GetBoundingSphere(OtherLocation, OtherRadius);
		float Distance = (OtherLocation - Location).Size() - OtherRadius;

		if (Distance < MinDistance)
		{
			MinDistance = Distance;
		}
	}

	// Second pass, add all source in the distance thresold
	for (int32 i = 0; i < AvailablePowerSources->Num(); i++)
	{
		UFlareSpacecraftComponent* PowerSource = (*AvailablePowerSources)[i];
		FVector OtherLocation;
		float OtherRadius;
		PowerSource->GetBoundingSphere(OtherLocation, OtherRadius);
		float Distance = (OtherLocation - Location).Size() - OtherRadius;

		if (Distance < MinDistance + DoubleConnectionThesold)
		{
			PowerSources.Add(PowerSource);
			//FLOGV("Component %s powered by %s", *GetReadableName(), *PowerSource->GetReadableName());
		}
	}
}

float UFlareSpacecraftComponent::GetUsableRatio() const
{
	return GetDamageRatio() * (IsPowered() ? 1 : 0) * (Spacecraft && Spacecraft->GetDamageSystem()->HasPowerOutage() ? 0 : 1);
}

float UFlareSpacecraftComponent::GetHeatProduction() const
{
	return HeatProduction * (0.5 + 0.5 *(-FMath::Pow((GetDamageRatio()-1),2)+1));
}

float UFlareSpacecraftComponent::GetHeatSinkSurface() const
{
	return HeatSinkSurface * (0.1 +  9 * GetUsableRatio() / 10);
}

bool UFlareSpacecraftComponent::IsHeatSink() const
{
	return HeatSinkSurface > 0;
}

float UFlareSpacecraftComponent::GetTotalHitPoints() const
{
	if (ComponentDescription)
	{
		return ComponentDescription->ArmorHitPoints + ComponentDescription->HitPoints;
	}
	else
	{
		return -1.f;
	}
}

void UFlareSpacecraftComponent::Repair()
{
	if (ComponentDescription)
	{
		ShipComponentData.Damage = 0;
		UpdateLight();
		if (DestroyedEffects)
		{
			DestroyedEffects->Deactivate();
		}
	}
}

void UFlareSpacecraftComponent::StartDestroyedEffects()
{
	if (!DestroyedEffects && DestroyedEffectTemplate)
	{
		// Calculate smoke origin
		FVector Position = GetComponentLocation();
		if (DoesSocketExist(FName("Smoke")))
		{
			Position = GetSocketLocation(FName("Smoke"));
		}

		// Start smoke

		DestroyedEffects = UGameplayStatics::SpawnEmitterAttached(
			DestroyedEffectTemplate,
			this,
			NAME_None,
			Position,
			GetComponentRotation(),
			EAttachLocation::KeepWorldPosition,
			true);
	}
}
