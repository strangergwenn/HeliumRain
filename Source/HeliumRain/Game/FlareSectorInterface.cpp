#include "../Flare.h"
#include "FlareSectorInterface.h"
#include "FlareSimulatedSector.h"
#include "../Spacecrafts/FlareSpacecraftInterface.h"

#define LOCTEXT_NAMESPACE "FlareSectorInterface"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSectorInterface::UFlareSectorInterface(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
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

uint32 UFlareSectorInterface::GetResourcePrice(FFlareResourceDescription* Resource)
{
	// DEBUGInflation
	float Inflation = 1.5;

	// TODO better
	if (Resource->Identifier == "h2")
	{
		return 25 * Inflation;
	}
	else if (Resource->Identifier == "feo")
	{
		return 100 * Inflation;
	}
	else if (Resource->Identifier == "ch4")
	{
		return 50 * Inflation;
	}
	else if (Resource->Identifier == "sio2")
	{
		return 100 * Inflation;
	}
	else if (Resource->Identifier == "he3")
	{
		return 100 * Inflation;
	}
	else if (Resource->Identifier == "h2o")
	{
		return 250 * Inflation;
	}
	else if (Resource->Identifier == "steel")
	{
		return 500 * Inflation;
	}
	else if (Resource->Identifier == "c")
	{
		return 300 * Inflation;
	}
	else if (Resource->Identifier == "plastics")
	{
		return 300 * Inflation;
	}
	else if (Resource->Identifier == "fleet-supply")
	{
		return 1800 * Inflation;
	}
	else if (Resource->Identifier == "food")
	{
		return 600 * Inflation;
	}
	else if (Resource->Identifier == "fuel")
	{
		return 260 * Inflation;
	}
	else if (Resource->Identifier == "tools")
	{
		return 1000 * Inflation;
	}
	else if (Resource->Identifier == "tech")
	{
		return 1300 * Inflation;

	}
	else
	{
		FLOGV("Unknown resource %s", *Resource->Identifier.ToString());
		return 0;
	}


}

#undef LOCTEXT_NAMESPACE
