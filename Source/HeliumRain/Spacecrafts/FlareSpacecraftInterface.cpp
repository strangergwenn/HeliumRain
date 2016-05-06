#include "../Flare.h"
#include "FlareSpacecraftInterface.h"
#include "FlareSpacecraftComponent.h"
#include "../Game/FlareGame.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftInterface::UFlareSpacecraftInterface(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}


/*----------------------------------------------------
	Content
----------------------------------------------------*/

const FSlateBrush* IFlareSpacecraftInterface::GetIcon(FFlareSpacecraftDescription* Characteristic)
{
	if (Characteristic)
	{
		if (IFlareSpacecraftInterface::IsStation(Characteristic))
		{
			return FFlareStyleSet::GetIcon("SS");
		}
		else if (IFlareSpacecraftInterface::IsMilitary(Characteristic))
		{
			if (Characteristic->Size == EFlarePartSize::S)
			{
				return FFlareStyleSet::GetIcon("MS");
			}
			else if (Characteristic->Size == EFlarePartSize::M)
			{
				return FFlareStyleSet::GetIcon("MM");
			}
			else if (Characteristic->Size == EFlarePartSize::L)
			{
				return FFlareStyleSet::GetIcon("ML");
			}
		}
		else
		{
			if (Characteristic->Size == EFlarePartSize::S)
			{
				return FFlareStyleSet::GetIcon("CS");
			}
			else if (Characteristic->Size == EFlarePartSize::M)
			{
				return FFlareStyleSet::GetIcon("CM");
			}
			else if (Characteristic->Size == EFlarePartSize::L)
			{
				return FFlareStyleSet::GetIcon("CL");
			}
		}
	}
	return NULL;
}

bool IFlareSpacecraftInterface::IsStation(FFlareSpacecraftDescription* SpacecraftDesc)
{
	return SpacecraftDesc->OrbitalEngineCount == 0;
}

bool IFlareSpacecraftInterface::IsMilitary(FFlareSpacecraftDescription* SpacecraftDesc)
{
	return SpacecraftDesc->GunSlots.Num() > 0 || SpacecraftDesc->TurretSlots.Num() > 0;
}

EFlareResourcePriceContext::Type IFlareSpacecraftInterface::GetResourceUseType(FFlareResourceDescription* Resource)
{
	// Check we're and station
	if (!IsStation())
	{
		return EFlareResourcePriceContext::Default;
	}
	FFlareSpacecraftDescription* SpacecraftDescription = GetDescription();

	// Parse factories
	for (int FactoryIndex = 0; FactoryIndex < SpacecraftDescription->Factories.Num(); FactoryIndex++)
	{
		FFlareFactoryDescription* FactoryDescription = &SpacecraftDescription->Factories[FactoryIndex]->Data;

		// Is input resource of a station ?
		for (int32 ResourceIndex = 0; ResourceIndex < FactoryDescription->CycleCost.InputResources.Num(); ResourceIndex++)
		{
			const FFlareFactoryResource* FactoryResource = &FactoryDescription->CycleCost.InputResources[ResourceIndex];
			if (&FactoryResource->Resource->Data == Resource)
			{
				return EFlareResourcePriceContext::FactoryInput;
			}
		}

		// Is output resource of a station ?
		for (int32 ResourceIndex = 0; ResourceIndex < FactoryDescription->CycleCost.OutputResources.Num(); ResourceIndex++)
		{
			const FFlareFactoryResource* FactoryResource = &FactoryDescription->CycleCost.OutputResources[ResourceIndex];
			if (&FactoryResource->Resource->Data == Resource)
			{
				return EFlareResourcePriceContext::FactoryOutput;
			}
		}
	}

	// Customer resource ?
	if (SpacecraftDescription->Capabilities.Contains(EFlareSpacecraftCapability::Consumer) && GetGame()->GetResourceCatalog()->IsCustomerResource(Resource))
	{
		return EFlareResourcePriceContext::ConsumerConsumption;
	}

	// Maintenance resource ?
	if (SpacecraftDescription->Capabilities.Contains(EFlareSpacecraftCapability::Maintenance) && GetGame()->GetResourceCatalog()->IsMaintenanceResource(Resource))
	{
		return EFlareResourcePriceContext::MaintenanceConsumption;
	}

	return EFlareResourcePriceContext::Default;
}
