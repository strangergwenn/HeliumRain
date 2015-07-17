
#include "Flare.h"
#include "FlareGame.h"
#include "FlareCompany.h"
#include "../Player/FlarePlayerController.h"
#include "../Spacecrafts/FlareSpacecraft.h"


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
}

FFlareCompanySave* UFlareCompany::Save()
{
	return &CompanyData;
}

void UFlareCompany::Register(IFlareSpacecraftInterface* Ship)
{
	if (Ship->IsStation())
	{
		CompanyStations.AddUnique(Ship);
	}
	else
	{
		CompanyShips.AddUnique(Ship);
	}
}

void UFlareCompany::Unregister(IFlareSpacecraftInterface* Ship)
{
	if (Ship->IsStation())
	{
		CompanyStations.Remove(Ship);
	}
	else
	{
		CompanyShips.Remove(Ship);
	}
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

EFlareHostility::Type UFlareCompany::GetPlayerHostility() const
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOuter()->GetWorld()->GetFirstPlayerController());

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
		if(Hostile)
		{
			CompanyData.HostileCompanies.AddUnique(TargetCompany->GetIdentifier());
		}
		else
		{
			CompanyData.HostileCompanies.Remove(TargetCompany->GetIdentifier());
		}
	}
}

FText UFlareCompany::GetInfoText(bool Minimized)
{
	// Static text
	FText ShipText = LOCTEXT("Ship", "ship");
	FText ShipsText = LOCTEXT("Ships", "ships");
	FText StationText = LOCTEXT("Station", "station");
	FText StationsText = LOCTEXT("Stations", "stations");
	FText MoneyText = LOCTEXT("Money", "credits");

	// Dynamic data
	int32 ShipCount = GetCompanyShips().Num();
	int32 StationCount = GetCompanyStations().Num();
	FString MoneyDescriptionString = FString::FromInt(GetMoney()) + " " + MoneyText.ToString();
	FString ShipDescriptionString = FString::FromInt(ShipCount) + " " + (ShipCount != 1 ? ShipsText : ShipText).ToString();
	FString StationDescriptionString = FString::FromInt(StationCount) + " " + (StationCount != 1 ? StationsText : StationText).ToString();

	// Build
	if (Minimized)
	{
		return FText::FromString(GetCompanyName() + " (" + MoneyDescriptionString + ", " + ShipDescriptionString + ")");
	}
	else
	{
		return FText::FromString(MoneyDescriptionString + "\n" + ShipDescriptionString + "\n" + StationDescriptionString);
	}
}


/*----------------------------------------------------
	Customization
----------------------------------------------------*/

void UFlareCompany::UpdateCompanyCustomization()
{
	// Update ships
	for (int32 i = 0; i < CompanyShips.Num(); i++)
	{
		AFlareSpacecraft* Ship = Cast<AFlareSpacecraft>(CompanyShips[i]);
		if (Ship)
		{
			Ship->UpdateCustomization();
		}
	}

	// Update stations
	for (int32 i = 0; i < CompanyStations.Num(); i++)
	{
		AFlareSpacecraft* Station = Cast<AFlareSpacecraft>(CompanyStations[i]);
		if (Station)
		{
			Station->UpdateCustomization();
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


#undef LOCTEXT_NAMESPACE
