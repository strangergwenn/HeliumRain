
#include "FlareSpacecraftComponent.h"
#include "../Flare.h"

#include "FlareSpacecraft.h"
#include "FlareInternalComponent.h"

#include "../Data/FlareCustomizationCatalog.h"
#include "../Data/FlareSpacecraftComponentsCatalog.h"

#include "../Game/FlareGame.h"

#include "../Player/FlareMenuPawn.h"
#include "../Player/FlarePlayerController.h"

#include "FlareOrbitalEngine.h"

#include "StaticMeshResources.h"
#include "PhysicsEngine/BodySetup.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftComponent::UFlareSpacecraftComponent(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, SpacecraftPawn(NULL)
	, Spacecraft(NULL)
	, PlayerCompany(NULL)
	, ComponentMaterial(NULL)
	, EffectMaterial(NULL)
	, LightMaterial(NULL)
	, BillboardMaterial(NULL)
	, ComponentDescription(NULL)
	, LocalHeatEffect(false)
	, LocalTemperature(0)
	, LightFlickeringStatus(EFlareLightStatus::Lit)
	, TimeLeftUntilFlicker(0)
	, TimeLeftInFlicker(0)
	, FlickerMaxOnPeriod(1)
	, FlickerMaxOffPeriod(3)
	, DestroyedEffects(NULL)
	, ImpactCount(0)
	, MaxImpactCount(3)
	, ImpactEffectChance(0.1)
{
	// Fire effects
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ImpactEffectTemplateSObj(TEXT("/Game/Master/Particles/DamageEffects/PS_Fire.PS_Fire"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ImpactEffectTemplateLObj(TEXT("/Game/Master/Particles/DamageEffects/PS_Fire_L.PS_Fire_L"));
	ImpactEffectTemplateS = ImpactEffectTemplateSObj.Object;
	ImpactEffectTemplateL = ImpactEffectTemplateLObj.Object;

	// Physics setup
	PrimaryComponentTick.bCanEverTick = true;
	SetNotifyRigidBodyCollision(true);
	SetGenerateOverlapEvents(false);
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
}

void UFlareSpacecraftComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Graphical updates
	if (ComponentMaterial && SpacecraftPawn)
	{
		// Update the light status
		if (HasFlickeringLights && IsComponentVisible())
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
	}

	// Need even if no ComponentDescription to heat airframes
	if (Spacecraft)
	{
		if (LocalHeatEffect && HeatProduction > 0.f)
		{
			float Alpha = GetHeatProduction() / HeatProduction;
			float TargetTemperature = (1.f- Alpha) * (Spacecraft->GetParent()->GetDamageSystem()->GetTemperature() * 0.3f)
						+ Alpha * (Spacecraft->GetParent()->GetDamageSystem()->GetTemperature() * 1.8f);
			float HalfLife = 3;
			float Variation = DeltaTime / HalfLife;
			LocalTemperature = (LocalTemperature + (TargetTemperature * Variation)) / (1+Variation);
		}
		else
		{
			LocalTemperature = Spacecraft->GetParent()->GetDamageSystem()->GetTemperature();
		}

		SetTemperature(Spacecraft->IsPresentationMode() ? 290 : LocalTemperature);
		SetHealth(     Spacecraft->IsPresentationMode() ? 1 :   FMath::Clamp(1 + (GetDamageRatio()-1) / (1 -BROKEN_RATIO) , 0.f , 1.f));
	}
}

void UFlareSpacecraftComponent::Initialize(FFlareSpacecraftComponentSave* Data, UFlareCompany* Company, AFlareSpacecraftPawn* OwnerSpacecraftPawn, bool IsInMenu)
{
	// Main data
	SpacecraftPawn = OwnerSpacecraftPawn;
	PlayerCompany = Company;

	Spacecraft = Cast<AFlareSpacecraft>(SpacecraftPawn);
	if (Spacecraft)
	{
		LocalTemperature = Spacecraft->GetParent()->GetDamageSystem()->GetTemperature();
	}

	// Setup properties
	if (Data)
	{
		ShipComponentData = Data;
		ComponentDescription = OwnerSpacecraftPawn->GetGame()->GetShipPartsCatalog()->Get(Data->ComponentIdentifier);

		if (!ComponentDescription)
		{
			FLOGV("UFlareSpacecraftComponent::Initialize : bad component identifier : '%s' in '%s'",
				*Data->ComponentIdentifier.ToString(),
				*Spacecraft->GetDescription()->Name.ToString());
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
	UpdateLight();
}

FFlareSpacecraftComponentSave* UFlareSpacecraftComponent::Save()
{
	if (ComponentDescription)
	{
		return ShipComponentData;
	}
	else
	{
		return NULL;
	}
}

float UFlareSpacecraftComponent::GetMeshScale()
{
	FVector Extent = GetBodySetup()->AggGeom.CalcAABB(FTransform()).GetExtent();
	return FMath::Max(Extent.Size(), 1.0f);
}

bool UFlareSpacecraftComponent::IsInitialized()
{
	return (SpacecraftPawn != NULL);
}

void UFlareSpacecraftComponent::SetVisibleInUpgrade(bool Visible)
{
	SetVisibility(Visible, true);
}

void UFlareSpacecraftComponent::SetTemperature(int32 TemperatureKelvin)
{
	if (ComponentMaterial && TemperatureKelvin != PreviousTemperatureKelvin && IsComponentVisible())
	{
		ComponentMaterial->SetScalarParameterValue("Temperature", TemperatureKelvin);
		PreviousTemperatureKelvin = TemperatureKelvin;
	}
}

void UFlareSpacecraftComponent::SetHealth(float HealthRatio)
{
	if (ComponentMaterial && HealthRatio != PreviousHealthRatio && IsComponentVisible())
	{
		ComponentMaterial->SetScalarParameterValue("Health", HealthRatio);
		PreviousHealthRatio = HealthRatio;
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

	Radius = (Max - LocalBoxCenter).GetMax();
	Location = GetComponentToWorld().TransformPosition(LocalBoxCenter);
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
		SetMaterial(1, Mesh->GetMaterial(1));
	}
	else if (ComponentDescription && !Mesh)
	{
		// In case of a turret we must hide the root component
		// as the turret will spawn 2 sub mobile components.
		SetVisibility(false, false);
	}

	if (GetStaticMesh())
	{
		// Base material
		int ComponentMaterialIndex = GetMaterialIndex("Base");
		if (ComponentMaterialIndex >= 0)
		{
			UMaterialInterface* BaseMaterial = GetMaterial(ComponentMaterialIndex);
			if (BaseMaterial->IsA(UMaterialInstanceDynamic::StaticClass()))
			{
				ComponentMaterial = Cast<UMaterialInstanceDynamic>(BaseMaterial);
			}
			else
			{
				ComponentMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, GetWorld());
			}
			SetMaterialByName("Base", ComponentMaterial);
		}

		// Exhaust material
		int EffectMaterialIndex = GetMaterialIndex("Exhaust");
		if (EffectMaterialIndex >= 0)
		{
			UMaterialInterface* BaseMaterial = GetMaterial(EffectMaterialIndex);
			if (BaseMaterial->IsA(UMaterialInstanceDynamic::StaticClass()))
			{
				EffectMaterial = Cast<UMaterialInstanceDynamic>(BaseMaterial);
			}
			else
			{
				EffectMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, GetWorld());
			}
			SetMaterialByName("Exhaust", EffectMaterial);
		}

		// Additional light material
		int LightMaterialIndex = GetMaterialIndex("Light");
		if (LightMaterialIndex >= 0)
		{
			UMaterialInterface* BaseMaterial = GetMaterial(LightMaterialIndex);
			if (BaseMaterial->IsA(UMaterialInstanceDynamic::StaticClass()))
			{
				LightMaterial = Cast<UMaterialInstanceDynamic>(BaseMaterial);
			}
			else
			{
				LightMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, GetWorld());
			}
			SetMaterialByName("Light", LightMaterial);
			LightMaterial->SetScalarParameterValue("Random", FMath::RandRange(0.0f, 1.0f));
		}

		// Additional billboard material
		int BillboardMaterialIndex = GetMaterialIndex("Billboard");
		if (BillboardMaterialIndex >= 0)
		{
			UMaterialInterface* BaseMaterial = GetMaterial(BillboardMaterialIndex);
			if (BaseMaterial->IsA(UMaterialInstanceDynamic::StaticClass()))
			{
				BillboardMaterial = Cast<UMaterialInstanceDynamic>(BaseMaterial);
			}
			else
			{
				BillboardMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, GetWorld());
			}
			SetMaterialByName("Billboard", BillboardMaterial);
		}
	}
}

void UFlareSpacecraftComponent::UpdateCustomization()
{
	if (Spacecraft && Spacecraft->IsStation() && Spacecraft->GetParent()->IsUnderConstruction(true) && Spacecraft->GetParent()->GetLevel() == 1)
	{
		return;
	}

	AFlareGame* Game = SpacecraftPawn->GetGame();
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(Game->GetWorld()->GetFirstPlayerController());

	// Regular paint
	if (ComponentMaterial)
	{
		if (PlayerCompany)
		{
			PlayerCompany->CustomizeMaterial(ComponentMaterial);
		}
		else
		{
			const FFlareCompanyDescription* CurrentCompanyData = PC->GetCompanyDescription();

			if (CurrentCompanyData)
			{
				CustomizeMaterial(ComponentMaterial, Game,
					CurrentCompanyData->CustomizationBasePaintColor,
					CurrentCompanyData->CustomizationPaintColor,
					CurrentCompanyData->CustomizationOverlayColor,
					CurrentCompanyData->CustomizationLightColor,
					CurrentCompanyData->CustomizationPatternIndex,
					CurrentCompanyData->Emblem);
			}
		}
	}

	// Lighting
	if (LightMaterial)
	{
		LightMaterial->SetScalarParameterValue("IsPainted", 1);
	}

	// Billboards
	if (BillboardMaterial)
	{		
		// Player
		if (PlayerCompany->IsPlayerCompany())
		{
			BillboardMaterial->SetTextureParameterValue("Texture", PC->GetPlayerBanner());
		}

		// Regular company
		else
		{
			UTexture2D* Billboard = NULL;
			const TArray<UTexture2D*> CompanyBillboards = Spacecraft->GetParent()->GetCompany()->GetDescription()->Billboards;
			const TArray<UTexture2D*> GameBillboards = Spacecraft->GetGame()->GetCustomizationCatalog()->Billboards;

			if (CompanyBillboards.Num())
			{
				Billboard = CompanyBillboards[FMath::RandRange(0, CompanyBillboards.Num() - 1)];
			}
			else if (GameBillboards.Num())
			{
				Billboard = GameBillboards[FMath::RandRange(0, GameBillboards.Num() - 1)];
			}

			if (Billboard)
			{
				BillboardMaterial->SetTextureParameterValue("Texture", Billboard);
			}
		}
	}	
}

void UFlareSpacecraftComponent::CustomizeMaterial(UMaterialInstanceDynamic* Mat, AFlareGame* Game, FLinearColor BasePaint, FLinearColor Paint, FLinearColor Overlay, FLinearColor Light, int32 Pattern, UTexture2D* Emblem)
{
	// Get data from storage
	UTexture2D* PatternTexture = Game->GetCustomizationCatalog()->GetPattern(Pattern);

	// Apply settings to the material instance
	Mat->SetVectorParameterValue("BasePaintColor", BasePaint);
	Mat->SetVectorParameterValue("PaintColor", Paint);
	Mat->SetVectorParameterValue("OverlayColor", Overlay);
	Mat->SetVectorParameterValue("LightColor", Light);
	Mat->SetVectorParameterValue("GlowColor", NormalizeColor(Light));
	Mat->SetTextureParameterValue("PaintPattern", PatternTexture);
	Mat->SetTextureParameterValue("Emblem", Emblem);
	Mat->SetScalarParameterValue("IsPainted", 1);
}


/*----------------------------------------------------
	Damages
----------------------------------------------------*/

float UFlareSpacecraftComponent::GetArmor()
{
	return UFlareSimulatedSpacecraftDamageSystem::GetArmor(ComponentDescription);
}

float UFlareSpacecraftComponent::GetArmorAtLocation(FVector Location)
{
	if (!ComponentDescription)
	{
		if (Spacecraft)
		{
			UFlareInternalComponent* Component = Spacecraft->GetInternalComponentAtLocation(Location);
			if (Component == this)
			{
				FLOGV("!!! GetArmorAtLocation loop ! %s may not be correctly bind to its description", *Component->GetReadableName());
				return 1;
			}

			if (Component)
			{
				return Component->GetArmorAtLocation(Location);
			}
		}
	}
	else
	{
		return GetArmor();
	}

	return 1;
}

float UFlareSpacecraftComponent::ApplyDamage(float Energy, EFlareDamage::Type DamageType, UFlareSimulatedSpacecraft* DamageSource)
{
	float InflictedDamageRatio = 0;
	if (ComponentDescription)
	{

		InflictedDamageRatio = Spacecraft->GetParent()->GetDamageSystem()->ApplyDamage(ComponentDescription,
																					   ShipComponentData, Energy, DamageType, DamageSource);

		if (InflictedDamageRatio > 0)
		{
			// No more armor, power outage risk
			if (Spacecraft && IsGenerator())
			{
				Spacecraft->GetDamageSystem()->OnElectricDamage(InflictedDamageRatio);
			}

			// Effects
			if (GetDamageRatio() == 0)
			{
				StartDestroyedEffects();
			}
			UpdateLight();
		}
	}
	return InflictedDamageRatio;
}

float UFlareSpacecraftComponent::GetDamageRatio() const
{
	if (ComponentDescription && Spacecraft && Spacecraft->GetParent())
	{
		float MaxHitPoints = Spacecraft->GetParent()->GetDamageSystem()->GetMaxHitPoints(ComponentDescription);
		float RemainingHitPoints = MaxHitPoints - ShipComponentData->Damage;
		return FMath::Clamp(RemainingHitPoints / MaxHitPoints, 0.f, 1.f);
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
	if (!ComponentDescription || !Spacecraft || !Spacecraft->GetParent())
	{
		return true;
	}
	else
	{
		return Spacecraft->GetParent()->GetDamageSystem()->IsPowered(ShipComponentData);
	}
}

float UFlareSpacecraftComponent::GetGeneratedPower() const
{
	return GeneratedPower*GetUsableRatio();
}

float UFlareSpacecraftComponent::GetMaxGeneratedPower() const
{
	return GeneratedPower;
}

bool UFlareSpacecraftComponent::IsGenerator() const
{
	return GeneratedPower > 0;
}

void UFlareSpacecraftComponent::UpdateLight()
{
	float AvailablePower = GetUsableRatio();
	if (AvailablePower <= 0)
	{
		SetLightStatus(EFlareLightStatus::Dark);
	}
	else if (AvailablePower < 0.75 || (Spacecraft && Spacecraft->GetParent()->GetDamageSystem()->HasPowerOutage()))
	{
		SetLightStatus(EFlareLightStatus::Flickering);
	}
	else
	{
		SetLightStatus(EFlareLightStatus::Lit);
	}
}

bool UFlareSpacecraftComponent::IsBroken() const
{
	return (GetDamageRatio() * (IsPowered() ? 1 : 0)) < BROKEN_RATIO;
}

float UFlareSpacecraftComponent::GetUsableRatio() const
{
	return (IsBroken() ? 0 : 1) * (GetDamageRatio() * (IsPowered() ? 1 : 0)) * (Spacecraft && Spacecraft->GetParent()->GetDamageSystem()->HasPowerOutage() ? 0 : 1);
}

float UFlareSpacecraftComponent::GetHeatProduction() const
{
	return HeatProduction * (0.5 + 0.5 *(-FMath::Pow((GetDamageRatio()-1),2)+1));
}

float UFlareSpacecraftComponent::GetHeatSinkSurface() const
{
	return HeatSinkSurface * (0.01 +  0.99 * GetUsableRatio());
}

bool UFlareSpacecraftComponent::IsHeatSink() const
{
	return HeatSinkSurface > 0;
}

float UFlareSpacecraftComponent::GetTotalHitPoints() const
{
	if (ComponentDescription)
	{
		return Spacecraft->GetParent()->GetDamageSystem()->GetMaxHitPoints(ComponentDescription);
	}
	else
	{
		return -1.f;
	}
}

void UFlareSpacecraftComponent::OnRepaired()
{
	if (ComponentDescription)
	{
		UpdateLight();
		if (DestroyedEffects)
		{
			DestroyedEffects->Deactivate();
		}
	}
}

void UFlareSpacecraftComponent::StartDestroyedEffects()
{
	if (!DestroyedEffects && DestroyedEffectTemplate && IsDestroyedEffectRelevant())
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

void UFlareSpacecraftComponent::StartDamagedEffect(FVector Location, FRotator Rotation, EFlarePartSize::Type WeaponSize)
{
	EFlarePartSize::Type Size = EFlarePartSize::S;

	// Get size
	if (ComponentDescription)
	{
		Size = ComponentDescription->Size;
	}
	else if (Spacecraft)
	{
		Size = Spacecraft->GetDescription()->Size;
	}

	// Limiters
	if (ImpactCount >= MaxImpactCount)
	{
		return;
	}
	else if (FMath::FRand() > ImpactEffectChance)
	{
		return;
	}
	else if (WeaponSize < Size)
	{
		return;
	}

	// Spawn
	UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(
		(Size == EFlarePartSize::L) ? ImpactEffectTemplateL : ImpactEffectTemplateS,
		this,
		NAME_None,
		Location,
		Rotation + FRotator::MakeFromEuler(FVector(0, -90, 0)),
		EAttachLocation::KeepWorldPosition,
		true);

	if (PSC)
	{
		ImpactCount++;
		PSC->SetWorldScale3D(FVector(1, 1, 1));
	}
}

bool UFlareSpacecraftComponent::IsDestroyedEffectRelevant()
{
	// No smoke
	return false;
}

bool UFlareSpacecraftComponent::IsComponentVisible() const
{
	// Optimization to relieve the uniform shader cache
	// Show if the last render is under 200ms old
	// Show if it belongs to the player (to avoid issue when going external)
	if (Spacecraft)
	{
		return (Spacecraft->IsPlayerShip() || (GetWorld()->TimeSeconds - LastRenderTime) < 0.2);
	}
	else
	{
		return true;
	}
}

FLinearColor UFlareSpacecraftComponent::NormalizeColor(FLinearColor Col)
{
	if (Col.GetLuminance() < KINDA_SMALL_NUMBER)
	{
		return FLinearColor::White;
	}
	else
	{
		return FLinearColor(FVector(Col.R, Col.G, Col.B) / Col.GetLuminance());
	}
}
