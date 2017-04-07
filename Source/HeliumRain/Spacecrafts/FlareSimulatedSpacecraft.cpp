
#include "../Flare.h"
#include "../Game/FlareSimulatedSector.h"
#include "../Game/FlareGame.h"
#include "../Game/FlareFleet.h"
#include "../Game/FlareWorld.h"
#include "../Player/FlarePlayerController.h"
#include "../Economy/FlareCargoBay.h"
#include "../Economy/FlareFactory.h"
#include "../Data/FlareFactoryCatalogEntry.h"
#include "FlareSimulatedSpacecraft.h"


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
				FactoryData.OrderShipClass = NAME_None;
				FactoryData.OrderShipCompany = NAME_None;
				FactoryData.OrderShipAdvancePayment = 0;
		}

		UFlareFactory* Factory = NewObject<UFlareFactory>(GetGame()->GetGameWorld(), UFlareFactory::StaticClass());
		Factory->Load(this, FactoryDescription, FactoryData);
		Factories.Add(Factory);
		if (!IsDestroyed())
		{
			Game->GetGameWorld()->AddFactory(Factory);
		}
	}

	CargoBay = NewObject<UFlareCargoBay>(this, UFlareCargoBay::StaticClass());
	CargoBay->Load(this, SpacecraftData.Cargo);


	// Lock resources
	LockResources();

	if(ActiveSpacecraft)
	{
		ActiveSpacecraft->Load(this);
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
	else if (GetDamageSystem()->IsUncontrollable())
	{
		OutInfo = LOCTEXT("CantFlyDestroyedInfo", "This ship is uncontrollable");
		return false;
	}
	else if (PlayerFleet && GetCurrentFleet() != PlayerFleet)
	{
		OutInfo = LOCTEXT("CantFlyOtherInfo", "You can only switch ships from whithin the same fleet");
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


bool UFlareSimulatedSpacecraft::CanTradeWith(UFlareSimulatedSpacecraft* OtherSpacecraft)
{
	// Check if both spacecraft are in the same sector
	if(GetCurrentSector() != OtherSpacecraft->GetCurrentSector())
	{
		return false;
	}

	if(GetDamageSystem()->IsUncontrollable() || OtherSpacecraft->GetDamageSystem()->IsUncontrollable())
	{
		return false;
	}

	// Check if spacecraft are not both stations
	if(IsStation() && OtherSpacecraft->IsStation())
	{
		return false;
	}

	// Check if spacecraft are not both ships
	if(!IsStation() && !OtherSpacecraft->IsStation())
	{
		return false;
	}

	// Check if spacecraft are are not already trading
	if(IsTrading() || OtherSpacecraft->IsTrading())
	{
		return false;
	}

	// Check if both spacecraft are not at war
	if(GetCompany()->GetWarState(OtherSpacecraft->GetCompany()) == EFlareHostility::Hostile)
	{
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
	for (int FactoryIndex = 0; FactoryIndex < SpacecraftDescription->Factories.Num(); FactoryIndex++)
	{
		FFlareFactoryDescription* FactoryDescription = &SpacecraftDescription->Factories[FactoryIndex]->Data;

		// Is input resource of a station ?
		for (int32 ResourceIndex = 0; ResourceIndex < FactoryDescription->CycleCost.InputResources.Num(); ResourceIndex++)
		{
			const FFlareFactoryResource* FactoryResource = &FactoryDescription->CycleCost.InputResources[ResourceIndex];
			if (&FactoryResource->Resource->Data == Resource)
			{
				return EFlareResourcePriceContext::FactoryInput;
			}
		}

		// Is output resource of a station ?
		for (int32 ResourceIndex = 0; ResourceIndex < FactoryDescription->CycleCost.OutputResources.Num(); ResourceIndex++)
		{
			const FFlareFactoryResource* FactoryResource = &FactoryDescription->CycleCost.OutputResources[ResourceIndex];
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

	if (GetDescription()->Factories.Num() > 0)
	{
		FFlareFactoryDescription* Factory = &GetDescription()->Factories[0]->Data;

		for (int32 ResourceIndex = 0 ; ResourceIndex < Factory->CycleCost.InputResources.Num() ; ResourceIndex++)
		{
			const FFlareFactoryResource* Resource = &Factory->CycleCost.InputResources[ResourceIndex];

			if (!GetCargoBay()->LockSlot(&Resource->Resource->Data, EFlareResourceLock::Input, false))
			{
				FLOGV("Fail to lock a slot of %s in %s", *(&Resource->Resource->Data)->Name.ToString(), *GetImmatriculation().ToString());
			}
		}

		for (int32 ResourceIndex = 0 ; ResourceIndex < Factory->CycleCost.OutputResources.Num() ; ResourceIndex++)
		{
			const FFlareFactoryResource* Resource = &Factory->CycleCost.OutputResources[ResourceIndex];

			if (!GetCargoBay()->LockSlot(&Resource->Resource->Data, EFlareResourceLock::Output, false))
			{
				FLOGV("Fail to lock a slot of %s in %s", *(&Resource->Resource->Data)->Name.ToString(), *GetImmatriculation().ToString());
			}
		}
	}

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
		&& GetCurrentTradeRoute() == NULL)
	{
		FFlareMenuParameterData Data;
		Data.Spacecraft = this;
		Game->GetPC()->Notify(LOCTEXT("TradingStateEnd", "Trading complete"),
			FText::Format(LOCTEXT("TravelEndedFormat", "{0} finished trading in {1}"),
				FText::FromString(GetImmatriculation().ToString()),
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

void UFlareSimulatedSpacecraft::SetRefilling(bool Refilling)
{
	SpacecraftData.IsRefilling = Refilling;
}

void UFlareSimulatedSpacecraft::SetRepairing(bool Repairing)
{
	SpacecraftData.IsRepairing = Repairing;
}

void UFlareSimulatedSpacecraft::SetReserve(bool InReserve)
{
	SpacecraftData.IsReserve = InReserve;
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
	}
	else
	{
		TArray<FName> CapturingCompany;
		SpacecraftData.CapturePoints.GetKeys(CapturingCompany);

		for(int CompanyIndex = 0; CompanyIndex < CapturingCompany.Num(); CompanyIndex++)
		{
			RemoveCapturePoint(CapturingCompany[CompanyIndex], ResetSpeedPoint);
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
			CanBeChanged = (GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_RCS) == 1.0f);
			break;
		case EFlarePartType::OrbitalEngine:
			CanBeChanged = (GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Propulsion) == 1.0f);
			break;
		case EFlarePartType::Weapon:
			CanBeChanged = (GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_WeaponAndAmmo) == 1.0f);
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

int32 UFlareSimulatedSpacecraft::GetCapturePointThreshold() const
{
	int32 BaseCapturePoint = SpacecraftData.Level * SpacecraftDescription->CapturePointThreshold;
	float DamageRatio = GetDamageSystem()->GetGlobalHealth();
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
	if (!IsMilitary() || !GetDamageSystem()->IsAlive() || (ReduceByDamage && GetDamageSystem()->IsDisarmed()))
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
