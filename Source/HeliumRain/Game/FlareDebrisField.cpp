
#include "../Flare.h"
#include "FlareGame.h"
#include "FlareDebrisField.h"
#include "FlareSimulatedSector.h"

#include "StaticMeshResources.h"


#define LOCTEXT_NAMESPACE "FlareDebrisField"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareDebrisField::UFlareDebrisField(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareDebrisField::Load(AFlareGame* GameMode, UFlareSimulatedSector* Sector)
{
	Game = GameMode;
	const FFlareDebrisFieldInfo* DebrisFieldInfo = &Sector->GetDescription()->DebrisFieldInfo;
	UFlareAsteroidCatalog* DebrisFieldMeshes = DebrisFieldInfo->DebrisCatalog;

	if (DebrisFieldInfo)
	{
		float SectorScale = 5000 * 100;
		int32 DebrisCount = 100 * DebrisFieldInfo->DebrisFieldDensity;
		FLOGV("UFlareDebrisField::Load : Spawning debris field, size = %d", DebrisCount);

		for (int32 Index = 0; Index < DebrisCount; Index++)
		{
			int32 DebrisIndex = FMath::RandRange(0, DebrisFieldMeshes->Asteroids.Num() - 1);

			float MinSize = DebrisFieldInfo->MinDebrisSize;
			float MaxSize = DebrisFieldInfo->MaxDebrisSize;
			float Size = FMath::FRandRange(MinSize, MaxSize);

			FName Name = FName(*(FString::Printf(TEXT("Debris%d"), Index)));

			AddDebris(Sector, DebrisFieldMeshes->Asteroids[DebrisIndex], Size, SectorScale, Name);
		}
	}
}

void UFlareDebrisField::Reset()
{
	FLOGV("UFlareDebrisField::Reset : Cleaning debris field, size = %d", DebrisField.Num());
	for (int DebrisIndex = 0; DebrisIndex < DebrisField.Num(); DebrisIndex++)
	{
		DebrisField[DebrisIndex]->Destroy();
	}
	DebrisField.Empty();
}

void UFlareDebrisField::SetWorldPause(bool Pause)
{
	for (int i = 0; i < DebrisField.Num(); i++)
	{
		DebrisField[i]->SetActorHiddenInGame(Pause);
		DebrisField[i]->CustomTimeDilation = (Pause ? 0.f : 1.0);
		Cast<UPrimitiveComponent>(DebrisField[i]->GetRootComponent())->SetSimulatePhysics(!Pause);
	}
}


/*----------------------------------------------------
	Internals
----------------------------------------------------*/

AStaticMeshActor* UFlareDebrisField::AddDebris(UFlareSimulatedSector* Sector, UStaticMesh* Mesh, float Size, float SectorScale, FName Name)
{
	FActorSpawnParameters Params;
	Params.Name = Name;
	Params.bNoFail = false;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;

	// Compute size, location and rotation
	FVector Location = FMath::VRand() * SectorScale * FMath::FRandRange(0.2, 1.0);
	FRotator Rotation = FRotator(FMath::FRandRange(0, 360), FMath::FRandRange(0, 360), FMath::FRandRange(0, 360));

	// Spawn
	AStaticMeshActor* DebrisMesh = Game->GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, Params);
	DebrisMesh->SetMobility(EComponentMobility::Movable);
	DebrisMesh->SetActorScale3D(Size * FVector(1, 1, 1));
	DebrisMesh->SetActorEnableCollision(true);

	// Setup
	UStaticMeshComponent* DebrisComponent = DebrisMesh->GetStaticMeshComponent();
	if (DebrisComponent)
	{
		DebrisComponent->SetStaticMesh(Mesh);
		DebrisComponent->SetSimulatePhysics(true);
		DebrisComponent->SetCollisionProfileName("BlockAllDynamic");

		// Set material
		if (DebrisComponent->GetMaterial(0)->IsA(UMaterialInstanceConstant::StaticClass()))
		{
			UMaterialInstanceDynamic* DebrisMaterial = UMaterialInstanceDynamic::Create(DebrisComponent->GetMaterial(0), DebrisComponent->GetWorld());
			if (DebrisMaterial && DebrisComponent->StaticMesh)
			{
				for (int32 LodIndex = 0; LodIndex < DebrisComponent->StaticMesh->RenderData->LODResources.Num(); LodIndex++)
				{
					DebrisComponent->SetMaterial(LodIndex, DebrisMaterial);
				}
				DebrisMaterial->SetScalarParameterValue("IceMask", Sector->GetDescription()->IsIcy);
			}
			else
			{
				FLOG("UFlareDebrisField::AddDebris : failed to set material (no material or mesh)")
			}
		}
		else
		{
			FLOG("UFlareDebrisField::AddDebris : failed to set material (not a constant material instance)")
		}
	}
	else
	{
		FLOG("UFlareDebrisField::AddDebris : failed to spawn debris")
	}

	DebrisField.Add(DebrisMesh);
	return DebrisMesh;
}


#undef LOCTEXT_NAMESPACE
