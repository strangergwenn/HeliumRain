
#include "../Flare.h"
#include "../Game/FlareSimulatedSector.h"
#include "../Game/FlareGame.h"
#include "../Game/FlareWorld.h"
#include "FlareSimulatedSpacecraft.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSimulatedSpacecraft::UFlareSimulatedSpacecraft(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
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

	// Initialize navigation system
	NavigationSystem = NewObject<UFlareSimulatedSpacecraftNavigationSystem>(this, UFlareSimulatedSpacecraftNavigationSystem::StaticClass());
	NavigationSystem->Initialize(this, &SpacecraftData);

	// Initialize docking system
	DockingSystem = NewObject<UFlareSimulatedSpacecraftDockingSystem>(this, UFlareSimulatedSpacecraftDockingSystem::StaticClass());
	DockingSystem->Initialize(this, &SpacecraftData);

	// Initialize weapons system
	WeaponsSystem = NewObject<UFlareSimulatedSpacecraftWeaponsSystem>(this, UFlareSimulatedSpacecraftWeaponsSystem::StaticClass());
	WeaponsSystem->Initialize(this, &SpacecraftData);

}

FFlareSpacecraftSave* UFlareSimulatedSpacecraft::Save()
{
	return &SpacecraftData;
}


UFlareCompany* UFlareSimulatedSpacecraft::GetCompany()
{
	return Game->GetGameWorld()->FindCompany(SpacecraftData.CompanyIdentifier);
}


EFlarePartSize::Type UFlareSimulatedSpacecraft::GetSize()
{
	return SpacecraftDescription->Size;
}

bool UFlareSimulatedSpacecraft::IsMilitary()
{
	return IFlareSpacecraftInterface::IsMilitary(SpacecraftDescription);
}

bool UFlareSimulatedSpacecraft::IsStation()
{
	return IFlareSpacecraftInterface::IsStation(SpacecraftDescription);
}

FName UFlareSimulatedSpacecraft::GetImmatriculation() const
{
	return SpacecraftData.Immatriculation;
}

UFlareSimulatedSpacecraftDamageSystem* UFlareSimulatedSpacecraft::GetDamageSystem() const
{
	return DamageSystem;
}

UFlareSimulatedSpacecraftNavigationSystem* UFlareSimulatedSpacecraft::GetNavigationSystem() const
{
	return NavigationSystem;
}

UFlareSimulatedSpacecraftDockingSystem* UFlareSimulatedSpacecraft::GetDockingSystem() const
{
	return DockingSystem;
}

UFlareSimulatedSpacecraftWeaponsSystem* UFlareSimulatedSpacecraft::GetWeaponsSystem() const
{
	return WeaponsSystem;
}

void UFlareSimulatedSpacecraft::InvalidateLocation()
{
	SpacecraftData.Location = FVector::ZeroVector;
	SpacecraftData.SafeLocation = false;
}
