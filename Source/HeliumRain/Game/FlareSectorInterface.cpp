#include "../Flare.h"
#include "FlareSectorInterface.h"
#include "FlareSimulatedSector.h"
#include "../Spacecrafts/FlareSpacecraftInterface.h"
#include "../Economy/FlareCargoBay.h"
#include "../Game/FlareGame.h"

#define LOCTEXT_NAMESPACE "FlareSectorInterface"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSectorInterface::UFlareSectorInterface(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}

void UFlareSectorInterface::LoadResourcePrices()
{
	ResourcePrices.Empty();
	LastResourcePrices.Empty();
	for (int PriceIndex = 0; PriceIndex < SectorData.ResourcePrices.Num(); PriceIndex++)
	{
		FFFlareResourcePrice* ResourcePrice = &SectorData.ResourcePrices[PriceIndex];
		FFlareResourceDescription* Resource = Game->GetResourceCatalog()->Get(ResourcePrice->ResourceIdentifier);
		float Price = ResourcePrice->Price;
		ResourcePrices.Add(Resource, Price);
		FFlareFloatBuffer* Prices = &ResourcePrice->Prices;
		Prices->Resize(50);
		LastResourcePrices.Add(Resource, *Prices);
	}
}

void UFlareSectorInterface::SaveResourcePrices()
{
	SectorData.ResourcePrices.Empty();

	for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
		if (ResourcePrices.Contains(Resource))
		{
			FFFlareResourcePrice Price;
			Price.ResourceIdentifier = Resource->Identifier;
			Price.Price = ResourcePrices[Resource];
			if (LastResourcePrices.Contains(Resource))
			{
				Price.Prices = LastResourcePrices[Resource];
				SectorData.ResourcePrices.Add(Price);
			}
		}
	}
}

FText UFlareSectorInterface::GetSectorName()
{
	if (SectorData.GivenName.ToString().Len())
	{
		return SectorData.GivenName;
	}
	else if (SectorDescription->Name.ToString().Len())
	{
		return SectorDescription->Name;
	}
	else
	{
		return FText::FromString(GetSectorCode());
	}
}

FString UFlareSectorInterface::GetSectorCode()
{
	// TODO cache ?
	return SectorOrbitParameters.CelestialBodyIdentifier.ToString() + "-" + FString::FromInt(SectorOrbitParameters.Altitude) + "-" + FString::FromInt(SectorOrbitParameters.Phase);
}


EFlareSectorFriendlyness::Type UFlareSectorInterface::GetSectorFriendlyness(UFlareCompany* Company)
{
	if (!Company->HasVisitedSector(GetSimulatedSector()))
	{
		return EFlareSectorFriendlyness::NotVisited;
	}

	if (GetSimulatedSector()->GetSectorSpacecrafts().Num() == 0)
	{
		return EFlareSectorFriendlyness::Neutral;
	}

	int HostileSpacecraftCount = 0;
	int NeutralSpacecraftCount = 0;
	int FriendlySpacecraftCount = 0;

	for (int SpacecraftIndex = 0 ; SpacecraftIndex < GetSimulatedSector()->GetSectorSpacecrafts().Num(); SpacecraftIndex++)
	{
		UFlareCompany* OtherCompany = GetSimulatedSector()->GetSectorSpacecrafts()[SpacecraftIndex]->GetCompany();

		if (OtherCompany == Company)
		{
			FriendlySpacecraftCount++;
		}
		else if (OtherCompany->GetWarState(Company) == EFlareHostility::Hostile)
		{
			HostileSpacecraftCount++;
		}
		else
		{
			NeutralSpacecraftCount++;
		}
	}

	if (FriendlySpacecraftCount > 0 && HostileSpacecraftCount > 0)
	{
		return EFlareSectorFriendlyness::Contested;
	}

	if (FriendlySpacecraftCount > 0)
	{
		return EFlareSectorFriendlyness::Friendly;
	}
	else if (HostileSpacecraftCount > 0)
	{
		return EFlareSectorFriendlyness::Hostile;
	}
	else
	{
		return EFlareSectorFriendlyness::Neutral;
	}
}

EFlareSectorBattleState::Type UFlareSectorInterface::GetSectorBattleState(UFlareCompany* Company)
{

	if (GetSectorShipInterfaces().Num() == 0)
	{
		return EFlareSectorBattleState::NoBattle;
	}

	int HostileSpacecraftCount = 0;
	int DangerousHostileSpacecraftCount = 0;


	int FriendlySpacecraftCount = 0;
	int DangerousFriendlySpacecraftCount = 0;
	int CrippledFriendlySpacecraftCount = 0;

	for (int SpacecraftIndex = 0 ; SpacecraftIndex < GetSectorShipInterfaces().Num(); SpacecraftIndex++)
	{

		IFlareSpacecraftInterface* Spacecraft = GetSectorShipInterfaces()[SpacecraftIndex];

		UFlareCompany* OtherCompany = Spacecraft->GetCompany();

		if (!Spacecraft->GetDamageSystem()->IsAlive())
		{
			continue;
		}

		if (OtherCompany == Company)
		{
			FriendlySpacecraftCount++;
			if (Spacecraft->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon) > 0)
			{
				DangerousFriendlySpacecraftCount++;
			}

			if (Spacecraft->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Propulsion) == 0)
			{
				CrippledFriendlySpacecraftCount++;
			}
		}
		else if (OtherCompany->GetWarState(Company) == EFlareHostility::Hostile)
		{
			HostileSpacecraftCount++;
			if (Spacecraft->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon) > 0)
			{
				DangerousHostileSpacecraftCount++;
			}
		}
	}

	// No friendly or no hostile ship
	if (FriendlySpacecraftCount == 0 || HostileSpacecraftCount == 0)
	{
		return EFlareSectorBattleState::NoBattle;
	}

	// No friendly and hostile ship are not dangerous
	if (DangerousFriendlySpacecraftCount == 0 && DangerousHostileSpacecraftCount == 0)
	{
		return EFlareSectorBattleState::NoBattle;
	}

	// No friendly dangerous ship so the enemy have one. Battle is lost
	if (DangerousFriendlySpacecraftCount == 0)
	{
		if (CrippledFriendlySpacecraftCount == FriendlySpacecraftCount)
		{
			return EFlareSectorBattleState::BattleLostNoRetreat;
		}
		else
		{
			return EFlareSectorBattleState::BattleLost;
		}
	}

	if (DangerousHostileSpacecraftCount == 0)
	{
		return EFlareSectorBattleState::BattleWon;
	}

	if (CrippledFriendlySpacecraftCount == FriendlySpacecraftCount)
	{
		return EFlareSectorBattleState::BattleNoRetreat;
	}
	else
	{
		return EFlareSectorBattleState::Battle;
	}
}

FText UFlareSectorInterface::GetSectorFriendlynessText(UFlareCompany* Company)
{
	FText Status;

	switch (GetSectorFriendlyness(Company))
	{
		case EFlareSectorFriendlyness::NotVisited:
			Status = LOCTEXT("Unknown", "UNKNOWN");
			break;
		case EFlareSectorFriendlyness::Neutral:
			Status = LOCTEXT("Neutral", "NEUTRAL");
			break;
		case EFlareSectorFriendlyness::Friendly:
			Status = LOCTEXT("Friendly", "FRIENDLY");
			break;
		case EFlareSectorFriendlyness::Contested:
			Status = LOCTEXT("Contested", "CONTESTED");
			break;
		case EFlareSectorFriendlyness::Hostile:
			Status = LOCTEXT("Hostile", "HOSTILE");
			break;
	}

	return Status;
}

FLinearColor UFlareSectorInterface::GetSectorFriendlynessColor(UFlareCompany* Company)
{
	FLinearColor Color;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	switch (GetSectorFriendlyness(Company))
	{
		case EFlareSectorFriendlyness::NotVisited:
			Color = Theme.UnknownColor;
			break;
		case EFlareSectorFriendlyness::Neutral:
			Color = Theme.NeutralColor;
			break;
		case EFlareSectorFriendlyness::Friendly:
			Color = Theme.FriendlyColor;
			break;
		case EFlareSectorFriendlyness::Contested:
			Color = Theme.DisputedColor;
			break;
		case EFlareSectorFriendlyness::Hostile:
			Color = Theme.EnemyColor;
			break;
	}

	return Color;
}


float UFlareSectorInterface::GetPreciseResourcePrice(FFlareResourceDescription* Resource, int32 Age)
{
	if(Age == 0)
	{

		if (!ResourcePrices.Contains(Resource))
		{
			ResourcePrices.Add(Resource, GetDefaultResourcePrice(Resource));
		}

		return ResourcePrices[Resource];
	}
	else
	{
		if (!LastResourcePrices.Contains(Resource))
		{
			FFlareFloatBuffer Prices;
			Prices.Init(50);
			Prices.Append(GetPreciseResourcePrice(Resource, 0));
			LastResourcePrices.Add(Resource, Prices);
		}

		return LastResourcePrices[Resource].GetValue(Age);
	}

}

void UFlareSectorInterface::SwapPrices()
{
	for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;

		if (!LastResourcePrices.Contains(Resource))
		{
			FFlareFloatBuffer Prices;
			Prices.Init(50);
			LastResourcePrices.Add(Resource, Prices);
		}

		LastResourcePrices[Resource].Append(GetPreciseResourcePrice(Resource, 0));
	}
}

void UFlareSectorInterface::SetPreciseResourcePrice(FFlareResourceDescription* Resource, float NewPrice)
{
	ResourcePrices[Resource] = FMath::Clamp(NewPrice, (float) Resource->MinPrice, (float) Resource->MaxPrice);
}

int64 UFlareSectorInterface::GetResourcePrice(FFlareResourceDescription* Resource, EFlareResourcePriceContext::Type PriceContext, int32 Age)
{
	int64 DefaultPrice = FMath::RoundToInt(GetPreciseResourcePrice(Resource, Age));

	switch (PriceContext)
	{
		case EFlareResourcePriceContext::Default:
			return DefaultPrice;
		break;
		case EFlareResourcePriceContext::FactoryOutput:
			return DefaultPrice - Resource->TransportFee;
		break;
		case EFlareResourcePriceContext::FactoryInput:
			return DefaultPrice + Resource->TransportFee;
		break;
		case EFlareResourcePriceContext::ConsumerConsumption:
			return DefaultPrice * 1.5;
		break;
		case EFlareResourcePriceContext::MaintenanceConsumption:
			return DefaultPrice * 1.5;
		break;
		default:
			return 0;
			break;
	}
}

float UFlareSectorInterface::GetDefaultResourcePrice(FFlareResourceDescription* Resource)
{
	return (Resource->MinPrice + Resource->MaxPrice)/2;
}

uint32 UFlareSectorInterface::GetTransfertResourcePrice(IFlareSpacecraftInterface* SourceSpacecraft, IFlareSpacecraftInterface* DestinationSpacecraft, FFlareResourceDescription* Resource)
{
	IFlareSpacecraftInterface* Station = NULL;

	// Which one is any is a station ?
	if (SourceSpacecraft->IsStation())
	{
		Station = SourceSpacecraft;
	}
	else if (DestinationSpacecraft->IsStation())
	{
		Station = DestinationSpacecraft;
	}
	else
	{
		return GetResourcePrice(Resource, EFlareResourcePriceContext::Default);
	}

	// Get context
	EFlareResourcePriceContext::Type ResourceUsage = Station->GetResourceUseType(Resource);
	if (ResourceUsage == EFlareResourcePriceContext::ConsumerConsumption ||
			ResourceUsage == EFlareResourcePriceContext::MaintenanceConsumption)
	{
		ResourceUsage = EFlareResourcePriceContext::FactoryInput;
	}

	// Get the usage and price
	return GetResourcePrice(Resource, ResourceUsage);
}


uint32 UFlareSectorInterface::TransfertResources(IFlareSpacecraftInterface* SourceSpacecraft, IFlareSpacecraftInterface* DestinationSpacecraft, FFlareResourceDescription* Resource, uint32 Quantity)
{
	// TODO Check docking capabilities

	if(SourceSpacecraft->GetCurrentSectorInterface() != DestinationSpacecraft->GetCurrentSectorInterface())
	{
		FLOG("Warning cannot transfert resource because both ship are not in the same sector");
		return 0;
	}

	if(SourceSpacecraft->IsStation() && DestinationSpacecraft->IsStation())
	{
		FLOG("Warning cannot transfert resource between 2 stations");
		return 0;
	}

	int32 ResourcePrice = GetTransfertResourcePrice(SourceSpacecraft, DestinationSpacecraft, Resource);
	int32 QuantityToTake = Quantity;

	if (SourceSpacecraft->GetCompany() != DestinationSpacecraft->GetCompany())
	{
		// Limit transaction bay available money
		int32 MaxAffordableQuantity = DestinationSpacecraft->GetCompany()->GetMoney() / ResourcePrice;
		QuantityToTake = FMath::Min(QuantityToTake, MaxAffordableQuantity);
	}
	int32 ResourceCapacity = DestinationSpacecraft->GetCargoBay()->GetFreeSpaceForResource(Resource);

	QuantityToTake = FMath::Min(QuantityToTake, ResourceCapacity);


	int32 TakenResources = SourceSpacecraft->GetCargoBay()->TakeResources(Resource, QuantityToTake);
	int32 GivenResources = DestinationSpacecraft->GetCargoBay()->GiveResources(Resource, TakenResources);

	if (GivenResources > 0 && SourceSpacecraft->GetCompany() != DestinationSpacecraft->GetCompany())
	{
		// Pay
		int64 Price = ResourcePrice * GivenResources;
		DestinationSpacecraft->GetCompany()->TakeMoney(Price);
		SourceSpacecraft->GetCompany()->GiveMoney(Price);

		SourceSpacecraft->GetCompany()->GiveReputation(DestinationSpacecraft->GetCompany(), 0.5f, true);
		DestinationSpacecraft->GetCompany()->GiveReputation(SourceSpacecraft->GetCompany(), 0.5f, true);
	}

	return GivenResources;
}

bool UFlareSectorInterface::CanUpgrade(UFlareCompany* Company)
{
	EFlareSectorBattleState::Type BattleState = GetSectorBattleState(Company);
	if(BattleState != EFlareSectorBattleState::NoBattle
			&& BattleState != EFlareSectorBattleState::BattleWon)
	{
		return false;
	}

	for(int StationIndex = 0 ; StationIndex < GetSectorStationInterfaces().Num(); StationIndex ++ )
	{
		IFlareSpacecraftInterface* StationInterface = GetSectorStationInterfaces()[StationIndex];
		if (StationInterface->GetCompany()->GetWarState(Company) != EFlareHostility::Hostile)
		{
			return true;
		}
	}

	return false;
}

#undef LOCTEXT_NAMESPACE
