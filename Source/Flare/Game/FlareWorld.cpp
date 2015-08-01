
#include "../Flare.h"

#include "FlareWorld.h"
#include "FlareGame.h"
#include "FlareSector.h"


/*----------------------------------------------------
    Constructor
----------------------------------------------------*/

UFlareWorld::UFlareWorld(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareWorld::Load(const FFlareWorldSave& Data)
{
	FLOG("UFlareWorld::Load");
	Game = Cast<AFlareGame>(GetOuter());
    WorldData = Data;

    // Load all companies
    for (int32 i = 0; i < WorldData.CompanyData.Num(); i++)
    {
		LoadCompany(WorldData.CompanyData[i]);
    }

	for (int32 OrbitalBodyIndex = 0; OrbitalBodyIndex < Game->GetSectorCatalog()->OrbitalBodies.Num(); OrbitalBodyIndex++)
	{
		FFlareSectorCelestialBodyDescription* SectorCelestialBodyDescription = &Game->GetSectorCatalog()->OrbitalBodies[OrbitalBodyIndex];
		for (int32 OrbitIndex = 0; OrbitIndex < SectorCelestialBodyDescription->Orbits.Num(); OrbitIndex++)
		{
			FFlareSectorOrbitDescription* SectorOrbitDescription = &SectorCelestialBodyDescription->Orbits[OrbitIndex];
			for (int32 SectorIndex = 0; SectorIndex < SectorOrbitDescription->Sectors.Num(); SectorIndex++)
			{
				const FFlareSectorDescription* SectorDescription = &SectorOrbitDescription->Sectors[SectorIndex];

				// Find save if exist
				FFlareSectorSave* SectorSave = NULL;
				for (int32 i = 0; i < WorldData.SectorData.Num(); i++)
				{
					if(WorldData.SectorData[i].Identifier == SectorDescription->Identifier)
					{
						// Old save found
						SectorSave = &WorldData.SectorData[i];
						break;
					}
				}

				FFlareSectorSave NewSectorData;
				if(!SectorSave)
				{
					// No save, init new sector
					NewSectorData.GivenName = "";
					NewSectorData.Identifier = SectorDescription->Identifier;
					SectorSave = &NewSectorData;
				}

				FFlareSectorOrbitParameters OrbitParameters;
				OrbitParameters.CelestialBodyIdentifier = SectorCelestialBodyDescription->CelestialBodyIdentifier;
				OrbitParameters.Altitude = SectorOrbitDescription->Altitude;
				OrbitParameters.Phase = SectorDescription->Phase;

				LoadSector(SectorDescription, *SectorSave, OrbitParameters);
			}
		}
	}

	// Init planetarium
	Planetarium = NewObject<UFlareSimulatedPlanetarium>(this, UFlareSimulatedPlanetarium::StaticClass());
	Planetarium->Load();
}


UFlareCompany* UFlareWorld::LoadCompany(const FFlareCompanySave& CompanyData)
{
    UFlareCompany* Company = NULL;

    // Create the new company
	Company = NewObject<UFlareCompany>(this, UFlareCompany::StaticClass(), CompanyData.Identifier);
    Company->Load(CompanyData);
    Companies.AddUnique(Company);

	FLOGV("UFlareWorld::LoadCompany : loaded '%s'", *Company->GetCompanyName().ToString());

    return Company;
}


UFlareSimulatedSector* UFlareWorld::LoadSector(const FFlareSectorDescription* Description, const FFlareSectorSave& SectorData, const FFlareSectorOrbitParameters& OrbitParameters)
{
	UFlareSimulatedSector* Sector = NULL;

	// Create the new company
	Sector = NewObject<UFlareSimulatedSector>(this, UFlareSimulatedSector::StaticClass(), SectorData.Identifier);
	Sector->Load(Description, SectorData, OrbitParameters);
	Sectors.AddUnique(Sector);

	FLOGV("UFlareWorld::LoadSector : loaded '%s'", *Sector->GetSectorName());

	return Sector;
}


FFlareWorldSave* UFlareWorld::Save(UFlareSector* ActiveSector)
{
	WorldData.CompanyData.Empty();
	WorldData.SectorData.Empty();

	// Companies
	for (int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];

		FLOGV("UFlareWorld::Save : saving company ('%s')", *Company->GetName());
		FFlareCompanySave* TempData = Company->Save();
		WorldData.CompanyData.Add(*TempData);
	}

	// Sectors
	for (int i = 0; i < Sectors.Num(); i++)
	{
		UFlareSimulatedSector* Sector = Sectors[i];
		FLOGV("UFlareWorld::Save : saving sector ('%s')", *Sector->GetName());

		FFlareSectorSave* TempData;
		if (ActiveSector && Sector->GetIdentifier() == ActiveSector->GetIdentifier())
		{
			FLOG("  sector saved as active sector");
			TempData = ActiveSector->Save();
		}
		else
		{
			TempData = Sector->Save();
		}
		WorldData.SectorData.Add(*TempData);
	}

	return &WorldData;
}


void UFlareWorld::Simulate(long Duration)
{
	WorldData.Time += Duration;
	WorldData.Time2 = WorldData.Time;
}


void UFlareWorld::ForceTime(int64 Time)
{
	WorldData.Time = Time;
}
