
#include "Flare.h"
#include "FlareGame.h"
#include "FlareCompany.h"
#include "../Player/FlarePlayerController.h"
#include "../Spacecrafts/FlareSpacecraft.h"

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
		return GetHostility(PC->GetCompany());
	}

	return EFlareHostility::Neutral;
}

EFlareHostility::Type UFlareCompany::GetHostility(UFlareCompany* TargetCompany) const
{
	if (TargetCompany == this)
	{
		return EFlareHostility::Owned;
	}
	else if (TargetCompany && true)// TODO hostility
	{
		return EFlareHostility::Hostile;
	}

	return EFlareHostility::Neutral;
}


/*----------------------------------------------------
	Customization
----------------------------------------------------*/

inline void UFlareCompany::SetBasePaintColorIndex(int32 Index)
{
	CompanyData.CustomizationBasePaintColorIndex = Index;
	UpdateCompanyCustomization();
}

inline void UFlareCompany::SetPaintColorIndex(int32 Index)
{
	CompanyData.CustomizationPaintColorIndex = Index;
	UpdateCompanyCustomization();
}

inline void UFlareCompany::SetOverlayColorIndex(int32 Index)
{
	CompanyData.CustomizationOverlayColorIndex = Index;
	UpdateCompanyCustomization();
}

inline void UFlareCompany::SetLightColorIndex(int32 Index)
{
	CompanyData.CustomizationLightColorIndex = Index;
	UpdateCompanyCustomization();
}

inline void UFlareCompany::SetPatternIndex(int32 Index)
{
	CompanyData.CustomizationPatternIndex = Index;
	UpdateCompanyCustomization();
}

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

	// Apply settings to the material instance
	Mat->SetVectorParameterValue("BasePaintColor", BasePaintColor);
	Mat->SetVectorParameterValue("PaintColor", PaintColor);
	Mat->SetVectorParameterValue("OverlayColor", OverlayColor);
	Mat->SetVectorParameterValue("GlowColor", NormalizeColor(LightColor));
	Mat->SetTextureParameterValue("PaintPattern", Pattern);
}

void UFlareCompany::CustomizeEffectMaterial(UMaterialInstanceDynamic* Mat)
{
}

FLinearColor UFlareCompany::NormalizeColor(FLinearColor Col) const
{
	return FLinearColor(FVector(Col.R, Col.G, Col.B) / Col.GetLuminance());
}
