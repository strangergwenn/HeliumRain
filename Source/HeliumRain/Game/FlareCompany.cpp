
#include "Flare.h"
#include "FlareGame.h"
#include "FlareCompany.h"
#include "FlareSector.h"
#include "FlareGameUserSettings.h"
#include "../Player/FlarePlayerController.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"
#include "AI/FlareCompanyAI.h"


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

	CompanyAI = NewObject<UFlareCompanyAI>(this, UFlareCompanyAI::StaticClass());
	CompanyAI->Load(this, CompanyData.AI);


	// Load ships
	for (int i = 0 ; i < CompanyData.ShipData.Num(); i++)
	{
		LoadSpacecraft(CompanyData.ShipData[i]);
	}

	// Load stations
	for (int i = 0 ; i < CompanyData.StationData.Num(); i++)
	{
		LoadSpacecraft(CompanyData.StationData[i]);
	}

	// Load all fleets
	for (int32 i = 0; i < CompanyData.Fleets.Num(); i++)
	{
		LoadFleet(CompanyData.Fleets[i]);
	}



	// Load emblem
	SetupEmblem();
}

void UFlareCompany::PostLoad()
{
	VisitedSectors.Empty();
	KnownSectors.Empty();
	CompanyTradeRoutes.Empty();

	// Load all trade routes
	for (int32 i = 0; i < CompanyData.TradeRoutes.Num(); i++)
	{
		LoadTradeRoute(CompanyData.TradeRoutes[i]);
	}

	// Load sector knowledge
	for (int32 i = 0; i < CompanyData.SectorsKnowledge.Num(); i++)
	{
		UFlareSimulatedSector* Sector = GetGame()->GetGameWorld()->FindSector(CompanyData.SectorsKnowledge[i].SectorIdentifier);
		if (Sector)
		{
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
		else
		{
			FLOGV("Fail to find known sector '%s'. Ignore it.", *CompanyData.SectorsKnowledge[i].SectorIdentifier.ToString());
		}
	}
}

FFlareCompanySave* UFlareCompany::Save()
{
	CompanyData.Fleets.Empty();
	CompanyData.TradeRoutes.Empty();
	CompanyData.ShipData.Empty();
	CompanyData.StationData.Empty();
	CompanyData.SectorsKnowledge.Empty();

	for (int i = 0 ; i < CompanyFleets.Num(); i++)
	{
		CompanyData.Fleets.Add(*CompanyFleets[i]->Save());
	}

	for (int i = 0 ; i < CompanyTradeRoutes.Num(); i++)
	{
		CompanyData.TradeRoutes.Add(*CompanyTradeRoutes[i]->Save());
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
		if (!VisitedSectors.Contains(KnownSectors[i]))
		{
			FFlareCompanySectorKnowledge SectorKnowledge;
			SectorKnowledge.Knowledge = EFlareSectorKnowledge::Known;
			SectorKnowledge.SectorIdentifier = KnownSectors[i]->GetIdentifier();

			CompanyData.SectorsKnowledge.Add(SectorKnowledge);
		}
	}

	CompanyData.CompanyValue = GetCompanyValue().TotalValue;

	return &CompanyData;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareCompany::SimulateAI()
{
	CompanyAI->Simulate();
}

void UFlareCompany::TickAI()
{
	CompanyAI->Tick();
}


EFlareHostility::Type UFlareCompany::GetPlayerHostility() const
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(Game->GetWorld()->GetFirstPlayerController());

	if (PC)
	{
		return GetHostility(PC->GetCompany());
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

EFlareHostility::Type UFlareCompany::GetPlayerWarState() const
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(Game->GetWorld()->GetFirstPlayerController());

	if (PC)
	{
		return GetWarState(PC->GetCompany());
	}

	return EFlareHostility::Neutral;
}

EFlareHostility::Type UFlareCompany::GetWarState(const UFlareCompany* TargetCompany) const
{
	if (TargetCompany == this)
	{
		return EFlareHostility::Owned;
	}
	else if (GetHostility(TargetCompany) == EFlareHostility::Hostile || TargetCompany->GetHostility(this) == EFlareHostility::Hostile)
	{
		return EFlareHostility::Hostile;
	}

	return GetHostility(TargetCompany);
}

void UFlareCompany::SetHostilityTo(UFlareCompany* TargetCompany, bool Hostile)
{
	if (TargetCompany && TargetCompany != this)
	{
		bool WasHostile = CompanyData.HostileCompanies.Contains(TargetCompany->GetIdentifier());
		if (Hostile && !WasHostile)
		{
			CompanyData.HostileCompanies.AddUnique(TargetCompany->GetIdentifier());
			TargetCompany->GiveReputation(this, -50, true);
		}
		else if(!Hostile && WasHostile)
		{
			CompanyData.HostileCompanies.Remove(TargetCompany->GetIdentifier());
			TargetCompany->GiveReputation(this, 20, true);
		}
	}
}

FText UFlareCompany::GetShortInfoText()
{
	// Static text
	FText ShipText = LOCTEXT("Ship", "ship");
	FText ShipsText = LOCTEXT("Ships", "ships");

	// Build
	int32 ShipCount = GetCompanyShips().Num();
	FText ShipDescriptionText = FText::Format(LOCTEXT("ShipDescriptionFormat", "{0} {1}"), FText::AsNumber(ShipCount), ShipCount != 1 ? ShipsText : ShipText);
	return FText::Format(LOCTEXT("ShortInfoFormat", "{0} ({1} credits, {2})"), GetCompanyName(), FText::AsNumber(UFlareGameTools::DisplayMoney(GetMoney())), ShipDescriptionText);
}

UFlareFleet* UFlareCompany::CreateFleet(FText FleetName, UFlareSimulatedSector* FleetSector)
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

UFlareFleet* UFlareCompany::CreateAutomaticFleet(UFlareSimulatedSpacecraft* Spacecraft)
{
	FText FleetName;
	int32 FleetIndex = 1;

	while(true)
	{
		if (Spacecraft->IsMilitary())
		{
			if (Spacecraft->GetDescription()->Size == EFlarePartSize::L)
			{
				FleetName = FText::Format(LOCTEXT("CombatFleetFormat", "Combat Fleet {0}"),
					FText::FromString(AFlareGame::ConvertToRoman(FleetIndex)));
			}
			else
			{
				FleetName = FText::Format(LOCTEXT("WarFleetFormat", "War Fleet {0}"),
					FText::FromString(AFlareGame::ConvertToRoman(FleetIndex)));
			}
		}
		else
		{
			FleetName = FText::Format(LOCTEXT("CivilianFleetFormat", "Trade Fleet {0}"),
				FText::AsNumber(FleetIndex));
		}

		// Check duplicate
		bool Duplicate = false;
		for (int i = 0 ; i < CompanyFleets.Num(); i++)
		{
			if (FleetName.CompareTo(CompanyFleets[i]->GetFleetName()) == 0)
			{
				Duplicate = true;
				break;
			}
		}

		if (Duplicate)
		{
			FleetIndex++;
		}
		else
		{
			break;
		}
	}


	UFlareFleet* NewFleet = CreateFleet(FleetName, Spacecraft->GetCurrentSector());
	NewFleet->AddShip(Spacecraft);

	return NewFleet;
}

UFlareFleet* UFlareCompany::LoadFleet(const FFlareFleetSave& FleetData)
{
	UFlareFleet* Fleet = NULL;

	// Create the new travel
	Fleet = NewObject<UFlareFleet>(this, UFlareFleet::StaticClass());
	Fleet->Load(FleetData);
	CompanyFleets.AddUnique(Fleet);

	FLOGV("UFlareWorld::LoadFleet : loaded fleet '%s'", *Fleet->GetFleetName().ToString());

	return Fleet;
}

void UFlareCompany::RemoveFleet(UFlareFleet* Fleet)
{
	CompanyFleets.Remove(Fleet);
}

UFlareTradeRoute* UFlareCompany::CreateTradeRoute(FText TradeRouteName)
{
	// Create the trade route
	FFlareTradeRouteSave TradeRouteData;
	TradeRouteData.Identifier = FName(*(GetIdentifier().ToString() + "-" + FString::FromInt(CompanyData.TradeRouteImmatriculationIndex++)));
	TradeRouteData.Name = TradeRouteName;
	TradeRouteData.TargetSectorIdentifier = NAME_None;
	TradeRouteData.CurrentOperationIndex = 0;
	TradeRouteData.CurrentOperationProgress = 0;
	TradeRouteData.CurrentOperationDuration = 0;
	TradeRouteData.IsPaused = false;

	UFlareTradeRoute* TradeRoute = LoadTradeRoute(TradeRouteData);
	return TradeRoute;
}


UFlareTradeRoute* UFlareCompany::LoadTradeRoute(const FFlareTradeRouteSave& TradeRouteData)
{
	UFlareTradeRoute* TradeRoute = NULL;

	// Create the new travel
	TradeRoute = NewObject<UFlareTradeRoute>(this, UFlareTradeRoute::StaticClass());
	TradeRoute->Load(TradeRouteData);
	CompanyTradeRoutes.AddUnique(TradeRoute);

	FLOGV("UFlareCompany::LoadTradeRoute : loaded trade route '%s'", *TradeRoute->GetTradeRouteName().ToString());

	return TradeRoute;
}

void UFlareCompany::RemoveTradeRoute(UFlareTradeRoute* TradeRoute)
{
	CompanyTradeRoutes.Remove(TradeRoute);
}

UFlareSimulatedSpacecraft* UFlareCompany::LoadSpacecraft(const FFlareSpacecraftSave& SpacecraftData)
{
	UFlareSimulatedSpacecraft* Spacecraft = NULL;
	FLOGV("UFlareCompany::LoadSpacecraft ('%s')", *SpacecraftData.Immatriculation.ToString());

	FFlareSpacecraftDescription* Desc = Game->GetSpacecraftCatalog()->Get(SpacecraftData.Identifier);
	if (Desc)
	{
		Spacecraft = NewObject<UFlareSimulatedSpacecraft>(this, UFlareSimulatedSpacecraft::StaticClass());

		if(FindSpacecraft(SpacecraftData.Immatriculation))
		{
			FFlareSpacecraftSave FixedSpacecraftData = SpacecraftData;
			Game->Immatriculate(this, SpacecraftData.Identifier, &FixedSpacecraftData);
			FLOGV("WARNING : Double immatriculation, fix that. New immatriculation is %s", *FixedSpacecraftData.Immatriculation.ToString());
			Spacecraft->Load(FixedSpacecraftData);
		}
		else
		{
			Spacecraft->Load(SpacecraftData);
		}


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
	if (Spacecraft->GetCurrentFleet())
	{
		Spacecraft->GetCurrentFleet()->RemoveShip(Spacecraft, true);
	}

	if (Spacecraft->GetCurrentSector())
	{
		Spacecraft->GetCurrentSector()->RemoveSpacecraft(Spacecraft);
	}
	GetGame()->GetGameWorld()->ClearFactories(Spacecraft);
	CompanyAI->DestroySpacecraft(Spacecraft);
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

bool UFlareCompany::TakeMoney(int64 Amount, bool AllowDepts)
{
	if (Amount < 0 || (Amount > CompanyData.Money && !AllowDepts))
	{
		FLOGV("Fail to take %f money from %s (balance: %f)", Amount/100., *GetCompanyName().ToString(), CompanyData.Money/100.);
		return false;
	}
	else
	{
		CompanyData.Money -= Amount;
		/*if (Amount > 0)
		{

			FLOGV("$ %s - %lld -> %llu", *GetCompanyName().ToString(), Amount, CompanyData.Money);
		}*/
		return true;
	}
}

void UFlareCompany::GiveMoney(int64 Amount)
{
	if (Amount < 0)
	{
		FLOGV("Fail to give %f money from %s (balance: %f)", Amount/100., *GetCompanyName().ToString(), CompanyData.Money/100.);
		return;
	}

	CompanyData.Money += Amount;
	/*if (Amount > 0)
	{
		FLOGV("$ %s + %lld -> %llu", *GetCompanyName().ToString(), Amount, CompanyData.Money);
	}*/
}

#define REPUTATION_RANGE 200.f
void UFlareCompany::GiveReputation(UFlareCompany* Company, float Amount, bool Propagate)
{
	FFlareCompanyReputationSave* CompanyReputation = NULL;

	if (Company == this)
	{
		FLOG("ERROR: A company don't have reputation for itself!");
		return;
	}

	for (int32 CompanyIndex = 0; CompanyIndex < CompanyData.CompaniesReputation.Num(); CompanyIndex++)
	{
		if(Company->GetIdentifier() == CompanyData.CompaniesReputation[CompanyIndex].CompanyIdentifier)
		{
			CompanyReputation = &CompanyData.CompaniesReputation[CompanyIndex];
			break;
		}
	}

	if (CompanyReputation == NULL)
	{
		FFlareCompanyReputationSave NewCompanyReputation;
		NewCompanyReputation.CompanyIdentifier = Company->GetIdentifier();
		NewCompanyReputation.Reputation = 0;
		CompanyData.CompaniesReputation.Add(NewCompanyReputation);
		CompanyReputation = &CompanyData.CompaniesReputation[CompanyData.CompaniesReputation.Num()-1];
	}

	// Gain reputation is easier with low reputation and loose reputation is easier with hight reputation.
	// Reputation vary between -200 and 200
	// 0% if reputation in variation direction = 200
	// 10% if reputation in variation direction = 100
	// 100% if reputation in variation direction = 0
	// 200% if reputation in variation direction = -100
	// 1000% if reputation in variation direction = -200

	// -200 = 0, 200 = 1
	float ReputationRatioInVarationDirection = (CompanyReputation->Reputation * FMath::Sign(Amount) + REPUTATION_RANGE) / (2*REPUTATION_RANGE);
	float ReputationGainFactor = 1.f;

	if (ReputationRatioInVarationDirection < 0.25f)
	{
		ReputationGainFactor = - 32.f * ReputationRatioInVarationDirection + 10.f;
	}
	else if (ReputationRatioInVarationDirection < 0.50f)
	{
		ReputationGainFactor = - 4.f * ReputationRatioInVarationDirection + 3.f;
	}
	else if (ReputationRatioInVarationDirection < 0.75f)
	{
		ReputationGainFactor = - 3.6f * ReputationRatioInVarationDirection + 2.8f;
	}
	else
	{
		ReputationGainFactor = - 0.4f * ReputationRatioInVarationDirection + 0.4f;
	}

	float ReputationScaledGain = Amount * ReputationGainFactor;

	CompanyReputation->Reputation = FMath::Clamp(CompanyReputation->Reputation + ReputationScaledGain, -200.f, 200.f);
	if (FMath::Abs(CompanyReputation->Reputation) < 1.f)
	{
		CompanyReputation->Reputation = 0.f;
	}

	if (Propagate)
	{
		// Other companies gain a part of reputation gain according to their affinity :
		// 200 = 50 % of the gain
		// -200 = -50 % of the gain
		// 0 = 0% the the gain


		for (int32 CompanyIndex = 0; CompanyIndex < Game->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
		{
			UFlareCompany* OtherCompany = Game->GetGameWorld()->GetCompanies()[CompanyIndex];

			if (OtherCompany == Company || OtherCompany == this)
			{
				continue;
			}

			float OtherReputation = OtherCompany->GetReputation(Company);
			float PropagationRatio = OtherReputation / 400.f;
			OtherCompany->GiveReputation(Company, PropagationRatio * ReputationScaledGain, false);
		}


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

	// Update the emblem
	SetupEmblem();
}

void UFlareCompany::CustomizeComponentMaterial(UMaterialInstanceDynamic* Mat)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(Game->GetWorld()->GetFirstPlayerController());
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());

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
	Mat->SetScalarParameterValue("IsPainted", 1);
	Mat->SetScalarParameterValue("TessellationMultiplier", 0.0);
}

void UFlareCompany::CustomizeEffectMaterial(UMaterialInstanceDynamic* Mat)
{
}

FLinearColor UFlareCompany::NormalizeColor(FLinearColor Col) const
{
	return FLinearColor(FVector(Col.R, Col.G, Col.B) / Col.GetLuminance());
}

void UFlareCompany::SetupEmblem()
{
	// Create the parameter
	FVector2D EmblemSize = 128 * FVector2D::UnitVector;
	UMaterial* BaseEmblemMaterial = Cast<UMaterial>(FFlareStyleSet::GetIcon("CompanyEmblem")->GetResourceObject());
	CompanyEmblem = UMaterialInstanceDynamic::Create(BaseEmblemMaterial, GetWorld());
	UFlareCustomizationCatalog* Catalog = Game->GetCustomizationCatalog();

	// Setup the material
	CompanyEmblem->SetTextureParameterValue("Emblem", CompanyDescription->Emblem);
	CompanyEmblem->SetVectorParameterValue("BasePaintColor", Catalog->GetColor(CompanyDescription->CustomizationBasePaintColorIndex));
	CompanyEmblem->SetVectorParameterValue("PaintColor", Catalog->GetColor(CompanyDescription->CustomizationPaintColorIndex));
	CompanyEmblem->SetVectorParameterValue("OverlayColor", Catalog->GetColor(CompanyDescription->CustomizationOverlayColorIndex));
	CompanyEmblem->SetVectorParameterValue("GlowColor", Catalog->GetColor(CompanyDescription->CustomizationLightColorIndex));

	// Create the brush dynamically
	CompanyEmblemBrush.ImageSize = EmblemSize;
	CompanyEmblemBrush.SetResourceObject(CompanyEmblem);
}

const FSlateBrush* UFlareCompany::GetEmblem() const
{
	return &CompanyEmblemBrush;
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

struct CompanyValue UFlareCompany::GetCompanyValue() const
{
	// Company value is the sum of :
	// - money
	// - value of its spacecraft
	// - value of the stock in these spacecraft
	// - value of the resources used in factory

	struct CompanyValue Value;
	Value.MoneyValue = GetMoney();
	Value.StockValue = 0;
	Value.ShipsValue = 0;
	Value.StationsValue = 0;

	for (int SpacecraftIndex = 0; SpacecraftIndex < CompanySpacecrafts.Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = CompanySpacecrafts[SpacecraftIndex];

		UFlareSimulatedSector *ReferenceSector =  Spacecraft->GetCurrentSector();

		if (!ReferenceSector)
		{
			if (Spacecraft->GetCurrentFleet() && Spacecraft->GetCurrentFleet()->GetCurrentTravel())
			{
				ReferenceSector = Spacecraft->GetCurrentFleet()->GetCurrentTravel()->GetDestinationSector();
			}
			else
			{
				FLOGV("Spacecraft %s is lost : no current sector, no travel", *Spacecraft->GetImmatriculation().ToString());
				continue;
			}

		}

		// Value of the spacecraft
		int64 SpacecraftPrice = UFlareGameTools::ComputeShipPrice(Spacecraft->GetDescription()->Identifier, ReferenceSector, true);

		if(Spacecraft->IsStation())
		{
			Value.StationsValue += SpacecraftPrice;
		}
		else
		{
			Value.ShipsValue += SpacecraftPrice;
		}

		// Value of the stock
		TArray<FFlareCargo>& CargoBaySlots = Spacecraft->GetCargoBay()->GetSlots();
		for (int CargoIndex = 0; CargoIndex < CargoBaySlots.Num(); CargoIndex++)
		{
			FFlareCargo& Cargo = CargoBaySlots[CargoIndex];

			if (!Cargo.Resource)
			{
				continue;
			}

			Value.StockValue += ReferenceSector->GetResourcePrice(Cargo.Resource, EFlareResourcePriceContext::Default) * Cargo.Quantity;
		}

		// Value of factory stock
		for (int32 FactoryIndex = 0; FactoryIndex < Spacecraft->GetFactories().Num(); FactoryIndex++)
		{
			UFlareFactory* Factory = Spacecraft->GetFactories()[FactoryIndex];



			for (int32 ReservedResourceIndex = 0 ; ReservedResourceIndex < Factory->GetReservedResources().Num(); ReservedResourceIndex++)
			{
				FName ResourceIdentifier = Factory->GetReservedResources()[ReservedResourceIndex].ResourceIdentifier;
				uint32 Quantity = Factory->GetReservedResources()[ReservedResourceIndex].Quantity;

				FFlareResourceDescription* Resource = Game->GetResourceCatalog()->Get(ResourceIdentifier);
				if (Resource)
				{
					Value.StockValue += ReferenceSector->GetResourcePrice(Resource, EFlareResourcePriceContext::Default) * Quantity;
				}
				else
				{
					FLOGV("WARNING: Invalid reserved resource %s (%d reserved) for %s)", *ResourceIdentifier.ToString(), Quantity, *Spacecraft->GetImmatriculation().ToString())
				}
			}
		}
	}

	Value.SpacecraftsValue = Value.ShipsValue + Value.StationsValue;
	Value.TotalValue = Value.MoneyValue + Value.StockValue + Value.SpacecraftsValue;

	return Value;
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
	return Sector && VisitedSectors.Contains(Sector);
}

float UFlareCompany::GetReputation(UFlareCompany* Company)
{
	for (int32 CompanyIndex = 0; CompanyIndex < CompanyData.CompaniesReputation.Num(); CompanyIndex++)
	{
		if(Company->GetIdentifier() == CompanyData.CompaniesReputation[CompanyIndex].CompanyIdentifier)
		{
			return CompanyData.CompaniesReputation[CompanyIndex].Reputation;
		}
	}

	return 0;
}

FText UFlareCompany::GetPlayerHostilityText() const
{
	FText Status;

	switch (GetPlayerWarState())
	{
		case EFlareHostility::Neutral:
			Status = LOCTEXT("Neutral", "Neutral");
			break;
		case EFlareHostility::Friendly:
			Status = LOCTEXT("Friendly", "Friendly");
			break;
		case EFlareHostility::Owned:
			Status = LOCTEXT("Owned", "Owned");
			break;
		case EFlareHostility::Hostile:
			Status = LOCTEXT("Hostile", "Hostile");
			break;
	}

	return Status;
}

#undef LOCTEXT_NAMESPACE
