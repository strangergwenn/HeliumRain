
#include "FlareDebrisField.h"
#include "../Flare.h"
#include "FlareGame.h"
#include "FlareSimulatedSector.h"

#include "StaticMeshResources.h"


#define LOCTEXT_NAMESPACE "FlareDebrisField"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareDebrisField::UFlareDebrisField(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CurrentGenerationIndex = 0;
}

void UFlareDebrisField::Setup(AFlareGame* GameMode, UFlareSimulatedSector* Sector)
{
	FCHECK(Sector);
	FCHECK(GameMode);

	// Get debris field parameters
	Game = GameMode;
	const FFlareDebrisFieldInfo* DebrisFieldInfo = &Sector->GetDescription()->DebrisFieldInfo;
	UFlareAsteroidCatalog* DebrisFieldMeshes = DebrisFieldInfo->DebrisCatalog;
	TArray<AStaticMeshActor*> NewDebris;

	// Add debris
	if (DebrisFieldInfo && DebrisFieldMeshes)
	{
		float SectorScale = 5000 * 100;
		int32 DebrisCount = 100 * DebrisFieldInfo->DebrisFieldDensity;
		FLOGV("UFlareDebrisField::Setup : debris catalog is %s", *DebrisFieldMeshes->GetName());
		FLOGV("UFlareDebrisField::Setup : spawning debris field : gen %d, size = %d, icy = %d", CurrentGenerationIndex, DebrisCount, Sector->GetDescription()->IsIcy);

		for (int32 Index = 0; Index < DebrisCount; Index++)
		{
			int32 DebrisIndex = FMath::RandRange(0, DebrisFieldMeshes->Asteroids.Num() - 1);

			float MinSize = DebrisFieldInfo->MinDebrisSize;
			float MaxSize = DebrisFieldInfo->MaxDebrisSize;
			float Size = FMath::FRandRange(MinSize, MaxSize);

			FName Name = FName(*(FString::Printf(TEXT("DebrisGen%dIndex%d"), CurrentGenerationIndex, Index)));

			NewDebris.Add(AddDebris(Sector, DebrisFieldMeshes->Asteroids[DebrisIndex], Size, SectorScale, Name));
		}
	}
	else
	{
		FLOG("UFlareDebrisField::Setup : debris catalog not available, skipping");
	}

	// Remember new debris
	DebrisField.Empty();
	for (int DebrisIndex = 0; DebrisIndex < NewDebris.Num(); DebrisIndex++)
	{
		if (NewDebris[DebrisIndex])
		{
			DebrisField.Add(NewDebris[DebrisIndex]);
		}
	}

	CurrentGenerationIndex++;
}

void UFlareDebrisField::Reset()
{
	FLOGV("UFlareDebrisField::Reset : clearing debris field, size = %d", DebrisField.Num());
	for (int i = 0; i < DebrisField.Num(); i++)
	{
		Game->GetWorld()->DestroyActor(DebrisField[i]);
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
	Params.Owner = Game;
	Params.bNoFail = false;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;

	// Compute size, location and rotation
	FVector Location = FMath::VRand() * SectorScale * FMath::FRandRange(0.2, 1.0);
	FRotator Rotation = FRotator(FMath::FRandRange(0, 360), FMath::FRandRange(0, 360), FMath::FRandRange(0, 360));

	// Spawn
	AStaticMeshActor* DebrisMesh = Game->GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, Params);
	if (DebrisMesh)
	{
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

			int32 LODCOunt = DebrisComponent->GetStaticMesh()->GetNumLODs();

			// Set material
			UMaterialInstanceDynamic* DebrisMaterial = UMaterialInstanceDynamic::Create(DebrisComponent->GetMaterial(0), DebrisComponent->GetWorld());
			if (DebrisMaterial)
			{
				for (int32 i = 0; i < LODCOunt; i++)
				{
					DebrisComponent->SetMaterial(i, DebrisMaterial);
				}
				DebrisMaterial->SetScalarParameterValue("IceMask", Sector->GetDescription()->IsIcy);
			}
			else
			{
				FLOG("UFlareDebrisField::AddDebris : failed to set material (no material or mesh)")
			}
		}
	}
	else
	{
		FLOG("UFlareDebrisField::AddDebris : failed to spawn debris")
	}

	return DebrisMesh;
}


#undef LOCTEXT_NAMESPACE
