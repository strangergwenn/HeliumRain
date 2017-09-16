
#include "FlareCompany.h"
#include "Flare.h"
#include "FlareGame.h"
#include "FlareGameTools.h"
#include "FlareSector.h"
#include "FlareGameUserSettings.h"
#include "FlareScenarioTools.h"

#include "../Economy/FlareCargoBay.h"
#include "../Economy/FlareFactory.h"

#include "../Data/FlareResourceCatalog.h"
#include "../Data/FlareSpacecraftCatalog.h"
#include "../Data/FlareTechnologyCatalog.h"

#include "../Player/FlarePlayerController.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"

#include "AI/FlareCompanyAI.h"
#include "AI/FlareAIBehavior.h"


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

	// Player description ID is -1
	if (Data.CatalogIdentifier >= 0)
	{
		CompanyDescription = GetGame()->GetCompanyDescription(Data.CatalogIdentifier);
	}
	else
	{
		CompanyDescription = GetGame()->GetPlayerCompanyDescription();
	}

	// Spawn AI
	CompanyAI = NewObject<UFlareCompanyAI>(this, UFlareCompanyAI::StaticClass());
	FCHECK(CompanyAI);

	// Spawn tactic manager
	TacticManager = NewObject<UFlareTacticManager>(this, UFlareTacticManager::StaticClass());
	FCHECK(TacticManager);
	TacticManager->Load(this);

	// Load technologies
	for (int i = 0; i < CompanyData.UnlockedTechnologies.Num(); i++)
	{
		UnlockTechnology(CompanyData.UnlockedTechnologies[i], true);
	}	

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

	// Load destroyed spacecraft
	for (int i = 0 ; i < CompanyData.DestroyedSpacecraftData.Num(); i++)
	{
		LoadSpacecraft(CompanyData.DestroyedSpacecraftData[i]);
	}

	// Load all fleets
	for (int32 i = 0; i < CompanyData.Fleets.Num(); i++)
	{
		LoadFleet(CompanyData.Fleets[i]);
	}

	// Load emblem
	SetupEmblem();

	InvalidateCompanyValueCache();
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

	CompanyAI->Load(this, CompanyData.AI);

}

FFlareCompanySave* UFlareCompany::Save()
{
	CompanyData.Fleets.Empty();
	CompanyData.TradeRoutes.Empty();
	CompanyData.ShipData.Empty();
	CompanyData.StationData.Empty();
	CompanyData.DestroyedSpacecraftData.Empty();
	CompanyData.SectorsKnowledge.Empty();
	CompanyData.UnlockedTechnologies.Empty();

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

	for (int i = 0 ; i < CompanyDestroyedSpacecrafts.Num(); i++)
	{
		CompanyData.DestroyedSpacecraftData.Add(*CompanyDestroyedSpacecrafts[i]->Save());
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

	for (auto& Technology : UnlockedTechnologies)
	{
		CompanyData.UnlockedTechnologies.Add(Technology.Key);
	}

	CompanyData.CompanyValue = GetCompanyValue().TotalValue;

	CompanyData.AI = *CompanyAI->Save();

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

bool UFlareCompany::IsAtWar(const UFlareCompany* TargetCompany) const
{
	return GetWarState(TargetCompany) == EFlareHostility::Hostile;
}

void UFlareCompany::ClearLastWarDate()
{
	CompanyData.PlayerLastWarDate = 0;
}

void UFlareCompany::SetLastWarDate()
{
	CompanyData.PlayerLastWarDate = Game->GetGameWorld()->GetDate();
}

void UFlareCompany::ResetLastPeaceDate()
{
	CompanyData.PlayerLastPeaceDate = Game->GetGameWorld()->GetDate();
}

void UFlareCompany::ResetLastTributeDate()
{
	CompanyData.PlayerLastTributeDate = Game->GetGameWorld()->GetDate();
}

void UFlareCompany::SetHostilityTo(UFlareCompany* TargetCompany, bool Hostile)
{
	if (TargetCompany && TargetCompany != this)
	{
		bool WasHostile = CompanyData.HostileCompanies.Contains(TargetCompany->GetIdentifier());
		if (Hostile && !WasHostile)
		{
			CompanyData.HostileCompanies.AddUnique(TargetCompany->GetIdentifier());


			UFlareCompany* PlayerCompany = Game->GetPC()->GetCompany();
			if (TargetCompany == PlayerCompany)
			{
				if (PlayerCompany->GetHostility(this) != EFlareHostility::Hostile)
				{
					FString UniqueId = "war-declared-" + GetIdentifier().ToString();
					FFlareMenuParameterData Data;
					Game->GetPC()->Notify(LOCTEXT("CompanyDeclareWar", "War declared"),
						FText::Format(LOCTEXT("CompanyDeclareWarFormat", "{0} declared war on you"), FText::FromString(GetCompanyName().ToString())),
						FName(*UniqueId),
						EFlareNotification::NT_Military,
						false,
						EFlareMenu::MENU_Leaderboard,
						Data);
				}

				PlayerCompany->SetHostilityTo(this, true);
			}

			if (this == PlayerCompany && TargetCompany->GetHostility(this) != EFlareHostility::Hostile)
			{
				TargetCompany->GivePlayerReputation(-50);
				TargetCompany->GetAI()->GetData()->Pacifism = FMath::Min(50.f, TargetCompany->GetAI()->GetData()->Pacifism);

				TargetCompany->SetLastWarDate();
			}

			Game->GetQuestManager()->OnWarStateChanged(this, TargetCompany);
		}
		else if(!Hostile && WasHostile)
		{
			CompanyData.HostileCompanies.Remove(TargetCompany->GetIdentifier());

			UFlareCompany* PlayerCompany = Game->GetPC()->GetCompany();

			if(this == PlayerCompany)
			{
				TargetCompany->ResetLastPeaceDate();
			}

			if(TargetCompany == PlayerCompany)
			{
				if(PlayerCompany->GetHostility(this) == EFlareHostility::Hostile)
				{
					FFlareMenuParameterData Data;
					Game->GetPC()->Notify(LOCTEXT("CompanyWantPeace", "Peace proposed"),
						FText::Format(LOCTEXT("CompanyWantPeaceFormat", "{0} is offering peace with you"), FText::FromString(GetCompanyName().ToString())),
						FName("peace-proposed"),
						EFlareNotification::NT_Military,
						false,
						EFlareMenu::MENU_Leaderboard,
						Data);
				}
				else
				{
					FFlareMenuParameterData Data;
					Game->GetPC()->Notify(LOCTEXT("CompanyAcceptPeace", "Peace accepted"),
						FText::Format(LOCTEXT("CompanyAcceptPeaceFormat", "{0} accepted to make peace with you"), FText::FromString(GetCompanyName().ToString())),
						FName("peace-accepted"),
						EFlareNotification::NT_Military,
						false,
						EFlareMenu::MENU_Leaderboard,
						Data);
				}

				ClearLastWarDate();
			}
			Game->GetQuestManager()->OnWarStateChanged(this, TargetCompany);
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

inline static bool SortByColor(const FLinearColor& A, const FLinearColor& B)
{
	return (A.LinearRGBToHSV().R < B.LinearRGBToHSV().R);
}

FLinearColor UFlareCompany::PickFleetColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	TArray<FLinearColor> FleetColors;
	FleetColors.Add(Theme.FriendlyColor);
	for (int32 i = 0; i < 7; ++i)
	{
		FleetColors.Add(FLinearColor((i % 6) * 60.f, 1.f, 1.f).HSVToLinearRGB());
	}
	
	return FleetColors[(CompanyFleets.Num() - 1) % (FleetColors.Num() - 1)];
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
	NewFleet->SetFleetColor(PickFleetColor());

	return NewFleet;
}

UFlareFleet* UFlareCompany::LoadFleet(const FFlareFleetSave& FleetData)
{
	UFlareFleet* Fleet = NULL;

	// Create the new travel
	Fleet = NewObject<UFlareFleet>(this, UFlareFleet::StaticClass());
	Fleet->Load(FleetData);
	CompanyFleets.AddUnique(Fleet);

	//FLOGV("UFlareWorld::LoadFleet : loaded fleet '%s'", *Fleet->GetFleetName().ToString());

	return Fleet;
}

void UFlareCompany::RemoveFleet(UFlareFleet* Fleet)
{
	CompanyFleets.Remove(Fleet);
}

void UFlareCompany::MoveFleetUp(UFlareFleet* Fleet)
{
	int32 Index = CompanyFleets.IndexOfByKey(Fleet);

	if(Index != INDEX_NONE && Index > 0)
	{
		int32 SwapIndex = Index - 1;
		UFlareFleet* SwapFleet = CompanyFleets[SwapIndex];

		 CompanyFleets[SwapIndex] = Fleet;
		 CompanyFleets[Index] = SwapFleet;
	}
}

void UFlareCompany::MoveFleetDown(UFlareFleet* Fleet)
{
	int32 Index = CompanyFleets.IndexOfByKey(Fleet);

	if(Index != INDEX_NONE && Index < CompanyFleets.Num() - 1)
	{
		int32 SwapIndex = Index + 1;
		UFlareFleet* SwapFleet = CompanyFleets[SwapIndex];

		 CompanyFleets[SwapIndex] = Fleet;
		 CompanyFleets[Index] = SwapFleet;
	}
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
	TradeRoute->ResetStats();
	return TradeRoute;
}


UFlareTradeRoute* UFlareCompany::LoadTradeRoute(const FFlareTradeRouteSave& TradeRouteData)
{
	UFlareTradeRoute* TradeRoute = NULL;

	// Create the new travel
	TradeRoute = NewObject<UFlareTradeRoute>(this, UFlareTradeRoute::StaticClass());
	TradeRoute->Load(TradeRouteData);
	CompanyTradeRoutes.AddUnique(TradeRoute);

	//FLOGV("UFlareCompany::LoadTradeRoute : loaded trade route '%s'", *TradeRoute->GetTradeRouteName().ToString());

	return TradeRoute;
}

void UFlareCompany::RemoveTradeRoute(UFlareTradeRoute* TradeRoute)
{
	CompanyTradeRoutes.Remove(TradeRoute);
}

UFlareSimulatedSpacecraft* UFlareCompany::LoadSpacecraft(const FFlareSpacecraftSave& SpacecraftData)
{
	UFlareSimulatedSpacecraft* Spacecraft = NULL;
	//FLOGV("UFlareCompany::LoadSpacecraft ('%s')", *SpacecraftData.Immatriculation.ToString());

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

		if(Spacecraft->IsDestroyed())
		{
			CompanyDestroyedSpacecrafts.AddUnique(Spacecraft);
		}
		else
		{
			if (Spacecraft->IsStation())
			{
				CompanyStations.AddUnique((Spacecraft));
			}
			else
			{
				CompanyShips.AddUnique((Spacecraft));
			}

			CompanySpacecrafts.AddUnique((Spacecraft));
		}
	}
	else
	{
		FLOG("UFlareCompany::LoadSpacecraft : Failed (no description available)");
	}

	return Spacecraft;
}

void UFlareCompany::DestroySpacecraft(UFlareSimulatedSpacecraft* Spacecraft)
{
	FLOGV("UFlareCompany::DestroySpacecraft : Remove %s from company %s", *Spacecraft->GetImmatriculation().ToString(), *GetCompanyName().ToString());

	Spacecraft->ResetCapture();

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
	Spacecraft->SetDestroyed(true);

	CompanyDestroyedSpacecrafts.Add(Spacecraft);
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
		FLOGV("UFlareCompany::TakeMoney : Failed to take %f money from %s (balance: %f)",
			Amount/100., *GetCompanyName().ToString(), CompanyData.Money/100.);
		return false;
	}
	else
	{
		CompanyData.Money -= Amount;
		/*if (Amount > 0)
		{

			FLOGV("$ %s - %lld -> %llu", *GetCompanyName().ToString(), Amount, CompanyData.Money);
		}*/

		InvalidateCompanyValueCache();

		return true;
	}
}

void UFlareCompany::GiveMoney(int64 Amount)
{
	if (Amount < 0)
	{
		FLOGV("UFlareCompany::GiveMoney : Failed to give %f money from %s (balance: %f)",
			Amount/100., *GetCompanyName().ToString(), CompanyData.Money/100.);
		return;
	}

	CompanyData.Money += Amount;

	if (this == Game->GetPC()->GetCompany() && GetGame()->GetQuestManager())
	{
		GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("gain-money").PutInt32("amount", Amount));
	}

	InvalidateCompanyValueCache();

	/*if (Amount > 0)
	{
		FLOGV("$ %s + %lld -> %llu", *GetCompanyName().ToString(), Amount, CompanyData.Money);
	}*/
}

void UFlareCompany::GiveResearch(int64 Amount)
{
	if (Amount < 0)
	{
		FLOGV("UFlareCompany::GiveMoney : Failed to give %d research from %s (balance: %d)", Amount, *GetCompanyName().ToString(), CompanyData.ResearchAmount);
		return;
	}

	// Instruments technology
	if (IsTechnologyUnlocked("instruments"))
	{
		Amount *= 1.5;
	}

	CompanyData.ResearchAmount += Amount;


	if (this == Game->GetPC()->GetCompany() && GetGame()->GetQuestManager())
	{
		GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("gain-research").PutInt32("amount", Amount));
	}
}

void UFlareCompany::GivePlayerReputation(float Amount, float Max)
{
	if(Max < -100)
	{
		CompanyData.PlayerReputation = FMath::Max(-100.f, CompanyData.PlayerReputation + Amount);
	}
	else
	{
		// Clamp
		float MaxReputation = FMath::Max(Max, CompanyData.PlayerReputation);

		CompanyData.PlayerReputation = FMath::Max(-100.f, CompanyData.PlayerReputation + Amount);
		CompanyData.PlayerReputation = FMath::Min(MaxReputation, CompanyData.PlayerReputation);
	}
}

void UFlareCompany::GivePlayerReputationToOthers(float Amount)
{
	for (int32 CompanyIndex = 0; CompanyIndex < Game->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
	{
		UFlareCompany* OtherCompany = Game->GetGameWorld()->GetCompanies()[CompanyIndex];

		if (OtherCompany != this)
		{
			OtherCompany->GivePlayerReputation(Amount);
		}
	}
}


float UFlareCompany::ComputeCompanyDiplomaticWeight(bool WithShame)
{
	CompanyValue Value = GetCompanyValue();

	return (Value.TotalValue + Value.ArmyCurrentCombatPoints * 10) * (WithShame ? FMath::Pow(1.5f, GetShame()) : 1.f);
}

bool UFlareCompany::WantWarWith(UFlareCompany* TargetCompany)
{
	if(this == Game->GetScenarioTools()->AxisSupplies || TargetCompany == Game->GetScenarioTools()->AxisSupplies)
	{
		return false;
	}

	UFlareCompany* PlayerCompany = Game->GetPC()->GetCompany();

	if(TargetCompany == PlayerCompany)
	{
		if(GetPlayerReputation() <= 0)
		{
			return true;
		}
	}
	else if (PlayerCompany->GetCompanyValue().ArmyCurrentCombatPoints < Game->GetGameWorld()->GetTotalWorldCombatPoint() / 4)
	{
		float MyWeight = ComputeCompanyDiplomaticWeight(false);
		return TargetCompany->ComputeCompanyDiplomaticWeight(true) > MyWeight;
	}

	return false;
}

//#define DEBUG_CONFIDENCE
float UFlareCompany::GetConfidenceLevel(UFlareCompany* TargetCompany, TArray<UFlareCompany*>& Allies)
{
	// Confidence level go from 1 to -1
	// 1 if if the army value of the company and its potential allies is infinite compared to the opposent
	// -1 if if the army value of the company and its potential allies is zero compared to the opposent
	//
	// The enemies are people at war with me, the reference company if provide, people than want war with me but are not allied
	// The allies are all the people at war the reference company or that want war with Reference company

	Allies.Empty();

	// Find allies
	for (UFlareCompany* CompanyCandidate : TargetCompany->GetOtherCompanies())
	{
		if(IsAtWar(CompanyCandidate))
		{
			continue;
		}
		UFlareCompany* PlayerCompany = Game->GetPC()->GetCompany();


		if (CompanyCandidate == this)
		{
			Allies.Add(CompanyCandidate);
		}
		else if (CompanyCandidate->IsAtWar(TargetCompany))
		{
			Allies.Add(CompanyCandidate);
		}
		else if (CompanyCandidate != PlayerCompany && CompanyCandidate->GetAI()->GetData()->Pacifism == 0 && CompanyCandidate->GetWarCount(PlayerCompany) == 0)
		{
			// Open to war
#ifdef DEBUG_CONFIDENCE
			FLOGV("GetConfidenceLevel ally %s to %s : %s want war ? %d",
				  *GetCompanyName().ToString(),
				  *TargetCompany->GetCompanyName().ToString(),
				  * CompanyCandidate->GetCompanyName().ToString(),
				  CompanyCandidate->WantWarWith(TargetCompany));
#endif
			if (CompanyCandidate->WantWarWith(TargetCompany))
			{
					Allies.Add(CompanyCandidate);
			}
		}
	}

	TArray<UFlareCompany*> Enemies;
	UFlareCompany* PlayerCompany = Game->GetPC()->GetCompany();


	// Find enemies
	for (UFlareCompany* OtherCompany : GetOtherCompanies())
	{
		if (OtherCompany == TargetCompany)
		{
			Enemies.Add(TargetCompany);
			continue;
		}


		bool IsEnemy = false;

		if (IsAtWar(OtherCompany))
		{
#ifdef DEBUG_CONFIDENCE
			FLOGV("GetConfidenceLevel enemy %s to %s : %s at war ? %d",
				  *GetCompanyName().ToString(),
				  *TargetCompany->GetCompanyName().ToString(),
				  *OtherCompany->GetCompanyName().ToString(),
				  IsAtWar(OtherCompany));
#endif
			IsEnemy = true;
		}
		else if (OtherCompany != PlayerCompany && OtherCompany->GetAI()->GetData()->Pacifism == 0 && OtherCompany->GetWarCount(PlayerCompany) == 0)
		{
#ifdef DEBUG_CONFIDENCE
			FLOGV("GetConfidenceLevel enemy %s to %s : %s want war ? %d",
				  *GetCompanyName().ToString(),
				  *TargetCompany->GetCompanyName().ToString(),
				  *OtherCompany->GetCompanyName().ToString(),
				  OtherCompany->WantWarWith(this));
#endif
			if (OtherCompany->WantWarWith(this) && !Allies.Contains(OtherCompany))
			{
				IsEnemy = true;
			}
		}

		if (IsEnemy)
		{
			Enemies.Add(OtherCompany);
		}
	}

	// Compute army values
	int32 EnemiesArmyCombatPoints = 0;
	int32 AlliesArmyCombatPoints = 0;
#ifdef DEBUG_CONFIDENCE
	FLOGV("Compute confidence for %s (ref: %s)", *GetCompanyName().ToString(), (TargetCompany ? *TargetCompany->GetCompanyName().ToString(): *FString("none")));
#endif
	for (int32 EnemyIndex = 0; EnemyIndex < Enemies.Num(); EnemyIndex++)
	{
		UFlareCompany* EnemyCompany = Enemies[EnemyIndex];
		EnemiesArmyCombatPoints += EnemyCompany->GetCompanyValue().ArmyCurrentCombatPoints;
#ifdef DEBUG_CONFIDENCE
		FLOGV("- enemy: %s (%d)", *EnemyCompany->GetCompanyName().ToString(), EnemyCompany->GetCompanyValue().ArmyCurrentCombatPoints);
#endif
	}

	for (int32 AllyIndex = 0; AllyIndex < Allies.Num(); AllyIndex++)
	{
		UFlareCompany* AllyCompany = Allies[AllyIndex];

		// Allies can be enemy to only a part of my ennemie. Cap to the value of the enemy if not me
		int32 AllyArmyCombatPoints= AllyCompany->GetCompanyValue().ArmyCurrentCombatPoints;
#ifdef DEBUG_CONFIDENCE
		FLOGV("- ally: %s (%d)", *AllyCompany->GetCompanyName().ToString(), AllyArmyCombatPoints);
#endif
		AlliesArmyCombatPoints += AllyArmyCombatPoints;
	}
#ifdef EnemiesArmyCombatPoints
	FLOGV("EnemiesArmyCombatPoints=%d AlliesArmyCombatPoints=%d", EnemiesArmyCombatPoints, AlliesArmyCombatPoints);
#endif
	// Compute confidence
	if(AlliesArmyCombatPoints == EnemiesArmyCombatPoints)
	{
		return 0;
	}
	else if(AlliesArmyCombatPoints > EnemiesArmyCombatPoints)
	{
		if(EnemiesArmyCombatPoints == 0)
		{
			return 1;
		}

		float Ratio =  (float) AlliesArmyCombatPoints /  (float) EnemiesArmyCombatPoints;
		float Confidence = 1.f-1.f / (Ratio + 1);
#ifdef DEBUG_CONFIDENCE
	FLOGV("Ratio=%f Confidence=%f", Ratio, Confidence);
#endif
		return Confidence;
	}
	else
	{
		if(AlliesArmyCombatPoints == 0)
		{
			return -1;
		}

		float Ratio =  (float) EnemiesArmyCombatPoints /  (float) AlliesArmyCombatPoints;
		float Confidence = 1.f-1.f / (Ratio + 1);
#ifdef DEBUG_CONFIDENCE
	FLOGV("Ratio=%f Confidence=%f", Ratio, -Confidence);
#endif
		return -Confidence;
	}

}

bool UFlareCompany::AtWar()
{
	for (UFlareCompany* OtherCompany : Game->GetGameWorld()->GetCompanies())
	{
		if(OtherCompany == this)
		{
			continue;
		}

		if(GetWarState(OtherCompany) == EFlareHostility::Hostile)
		{
			return true;
		}
	}
	return false;
}

int32 UFlareCompany::GetTransportCapacity()
{
	int32 CompanyCapacity = 0;

	for(UFlareSimulatedSpacecraft* Ship : CompanyShips)
	{
		if(Ship->GetDamageSystem()->IsStranded())
		{
			continue;
		}

		CompanyCapacity += Ship->GetCargoBay()->GetCapacity();
	}

	return CompanyCapacity;
}

TArray<UFlareCompany*> UFlareCompany::GetOtherCompanies(bool Shuffle)
{
	// TODO Cache
	TArray<UFlareCompany*> OtherCompanies;
	for(UFlareCompany* Company : GetGame()->GetGameWorld()->GetCompanies())
	{
		if(Company !=  this)
		{
			OtherCompanies.Add(Company);
		}
	}

	if(Shuffle)
	{
		TArray<UFlareCompany*> ShuffleCompanies;
		while(OtherCompanies.Num())
		{
			int32 Index = FMath::RandRange(0, OtherCompanies.Num() - 1);
			ShuffleCompanies.Add(OtherCompanies[Index]);
			OtherCompanies.RemoveAt(Index);
		}
		return ShuffleCompanies;
	}
	else
	{
		return OtherCompanies;
	}
}

bool UFlareCompany::HasKnowResourceInput(FFlareResourceDescription* Resource)
{
	for(UFlareSimulatedSector* Sector : VisitedSectors)
	{
		for(UFlareSimulatedSpacecraft* Station : Sector->GetSectorStations())
		{
			EFlareResourcePriceContext::Type StationResourceUsage = Station->GetResourceUseType(Resource);

			if(StationResourceUsage != EFlareResourcePriceContext::FactoryInput &&
				StationResourceUsage != EFlareResourcePriceContext::ConsumerConsumption &&
				StationResourceUsage != EFlareResourcePriceContext::MaintenanceConsumption)
			{
				continue;
			}

			FLOGV("HasKnowResourceBuyer: %s want buy %s in %s",
				  *Station->GetImmatriculation().ToString(),
				  *Resource->Name.ToString(),
				  *Sector->GetSectorName().ToString())
			return true;
		}
	}

	return false;
}

bool UFlareCompany::HasKnowResourceOutput(FFlareResourceDescription* Resource)
{
	for(UFlareSimulatedSector* Sector : VisitedSectors)
	{
		for(UFlareSimulatedSpacecraft* Station : Sector->GetSectorStations())
		{
			EFlareResourcePriceContext::Type StationResourceUsage = Station->GetResourceUseType(Resource);

			if(StationResourceUsage != EFlareResourcePriceContext::FactoryOutput)
			{
				continue;
			}

			FLOGV("HasKnowResourceSeller: %s want sell %s in %s",
				  *Station->GetImmatriculation().ToString(),
				  *Resource->Name.ToString(),
				  *Sector->GetSectorName().ToString())
			return true;
		}
	}

	return false;
}

int32 UFlareCompany::GetWarCount(UFlareCompany* ExcludeCompany) const
{
	int32 WarCount = 0;

	for(UFlareCompany* OtherCompany : Game->GetGameWorld()->GetCompanies())
	{
		if(OtherCompany == this || OtherCompany == ExcludeCompany)
		{
			continue;
		}

		if (IsAtWar(OtherCompany))
		{
			++WarCount;
		}
	}

	return WarCount;
}

int64 UFlareCompany::GetTributeCost(UFlareCompany* Company)
{
	return 0.01 * GetCompanyValue().TotalValue + 0.1 * GetCompanyValue().MoneyValue;
}

void UFlareCompany::PayTribute(UFlareCompany* Company, bool AllowDepts)
{
	int64 Cost = GetTributeCost(Company);

	if (Cost <= GetMoney() || AllowDepts)
	{
		FLOGV("UFlareCompany::PayTribute: %s paying %ld to %s", *GetCompanyName().ToString(), Cost, *Company->GetCompanyName().ToString());

		// Exchange money
		TakeMoney(Cost, AllowDepts);
		Company->GiveMoney(Cost);

		// Reset pacifism
		GetAI()->GetData()->Pacifism = FMath::Max(50.f, GetAI()->GetData()->Pacifism);
		Company->GetAI()->GetData()->Pacifism = FMath::Max(50.f, Company->GetAI()->GetData()->Pacifism);


		// Reset hostilities
		SetHostilityTo(Company, false);
		Company->SetHostilityTo(this, false);

		if(Company == Game->GetPC()->GetCompany())
		{
			ResetLastTributeDate();
		}
	}
	else
	{
		FLOG("UFlareCompany::PayTribute: not enough money to pay tribute");
	}
}

void UFlareCompany::GiveShame(float ShameGain)
{
	CompanyData.Shame += ShameGain;
}

void UFlareCompany::StartCapture(UFlareSimulatedSpacecraft* Station)
{
	if(!CanStartCapture(Station))
	{
		return;
	}

	CompanyData.CaptureOrders.AddUnique(Station->GetImmatriculation());
}

void UFlareCompany::StopCapture(UFlareSimulatedSpacecraft* Station)
{

	CompanyData.CaptureOrders.Remove(Station->GetImmatriculation());
}

bool UFlareCompany::CanStartCapture(UFlareSimulatedSpacecraft* Station)
{

	if(CompanyData.CaptureOrders.Contains(Station->GetImmatriculation()))
	{
		return false;
	}

	int32 StationCount = Station->GetCurrentSector()->GetSectorCompanyStationCount(this, true);
	int32 MaxStationCount = IsTechnologyUnlocked("dense-sectors") ? Station->GetCurrentSector()->GetMaxStationsPerCompany() : Station->GetCurrentSector()->GetMaxStationsPerCompany() / 2;

	if(StationCount >= MaxStationCount)
	{

		return false;
	}

	if ((GetWarState(Station->GetCompany()) != EFlareHostility::Hostile)
		|| !Station->GetCurrentSector()->GetSectorBattleState(this).BattleWon)
	{
		// Friend don't capture and not winner don't capture
		return false;
	}


	return true;

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

void UFlareCompany::CustomizeMaterial(UMaterialInstanceDynamic* Mat)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(Game->GetWorld()->GetFirstPlayerController());

	// Apply settings to the material instance
	UFlareSpacecraftComponent::CustomizeMaterial(Mat, Game,
		GetBasePaintColor(),
		GetPaintColor(),
		GetOverlayColor(),
		GetLightColor(),
		GetPatternIndex(),
		CompanyDescription->Emblem);
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
	CompanyEmblem->SetVectorParameterValue("BasePaintColor", CompanyDescription->CustomizationBasePaintColor);
	CompanyEmblem->SetVectorParameterValue("PaintColor", CompanyDescription->CustomizationPaintColor);
	CompanyEmblem->SetVectorParameterValue("OverlayColor", CompanyDescription->CustomizationOverlayColor);
	CompanyEmblem->SetVectorParameterValue("GlowColor", CompanyDescription->CustomizationLightColor);

	// Create the brush dynamically
	CompanyEmblemBrush.ImageSize = EmblemSize;
	CompanyEmblemBrush.SetResourceObject(CompanyEmblem);
}

const FSlateBrush* UFlareCompany::GetEmblem() const
{
	return &CompanyEmblemBrush;
}


/*----------------------------------------------------
	Technology
----------------------------------------------------*/

bool UFlareCompany::IsTechnologyUnlocked(FName Identifier) const
{
	if (UnlockedTechnologies.Contains(Identifier))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool UFlareCompany::IsTechnologyAvailable(FName Identifier, FText& Reason, bool IgnoreCost) const
{
	FFlareTechnologyDescription* Technology = GetGame()->GetTechnologyCatalog()->Get(Identifier);
 
	if (GetTechnologyLevel() < Technology->Level)
	{
		Reason = LOCTEXT("CantUnlockTechLevel", "You don't have the technology level to research this technology");
		return false;
	}
	else if (!IgnoreCost && GetResearchAmount() < GetTechnologyCost(Technology))
	{
		Reason = LOCTEXT("CantUnlockTechCost", "You haven't done enough research for this technology");
		return false;
	}
	else if (IsTechnologyUnlocked(Identifier))
	{
		Reason = LOCTEXT("CantUnlockTechAlready", "You have already researched this technology");
		return false;
	}
	else
	{
		return true;
	}
}

bool UFlareCompany::HasStationTechnologyUnlocked() const
{
	TArray<UFlareSpacecraftCatalogEntry*>& StationCatalog = GetGame()->GetSpacecraftCatalog()->StationCatalog;

	// Loop on catalog
	for (UFlareSpacecraftCatalogEntry* Entry : StationCatalog)
	{
		if (!Entry->Data.IsSubstation && IsTechnologyUnlockedStation(&Entry->Data))
		{
			return true;
		}
	}
	return false;
}


bool UFlareCompany::IsTechnologyUnlockedStation(const FFlareSpacecraftDescription* Description) const
{
	FName Identifier = Description->Identifier;

	if (    Identifier == "station-habitation" ||
			Identifier == "station-outpost" ||
			Identifier == "station-solar-plant")
	{
		return IsTechnologyUnlocked("stations");
	}
	else if(Identifier == "station-ice-mine"||
			Identifier == "station-silica-mine" ||
			Identifier == "station-iron-mine")
	{
		return IsTechnologyUnlocked("mining");
	}
	else if(Identifier == "station-carbon-refinery" ||
			Identifier == "station-farm" ||
			Identifier == "station-plastics-refinery")
	{
		return IsTechnologyUnlocked("chemicals");
	}
	else if(Identifier == "station-ch4-pump"||
			Identifier == "station-he3-pump" ||
			Identifier == "station-h2-pump")
	{
		return IsTechnologyUnlocked("orbital-pumps");
	}
	else if(Identifier == "station-steelworks" ||
			Identifier == "station-tool-factory" ||
			Identifier == "station-arsenal")
	{
		return IsTechnologyUnlocked("metallurgy");
	}
	else if(Identifier == "station-shipyard")
	{
		return IsTechnologyUnlocked("shipyard-station");
	}
	else if(Identifier == "station-tokamak" ||
			Identifier == "station-hub" ||
			Identifier == "station-complex" ||
			Identifier == "station-foundry")
	{
		return IsTechnologyUnlocked("advanced-stations");
	}
	else if(Identifier == "station-telescope" ||
			Identifier == "station-research")
	{
		return IsTechnologyUnlocked("science");
	}


	FLOGV("WARNING: station %s don't need technology", *Description->Identifier.ToString());
	return true;
}

bool UFlareCompany::IsTechnologyUnlockedPart(const FFlareSpacecraftComponentDescription* Description) const
{
	FName Identifier = Description->Identifier;

	if (Identifier == "weapon-heavy-salvage" ||
		Identifier == "weapon-light-salvage")
	{
		return IsTechnologyUnlocked("pirate-tech");
	}

	if (Identifier == "weapon-hades" ||
		Identifier == "weapon-mjolnir")
	{
		return IsTechnologyUnlocked("flak");
	}

	if (Identifier == "weapon-wyrm" ||
		Identifier == "weapon-sparrow" ||
		Identifier == "weapon-hydra")
	{
		return IsTechnologyUnlocked("bombing");
	}

	return true;
}

int32 UFlareCompany::GetTechnologyCost(const FFlareTechnologyDescription* Technology) const
{
	return 20 * Technology->Level * CompanyData.ResearchRatio;
}

int32 UFlareCompany::GetTechnologyLevel() const
{
	// 0 technologies -> Level 1
	// 1 technologies -> Level 2
	// 2 technologies -> Level 3
	// 3 technologies -> Level 4

	return FMath::Clamp(1 + UnlockedTechnologies.Num(), 1, 5);
}

int32 UFlareCompany::GetResearchAmount() const
{
	return CompanyData.ResearchAmount;
}

int32 UFlareCompany::GetResearchSpent() const
{
	return CompanyData.ResearchSpent;
}

int32 UFlareCompany::GetResearchValue() const
{
	return GetResearchSpent() + GetResearchAmount();
}


void UFlareCompany::UnlockTechnology(FName Identifier, bool FromSave, bool Force)
{
	FFlareTechnologyDescription* Technology = GetGame()->GetTechnologyCatalog()->Get(Identifier);
	FText Unused;

	if (Identifier != NAME_None && Technology && (IsTechnologyAvailable(Identifier, Unused) || FromSave || Force))
	{
		// Check before research
		float CurrentResearchInflation = IsTechnologyUnlocked("instruments") ? 1.22 : 1.3;

		// Unlock
		UnlockedTechnologies.Add(Identifier, Technology);

		if (!FromSave)
		{
			int32 Cost = GetTechnologyCost(Technology);

			if (!Force)
			{
				CompanyData.ResearchAmount -= Cost;
			}

			CompanyData.ResearchSpent += Cost;

			CompanyData.ResearchRatio *= CurrentResearchInflation;

			if (this == Game->GetPC()->GetCompany())
			{
				FString UniqueId = "technology-unlocked-" + Identifier.ToString();
				Game->GetPC()->Notify(LOCTEXT("CompanyUnlockTechnology", "Technology unlocked"),
					FText::Format(LOCTEXT("CompanyUnlockTechnologyFormat", "You have researched {0} for your company !"), Technology->Name),
					FName(*UniqueId),
					EFlareNotification::NT_Info,
					false);
				GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("unlock-technology").PutName("technology", Identifier).PutInt32("level", Technology->Level));

				GetGame()->GetPC()->SetAchievementProgression("ACHIEVEMENT_ONE_TECHNOLOGY", 1);
				if(UnlockedTechnologies.Num() >= GetGame()->GetTechnologyCatalog()->TechnologyCatalog.Num())
				{
					GetGame()->GetPC()->SetAchievementProgression("ACHIEVEMENT_ALL_TECHNOLOGIES", 1);
				}
			}

			GameLog::UnlockResearch(this, Technology);
		}
	}
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/


const struct CompanyValue UFlareCompany::GetCompanyValue(UFlareSimulatedSector* SectorFilter, bool IncludeIncoming) const
{
	bool globalRequest = SectorFilter == nullptr;


	if(CompanyValueCacheValid && globalRequest)
	{
		return CompanyValueCache;
	}



	// Company value is the sum of :
	// - money
	// - value of its spacecraft
	// - value of the stock in these spacecraft
	// - value of the resources used in factory
	CompanyValue CompanyValue;
	CompanyValue.MoneyValue = GetMoney();
	CompanyValue.StockValue = 0;
	CompanyValue.ShipsValue = 0;
	CompanyValue.ArmyValue = 0;
	CompanyValue.ArmyCurrentCombatPoints = 0;
	CompanyValue.ArmyTotalCombatPoints = 0;
	CompanyValue.StationsValue = 0;

	for (int SpacecraftIndex = 0; SpacecraftIndex < CompanySpacecrafts.Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = CompanySpacecrafts[SpacecraftIndex];

		UFlareSimulatedSector *ReferenceSector =  Spacecraft->GetCurrentSector();

		if(SectorFilter && !IncludeIncoming && SectorFilter != ReferenceSector)
		{
			// Not in sector filter
			continue;
		}

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

		if(SectorFilter && IncludeIncoming && SectorFilter != ReferenceSector)
		{
			// Not in sector filter
			continue;
		}

		// Value of the spacecraft
		int64 SpacecraftPrice = UFlareGameTools::ComputeSpacecraftPrice(Spacecraft->GetDescription()->Identifier, ReferenceSector, true);

		if(Spacecraft->IsStation())
		{
			CompanyValue.StationsValue += SpacecraftPrice * Spacecraft->GetLevel();
		}
		else
		{
			CompanyValue.ShipsValue += SpacecraftPrice;
		}

		if(Spacecraft->IsMilitary())
		{
			CompanyValue.ArmyValue += SpacecraftPrice;
			CompanyValue.ArmyTotalCombatPoints += Spacecraft->GetCombatPoints(false);
			CompanyValue.ArmyCurrentCombatPoints += Spacecraft->GetCombatPoints(true);
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

			CompanyValue.StockValue += ReferenceSector->GetResourcePrice(Cargo.Resource, EFlareResourcePriceContext::Default) * Cargo.Quantity;
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
					CompanyValue.StockValue += ReferenceSector->GetResourcePrice(Resource, EFlareResourcePriceContext::Default) * Quantity;
				}
				else
				{
					FLOGV("WARNING: Invalid reserved resource %s (%d reserved) for %s)", *ResourceIdentifier.ToString(), Quantity, *Spacecraft->GetImmatriculation().ToString())
				}
			}
		}
	}

	CompanyValue.SpacecraftsValue = CompanyValue.ShipsValue + CompanyValue.StationsValue;
	CompanyValue.TotalValue = CompanyValue.MoneyValue + CompanyValue.StockValue + CompanyValue.SpacecraftsValue;

	if(globalRequest)
	{
		CompanyValueCacheValid = true;
		CompanyValueCache = CompanyValue;
	}

	return CompanyValue;
}

UFlareSimulatedSpacecraft* UFlareCompany::FindSpacecraft(FName ShipImmatriculation, bool Destroyed)
{
	if(!Destroyed )
	{
		for (UFlareSimulatedSpacecraft* Spacecraft : CompanySpacecrafts)
		{
			if (Spacecraft->GetImmatriculation() == ShipImmatriculation)
			{
				return Spacecraft;
			}
		}
	}
	else
	{
		for (UFlareSimulatedSpacecraft* Spacecraft : CompanyDestroyedSpacecrafts)
		{
			if (Spacecraft->GetImmatriculation() == ShipImmatriculation)
			{
				return Spacecraft;
			}
		}
	}

	return NULL;
}

bool UFlareCompany::HasVisitedSector(const UFlareSimulatedSector* Sector) const
{
	return Sector && VisitedSectors.Contains(Sector);
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

bool UFlareCompany::IsPlayerCompany() const
{
	return this == Game->GetPC()->GetCompany();
}

bool UFlareCompany::WantCapture(UFlareSimulatedSpacecraft const* Station) const
{
	return CompanyData.CaptureOrders.Contains(Station->GetImmatriculation());
}

int32 UFlareCompany::GetCaptureOrderCountInSector(UFlareSimulatedSector const* Sector) const
{
	int32 Orders = 0;

	for(UFlareSimulatedSpacecraft const* Station: Sector->GetSectorStations())
	{
		if(WantCapture(Station))
		{
			++Orders;
		}
	}
	return Orders;
}

#undef LOCTEXT_NAMESPACE
