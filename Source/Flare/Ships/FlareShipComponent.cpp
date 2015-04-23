;
#include "../Flare.h"
#include "FlareShip.h"
#include "FlareShipComponent.h"
#include "../Player/FlarePlayerController.h"

#include "StaticMeshResources.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareShipComponent::UFlareShipComponent(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, Ship(NULL)
	, PlayerCompany(NULL)
	, ComponentMaterial(NULL)
	, ComponentDescription(NULL)
	, LightFlickeringStatus(EFlareLightStatus::Lit)
	, TimeLeftUntilFlicker(0)
	, TimeLeftInFlicker(0)
	, FlickerMaxOnPeriod(1)
	, FlickerMaxOffPeriod(3)
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentFlickerMaxPeriod = FlickerMaxOnPeriod;
	SetNotifyRigidBodyCollision(true);
	bGenerateOverlapEvents = false;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareShipComponent::OnRegister()
{
	Super::OnRegister();

	Activate(true);
	SetCollisionProfileName("BlockAllDynamic");
}

void UFlareShipComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update the light status
	if (ComponentMaterial && IsVisibleByPlayer())
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

	if (ComponentDescription)
	{
		SetHealth(GetDamageRatio());
	}
}

void UFlareShipComponent::Initialize(const FFlareShipComponentSave* Data, UFlareCompany* Company, AFlareShipBase* OwnerShip, bool IsInMenu)
{
	// Main data
	Ship = OwnerShip;
	PlayerCompany = Company;

	// Setup properties
	if (Data)
	{
		ShipComponentData = *Data;
		ComponentDescription = OwnerShip->GetGame()->GetShipPartsCatalog()->Get(Data->ComponentIdentifier);

		if(!ComponentDescription)
		{
			FLOGV("!!! Bad Component Identifier : %s", *Data->ComponentIdentifier.ToString());
		}
		else
		{

			for (int32 i = 0; i < ComponentDescription->Characteristics.Num(); i++)
			{
				const FFlareShipComponentCharacteristic& Characteristic = ComponentDescription->Characteristics[i];
				switch (Characteristic.CharacteristicType)
				{
					case EFlarePartCharacteristicType::LifeSupport:
						LifeSupport = Characteristic.CharacteristicValue;
					break;
					case EFlarePartCharacteristicType::ElectricSystem:
						GeneratedPower = Characteristic.CharacteristicValue;
					break;
					case EFlarePartCharacteristicType::HeatProduction:
						HeatProduction = Characteristic.CharacteristicValue;
					break;
					case EFlarePartCharacteristicType::HeatSink:
						HeatSinkSurface = Characteristic.CharacteristicValue;
					break;
				}
			}
		}
	}

	// Mesh and material setup
	if (!IsInMenu)
	{
		SetupEffectMesh();
	}
	SetupComponentMesh();
	UpdateCustomization();
}

FFlareShipComponentSave* UFlareShipComponent::Save()
{
	if(ComponentDescription)
	{
		return &ShipComponentData;
	}
	else
	{
		return NULL;
	}
}

float UFlareShipComponent::GetMeshScale()
{
	FVector Extent = GetCollisionShape().GetExtent();
	return FMath::Max(Extent.Size(), 1.0f);
}

bool UFlareShipComponent::IsInitialized()
{
	return (Ship != NULL);
}

void UFlareShipComponent::SetTemperature(int32 TemperatureKelvin)
{
	if (ComponentMaterial)
	{
		ComponentMaterial->SetScalarParameterValue("Temperature", TemperatureKelvin);
	}
}

void UFlareShipComponent::SetHealth(float HealthRatio)
{
	if (ComponentMaterial)
	{
		ComponentMaterial->SetScalarParameterValue("Health", HealthRatio);
	}
}

void UFlareShipComponent::SetLightStatus(EFlareLightStatus::Type Status)
{
	LightFlickeringStatus = Status;
}


void UFlareShipComponent::GetBoundingSphere(FVector& Location, float& Radius)
{
	FVector Min;
	FVector Max;
	GetLocalBounds(Min,Max);

	FVector LocalBoxCenter = (Max + Min) /2;

	Radius = (Max - LocalBoxCenter).Size();
	Location = GetComponentToWorld().TransformPosition(LocalBoxCenter);
}

bool UFlareShipComponent::IsVisibleByPlayer()
{
	return Ship && (GetWorld()->TimeSeconds - LastRenderTime < 0.2f);
}


/*----------------------------------------------------
	Customization
----------------------------------------------------*/

void UFlareShipComponent::SetupComponentMesh()
{
	// Set the mesh
	if (ComponentDescription && ComponentDescription->Mesh)
	{
		SetStaticMesh(ComponentDescription->Mesh);
		SetMaterial(0, ComponentDescription->Mesh->GetMaterial(0));
	}

	if(StaticMesh)
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

				// Generate MIDs from LOD 0 only
				if (LODIndex == 0 && BaseMaterial && !BaseMaterial->IsA(UMaterialInstanceDynamic::StaticClass()))
				{
					ComponentMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, GetWorld());
				}

				// Apply generated materials at each LOD
				if (ComponentMaterial)
				{
					SetMaterial(Element.MaterialIndex, ComponentMaterial);
				}
			}
		}
	}
}

void UFlareShipComponent::SetupEffectMesh()
{
	// Remove the previous effect mesh is available
	if (EffectMesh)
	{
		EffectMesh->DestroyComponent();
	}

	// Add and register the effect mesh if available
	if (Ship && ComponentDescription && ComponentDescription->EffectMesh)
	{
		// Create
		EffectMesh = ConstructObject<UStaticMeshComponent>(UStaticMeshComponent::StaticClass(), Ship);
		EffectMesh->SetStaticMesh(ComponentDescription->EffectMesh);

		// Place and register
		EffectMesh->SetWorldLocation(GetComponentLocation());
		EffectMesh->SetWorldRotation(GetComponentRotation());
		EffectMesh->SetWorldScale3D(GetComponentScale());
		EffectMesh->RegisterComponentWithWorld(GetWorld());
		EffectMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		EffectMesh->AttachTo(this, NAME_None, EAttachLocation::KeepWorldPosition);
		EffectMesh->LDMaxDrawDistance = 100000; // 1km

		// Generate a MID
		UMaterialInterface* BaseMaterial = EffectMesh->GetMaterial(0);
		if (BaseMaterial)
		{
			EffectMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, GetWorld());
			EffectMesh->SetMaterial(0, EffectMaterial);
		}
	}
}

void UFlareShipComponent::UpdateCustomization()
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

float UFlareShipComponent::GetRemainingArmorAtLocation(FVector Location)
{
	if (!ComponentDescription || (ComponentDescription->ArmorHitPoints == 0.0f && ComponentDescription->HitPoints == 0.0f))
	{
		// Not destructible
		return -1.0f;
	}
	else
	{
		return FMath::Max(0.0f, ComponentDescription->ArmorHitPoints - ShipComponentData.Damage);
	}
}

void UFlareShipComponent::ApplyDamage(float Energy)
{
	if (ComponentDescription)
	{
		float StateBeforeDamage = GetDamageRatio();

		ShipComponentData.Damage += Energy;
		//FLOGV("Apply %f damages to %s %s. Total damages: %f (%f|%f)", Energy, *GetReadableName(), *ShipComponentData.ShipSlotIdentifier.ToString(),  ShipComponentData.Damage, ComponentDescription->ArmorHitPoints, ComponentDescription->HitPoints);

		float StateAfterDamage = GetDamageRatio();

		// No power outage if
		if (IsGenerator() && StateAfterDamage < 1.0 && StateBeforeDamage > 0)
		{
			// No more armo, power outage risk
			Ship->OnElectricDamage(StateBeforeDamage - StateAfterDamage);
		}

		UpdateLight();

	}
}

void UFlareShipComponent::ApplyHeatDamage(float OverheatEnergy, float BurnEnergy)
{
	// Standard component have no overheat damage.
}


float UFlareShipComponent::GetDamageRatio(bool WithArmor) const
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

bool UFlareShipComponent::IsDestroyed() const
{
	return (GetDamageRatio() <= 0);
}

bool UFlareShipComponent::IsAlive() const
{
	return (LifeSupport * GetDamageRatio() > 0);
}

bool UFlareShipComponent::IsPowered() const
{
	return (GetAvailablePower()> 0);
}

float UFlareShipComponent::GetGeneratedPower() const
{
	return GeneratedPower*GetDamageRatio();
}

float UFlareShipComponent::GetMaxGeneratedPower() const
{
	return GeneratedPower;
}

float UFlareShipComponent::GetAvailablePower() const
{
	return Power*GetDamageRatio();
}

bool UFlareShipComponent::IsGenerator() const
{
	return GeneratedPower > 0;
}

void UFlareShipComponent::UpdatePower()
{

	Power = 0;
	for (int32 i = 0; i < PowerSources.Num(); i++)
	{
		Power += PowerSources[i]->GetGeneratedPower();
	}

	UpdateLight();
}

void UFlareShipComponent::UpdateLight()
{
	float AvailablePower = GetAvailablePower();
	if( AvailablePower <=0)
	{
		SetLightStatus(EFlareLightStatus::Dark);
	}
	else if (AvailablePower < 0.5)
	{
		SetLightStatus(EFlareLightStatus::Flickering);
	}
	else
	{
		SetLightStatus(EFlareLightStatus::Lit);
	}

}

void UFlareShipComponent::UpdatePowerSources(TArray<UFlareShipComponent*>* AvailablePowerSources)
{
	PowerSources.Empty();

	float MinDistance = 100000; // 1km max attach MinDistance
	float DoubleConnectionThesold = 100; // 1m
	FVector Location = GetComponentLocation();

	// First pass, find the closest distance
	for (int32 i = 0; i < AvailablePowerSources->Num(); i++)
	{
		UFlareShipComponent* PowerSource = (*AvailablePowerSources)[i];

		FVector OtherLocation;
		float OtherRadius;
		PowerSource->GetBoundingSphere(OtherLocation, OtherRadius);
		float Distance = (OtherLocation - Location).Size() - OtherRadius;

		if(Distance < MinDistance)
		{
			MinDistance = Distance;
		}
	}

	// Second pass, add all source in the distance thresold
	for (int32 i = 0; i < AvailablePowerSources->Num(); i++)
	{
		UFlareShipComponent* PowerSource = (*AvailablePowerSources)[i];
		FVector OtherLocation;
		float OtherRadius;
		PowerSource->GetBoundingSphere(OtherLocation, OtherRadius);
		float Distance = (OtherLocation - Location).Size() - OtherRadius;

		if(Distance < MinDistance + DoubleConnectionThesold)
		{
			PowerSources.Add(PowerSource);
			FLOGV("Component %s powered by %s", *GetReadableName(), *PowerSource->GetReadableName());
		}
	}
}


float UFlareShipComponent::GetHeatProduction() const
{
	return HeatProduction * (-FMath::Pow((GetDamageRatio()-1),2)+1);
}

float UFlareShipComponent::GetHeatSinkSurface() const
{
	return HeatSinkSurface * (0.25 +  3 * GetDamageRatio() * (IsPowered() ? 1 : 0) * (Ship->HasPowerOutage() ? 0 : 1) / 4);
}

bool UFlareShipComponent::IsHeatSink() const
{
	return HeatSinkSurface > 0;
}

float UFlareShipComponent::GetTotalHitPoints() const
{
	if(ComponentDescription)
	{
		return ComponentDescription->ArmorHitPoints + ComponentDescription->HitPoints;
	} else {
		return -1.f;
	}
}

void UFlareShipComponent::Repair()
{
	if (ComponentDescription)
	{
		ShipComponentData.Damage = 0;
		UpdateLight();
	}
}
