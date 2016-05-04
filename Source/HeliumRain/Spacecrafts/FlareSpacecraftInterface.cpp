#include "../Flare.h"
#include "FlareSpacecraftInterface.h"
#include "FlareSpacecraftComponent.h"


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

bool IFlareSpacecraftInterface::IsInputResource(FFlareResourceDescription* Resource)
{
	// TODO FRED #271 : generalize to AFlareSpacecraft

	bool IsInput = false;
	UFlareSimulatedSpacecraft* SimulatedSpacecraft = Cast<UFlareSimulatedSpacecraft>(this);
	if (SimulatedSpacecraft)
	{
		TArray<UFlareFactory*> Factories = SimulatedSpacecraft->GetFactories();
		for (int32 FactoryIndex = 0; FactoryIndex < Factories.Num(); FactoryIndex++)
		{
			if (Factories[FactoryIndex]->HasInputResource(Resource))
			{
				IsInput = true;
				break;
			}
		}
	}

	return IsInput;
}

bool IFlareSpacecraftInterface::IsOutputResource(FFlareResourceDescription* Resource)
{
	// TODO FRED #271 : generalize to AFlareSpacecraft

	bool IsOutput = false;
	UFlareSimulatedSpacecraft* SimulatedSpacecraft = Cast<UFlareSimulatedSpacecraft>(this);
	if (SimulatedSpacecraft)
	{
		TArray<UFlareFactory*> Factories = SimulatedSpacecraft->GetFactories();
		for (int32 FactoryIndex = 0; FactoryIndex < Factories.Num(); FactoryIndex++)
		{
			if (Factories[FactoryIndex]->HasOutputResource(Resource))
			{
				IsOutput = true;
				break;
			}
		}
	}

	return IsOutput;
}