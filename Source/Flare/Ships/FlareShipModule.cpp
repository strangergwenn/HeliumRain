;
#include "../Flare.h"
#include "FlareShip.h"
#include "FlareShipModule.h"
#include "../Player/FlarePlayerController.h"

#include "StaticMeshResources.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareShipModule::UFlareShipModule(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, Ship(NULL)
	, PlayerCompany(NULL)
	, ModuleMaterial(NULL)
	, ModuleDescription(NULL)
{
	PrimaryComponentTick.bCanEverTick = true;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareShipModule::OnRegister()
{
	Super::OnRegister();

	Activate(true);
	SetCollisionProfileName("BlockAllDynamic");
}

void UFlareShipModule::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UFlareShipModule::Initialize(const FFlareShipModuleDescription* Description, UFlareCompany* Company, AFlareShipBase* OwnerShip, bool IsInMenu)
{
	// Main data
	Ship = OwnerShip;
	PlayerCompany = Company;

	// Setup properties
	if (Description)
	{
		ModuleDescription = Description;
		for (int32 i = 0; i < Description->Characteristics.Num(); i++)
		{
			const FFlarePartCharacteristic& Characteristic = Description->Characteristics[i];
			switch (Characteristic.CharacteristicType)
			{
				case EFlarePartCharacteristicType::Armor:
					// TODO
					break;
			}
		}
	}

	// Mesh and material setup
	if (!IsInMenu)
	{
		SetupEffectMesh();
	}
	SetupModuleMesh();
	UpdateCustomization();
}

float UFlareShipModule::GetMeshScale()
{
	FVector Extent = GetCollisionShape().GetExtent();
	return FMath::Max(Extent.Size(), 1.0f);
}

bool UFlareShipModule::IsInitialized()
{
	return (Ship != NULL);
}

void UFlareShipModule::SetTemperature(int32 TemperatureKelvin)
{
	if (ModuleMaterial)
	{
		ModuleMaterial->SetScalarParameterValue("Temperature", TemperatureKelvin);
	}
}


/*----------------------------------------------------
	Customization
----------------------------------------------------*/

void UFlareShipModule::SetupModuleMesh()
{
	// Set the mesh
	if (ModuleDescription)
	{
		SetStaticMesh(ModuleDescription->Mesh);
		SetMaterial(0, ModuleDescription->Mesh->GetMaterial(0));
	}

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
				ModuleMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, GetWorld());
			}

			// Apply generated materials at each LOD
			if (ModuleMaterial)
			{
				SetMaterial(Element.MaterialIndex, ModuleMaterial);
			}
		}
	}
}

void UFlareShipModule::SetupEffectMesh()
{
	// Remove the previous effect mesh is available
	if (EffectMesh)
	{
		EffectMesh->DestroyComponent();
	}

	// Add and register the effect mesh if available
	if (Ship && ModuleDescription && ModuleDescription->EffectMesh)
	{
		EffectMesh = ConstructObject<UStaticMeshComponent>(UStaticMeshComponent::StaticClass(), Ship);
		EffectMesh->SetStaticMesh(ModuleDescription->EffectMesh);

		// Place and register
		EffectMesh->SetWorldLocation(GetComponentLocation());
		EffectMesh->SetWorldRotation(GetComponentRotation());
		EffectMesh->SetWorldScale3D(GetComponentScale());
		EffectMesh->RegisterComponentWithWorld(GetWorld());
		EffectMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		EffectMesh->AttachTo(this, NAME_None, EAttachLocation::KeepWorldPosition);

		// Generate a MID
		UMaterialInterface* BaseMaterial = EffectMesh->GetMaterial(0);
		if (BaseMaterial)
		{
			EffectMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, GetWorld());
			EffectMesh->SetMaterial(0, EffectMaterial);
		}
	}
}

void UFlareShipModule::TickModule(float DeltaTime)
{
	// Do nothing
}

void UFlareShipModule::UpdateCustomization()
{
	if (PlayerCompany)
	{
		if (ModuleMaterial)
		{
			PlayerCompany->CustomizeModuleMaterial(ModuleMaterial);
		}
		if (EffectMaterial)
		{
			PlayerCompany->CustomizeEffectMaterial(EffectMaterial);
		}
	}
}
