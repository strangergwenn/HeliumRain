
#include "../Flare.h"
#include "../Spacecrafts/FlareSpacecraftInterface.h"
#include "../Game/FlareGame.h"
#include "FlareCargoBay.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareCargoBay::UFlareCargoBay(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareCargoBay::Load(IFlareSpacecraftInterface* ParentSpacecraft, TArray<FFlareCargoSave>& Data)
{
	Parent = ParentSpacecraft;
	Game = Parent->GetGame();
	CargoBayCount = Parent->GetDescription()->CargoBayCount;
	CargoBayCapacity = Parent->GetDescription()->CargoBayCapacity;
	// Initialize cargo bay
	CargoBay.Empty();
	for (uint32 CargoIndex = 0; CargoIndex < CargoBayCount; CargoIndex++)
	{
		FFlareCargo Cargo;
		Cargo.Resource = NULL;
		Cargo.Capacity = CargoBayCapacity;
		Cargo.Quantity = 0;

		if (CargoIndex < (uint32)Data.Num())
		{
			// Existing save
			FFlareCargoSave* CargoSave = &Data[CargoIndex];

			if (CargoSave->Quantity > 0)
			{
				Cargo.Resource = Game->GetResourceCatalog()->Get(CargoSave->ResourceIdentifier);
				Cargo.Quantity = FMath::Min(CargoSave->Quantity, CargoBayCapacity);
			}
		}

		CargoBay.Add(Cargo);
	}
}


TArray<FFlareCargoSave>* UFlareCargoBay::Save()
{
	CargoBayData.Empty();
	for (int CargoIndex = 0; CargoIndex < CargoBay.Num() ; CargoIndex++)
	{
		FFlareCargo& Cargo = CargoBay[CargoIndex];
		FFlareCargoSave CargoSave;
		CargoSave.Quantity = Cargo.Quantity;
		if (Cargo.Resource != NULL)
		{
			CargoSave.ResourceIdentifier = Cargo.Resource->Identifier;
		}
		else
		{
			CargoSave.ResourceIdentifier = NAME_None;
		}
		CargoBayData.Add(CargoSave);
	}

	return &CargoBayData;
}


/*----------------------------------------------------
   Gameplay
----------------------------------------------------*/


bool UFlareCargoBay::HasResources(FFlareResourceDescription* Resource, uint32 Quantity)
{
	uint32 PresentQuantity = 0;

	if (Quantity == 0)
	{
		return true;
	}

	for (int CargoIndex = 0; CargoIndex < CargoBay.Num() ; CargoIndex++)
	{
		FFlareCargo* Cargo = &CargoBay[CargoIndex];
		if (Cargo->Resource == Resource)
		{
			PresentQuantity += Cargo->Quantity;
			if (PresentQuantity >= Quantity)
			{
				return true;
			}
		}
	}
	return false;
}

uint32 UFlareCargoBay::TakeResources(FFlareResourceDescription* Resource, uint32 Quantity)
{
	uint32 QuantityToTake = Quantity;


	if (QuantityToTake == 0)
	{
		return 0;
	}

	// First pass: take resource from the less full cargo
	uint32 MinQuantity = 0;
	FFlareCargo* MinQuantityCargo = NULL;

	for (int CargoIndex = 0; CargoIndex < CargoBay.Num() ; CargoIndex++)
	{
		FFlareCargo& Cargo = CargoBay[CargoIndex];
		if (Cargo.Resource == Resource)
		{
			if (MinQuantityCargo == NULL || MinQuantity > Cargo.Quantity)
			{
				MinQuantityCargo = &Cargo;
				MinQuantity = Cargo.Quantity;
			}
		}
	}

	if (MinQuantityCargo)
	{
		uint32 TakenQuantity = FMath::Min(MinQuantityCargo->Quantity, QuantityToTake);
		if (TakenQuantity > 0)
		{
			MinQuantityCargo->Quantity -= TakenQuantity;
			QuantityToTake -= TakenQuantity;

			if (MinQuantityCargo->Quantity == 0)
			{
				MinQuantityCargo->Resource = NULL;
			}

			if (QuantityToTake == 0)
			{
				return Quantity;
			}
		}
	}


	for (int CargoIndex = 0; CargoIndex < CargoBay.Num() ; CargoIndex++)
	{
		FFlareCargo& Cargo = CargoBay[CargoIndex];
		if (Cargo.Resource == Resource)
		{
			uint32 TakenQuantity = FMath::Min(Cargo.Quantity, QuantityToTake);
			if (TakenQuantity > 0)
			{
				Cargo.Quantity -= TakenQuantity;
				QuantityToTake -= TakenQuantity;

				if (Cargo.Quantity == 0)
				{
					Cargo.Resource = NULL;
				}

				if (QuantityToTake == 0)
				{
					return Quantity;
				}
			}
		}
	}
	return Quantity - QuantityToTake;
}

uint32 UFlareCargoBay::GiveResources(FFlareResourceDescription* Resource, uint32 Quantity)
{
	uint32 QuantityToGive = Quantity;

	if (QuantityToGive == 0)
	{
		return Quantity;
	}

	// First pass, fill already existing slots
	for (int CargoIndex = 0 ; CargoIndex < CargoBay.Num() ; CargoIndex++)
	{
		FFlareCargo& Cargo = CargoBay[CargoIndex];
		if (Resource == Cargo.Resource)
		{
			// Same resource
			uint32 AvailableCapacity = Cargo.Capacity - Cargo.Quantity;
			uint32 GivenQuantity = FMath::Min(AvailableCapacity, QuantityToGive);
			if (GivenQuantity > 0)
			{
				Cargo.Quantity += GivenQuantity;
				QuantityToGive -= GivenQuantity;

				if (QuantityToGive == 0)
				{
					return Quantity;
				}
			}
		}
	}

	// Fill free cargo slots
	for (int CargoIndex = 0 ; CargoIndex < CargoBay.Num() ; CargoIndex++)
	{
		FFlareCargo& Cargo = CargoBay[CargoIndex];
		if (Cargo.Quantity == 0)
		{
			// Empty Cargo
			uint32 GivenQuantity = FMath::Min(Cargo.Capacity, QuantityToGive);
			if (GivenQuantity > 0)
			{
				Cargo.Quantity += GivenQuantity;
				Cargo.Resource = Resource;

				QuantityToGive -= GivenQuantity;

				if (QuantityToGive == 0)
				{
					return Quantity;
				}
			}
			else
			{
				FLOGV("Zero sized cargo bay for %s", *Parent->GetImmatriculation().ToString())
			}

		}
	}

	return Quantity - QuantityToGive;
}

uint32 UFlareCargoBay::GetCapacity()
{
	return CargoBayCapacity * CargoBayCount;
}

uint32 UFlareCargoBay::GetResourceQuantity(FFlareResourceDescription* Resource)
{
	uint32 Quantity = 0;

	for (int CargoIndex = 0; CargoIndex < CargoBay.Num() ; CargoIndex++)
	{
		FFlareCargo& Cargo = CargoBay[CargoIndex];
		if (Cargo.Resource == Resource)
		{
			Quantity += Cargo.Quantity;
		}
	}

	return Quantity;
}
uint32 UFlareCargoBay::GetFreeSpaceForResource(FFlareResourceDescription* Resource)
{
	uint32 Quantity = 0;

	for (int CargoIndex = 0; CargoIndex < CargoBay.Num() ; CargoIndex++)
	{
		FFlareCargo& Cargo = CargoBay[CargoIndex];
		if (Cargo.Resource == NULL)
		{
			Quantity += Cargo.Capacity;
		}
		else if (Cargo.Resource == Resource)
		{
			Quantity += Cargo.Capacity - Cargo.Quantity;
		}
	}

	return Quantity;
}

uint32 UFlareCargoBay::GetSlotCount()
{
	return CargoBayCount;
}


FFlareCargo* UFlareCargoBay::GetSlot(uint32 Index)
{
	return &CargoBay[Index];
}

