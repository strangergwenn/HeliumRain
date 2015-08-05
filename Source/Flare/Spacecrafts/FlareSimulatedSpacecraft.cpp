
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


UFlareSpacecraftDamageSystem* UFlareSimulatedSpacecraft::GetDamageSystem() const
{
	// TODO
	return NULL;
}

UFlareSpacecraftNavigationSystem* UFlareSimulatedSpacecraft::GetNavigationSystem() const
{
	// TODO
	return NULL;
}

UFlareSpacecraftDockingSystem* UFlareSimulatedSpacecraft::GetDockingSystem() const
{
	// TODO
	return NULL;
}

UFlareSpacecraftWeaponsSystem* UFlareSimulatedSpacecraft::GetWeaponsSystem() const
{
	// TODO
	return NULL;
}
