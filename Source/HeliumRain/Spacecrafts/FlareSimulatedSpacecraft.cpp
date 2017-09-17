
#include "FlareSimulatedSpacecraft.h"
#include "../Flare.h"

#include "../Game/FlareSimulatedSector.h"
#include "../Game/FlareGame.h"
#include "../Game/FlareFleet.h"
#include "../Game/FlareWorld.h"
#include "../Game/FlareSectorHelper.h"
#include "../Game/FlareGameTools.h"

#include "../Player/FlarePlayerController.h"
#include "../Economy/FlareCargoBay.h"
#include "../Economy/FlareFactory.h"

#include "../Data/FlareResourceCatalog.h"
#include "../Data/FlareFactoryCatalogEntry.h"
#include "../Data/FlareSpacecraftCatalog.h"
#include "../Data/FlareSpacecraftComponentsCatalog.h"

#include "Subsystems/FlareSimulatedSpacecraftDamageSystem.h"
#include "Subsystems/FlareSimulatedSpacecraftWeaponsSystem.h"


#define LOCTEXT_NAMESPACE "FlareSimulatedSpacecraft"

#define CAPTURE_RESET_SPEED 0.1f
#define CAPTURE_THRESOLD_MIN 0.05f
#define CAPTURE_THRESOLD_MIN_DAMAGE 0.25f

#define STATION_DAMAGE_THRESOLD 0.75f

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSimulatedSpacecraft::UFlareSimulatedSpacecraft(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActiveSpacecraft = NULL;
}


void UFlareSimulatedSpacecraft::Load(const FFlareSpacecraftSave& Data)
{
	Game = Cast<UFlareCompany>(GetOuter())->GetGame();
	SpacecraftData = Data;

	// Load spacecraft description
	SpacecraftDescription = Game->GetSpacecraftCatalog()->Get(Data.Identifier);

	// Initialize damage system
	DamageSystem = NewObject<UFlareSimulatedSpacecraftDamageSystem>(this, UFlareSimulatedSpacecraftDamageSystem::StaticClass());
	DamageSystem->Initialize(this, &SpacecraftData);

	// Initialize weapons system
	WeaponsSystem = NewObject<UFlareSimulatedSpacecraftWeaponsSystem>(this, UFlareSimulatedSpacecraftWeaponsSystem::StaticClass());
	WeaponsSystem->Initialize(this, &SpacecraftData);

	Game->GetGameWorld()->ClearFactories(this);
	Factories.Empty();

	if(!IsUnderConstruction())
	{
		// Load factories
		for (int FactoryIndex = 0; FactoryIndex < SpacecraftDescription->Factories.Num(); FactoryIndex++)
		{
			FFlareFactorySave FactoryData;
			FFlareFactoryDescription* FactoryDescription = &SpacecraftDescription->Factories[FactoryIndex]->Data;

			if (FactoryIndex < SpacecraftData.FactoryStates.Num())
			{
				FactoryData = SpacecraftData.FactoryStates[FactoryIndex];
			}
			else
			{
					FactoryData.Active = FactoryDescription->AutoStart;
					FactoryData.CostReserved = 0;
					FactoryData.ProductedDuration = 0;
					FactoryData.InfiniteCycle = true;
					FactoryData.CycleCount = 0;
					FactoryData.TargetShipClass = NAME_None;
					FactoryData.TargetShipCompany = NAME_None;
			}

			UFlareFactory* Factory = NewObject<UFlareFactory>(GetGame()->GetGameWorld(), UFlareFactory::StaticClass());
			Factory->Load(this, FactoryDescription, FactoryData);
			Factories.Add(Factory);
			if (!IsDestroyed())
			{
				Game->GetGameWorld()->AddFactory(Factory);
			}
		}
	}
	else
	{
		UFlareFactory* Factory = NewObject<UFlareFactory>(GetGame()->GetGameWorld(), UFlareFactory::StaticClass());

		FFlareFactoryDescription* FactoryDescription = &Factory->ConstructionFactoryDescription;
		FactoryDescription->AutoStart = true;
		FactoryDescription->Name = LOCTEXT("Constructionfactory", "Station under constuction");
		FactoryDescription->Description = LOCTEXT("ConstructionfactoryDescription", "Bring construction resources");
		FactoryDescription->Identifier = FName(*(FString("construction-") + SpacecraftDescription->Identifier.ToString()));

		FFlareFactoryAction OutputAction;
		OutputAction.Action = EFlareFactoryAction::BuildStation;
		OutputAction.Identifier = "build-station";
		OutputAction.Quantity = 1;
		FactoryDescription->OutputActions.Add(OutputAction);
		FactoryDescription->CycleCost.ProductionCost = 0;
		FactoryDescription->CycleCost.ProductionTime = 0;
		FactoryDescription->CycleCost.InputResources.Append(SpacecraftDescription->CycleCost.InputResources);
		FactoryDescription->VisibleStates = true;

		// Create construction factory
		FFlareFactorySave FactoryData;
		FactoryData.Active = FactoryDescription->AutoStart;
		FactoryData.CostReserved = 0;
		FactoryData.ProductedDuration = 0;
		FactoryData.InfiniteCycle = true;
		FactoryData.CycleCount = 0;
		FactoryData.TargetShipClass = NAME_None;
		FactoryData.TargetShipCompany = NAME_None;

		Factory->Load(this, FactoryDescription, FactoryData);
		Factories.Add(Factory);
		if (!IsDestroyed())
		{
			Game->GetGameWorld()->AddFactory(Factory);
		}
	}

	// Construction cargo bay
	CargoBay = NewObject<UFlareCargoBay>(this, UFlareCargoBay::StaticClass());
	CargoBay->Load(this, SpacecraftData.Cargo);

	// Lock resources
	LockResources();

	if(ActiveSpacecraft)
	{
		ActiveSpacecraft->Load(this);
		ActiveSpacecraft->Redock();
	}
}

FFlareSpacecraftSave* UFlareSimulatedSpacecraft::Save()
{
	SpacecraftData.FactoryStates.Empty();
	for (int FactoryIndex = 0; FactoryIndex < Factories.Num(); FactoryIndex++)
	{
		SpacecraftData.FactoryStates.Add(*Factories[FactoryIndex]->Save());
	}

	SpacecraftData.Cargo = *CargoBay->Save();

	if(IsActive())
	{
		GetActive()->Save();
	}

	return &SpacecraftData;
}


UFlareCompany* UFlareSimulatedSpacecraft::GetCompany() const
{
	// TODO Cache
	return Game->GetGameWorld()->FindCompany(SpacecraftData.CompanyIdentifier);
}


EFlarePartSize::Type UFlareSimulatedSpacecraft::GetSize()
{
	return SpacecraftDescription->Size;
}

bool UFlareSimulatedSpacecraft::IsMilitary() const
{
	return SpacecraftDescription->IsMilitary();
}

bool UFlareSimulatedSpacecraft::IsStation() const
{
	return SpacecraftDescription->IsStation();
}

bool UFlareSimulatedSpacecraft::CanFight() const
{
	return GetDamageSystem()->IsAlive() && IsMilitary() && !GetDamageSystem()->IsDisarmed();
}

bool UFlareSimulatedSpacecraft::CanTravel() const
{
	return !IsTrading() && !IsIntercepted() && GetDamageSystem()->IsAlive() && !GetDamageSystem()->IsStranded();
}

FName UFlareSimulatedSpacecraft::GetImmatriculation() const
{
	return SpacecraftData.Immatriculation;
}

UFlareSimulatedSpacecraftDamageSystem* UFlareSimulatedSpacecraft::GetDamageSystem() const
{
	return DamageSystem;
}

UFlareSimulatedSpacecraftWeaponsSystem* UFlareSimulatedSpacecraft::GetWeaponsSystem() const
{
	return WeaponsSystem;
}

void UFlareSimulatedSpacecraft::SetSpawnMode(EFlareSpawnMode::Type SpawnMode)
{
	SpacecraftData.SpawnMode = SpawnMode;
}

bool UFlareSimulatedSpacecraft::CanBeFlown(FText& OutInfo) const
{
	UFlareFleet* PlayerFleet = GetGame()->GetPC()->GetPlayerFleet();

	if (IsStation())
	{
		return false;
	}
	else if (this == GetGame()->GetPC()->GetPlayerShip())
	{
		OutInfo = LOCTEXT("CantFlySame", "You are already flying this ship");
		return false;
	}
	else if (GetDamageSystem()->IsUncontrollable())
	{
		OutInfo = LOCTEXT("CantFlyDestroyedInfo", "This ship is uncontrollable");
		return false;
	}
	else if (PlayerFleet && GetCurrentFleet() != PlayerFleet)
	{
		OutInfo = LOCTEXT("CantFlyOtherInfo", "You can only switch ships from within the same fleet");
		return false;
	}
	else if (!IsActive())
	{
		OutInfo = LOCTEXT("CantFlyDistantInfo", "Can't fly a ship from another sector");
		return false;
	}
	else
	{
		return true;
	}
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/


void UFlareSimulatedSpacecraft::SetCurrentSector(UFlareSimulatedSector* Sector)
{
	CurrentSector = Sector;

	// Mark the sector as visited
	if (!Sector->IsTravelSector())
	{
		GetCompany()->VisitSector(Sector);
	}
}


/*----------------------------------------------------
	Resources
----------------------------------------------------*/


bool UFlareSimulatedSpacecraft::CanTradeWith(UFlareSimulatedSpacecraft* OtherSpacecraft, FText& Reason)
{
	// Check if both spacecraft are in the same sector
	if (GetCurrentSector() != OtherSpacecraft->GetCurrentSector())
	{
		Reason = LOCTEXT("CantTradeSector", "Can't trade between different sectors");
		return false;
	}

	// Damaged
	if (GetDamageSystem()->IsUncontrollable() || OtherSpacecraft->GetDamageSystem()->IsUncontrollable())
	{
		Reason = LOCTEXT("CantTradeUncontrollable", "Can't trade with incontrollable ships");
		return false;
	}

	// Check if spacecraft are both stations
	if (IsStation() && OtherSpacecraft->IsStation())
	{
		Reason = LOCTEXT("CantTradeStations", "Can't trade between two stations");
		return false;
	}

	// Check if spacecraft are not both ships
	if (!IsStation() && !OtherSpacecraft->IsStation())
	{
		Reason = LOCTEXT("CantTradeShips", "Can't trade between two ships");
		return false;
	}

	// Check if spacecraft are are not already trading
	if (IsTrading() || OtherSpacecraft->IsTrading())
	{
		Reason = LOCTEXT("CantTradeAlreadyTrading", "Can't trade with spacecraft that are already in a trading transaction");
		return false;
	}

	// Check if both spacecraft are not at war
	if (GetCompany()->GetWarState(OtherSpacecraft->GetCompany()) == EFlareHostility::Hostile)
	{
		Reason = LOCTEXT("CantTradeWar", "Can't trade between enemies");
		return false;
	}

	return true;
}

EFlareResourcePriceContext::Type UFlareSimulatedSpacecraft::GetResourceUseType(FFlareResourceDescription* Resource)
{
	// Check we're and station
	if (!IsStation())
	{
		return EFlareResourcePriceContext::Default;
	}

	// Parse factories
	for (UFlareFactory* Factory : Factories)
	{

		// Is input resource of a station ?
		for (int32 ResourceIndex = 0; ResourceIndex < Factory->GetCycleData().InputResources.Num(); ResourceIndex++)
		{
			const FFlareFactoryResource* FactoryResource = &Factory->GetCycleData().InputResources[ResourceIndex];
			if (&FactoryResource->Resource->Data == Resource)
			{
				return EFlareResourcePriceContext::FactoryInput;
			}
		}

		// Is output resource of a station ?
		for (int32 ResourceIndex = 0; ResourceIndex < Factory->GetCycleData().OutputResources.Num(); ResourceIndex++)
		{
			const FFlareFactoryResource* FactoryResource = &Factory->GetCycleData().OutputResources[ResourceIndex];
			if (&FactoryResource->Resource->Data == Resource)
			{
				return EFlareResourcePriceContext::FactoryOutput;
			}
		}
	}

	// Customer resource ?
	if (SpacecraftDescription->Capabilities.Contains(EFlareSpacecraftCapability::Consumer) && Resource->IsConsumerResource)
	{
		return EFlareResourcePriceContext::ConsumerConsumption;
	}

	// Maintenance resource ?
	if (SpacecraftDescription->Capabilities.Contains(EFlareSpacecraftCapability::Maintenance) && Resource->IsMaintenanceResource)
	{
		return EFlareResourcePriceContext::MaintenanceConsumption;
	}

	return EFlareResourcePriceContext::Default;
}

void UFlareSimulatedSpacecraft::LockResources()
{
	GetCargoBay()->UnlockAll();


	if (Factories.Num() > 0)
	{
		UFlareFactory* Factory = Factories[0];

		for (int32 ResourceIndex = 0 ; ResourceIndex < Factory->GetCycleData().InputResources.Num() ; ResourceIndex++)
		{
			const FFlareFactoryResource* Resource = &Factory->GetCycleData().InputResources[ResourceIndex];

			if (!GetCargoBay()->LockSlot(&Resource->Resource->Data, EFlareResourceLock::Input, false))
			{
				FLOGV("Fail to lock a slot of %s in %s", *(&Resource->Resource->Data)->Name.ToString(), *GetImmatriculation().ToString());
			}
		}

		for (int32 ResourceIndex = 0 ; ResourceIndex < Factory->GetCycleData().OutputResources.Num() ; ResourceIndex++)
		{
			const FFlareFactoryResource* Resource = &Factory->GetCycleData().OutputResources[ResourceIndex];

			if (!GetCargoBay()->LockSlot(&Resource->Resource->Data, EFlareResourceLock::Output, false))
			{
				FLOGV("Fail to lock a slot of %s in %s", *(&Resource->Resource->Data)->Name.ToString(), *GetImmatriculation().ToString());
			}
		}
	}

	if (!IsUnderConstruction())
	{

		if (HasCapability(EFlareSpacecraftCapability::Consumer))
		{
			for (int32 ResourceIndex = 0; ResourceIndex < GetGame()->GetResourceCatalog()->ConsumerResources.Num(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = &GetGame()->GetResourceCatalog()->ConsumerResources[ResourceIndex]->Data;
				if (!GetCargoBay()->LockSlot(Resource, EFlareResourceLock::Input, false))
				{
					FLOGV("Fail to lock a slot of %s in %s", *Resource->Name.ToString(), *GetImmatriculation().ToString());
				}
			}
		}

		if (HasCapability(EFlareSpacecraftCapability::Maintenance))
		{
			for (int32 ResourceIndex = 0; ResourceIndex < GetGame()->GetResourceCatalog()->MaintenanceResources.Num(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = &GetGame()->GetResourceCatalog()->MaintenanceResources[ResourceIndex]->Data;

				if (!GetCargoBay()->LockSlot(Resource, EFlareResourceLock::Trade, false))
				{
					FLOGV("Fail to lock a slot of %s in %s", *Resource->Name.ToString(), *GetImmatriculation().ToString());
				}
			}
		}
	}
}

void UFlareSimulatedSpacecraft::SetAsteroidData(FFlareAsteroidSave* Data)
{
	SpacecraftData.AsteroidData.Identifier = Data->Identifier;
	SpacecraftData.AsteroidData.AsteroidMeshID = Data->AsteroidMeshID;
	SpacecraftData.AsteroidData.Scale = Data->Scale;
	SpacecraftData.Location = Data->Location;
	SpacecraftData.Rotation = Data->Rotation;
}

void UFlareSimulatedSpacecraft::SetActorAttachment(FName ActorName)
{
	FLOGV("UFlareSimulatedSpacecraft::SetActorAttachment : %s will attach to %s",
		*GetImmatriculation().ToString(), *ActorName.ToString());

	SpacecraftData.AttachActorName = ActorName;
}

void UFlareSimulatedSpacecraft::SetDynamicComponentState(FName Identifier, float Progress)
{
	SpacecraftData.DynamicComponentStateIdentifier = Identifier;
	SpacecraftData.DynamicComponentStateProgress = Progress;
}

void UFlareSimulatedSpacecraft::Upgrade()
{
	FLOGV("UFlareSimulatedSpacecraft::Upgrade %s to level %d", *GetImmatriculation().ToString(), SpacecraftData.Level+1);

	SpacecraftData.Level++;
	SpacecraftData.IsUnderConstruction = true;
	SpacecraftData.CargoBackup = SpacecraftData.Cargo;
	SpacecraftData.Cargo.Empty();
	Load(SpacecraftData);

	if(GetCompany() == Game->GetPC()->GetCompany())
	{
		Game->GetQuestManager()->OnEvent(FFlareBundle().PutTag("start-station-construction").PutInt32("upgrade", 1));
	}

	FLOGV("UFlareSimulatedSpacecraft::Upgrade %s to level %d done", *GetImmatriculation().ToString(), SpacecraftData.Level);
}



void UFlareSimulatedSpacecraft::ForceUndock()
{
	SpacecraftData.DockedTo = NAME_None;
	SpacecraftData.DockedAt = -1;
}

void UFlareSimulatedSpacecraft::SetTrading(bool Trading)
{
	if (IsStation())
	{
		FLOGV("Fail to set trading state to %s : station are never locked in trading state", *GetImmatriculation().ToString());
		return;
	}

	// Notify end of state if not on trade route
	if (!Trading
		&& SpacecraftData.IsTrading
		&& GetCompany() == GetGame()->GetPC()->GetCompany()
		&& (GetCurrentTradeRoute() == NULL || GetCurrentTradeRoute()->IsPaused()))
	{
		FFlareMenuParameterData Data;
		Data.Spacecraft = this;
		Game->GetPC()->Notify(LOCTEXT("TradingStateEnd", "Trading complete"),
			FText::Format(LOCTEXT("TravelEndedFormat", "{0} finished trading in {1}"),
				UFlareGameTools::DisplaySpacecraftName(this),
				GetCurrentSector()->GetSectorName()),
			FName("trading-state-end"),
			EFlareNotification::NT_Economy,
			false,
			EFlareMenu::MENU_Ship,
			Data);
	}

	SpacecraftData.IsTrading = Trading;
}

void UFlareSimulatedSpacecraft::SetIntercepted(bool Intercepted)
{
	SpacecraftData.IsIntercepted = Intercepted;
}

void UFlareSimulatedSpacecraft::SetReserve(bool InReserve)
{
	SpacecraftData.IsReserve = InReserve;
}


void UFlareSimulatedSpacecraft::Repair()
{
	if(GetRepairStock() <= 0 || (GetCurrentSector() && GetCurrentSector()->IsInDangerousBattle(GetCompany())))
	{
		// No repair possible
		return;
	}

	UFlareSpacecraftComponentsCatalog* Catalog = GetGame()->GetShipPartsCatalog();

	float SpacecraftPreciseCurrentNeededFleetSupply = 0;

	// List components
	for (int32 ComponentIndex = 0; ComponentIndex < GetData().Components.Num(); ComponentIndex++)
	{
		FFlareSpacecraftComponentSave* ComponentData = &GetData().Components[ComponentIndex];
		FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

		float DamageRatio = GetDamageSystem()->GetDamageRatio(ComponentDescription, ComponentData);
		float TechnologyBonus = GetCompany()->IsTechnologyUnlocked("quick-repair") ? 1.5f: 1.f;
		float ComponentMaxRepairRatio = SectorHelper::GetComponentMaxRepairRatio(ComponentDescription) * (GetSize() == EFlarePartSize::L ? 0.2f : 1.f) * TechnologyBonus;
		float CurrentRepairRatio = FMath::Min(ComponentMaxRepairRatio, (1.f - DamageRatio));


		SpacecraftPreciseCurrentNeededFleetSupply += CurrentRepairRatio * UFlareSimulatedSpacecraftDamageSystem::GetRepairCost(ComponentDescription);
	}

	if(SpacecraftPreciseCurrentNeededFleetSupply != 0)
	{

		float MaxRepairRatio = FMath::Min(1.f, GetRepairStock() / SpacecraftPreciseCurrentNeededFleetSupply);

		for (int32 ComponentIndex = 0; ComponentIndex < GetData().Components.Num(); ComponentIndex++)
		{
			FFlareSpacecraftComponentSave* ComponentData = &GetData().Components[ComponentIndex];
			FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

			float TechnologyBonus = GetCompany()->IsTechnologyUnlocked("quick-repair") ? 1.5f: 1.f;
			float ComponentMaxRepairRatio = SectorHelper::GetComponentMaxRepairRatio(ComponentDescription) * (GetSize() == EFlarePartSize::L ? 0.2f : 1.f) * TechnologyBonus;
			float ConsumedFS = GetDamageSystem()->Repair(ComponentDescription,ComponentData, MaxRepairRatio * ComponentMaxRepairRatio, SpacecraftData.RepairStock);

			SpacecraftData.RepairStock -= ConsumedFS;
			if(GetRepairStock() <= 0)
			{
				break;
			}
		}
	}


	if (GetDamageSystem()->GetGlobalDamageRatio() >= 1.f)
	{
		SpacecraftData.RepairStock = 0;

		if(SpacecraftPreciseCurrentNeededFleetSupply > 0)
		{
			if(GetCompany() == GetGame()->GetPC()->GetCompany())
			{
				GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("repair-end"));
			}
		}
	}
}

void UFlareSimulatedSpacecraft::RecoveryRepair()
{
	SpacecraftData.RepairStock = 0;


	UFlareSpacecraftComponentsCatalog* Catalog = GetGame()->GetShipPartsCatalog();


	for (int32 ComponentIndex = 0; ComponentIndex < GetData().Components.Num(); ComponentIndex++)
	{
		FFlareSpacecraftComponentSave* ComponentData = &GetData().Components[ComponentIndex];
		FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

		if (ComponentDescription->Type == EFlarePartType::RCS
				|| ComponentDescription->Type == EFlarePartType::OrbitalEngine
				|| ComponentDescription->Type == EFlarePartType::InternalComponent)
		{
			float MaxDamage = (1-(BROKEN_RATIO + 0.1)) * ComponentDescription->HitPoints;
			if(ComponentDescription->Type == EFlarePartType::OrbitalEngine)
			{
				MaxDamage = BROKEN_RATIO * 0.6 * ComponentDescription->HitPoints;
			}

			ComponentData->Damage = FMath::Min(ComponentData->Damage, MaxDamage);

			GetDamageSystem()->SetDamageDirty(ComponentDescription);

			if(!GetDamageSystem()->IsStranded())
			{
				return;
			}
		}
	}
}

void UFlareSimulatedSpacecraft::Stabilize()
{
	if(!GetDamageSystem()->IsUncontrollable())
	{
		SpacecraftData.LinearVelocity = FVector::ZeroVector;
		SpacecraftData.AngularVelocity = FVector::ZeroVector;

		float Limits = UFlareSector::GetSectorLimits();
		float Distance = SpacecraftData.Location.Size();

		if(Distance > Limits)
		{
			float Correction = 0.8* Limits / Distance;
			SpacecraftData.Location *= Correction;
		}
	}
}

void UFlareSimulatedSpacecraft::Refill()
{
	if(GetRefillStock() == 0 || (GetCurrentSector() && GetCurrentSector()->IsInDangerousBattle(GetCompany())))
	{
		// No refill possible
		return;
	}

	UFlareSpacecraftComponentsCatalog* Catalog = GetGame()->GetShipPartsCatalog();
	float SpacecraftPreciseCurrentNeededFleetSupply = 0;

	// List components
	for (int32 ComponentIndex = 0; ComponentIndex < GetData().Components.Num(); ComponentIndex++)
	{
		FFlareSpacecraftComponentSave* ComponentData = &GetData().Components[ComponentIndex];
		FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);
		if(ComponentDescription->Type == EFlarePartType::Weapon)
		{
			int32 MaxAmmo = ComponentDescription->WeaponCharacteristics.AmmoCapacity;
			int32 CurrentAmmo = MaxAmmo - ComponentData->Weapon.FiredAmmo;
			float FillRatio = (float) CurrentAmmo / (float) MaxAmmo;
			float CurrentRefillRatio = FMath::Min(MAX_REFILL_RATIO_BY_DAY * (GetSize() == EFlarePartSize::L ? 0.2f : 1.f), (1.f - FillRatio));

			SpacecraftPreciseCurrentNeededFleetSupply += CurrentRefillRatio * UFlareSimulatedSpacecraftDamageSystem::GetRefillCost(ComponentDescription);
		}

	}

	if(SpacecraftPreciseCurrentNeededFleetSupply != 0)
	{
		float MaxRefillRatio = MAX_REFILL_RATIO_BY_DAY * (GetSize() == EFlarePartSize::L ? 0.2f : 1.f) * FMath::Min(1.f,GetRefillStock() / SpacecraftPreciseCurrentNeededFleetSupply);

		for (int32 ComponentIndex = 0; ComponentIndex < GetData().Components.Num(); ComponentIndex++)
		{
			FFlareSpacecraftComponentSave* ComponentData = &GetData().Components[ComponentIndex];
			FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

			if(ComponentDescription->Type == EFlarePartType::Weapon)
			{
				float ConsumedFS = GetDamageSystem()->Refill(ComponentDescription,ComponentData, MaxRefillRatio, SpacecraftData.RefillStock);

				SpacecraftData.RefillStock -= ConsumedFS;

				if(GetRefillStock() <= 0)
				{
					break;
				}
			}
		}
	}

	if (!NeedRefill())
	{
		SpacecraftData.RefillStock = 0;


		if(SpacecraftPreciseCurrentNeededFleetSupply > 0)
		{
			if(GetCompany() == GetGame()->GetPC()->GetCompany())
			{
				GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("refill-end"));
			}
		}
	}

}

void UFlareSimulatedSpacecraft::SetHarpooned(UFlareCompany* OwnerCompany)
{
	if (OwnerCompany) {
		if (SpacecraftData.HarpoonCompany != OwnerCompany->GetIdentifier())
		{
			CombatLog::SpacecraftHarpooned(this, OwnerCompany);
			SpacecraftData.HarpoonCompany  = OwnerCompany->GetIdentifier();
		}
	}
	else
	{
		SpacecraftData.HarpoonCompany = NAME_None;
	}
}

UFlareCompany* UFlareSimulatedSpacecraft::GetHarpoonCompany()
{
	return Game->GetGameWorld()->FindCompany(SpacecraftData.HarpoonCompany);
}

void UFlareSimulatedSpacecraft::RemoveCapturePoint(FName CompanyIdentifier, int32 CapturePoint)
{
	if(SpacecraftData.CapturePoints.Contains(CompanyIdentifier))
	{
		int32 CurrentCapturePoint = SpacecraftData.CapturePoints[CompanyIdentifier];
		if(CapturePoint >= CurrentCapturePoint)
		{
			SpacecraftData.CapturePoints.Remove(CompanyIdentifier);
		}
		else
		{
			SpacecraftData.CapturePoints[CompanyIdentifier] = CurrentCapturePoint - CapturePoint;
		}
	}
}

void UFlareSimulatedSpacecraft::ResetCapture(UFlareCompany* Company)
{
	int32 ResetSpeedPoint = FMath::CeilToInt(GetCapturePointThreshold() * CAPTURE_RESET_SPEED);

	if (Company)
	{
		FName CompanyIdentifier = Company->GetIdentifier();
		RemoveCapturePoint(CompanyIdentifier, ResetSpeedPoint);

		Company->StopCapture(this);
	}
	else
	{
		TArray<FName> CapturingCompany;
		SpacecraftData.CapturePoints.GetKeys(CapturingCompany);

		for(int CompanyIndex = 0; CompanyIndex < CapturingCompany.Num(); CompanyIndex++)
		{
			RemoveCapturePoint(CapturingCompany[CompanyIndex], ResetSpeedPoint);
		}

		for(UFlareCompany* CompanyCandidate: Game->GetGameWorld()->GetCompanies())
		{
			CompanyCandidate->StopCapture(this);
		}
	}
}

bool UFlareSimulatedSpacecraft::TryCapture(UFlareCompany* Company, int32 CapturePoint)
{
	int32 CurrentCapturePoint = 0;
	FName CompanyIdentifier = Company->GetIdentifier();
	if (SpacecraftData.CapturePoints.Contains(CompanyIdentifier))
	{
		CurrentCapturePoint = SpacecraftData.CapturePoints[CompanyIdentifier];
	}

	CurrentCapturePoint += CapturePoint;

	if(SpacecraftData.CapturePoints.Contains(CompanyIdentifier)){
		SpacecraftData.CapturePoints[CompanyIdentifier] = CurrentCapturePoint;
	}
	else
	{
		SpacecraftData.CapturePoints.Add(CompanyIdentifier, CurrentCapturePoint);
	}

	if (CurrentCapturePoint > GetCapturePointThreshold())
	{
		// Can be captured
		return true;
	}

	return false;
}

bool UFlareSimulatedSpacecraft::UpgradePart(FFlareSpacecraftComponentDescription* NewPartDesc, int32 WeaponGroupIndex)
{

	UFlareSpacecraftComponentsCatalog* Catalog = Game->GetPC()->GetGame()->GetShipPartsCatalog();
	int32 TransactionCost = 0;

	// Update all components
	for (int32 i = 0; i < SpacecraftData.Components.Num(); i++)
	{
		bool UpdatePart = false;
		FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(SpacecraftData.Components[i].ComponentIdentifier);

		if (ComponentDescription->Type == NewPartDesc->Type)
		{

			// For a weapon, check if this slot belongs to the current group
			if (ComponentDescription->Type == EFlarePartType::Weapon)
			{
				FName SlotName = SpacecraftData.Components[i].ShipSlotIdentifier;
				int32 TargetGroupIndex = UFlareSimulatedSpacecraftWeaponsSystem::GetGroupIndexFromSlotIdentifier(GetDescription(), SlotName);
				UpdatePart = (TargetGroupIndex == WeaponGroupIndex);
			}
			else
			{
				UpdatePart = true;
			}
		}

		// Set the new description and reload the weapon if it was marked for change
		if (UpdatePart)
		{
			if(TransactionCost == 0)
			{
				TransactionCost = GetUpgradeCost(NewPartDesc, ComponentDescription);
				if(TransactionCost > GetCompany()->GetMoney())
				{
					// Cannot afford upgrade
					return false;
				}
			}
			SpacecraftData.Components[i].ComponentIdentifier = NewPartDesc->Identifier;
			SpacecraftData.Components[i].Weapon.FiredAmmo = 0;
			GetDamageSystem()->SetDamageDirty(ComponentDescription);
		}
	}

	// Update the world ship, take money from player, etc
	if (TransactionCost > 0)
	{
		GetCompany()->TakeMoney(TransactionCost);
	}
	else
	{
		GetCompany()->GiveMoney(FMath::Abs(TransactionCost));
	}

	UFlareSimulatedSector* Sector = GetCurrentSector();
	if (Sector)
	{
		if (TransactionCost > 0)
		{
			Sector->GetPeople()->Pay(TransactionCost);
		}
		else
		{
			Sector->GetPeople()->TakeMoney(FMath::Abs(TransactionCost));
		}
	}

	Load(SpacecraftData);

	return true;
}

FFlareSpacecraftComponentDescription* UFlareSimulatedSpacecraft::GetCurrentPart(EFlarePartType::Type Type, int32 WeaponGroupIndex)
{
	UFlareSpacecraftComponentsCatalog* Catalog = Game->GetPC()->GetGame()->GetShipPartsCatalog();

	// Update all components
	for (int32 i = 0; i < SpacecraftData.Components.Num(); i++)
	{
		bool UpdatePart = false;
		FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(SpacecraftData.Components[i].ComponentIdentifier);

		if (ComponentDescription->Type == Type)
		{

			// For a weapon, check if this slot belongs to the current group
			if (ComponentDescription->Type == EFlarePartType::Weapon)
			{
				FName SlotName = SpacecraftData.Components[i].ShipSlotIdentifier;
				int32 TargetGroupIndex = UFlareSimulatedSpacecraftWeaponsSystem::GetGroupIndexFromSlotIdentifier(GetDescription(), SlotName);
				if(TargetGroupIndex == WeaponGroupIndex)
				{
					return ComponentDescription;
				}
			}
			else
			{
				return ComponentDescription;
			}
		}
	}
	return NULL;
}

void UFlareSimulatedSpacecraft::FinishConstruction()
{
	if(!IsUnderConstruction())
	{
		return;
	}

	SpacecraftData.IsUnderConstruction = false;
	SpacecraftData.Cargo = SpacecraftData.CargoBackup;
	Load(SpacecraftData);
	if (IsShipyard())
	{
		for (UFlareFactory* Factory : Factories)
		{
			Factory->Stop();
		}
	}
}

void UFlareSimulatedSpacecraft::OrderRepairStock(float FS)
{
	SpacecraftData.RepairStock += FS;
}

void UFlareSimulatedSpacecraft::OrderRefillStock(float FS)
{
	SpacecraftData.RefillStock += FS;
}

bool UFlareSimulatedSpacecraft::NeedRefill()
{
	UFlareSpacecraftComponentsCatalog* Catalog = GetGame()->GetShipPartsCatalog();

	// List components
	for (int32 ComponentIndex = 0; ComponentIndex < GetData().Components.Num(); ComponentIndex++)
	{
		FFlareSpacecraftComponentSave* ComponentData = &GetData().Components[ComponentIndex];
		FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

		if(ComponentDescription->Type == EFlarePartType::Weapon)
		{

			int32 MaxAmmo = ComponentDescription->WeaponCharacteristics.AmmoCapacity;
			int32 CurrentAmmo = MaxAmmo - ComponentData->Weapon.FiredAmmo;

			float FillRatio = (float) CurrentAmmo / (float) MaxAmmo;

			if(FillRatio < 1.f)
			{
				return true;
			}
		}
	}

	return false;
}

bool UFlareSimulatedSpacecraft::IsShipyard()
{
	return GetFactories().Num() && GetFactories()[0]->IsShipyard();
}

bool UFlareSimulatedSpacecraft::IsComplex() const
{
	if (GetDescription()->Identifier == "station-complex")
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool UFlareSimulatedSpacecraft::ShipyardOrderShip(UFlareCompany* OrderCompany, FName ShipIdentifier)
{
	FFlareSpacecraftDescription* ShipDescription = GetGame()->GetSpacecraftCatalog()->Get(ShipIdentifier);

	if (!ShipDescription || !CanOrder(ShipDescription, OrderCompany))
	{
		return false;
	}

	uint32 ShipPrice = 0;

	if(GetCompany() != OrderCompany)
	{
		ShipPrice = UFlareGameTools::ComputeSpacecraftPrice(ShipIdentifier, GetCurrentSector(), true);
		if(!OrderCompany->TakeMoney(ShipPrice))
		{
			// Not enough money
			return false;
		}
	}

	FFlareShipyardOrderSave newOrder;
	newOrder.ShipClass = ShipIdentifier;
	newOrder.Company = OrderCompany->GetIdentifier();
	newOrder.AdvancePayment = ShipPrice;

	SpacecraftData.ShipyardOrderQueue.Add(newOrder);

	UpdateShipyardProduction();

	if (OrderCompany == Game->GetPC()->GetCompany())
	{
		FFlareSpacecraftDescription* Desc = Game->GetSpacecraftCatalog()->Get(ShipIdentifier);

		int32 Size = 0;
		if (Desc)
		{
			Size = Desc->Size;
		}

		int32 Military = 0;
		if(Desc && (Desc->WeaponGroups.Num()  > 0 || Desc->TurretSlots.Num()  > 0 ))
		{
			Military = 1;
		}

		Game->GetQuestManager()->OnEvent(FFlareBundle().PutTag("order-ship").PutInt32("size", Size).PutInt32("military", Military));

		GetCompany()->GivePlayerReputation(ShipPrice / 100000);
	}

	return true;
}

void UFlareSimulatedSpacecraft::CancelShipyardOrder(int32 OrderIndex)
{
	if(OrderIndex < 0 || OrderIndex > SpacecraftData.ShipyardOrderQueue.Num())
	{
		return;
	}

	FFlareShipyardOrderSave Order = SpacecraftData.ShipyardOrderQueue[OrderIndex];

	UFlareCompany* Company = GetGame()->GetGameWorld()->FindCompany(Order.Company);
	Company->GiveMoney(Order.AdvancePayment);


	if(Order.Company == Game->GetPC()->GetCompany()->GetIdentifier())
	{
		GetCompany()->GivePlayerReputation(-Order.AdvancePayment / 100000);
	}

	SpacecraftData.ShipyardOrderQueue.RemoveAt(OrderIndex);

	UpdateShipyardProduction();
}

TArray<FFlareShipyardOrderSave>& UFlareSimulatedSpacecraft::GetShipyardOrderQueue()
{
	return SpacecraftData.ShipyardOrderQueue;
}

TArray<FFlareShipyardOrderSave> UFlareSimulatedSpacecraft::GetOngoingProductionList()
{
	TArray<FFlareShipyardOrderSave> productionList;

	for (UFlareFactory* Factory : Factories)
	{
		if (Factory->IsShipyard() && Factory->GetTargetShipClass() != NAME_None)
		{
			FFlareShipyardOrderSave production;
			production.ShipClass = Factory->GetTargetShipClass();
			production.Company = Factory->GetTargetShipCompany();
			production.RemainingProductionDuration = Factory->GetRemainingProductionDuration();
			productionList.Add(production);
		}
	}
	return productionList;
}

FText UFlareSimulatedSpacecraft::GetNextShipyardOrderStatus()
{
	if(SpacecraftData.ShipyardOrderQueue.Num() == 0)
	{
		return FText(LOCTEXT("NextShipyardOrderStatusNone", "No ship order"));
	}

	FFlareShipyardOrderSave& NextOrder = SpacecraftData.ShipyardOrderQueue[0];

	UFlareFactory* Factory = GetCompatibleIdleShipyardFactory(NextOrder.ShipClass);

	if(!Factory)
	{
		return FText(LOCTEXT("NextShipyardOrderStatusWaitFactory", "Waiting for an available production line"));
	}

	const FFlareProductionData& ProductionData = GetCycleDataForShipClass(NextOrder.ShipClass);

	FText MissingResources;
	int32 MissingResourcesCount = 0;

	for(const FFlareFactoryResource& InputResource : ProductionData.InputResources)
	{
		int32 AvailableQuantity = GetCargoBay()->GetResourceQuantity(&InputResource.Resource->Data, GetCompany());
		if(InputResource.Quantity > AvailableQuantity)
		{
			// Not enought resource
			MissingResources = FText::Format(LOCTEXT("NextShipyardOrderStatusMissingResource", "{0}{1}{2} {3}"),
			MissingResources,
			MissingResources.IsEmpty() ? FText() : LOCTEXT("NextShipyardOrderStatusMissingResourceSeparator", ", "),
			FText::AsNumber(InputResource.Quantity - AvailableQuantity),
			InputResource.Resource->Data.Name);
			++MissingResourcesCount;
		}
	}

	if(MissingResourcesCount > 0)
	{
		return FText::Format(LOCTEXT("NextShipyardOrderStatusWaitResources", "Waiting for resources ({0})"), MissingResources);
	}
	else
	{
		return LOCTEXT("NextShipyardOrderStatusUnknownCause", "Construction starting soon");
	}
}

UFlareFactory* UFlareSimulatedSpacecraft::GetCompatibleIdleShipyardFactory(FName ShipIdentifier)
{
	FFlareSpacecraftDescription* Desc = GetGame()->GetSpacecraftCatalog()->Get(ShipIdentifier);

	for (UFlareFactory* Factory : Factories)
	{
		if((Factory->IsSmallShipyard() && Desc->Size == EFlarePartSize::S) ||
			(Factory->IsLargeShipyard() && Desc->Size == EFlarePartSize::L))
		{
			if(!Factory->IsActive())
			{
				return Factory;
			}
		}
	}
	return nullptr;
}

void UFlareSimulatedSpacecraft::UpdateShipyardProduction()
{
	TArray<int32> IndexToRemove;

	int32 Index = 0;
	for (FFlareShipyardOrderSave& Order : SpacecraftData.ShipyardOrderQueue)
	{
		FLOGV("Order %s for %s", *Order.ShipClass.ToString(), *Order.Company.ToString())

		bool MissingResource = false;
		const FFlareProductionData& ProductionData = GetCycleDataForShipClass(Order.ShipClass);

		for (const FFlareFactoryResource& InputResource : ProductionData.InputResources)
		{
			if(InputResource.Quantity > GetCargoBay()->GetResourceQuantity(&InputResource.Resource->Data, GetCompany()))
			{
				// Missing resources, stop all
				MissingResource = true;
				break;
			}
		}

		if(MissingResource)
		{
			FLOG("- Missing resource");
			break;
		}

		UFlareFactory* Factory = GetCompatibleIdleShipyardFactory(Order.ShipClass);

		if(Factory)
		{
			FLOG("- Start");
			Factory->StartShipBuilding(Order);
			IndexToRemove.Add(Index);
		}
		else
		{
			FLOG("- No factory");
		}
		Index++;
	}

	for (int i = IndexToRemove.Num()-1 ; i >= 0; --i)
	{
		SpacecraftData.ShipyardOrderQueue.RemoveAt(IndexToRemove[i]);
	}
}

bool UFlareSimulatedSpacecraft::CanOrder(const FFlareSpacecraftDescription* ShipDescription, UFlareCompany* OrderCompany)
{
	if(!IsShipyard())
	{
		return false;
	}

	if(GetCompany()->GetWarState(OrderCompany) == EFlareHostility::Hostile)
	{
		// Not possible to buy ship to hostile company
		return false;
	}

	if(!IsAllowExternalOrder() && OrderCompany != GetCompany())
	{
		return false;
	}

	UFlareCompany* PlayerCompany = Game->GetPC()->GetCompany();

	if(OrderCompany == PlayerCompany)
	{
		for (FFlareShipyardOrderSave& Order : SpacecraftData.ShipyardOrderQueue)
		{
			if (Order.Company == PlayerCompany->GetIdentifier())
			{
				FFlareSpacecraftDescription* OrderShip = GetGame()->GetSpacecraftCatalog()->Get(Order.ShipClass);

				if(OrderShip->Size == ShipDescription->Size)
				{
					return false;
				}
			}
		}
	}
	else
	{
		if(SpacecraftData.ShipyardOrderQueue.Num() > 0)
		{
			if(SpacecraftData.ShipyardOrderQueue[SpacecraftData.ShipyardOrderQueue.Num()-1].Company != PlayerCompany->GetIdentifier())
			{
				return false;
			}
		}
	}

	return true;
}

bool UFlareSimulatedSpacecraft::IsShipyardMissingResources()
{
	bool MissingResource = false;
	if(SpacecraftData.ShipyardOrderQueue.Num() > 0)
	{
		const FFlareProductionData& ProductionData = GetCycleDataForShipClass(SpacecraftData.ShipyardOrderQueue[0].ShipClass);

		for (const FFlareFactoryResource& InputResource : ProductionData.InputResources)
		{
			if(InputResource.Quantity > GetCargoBay()->GetResourceQuantity(&InputResource.Resource->Data, GetCompany()))
			{
				// Missing resources, stop all
				MissingResource = true;
				break;
			}
		}
	}

	return MissingResource;
}

const FFlareProductionData& UFlareSimulatedSpacecraft::GetCycleDataForShipClass(FName ShipIdentifier)
{
	return GetGame()->GetSpacecraftCatalog()->Get(ShipIdentifier)->CycleCost;
}

int32 UFlareSimulatedSpacecraft::GetShipProductionTime(FName ShipIdentifier)
{
	return GetCycleDataForShipClass(ShipIdentifier).ProductionTime;
}

int32 UFlareSimulatedSpacecraft::GetEstimatedQueueAndProductionDuration(FName ShipIdentifier, int32 OrderIndex)
{
	if(OrderIndex >= SpacecraftData.ShipyardOrderQueue.Num())
	{
		FLOGV("WARNING: invalid OrderIndex %d : ShipyardOrderQueue have %d elements", OrderIndex, SpacecraftData.ShipyardOrderQueue.Num());
		return 0;
	}

	FFlareSpacecraftDescription* Desc = GetGame()->GetSpacecraftCatalog()->Get(ShipIdentifier);

	int32 Duration = 0;

	TArray<int32> ShipyardSimulators;

	for (UFlareFactory* Factory : Factories)
	{
		if((Factory->IsSmallShipyard() && Desc->Size == EFlarePartSize::S) ||
			(Factory->IsLargeShipyard() && Desc->Size == EFlarePartSize::L))
		{
			if(Factory->IsActive())
			{
				ShipyardSimulators.Add(Factory->GetRemainingProductionDuration());
			}
			else
			{
				ShipyardSimulators.Add(0);
			}
		}
	}

	if(ShipyardSimulators.Num() == 0)
	{
		FLOG("WARNING: no shipyard simulator");
		return 0;
	}

	if(OrderIndex < 0)
	{
		OrderIndex = SpacecraftData.ShipyardOrderQueue.Num();
	}

	int32 NextOrderIndex = 0;

	while(true)
	{
		int32 MinFactoryDuration = INT32_MAX;
		// Find the minimum duration
		for (int32 ProductionDuration : ShipyardSimulators)
		{
			if(ProductionDuration < MinFactoryDuration)
			{
				MinFactoryDuration = ProductionDuration;
			}
		}

		Duration += MinFactoryDuration;

		// Remove this duration to shipyard
		for (int32& ProductionDuration : ShipyardSimulators)
		{
			ProductionDuration -= MinFactoryDuration;

			if (ProductionDuration <= 0)
			{
				// Try to queue
				if(NextOrderIndex == OrderIndex)
				{
					// Enqueue the target, return duration
					return  Duration;
				}

				FFlareShipyardOrderSave& Order = SpacecraftData.ShipyardOrderQueue[NextOrderIndex];
				int32 ShipProductionDuration = GetShipProductionTime(Order.ShipClass);
				ProductionDuration = ShipProductionDuration;
				NextOrderIndex++;
			}
		}
	}
}

FText UFlareSimulatedSpacecraft::GetShipCost(FName ShipIdentifier)
{
	FText ProductionCostText;
	FText CommaTextReference = LOCTEXT("Comma", " +");

	const FFlareProductionData& ProductionData = GetCycleDataForShipClass(ShipIdentifier);


	// Cycle cost in credits
	uint32 CycleProductionCost = ProductionData.ProductionCost;
	if (CycleProductionCost > 0)
	{
		ProductionCostText = FText::Format(LOCTEXT("ShipProductionCostFormat", "{0} credits"), FText::AsNumber(UFlareGameTools::DisplayMoney(CycleProductionCost)));
	}

	// Cycle cost in resources
	for (int ResourceIndex = 0; ResourceIndex < ProductionData.InputResources.Num(); ResourceIndex++)
	{
		FText CommaText = ProductionCostText.IsEmpty() ? FText() : CommaTextReference;
		const FFlareFactoryResource* FactoryResource = &ProductionData.InputResources[ResourceIndex];
		FCHECK(FactoryResource);

		ProductionCostText = FText::Format(LOCTEXT("ShipProductionResourcesFormat", "{0}{1} {2} {3}"),
			ProductionCostText, CommaText, FText::AsNumber(FactoryResource->Quantity), FactoryResource->Resource->Data.Acronym);
	}

	return ProductionCostText;
}

bool UFlareSimulatedSpacecraft::IsAllowExternalOrder()
{
	return SpacecraftData.AllowExternalOrder;
}

void UFlareSimulatedSpacecraft::SetAllowExternalOrder(bool Allow)
{
	SpacecraftData.AllowExternalOrder = Allow;
}

const FFlareProductionData* UFlareSimulatedSpacecraft::GetNextOrderShipProductionData(EFlarePartSize::Type Size)
{
	for (FFlareShipyardOrderSave& Order : SpacecraftData.ShipyardOrderQueue)
	{
		FFlareSpacecraftDescription* Desc = GetGame()->GetSpacecraftCatalog()->Get(Order.ShipClass);

		if (Desc->Size == Size)
		{
			return &GetCycleDataForShipClass(Order.ShipClass);
		}
	}
	return NULL;
}

int64 UFlareSimulatedSpacecraft::GetUpgradeCost(FFlareSpacecraftComponentDescription* NewPart, FFlareSpacecraftComponentDescription* OldPart)
{
	return NewPart->Cost - OldPart->Cost;
}

bool UFlareSimulatedSpacecraft::CanUpgrade(EFlarePartType::Type Type)
{
	if(!GetCurrentSector()->CanUpgrade(GetCompany()))
	{
		return false;
	}

	bool CanBeChanged = false;

	switch(Type)
	{
		case EFlarePartType::RCS:
			CanBeChanged = (GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_RCS) >= 0.99f);
			break;
		case EFlarePartType::OrbitalEngine:
			CanBeChanged = (GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Propulsion) >= 0.99f);
			break;
		case EFlarePartType::Weapon:
			CanBeChanged = (GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_WeaponAndAmmo) >= 0.99f);
			break;
	}
	return CanBeChanged;
}

EFlareHostility::Type UFlareSimulatedSpacecraft::GetPlayerWarState() const
{
	return GetCompany()->GetPlayerWarState();
}

int32 UFlareSimulatedSpacecraft::GetCapturePoint(UFlareCompany* Company) const
{
	if(SpacecraftData.CapturePoints.Contains(Company->GetIdentifier()))
	{
		return SpacecraftData.CapturePoints[Company->GetIdentifier()];
	}
	return 0;
}

bool UFlareSimulatedSpacecraft::IsBeingCaptured() const
{
	return SpacecraftData.CapturePoints.Num() > 0;
}

int32 UFlareSimulatedSpacecraft::GetCapturePointThreshold() const
{
	int32 BaseCapturePoint = SpacecraftData.Level * SpacecraftDescription->CapturePointThreshold;
	float DamageRatio = GetDamageSystem()->GetGlobalDamageRatio();
	float CaptureRatio = CAPTURE_THRESOLD_MIN;

	if (DamageRatio > CAPTURE_THRESOLD_MIN_DAMAGE)
	{
		// if Damage ratio == 1 , Capture ratio == 1
		// if Damage ratio == CAPTURE_THRESOLD_MIN_DAMAGE , Capture ratio == CAPTURE_THRESOLD_MIN
		float Coef = (CAPTURE_THRESOLD_MIN - CAPTURE_THRESOLD_MIN_DAMAGE) / (1.f - CAPTURE_THRESOLD_MIN_DAMAGE);
		CaptureRatio = (1.f - Coef) * DamageRatio + Coef;
	}

	return FMath::CeilToInt(BaseCapturePoint * CaptureRatio);
}

float UFlareSimulatedSpacecraft::GetStationEfficiency()
{
	// if Damage ratio == 1 ,  efficiency == 1
	// if Damage ratio == STATION_DAMAGE_THRESOLD , efficiency = 0
	float DamageRatio = GetDamageSystem()->GetGlobalHealth();
	float Efficiency = 0.f;

	if (DamageRatio > STATION_DAMAGE_THRESOLD)
	{
		float Coef = -STATION_DAMAGE_THRESOLD / (1.f - STATION_DAMAGE_THRESOLD);
		Efficiency = (1.f - Coef) * DamageRatio + Coef;
	}

	return Efficiency;
}

int32 UFlareSimulatedSpacecraft::GetCombatPoints(bool ReduceByDamage)
{
	if (!IsMilitary() || (ReduceByDamage && GetDamageSystem()->IsDisarmed()))
	{
		return 0;
	}

	int32 SpacecraftCombatPoints = GetDescription()->CombatPoints;

	int32 WeaponGroupCount = GetDescription()->WeaponGroups.Num();

	for(int32 WeaponGroupIndex = 0; WeaponGroupIndex < WeaponGroupCount; WeaponGroupIndex++)
	{
		FFlareSpacecraftComponentDescription* CurrentWeapon = GetCurrentPart(EFlarePartType::Weapon, WeaponGroupIndex);
		SpacecraftCombatPoints += CurrentWeapon->CombatPoints;
	}


	FFlareSpacecraftComponentDescription* CurrentPart = GetCurrentPart(EFlarePartType::RCS, 0);
	SpacecraftCombatPoints += CurrentPart->CombatPoints;

	CurrentPart = GetCurrentPart(EFlarePartType::OrbitalEngine, 0);
	SpacecraftCombatPoints += CurrentPart->CombatPoints;

	if(ReduceByDamage)
	{
		SpacecraftCombatPoints *= GetDamageSystem()->GetGlobalHealth();
	}

	return SpacecraftCombatPoints;
}

bool UFlareSimulatedSpacecraft::IsPlayerShip()
{
	return (this == GetGame()->GetPC()->GetPlayerShip());
}

bool UFlareSimulatedSpacecraft::IsRepairing()
{
	return GetRepairStock() > 0 && GetDamageSystem()->GetGlobalDamageRatio() < 1.f;
}

bool UFlareSimulatedSpacecraft::IsRefilling()
{
	return GetRefillStock() > 0 && NeedRefill();
}

int32 UFlareSimulatedSpacecraft::GetRepairDuration()
{
	if(GetRepairStock() <= 0 || (GetCurrentSector() && GetCurrentSector()->IsInDangerousBattle(GetCompany())))
	{
		// No repair possible
		return 0;
	}

	UFlareSpacecraftComponentsCatalog* Catalog = GetGame()->GetShipPartsCatalog();

	float SpacecraftPreciseCurrentNeededFleetSupply = 0;

	// List components
	for (int32 ComponentIndex = 0; ComponentIndex < GetData().Components.Num(); ComponentIndex++)
	{
		FFlareSpacecraftComponentSave* ComponentData = &GetData().Components[ComponentIndex];
		FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

		float DamageRatio = GetDamageSystem()->GetDamageRatio(ComponentDescription, ComponentData);
		float TechnologyBonus = GetCompany()->IsTechnologyUnlocked("quick-repair") ? 1.5f: 1.f;
		float ComponentMaxRepairRatio = SectorHelper::GetComponentMaxRepairRatio(ComponentDescription) * (GetSize() == EFlarePartSize::L ? 0.2f : 1.f) * TechnologyBonus;
		float CurrentRepairRatio = FMath::Min(ComponentMaxRepairRatio, (1.f - DamageRatio));


		SpacecraftPreciseCurrentNeededFleetSupply += CurrentRepairRatio * UFlareSimulatedSpacecraftDamageSystem::GetRepairCost(ComponentDescription);
	}

	int32 ShipRepairDuration = 0;

	if(SpacecraftPreciseCurrentNeededFleetSupply != 0)
	{
		float MaxRepairRatio = FMath::Min(1.f, GetRepairStock() / SpacecraftPreciseCurrentNeededFleetSupply);

		for (int32 ComponentIndex = 0; ComponentIndex < GetData().Components.Num(); ComponentIndex++)
		{
			FFlareSpacecraftComponentSave* ComponentData = &GetData().Components[ComponentIndex];
			FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

			float TechnologyBonus = GetCompany()->IsTechnologyUnlocked("quick-repair") ? 1.5f: 1.f;
			float ComponentMaxRepairRatio = SectorHelper::GetComponentMaxRepairRatio(ComponentDescription) * (GetSize() == EFlarePartSize::L ? 0.2f : 1.f) * TechnologyBonus;
			float DamageRatio = GetDamageSystem()->GetDamageRatio(ComponentDescription, ComponentData);


			float RepairRatio = MaxRepairRatio * (1.f - DamageRatio);

			float PartRepairDurationFloat = RepairRatio/ComponentMaxRepairRatio;
			int32 PartRepairDuration = FMath::CeilToInt( FMath::RoundToFloat(PartRepairDurationFloat * 10) / 10);

			if(PartRepairDuration > ShipRepairDuration)
			{
				ShipRepairDuration = PartRepairDuration;
			}
		}
	}

	return ShipRepairDuration;
}

int32 UFlareSimulatedSpacecraft::GetRefillDuration()
{
	if(GetRefillStock() == 0 || (GetCurrentSector() && GetCurrentSector()->IsInDangerousBattle(GetCompany())))
	{
		// No refill possible
		return 0;
	}

	UFlareSpacecraftComponentsCatalog* Catalog = GetGame()->GetShipPartsCatalog();
	float SpacecraftPreciseCurrentNeededFleetSupply = 0;

	// List components
	for (int32 ComponentIndex = 0; ComponentIndex < GetData().Components.Num(); ComponentIndex++)
	{
		FFlareSpacecraftComponentSave* ComponentData = &GetData().Components[ComponentIndex];
		FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);
		if(ComponentDescription->Type == EFlarePartType::Weapon)
		{
			int32 MaxAmmo = ComponentDescription->WeaponCharacteristics.AmmoCapacity;
			int32 CurrentAmmo = MaxAmmo - ComponentData->Weapon.FiredAmmo;
			float FillRatio = (float) CurrentAmmo / (float) MaxAmmo;
			float CurrentRefillRatio = FMath::Min(MAX_REFILL_RATIO_BY_DAY * (GetSize() == EFlarePartSize::L ? 0.2f : 1.f), (1.f - FillRatio));

			SpacecraftPreciseCurrentNeededFleetSupply += CurrentRefillRatio * UFlareSimulatedSpacecraftDamageSystem::GetRefillCost(ComponentDescription);
		}

	}

	int32 ShipRefillDuration = 0;

	if(SpacecraftPreciseCurrentNeededFleetSupply != 0)
	{
		float MaxRefillRatio = FMath::Min(1.f,GetRefillStock() / SpacecraftPreciseCurrentNeededFleetSupply);

		for (int32 ComponentIndex = 0; ComponentIndex < GetData().Components.Num(); ComponentIndex++)
		{
			FFlareSpacecraftComponentSave* ComponentData = &GetData().Components[ComponentIndex];
			FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

			if(ComponentDescription->Type == EFlarePartType::Weapon)
			{
				int32 MaxAmmo = ComponentDescription->WeaponCharacteristics.AmmoCapacity;
				int32 CurrentAmmo = MaxAmmo - ComponentData->Weapon.FiredAmmo;
				float FillRatio = (float) CurrentAmmo / (float) MaxAmmo;
				float RefillRatio = MaxRefillRatio * (1.f - FillRatio);


				int32 PartRefillDuration = FMath::CeilToInt(RefillRatio/(MAX_REFILL_RATIO_BY_DAY * (GetSize() == EFlarePartSize::L ? 0.2f : 1.f)));

				if(PartRefillDuration > ShipRefillDuration)
				{
					ShipRefillDuration = PartRefillDuration;
				}
			}
		}
	}

	return ShipRefillDuration;
}



bool UFlareSimulatedSpacecraft::IsLastPlayerShip()
{
	if(GetCurrentFleet() == GetGame()->GetPC()->GetPlayerFleet())
	{
		for(UFlareSimulatedSpacecraft* OtherSpacecraft : GetCurrentFleet()->GetShips())
		{
			if(OtherSpacecraft == this)
			{
				continue;
			}

			if(!OtherSpacecraft->GetDamageSystem()->IsUncontrollable())
			{
				return false;
			}
		}

		return true;
	}
	else
	{
		return false;
	}
}


bool UFlareSimulatedSpacecraft::IsResponsible(EFlareDamage::Type DamageType)
{
	UFlareSimulatedSpacecraft* PlayerShip = GetGame()->GetPC()->GetPlayerShip();

	if(this != PlayerShip)
	{
		return false;
	}

	if(PlayerShip->GetSize() == EFlarePartSize::S)
	{
		return true;
	}
	else
	{
		if(PlayerShip->IsActive())
		{
			if (PlayerShip->GetActive()->GetWeaponsSystem()->IsInFireDirector())
			{
				if (DamageType == EFlareDamage::DAM_Collision)
				{
					return false;
				}
				else
				{
					return true;
				}

			}
			else
			{
				if (DamageType == EFlareDamage::DAM_Collision)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
		}
		else
		{
			return false;
		}
	}
}



const FSlateBrush* FFlareSpacecraftDescription::GetIcon(FFlareSpacecraftDescription* Characteristic)
{
	if (Characteristic)
	{
		if (Characteristic->IsStation())
		{
			return FFlareStyleSet::GetIcon("SS");
		}
		else if (Characteristic->IsMilitary())
		{
			if (Characteristic->Size == EFlarePartSize::S)
			{
				return FFlareStyleSet::GetIcon("MS");
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
			else if (Characteristic->Size == EFlarePartSize::L)
			{
				return FFlareStyleSet::GetIcon("CL");
			}
		}
	}
	return NULL;
}

#undef LOCTEXT_NAMESPACE
