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
{
	PrimaryComponentTick.bCanEverTick = true;
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
}

void UFlareShipComponent::Initialize(const FFlareShipComponentSave* Data, UFlareCompany* Company, AFlareShipBase* OwnerShip, bool IsInMenu)
{
	// Main data
	Ship = OwnerShip;
	PlayerCompany = Company;

	// Setup properties
	if (Data)
	{
		ComponentDescription = OwnerShip->GetGame()->GetShipPartsCatalog()->Get(Data->ComponentIdentifier);
	}

	// Mesh and material setup
	if (!IsInMenu)
	{
		SetupEffectMesh();
	}
	SetupComponentMesh();
	UpdateCustomization();
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


/*----------------------------------------------------
	Customization
----------------------------------------------------*/

void UFlareShipComponent::SetupComponentMesh()
{
	// Set the mesh
	if (ComponentDescription)
	{
		SetStaticMesh(ComponentDescription->Mesh);
		SetMaterial(0, ComponentDescription->Mesh->GetMaterial(0));
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

		EffectMesh = ConstructObject<UStaticMeshComponent>(UStaticMeshComponent::StaticClass(), Ship);
		EffectMesh->SetStaticMesh(ComponentDescription->EffectMesh);

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

void UFlareShipComponent::ShipTickComponent(float DeltaTime)
{
	// Do nothing
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
