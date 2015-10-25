
#include "Flare.h"
#include "FlareGame.h"
#include "FlareCompany.h"
#include "../Player/FlarePlayerController.h"
#include "FlareSector.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"

#define LOCTEXT_NAMESPACE "FlareCompany"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareCompany::UFlareCompany(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


/*----------------------------------------------------
	Save
----------------------------------------------------*/

void UFlareCompany::Load(const FFlareCompanySave& Data)
{
	Game = Cast<UFlareWorld>(GetOuter())->GetGame();

	CompanyData = Data;
	CompanyData.Identifier = FName(*GetName());
	CompanyDescription = NULL;

	// Player description ID is -1
	if (Data.CatalogIdentifier >= 0)
	{
		CompanyDescription = GetGame()->GetCompanyDescription(Data.CatalogIdentifier);
	}
	else
	{
		CompanyDescription = GetGame()->GetPlayerCompanyDescription();
	}


	for (int i = 0 ; i < CompanyData.ShipData.Num(); i++)
	{
		LoadSpacecraft(CompanyData.ShipData[i]);
	}

	for (int i = 0 ; i < CompanyData.StationData.Num(); i++)
	{
		LoadSpacecraft(CompanyData.StationData[i]);
	}


	// Load all fleets
	for (int32 i = 0; i < CompanyData.Fleets.Num(); i++)
	{
		LoadFleet(CompanyData.Fleets[i]);
	}
}

void UFlareCompany::PostLoad()
{
	VisitedSectors.Empty();
	KnownSectors.Empty();

	// Load sector knowledge
	for (int32 i = 0; i < CompanyData.SectorsKnowledge.Num(); i++)
	{
		UFlareSimulatedSector* Sector = GetGame()->GetGameWorld()->FindSector(CompanyData.SectorsKnowledge[i].SectorIdentifier);
		switch (CompanyData.SectorsKnowledge[i].Knowledge) {
		case EFlareSectorKnowledge::Visited:
			VisitedSectors.Add(Sector);
			// No break
		case EFlareSectorKnowledge::Known:
			KnownSectors.Add(Sector);
			break;
		default:
			break;
		}
	}
}

FFlareCompanySave* UFlareCompany::Save()
{
	CompanyData.Fleets.Empty();
	CompanyData.ShipData.Empty();
	CompanyData.StationData.Empty();
	CompanyData.SectorsKnowledge.Empty();

	for (int i = 0 ; i < CompanyFleets.Num(); i++)
	{
		CompanyData.Fleets.Add(*CompanyFleets[i]->Save());
	}

	for (int i = 0 ; i < CompanyShips.Num(); i++)
	{
		CompanyData.ShipData.Add(*CompanyShips[i]->Save());
	}

	for (int i = 0 ; i < CompanyStations.Num(); i++)
	{
		CompanyData.StationData.Add(*CompanyStations[i]->Save());
	}

	for (int i = 0 ; i < VisitedSectors.Num(); i++)
	{
		FFlareCompanySectorKnowledge SectorKnowledge;
		SectorKnowledge.Knowledge = EFlareSectorKnowledge::Visited;
		SectorKnowledge.SectorIdentifier = VisitedSectors[i]->GetIdentifier();

		CompanyData.SectorsKnowledge.Add(SectorKnowledge);
	}

	for (int i = 0 ; i < KnownSectors.Num(); i++)
	{
		// The visited sector are already saved
		if(!VisitedSectors.Contains(KnownSectors[i]))
		{
			FFlareCompanySectorKnowledge SectorKnowledge;
			SectorKnowledge.Knowledge = EFlareSectorKnowledge::Known;
			SectorKnowledge.SectorIdentifier = KnownSectors[i]->GetIdentifier();

			CompanyData.SectorsKnowledge.Add(SectorKnowledge);
		}
	}


	return &CompanyData;
}

/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

EFlareHostility::Type UFlareCompany::GetPlayerHostility() const
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(Game->GetWorld()->GetFirstPlayerController());

	if (PC)
	{
		return PC->GetCompany()->GetHostility(this);
	}

	return EFlareHostility::Neutral;
}

EFlareHostility::Type UFlareCompany::GetHostility(const UFlareCompany* TargetCompany) const
{
	if (TargetCompany == this)
	{
		return EFlareHostility::Owned;
	}
	else if (TargetCompany && CompanyData.HostileCompanies.Contains(TargetCompany->GetIdentifier()))
	{
		return EFlareHostility::Hostile;
	}

	return EFlareHostility::Neutral;
}

void UFlareCompany::SetHostilityTo(const UFlareCompany* TargetCompany, bool Hostile)
{
	if (TargetCompany && TargetCompany != this)
	{
		if (Hostile)
		{
			CompanyData.HostileCompanies.AddUnique(TargetCompany->GetIdentifier());
		}
		else
		{
			CompanyData.HostileCompanies.Remove(TargetCompany->GetIdentifier());
		}
	}
}

FText UFlareCompany::GetShortInfoText()
{
	// Static text
	FText ShipText = LOCTEXT("Ship", "ship");
	FText ShipsText = LOCTEXT("Ships", "ships");
	FText MoneyText = LOCTEXT("Money", "credits");

	// Dynamic data
	int32 ShipCount = GetCompanyShips().Num();
	FString MoneyDescriptionString = FString::FromInt(GetMoney()) + " " + MoneyText.ToString();
	FString ShipDescriptionString = FString::FromInt(ShipCount) + " " + (ShipCount != 1 ? ShipsText : ShipText).ToString();

	// Build
	return FText::FromString(GetCompanyName().ToString() + " (" + MoneyDescriptionString + ", " + ShipDescriptionString + ")");
}

UFlareFleet* UFlareCompany::CreateFleet(FString FleetName, UFlareSimulatedSector* FleetSector)
{
	// Create the fleet
	FFlareFleetSave FleetData;
	FleetData.Identifier = FName(*(GetIdentifier().ToString() + "-" + FString::FromInt(CompanyData.FleetImmatriculationIndex++)));
	FleetData.Name = FleetName;
	UFlareFleet* Fleet = LoadFleet(FleetData);
	Fleet->SetCurrentSector(FleetSector);
	FleetSector->AddFleet(Fleet);
	return Fleet;
}


UFlareFleet* UFlareCompany::LoadFleet(const FFlareFleetSave& FleetData)
{
	UFlareFleet* Fleet = NULL;

	// Create the new travel
	Fleet = NewObject<UFlareFleet>(this, UFlareFleet::StaticClass());
	Fleet->Load(FleetData);
	CompanyFleets.AddUnique(Fleet);

	FLOGV("UFlareWorld::LoadFleet : loaded fleet '%s'", *Fleet->GetFleetName());

	return Fleet;
}

void UFlareCompany::RemoveFleet(UFlareFleet* Fleet)
{
	CompanyFleets.Remove(Fleet);
}

UFlareSimulatedSpacecraft* UFlareCompany::LoadSpacecraft(const FFlareSpacecraftSave& SpacecraftData)
{
	UFlareSimulatedSpacecraft* Spacecraft = NULL;
	FLOGV("UFlareCompany::LoadSpacecraft ('%s')", *SpacecraftData.Immatriculation.ToString());

	FFlareSpacecraftDescription* Desc = Game->GetSpacecraftCatalog()->Get(SpacecraftData.Identifier);
	if (Desc)
	{
		Spacecraft = NewObject<UFlareSimulatedSpacecraft>(this, UFlareSimulatedSpacecraft::StaticClass());
		Spacecraft->Load(SpacecraftData);


		if ((Spacecraft)->IsStation())
		{
			CompanyStations.AddUnique((Spacecraft));
		}
		else
		{
			CompanyShips.AddUnique((Spacecraft));
		}

		CompanySpacecrafts.AddUnique((Spacecraft));
	}
	else
	{
		FLOG("UFlareCompany::LoadSpacecraft failed (no description available)");
	}

	return Spacecraft;
}

void UFlareCompany::DestroySpacecraft(UFlareSimulatedSpacecraft* Spacecraft)
{
	FLOGV("UFlareCompany::DestroySpacecraft remove %s from company %s", *Spacecraft->GetImmatriculation().ToString(), *GetCompanyName().ToString());

	CompanySpacecrafts.Remove(Spacecraft);
	CompanyStations.Remove(Spacecraft);
	CompanyShips.Remove(Spacecraft);
	if(Spacecraft->GetCurrentFleet())
	{
		Spacecraft->GetCurrentFleet()->RemoveShip(Spacecraft);
	}

	if(Spacecraft->GetCurrentSector())
	{
		Spacecraft->GetCurrentSector()->RemoveSpacecraft(Spacecraft);
	}
}

void UFlareCompany::DiscoverSector(UFlareSimulatedSector* Sector)
{
	KnownSectors.AddUnique(Sector);
}

void UFlareCompany::VisitSector(UFlareSimulatedSector* Sector)
{
	DiscoverSector(Sector);
	VisitedSectors.AddUnique(Sector);
	if (GetGame()->GetQuestManager())
	{
		GetGame()->GetQuestManager()->OnSectorVisited(Sector);
	}
}


/*----------------------------------------------------
	Customization
----------------------------------------------------*/

void UFlareCompany::UpdateCompanyCustomization()
{
	// Update spacecraft if there is an active sector
	UFlareSector* ActiveSector = Game->GetActiveSector();
	if (ActiveSector)
	{
		for (int32 i = 0; i < ActiveSector->GetSpacecrafts().Num(); i++)
		{
			ActiveSector->GetSpacecrafts()[i]->UpdateCustomization();
		}
	}
}

void UFlareCompany::CustomizeComponentMaterial(UMaterialInstanceDynamic* Mat)
{
	// Get data from storage
	FLinearColor BasePaintColor = GetGame()->GetCustomizationCatalog()->GetColor(GetBasePaintColorIndex());
	FLinearColor PaintColor = GetGame()->GetCustomizationCatalog()->GetColor(GetPaintColorIndex());
	FLinearColor OverlayColor = GetGame()->GetCustomizationCatalog()->GetColor(GetOverlayColorIndex());
	FLinearColor LightColor = GetGame()->GetCustomizationCatalog()->GetColor(GetLightColorIndex());
	UTexture2D* Pattern = GetGame()->GetCustomizationCatalog()->GetPattern(GetPatternIndex());
	UTexture2D* Emblem = CompanyDescription->Emblem;

	// Apply settings to the material instance
	Mat->SetVectorParameterValue("BasePaintColor", BasePaintColor);
	Mat->SetVectorParameterValue("PaintColor", PaintColor);
	Mat->SetVectorParameterValue("OverlayColor", OverlayColor);
	Mat->SetVectorParameterValue("LightColor", LightColor);
	Mat->SetVectorParameterValue("GlowColor", NormalizeColor(LightColor));
	Mat->SetTextureParameterValue("PaintPattern", Pattern);
	Mat->SetTextureParameterValue("Emblem", Emblem);
}

void UFlareCompany::CustomizeEffectMaterial(UMaterialInstanceDynamic* Mat)
{
}

FLinearColor UFlareCompany::NormalizeColor(FLinearColor Col) const
{
	return FLinearColor(FVector(Col.R, Col.G, Col.B) / Col.GetLuminance());
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

const FSlateBrush* UFlareCompany::GetEmblem() const
{
	return GetGame()->GetCompanyEmblem(CompanyData.CatalogIdentifier);
}

UFlareSimulatedSpacecraft* UFlareCompany::FindSpacecraft(FName ShipImmatriculation)
{
	for (int i = 0; i < CompanySpacecrafts.Num(); i++)
	{
		if (CompanySpacecrafts[i]->GetImmatriculation() == ShipImmatriculation)
		{
			return CompanySpacecrafts[i];
		}
	}

	return NULL;
}

bool UFlareCompany::HasVisitedSector(const UFlareSimulatedSector* Sector) const
{
	return VisitedSectors.Contains(Sector);
}

#undef LOCTEXT_NAMESPACE
