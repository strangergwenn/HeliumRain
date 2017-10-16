
#include "FlareMeteorite.h"
#include "../Flare.h"
#include "../Game/FlareGame.h"
#include "../Game/FlareGameTools.h"
#include "../Data/FlareMeteoriteCatalog.h"
#include "../Player/FlarePlayerController.h"

#include "DestructibleMesh.h"
#include "DestructibleComponent.h"

#define LOCTEXT_NAMESPACE "FlareMeteorite"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareMeteorite::AFlareMeteorite(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	// Mesh
	Meteorite = PCIP.CreateDefaultSubobject<UDestructibleComponent>(this, TEXT("Meteorite"));
	Meteorite->bTraceComplexOnMove = true;
	Meteorite->SetLinearDamping(0);
	Meteorite->SetAngularDamping(0);
	Meteorite->SetSimulatePhysics(true);
	RootComponent = Meteorite;
	SetActorEnableCollision(true);

	// Physics
	Meteorite->SetMobility(EComponentMobility::Movable);
	Meteorite->SetCollisionProfileName("Destructible");
	Meteorite->GetBodyInstance()->SetUseAsyncScene(false);
	Meteorite->GetBodyInstance()->SetInstanceSimulatePhysics(true);
	Meteorite->SetNotifyRigidBodyCollision(true);

	// Settings
	Meteorite->PrimaryComponentTick.bCanEverTick = true;
	PrimaryActorTick.bCanEverTick = true;
	Paused = false;

	// Content
	static ConstructorHelpers::FObjectFinder<UParticleSystem> RockEffectTemplateObj(TEXT("/Game/Environment/Meteorites/Common/PS_Breakup_Rock.PS_Breakup_Rock"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> MetalEffectTemplateObj(TEXT("/Game/Environment/Meteorites/Common/PS_Breakup_Metal.PS_Breakup_Metal"));
	RockEffectTemplate = RockEffectTemplateObj.Object;
	MetalEffectTemplate = MetalEffectTemplateObj.Object;
}

void AFlareMeteorite::Load(FFlareMeteoriteSave* Data, UFlareSector* ParentSector)
{
	Parent = ParentSector;

	MeteoriteData = Data;
	SetupMeteoriteMesh();
	Meteorite->SetPhysicsLinearVelocity(Data->LinearVelocity);
	Meteorite->SetPhysicsAngularVelocityInDegrees(Data->AngularVelocity);
	LifeTime = 0.f;
}

FFlareMeteoriteSave* AFlareMeteorite::Save()
{
	// Physical data
	MeteoriteData->Location = GetActorLocation();
	MeteoriteData->Rotation = GetActorRotation();
	if (!Paused)
	{
		MeteoriteData->LinearVelocity = Meteorite->GetPhysicsLinearVelocity();
		MeteoriteData->AngularVelocity = Meteorite->GetPhysicsAngularVelocityInDegrees();
	}

	return MeteoriteData;
}

void AFlareMeteorite::BeginPlay()
{
	Super::BeginPlay();
}

void AFlareMeteorite::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(!Target)
	{
		Target = Parent->FindSpacecraft(MeteoriteData->TargetStation);
	}

	float DeltaveSq = (MeteoriteData->LinearVelocity - Meteorite->GetPhysicsLinearVelocity()).SizeSquared();

	float const SafeThresold = 5.f;

	if(LifeTime < SafeThresold)
	{
		LifeTime += DeltaSeconds;
		if(DeltaveSq > 100)
		{
			Meteorite->SetPhysicsLinearVelocity(MeteoriteData->LinearVelocity);
		}
	}

	/*FLOGV("Meteorite %s vel=%s", *GetName(), *Meteorite->GetPhysicsLinearVelocity().ToString());
	FLOGV(" - IsPhysicsCollisionEnabled %d", Meteorite->IsPhysicsCollisionEnabled());
	FLOGV(" - IsPhysicsStateCreated %d", Meteorite->IsPhysicsStateCreated());
	FLOGV(" - IsAnySimulatingPhysics %d", Meteorite->IsAnySimulatingPhysics());
	FLOGV(" - IsAnyRigidBodyAwake %d", Meteorite->IsAnyRigidBodyAwake());
	FLOGV(" - IsCollisionEnabled %d", Meteorite->IsCollisionEnabled());
	FLOGV(" - IsSimulatingPhysics %d", Meteorite->IsSimulatingPhysics());*/

	/*float CollisionSize = Asteroid->GetCollisionShape().GetExtent().Size();
	if (SpawnLocation.Size() <= 0.1)
	{
		SpawnLocation = GetActorLocation();
		DrawDebugSphere(GetWorld(), SpawnLocation, CollisionSize / 2, 16, FColor::Red, true);
	}
	else
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), CollisionSize / 2, 16, FColor::Blue, false);
		DrawDebugLine(GetWorld(), GetActorLocation(), SpawnLocation, FColor::Green, false);
	}*/

	if (LifeTime > SafeThresold)
	{
		if (!IsBroken()  && !MeteoriteData->HasMissed && Target)
		{
			FVector CurrentVelocity =  Meteorite->GetPhysicsLinearVelocity();
			FVector CurrentDirection = CurrentVelocity.GetUnsafeNormal();
			float Velocity = MeteoriteData->LinearVelocity.Size();

			FVector TargetDirection = (Target->GetActorLocation() + MeteoriteData->TargetOffset - GetActorLocation()).GetUnsafeNormal();
			FVector TargetVelocity = TargetDirection * Velocity;
			float Dot = FVector::DotProduct(CurrentDirection, TargetDirection);

			//UKismetSystemLibrary::DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + TargetDirection * 10000,FColor::Green, 1000.f);
			//UKismetSystemLibrary::DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + CurrentDirection * 10000,FColor::Red, 1000.f);

			if (Dot > 0)
			{
				// Don't fix if miss
				FVector DeltaVelocity = TargetVelocity - CurrentVelocity;				
				FVector DeltaVelocityClamped = DeltaVelocity.GetClampedToMaxSize(50.f * DeltaSeconds);
				FVector CorrectedVelocity = CurrentVelocity + DeltaVelocityClamped;
				FVector CorrectedDirection = CorrectedVelocity.GetUnsafeNormal();

				//UKismetSystemLibrary::DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + CorrectedDirection * 10000 ,FColor::Blue, 1000.f);
				Meteorite->SetPhysicsLinearVelocity(CorrectedDirection * Velocity);
			}
			else if (Dot < -0.5f)
			{
				MeteoriteData->HasMissed = true;

				Parent->GetGame()->GetPC()->Notify(LOCTEXT("MeteoriteMiss", "Meteorite miss"),
										FText::Format(LOCTEXT("MeteoriteMissFormat", "A meteorite missed {0}. It's not dangerous anymore."), UFlareGameTools::DisplaySpacecraftName(Target->GetParent())),
										FName("meteorite-miss"),
										EFlareNotification::NT_Military,
										false);
				
				Parent->GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("meteorite-miss-station").PutName("sector", Parent->GetSimulatedSector()->GetIdentifier()));
			}
		}
	}
}


void AFlareMeteorite::OnCollision(class AActor* Other, FVector HitLocation)
{
	AFlareAsteroid* Asteroid = Cast<AFlareAsteroid>(Other);
	AFlareSpacecraft* Spacecraft = Cast<AFlareSpacecraft>(Other);

	if(Asteroid)
	{
		ApplyDamage(MeteoriteData->BrokenDamage, 1.f , HitLocation, EFlareDamage::DAM_Collision, NULL, FString());
	}
	else if (Spacecraft && (Spacecraft->IsStation() || Spacecraft->GetSize() == EFlarePartSize::L))
	{
		if (!IsBroken())
		{
			FVector DeltaVelocity = Spacecraft->GetLinearVelocity() - Meteorite->GetPhysicsLinearVelocity();

			float EnergyBaseRatio = 0.00001f;
			float Energy = EnergyBaseRatio * MeteoriteData->BrokenDamage * DeltaVelocity.SizeSquared() * (Spacecraft->IsStation() ? 0.2f : 0.0001f);

			Spacecraft->GetDamageSystem()->ApplyDamage(Energy, MeteoriteData->BrokenDamage, HitLocation, EFlareDamage::DAM_Collision, NULL, FString());

			// Spawn FX
			UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(
				Spacecraft,
				MeteoriteData->IsMetal ? MetalEffectTemplate : RockEffectTemplate,
				GetActorLocation(),
				FRotator::ZeroRotator,
				false);

			// Notify PC
			Parent->GetGame()->GetPC()->Notify(LOCTEXT("MeteoriteCrash", "Meteorite crashed"),
									FText::Format(LOCTEXT("MeteoriteCrashFormat", "A meteorite crashed on {0}."), UFlareGameTools::DisplaySpacecraftName(Spacecraft->GetParent())),
									FName("meteorite-crash"),
									EFlareNotification::NT_Military,
									false);
			
			// Apply damage
			if (Spacecraft->IsStation())
			{
				Parent->GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("meteorite-hit-station").PutName("sector", Parent->GetSimulatedSector()->GetIdentifier()));
			}
			ApplyDamage(MeteoriteData->BrokenDamage, 1.f , HitLocation, EFlareDamage::DAM_Collision, NULL, FString());
		}
	}
}

void AFlareMeteorite::SetPause(bool Pause)
{
	FLOGV("AFlareMeteorite::SetPause Pause=%d", Pause);
	
	if (Paused == Pause)
	{
		return;
	}

	CustomTimeDilation = (Pause ? 0.f : 1.0);
	if (Pause)
	{
		Save(); // Save must be performed with the old pause state
	}

	Meteorite->SetSimulatePhysics(!Pause);

	Paused = Pause;
	SetActorHiddenInGame(Pause);

	if (!Pause)
	{
		Meteorite->SetPhysicsLinearVelocity(MeteoriteData->LinearVelocity);
		Meteorite->SetPhysicsAngularVelocityInDegrees(MeteoriteData->AngularVelocity);
	}
}

void AFlareMeteorite::SetupMeteoriteMesh()
{
	AFlareGame* Game = Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
	
	if (Game->GetMeteoriteCatalog())
	{
		const TArray<UDestructibleMesh*>& MeteoriteList = MeteoriteData->IsMetal ? Game->GetMeteoriteCatalog()->MetalMeteorites : Game->GetMeteoriteCatalog()->RockMeteorites;
		FCHECK(MeteoriteData->MeteoriteMeshID >= 0 && MeteoriteData->MeteoriteMeshID < MeteoriteList.Num());

		Meteorite->SetDestructibleMesh(MeteoriteList[MeteoriteData->MeteoriteMeshID]);
		Meteorite->GetBodyInstance()->SetMassScale(10000);
		Meteorite->GetBodyInstance()->UpdateMassProperties();
	}
	else
	{
		return;
	}
}

bool AFlareMeteorite::IsBroken()
{
	return MeteoriteData->Damage >= MeteoriteData->BrokenDamage;
}

void AFlareMeteorite::ApplyDamage(float Energy, float Radius, FVector Location, EFlareDamage::Type DamageType, UFlareSimulatedSpacecraft* DamageSource, FString DamageCauser)
{
	FVector CurrentVelocity = Meteorite->GetPhysicsLinearVelocity();
	AFlarePlayerController* PC = Parent->GetGame()->GetPC();
	if (DamageSource == PC->GetShipPawn()->GetParent())
	{
		PC->SignalHit(NULL, DamageType);
	}

	if (!IsBroken())
	{
		MeteoriteData->Damage+= Energy;

		if (IsBroken())
		{
			// Notify PC
			if (DamageType != EFlareDamage::DAM_Collision)
			{
				Parent->GetGame()->GetPC()->Notify(LOCTEXT("MeteoriteDestroyed", "Meteorite destroyed"),
										LOCTEXT("MeteoriteDestroyedFormat", "A meteorite has been destroyed."),
										FName("meteorite-destroyed"),
										EFlareNotification::NT_Military,
										false);
			}

			if (DamageSource == PC->GetShipPawn()->GetParent())
			{
				PC->SetAchievementProgression("ACHIEVEMENT_METEORITE", 1);
			}

			// Spawn FX
			UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(
				PC,
				MeteoriteData->IsMetal ? MetalEffectTemplate : RockEffectTemplate,
				GetActorLocation(),
				FRotator::ZeroRotator,
				false);
			PSC->SetVectorParameter("Velocity", CurrentVelocity);

			// Apply damage
			if (!MeteoriteData->HasMissed)
			{
				Parent->GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("meteorite-destroyed").PutName("sector", Parent->GetSimulatedSector()->GetIdentifier()));
			}			
			Meteorite->ApplyRadiusDamage(FMath::Max(200.f, Energy), Location, 100.f, FMath::Max(200.f, Energy )* 200 , false);
		}
	}
	else
	{
		if (DamageType != EFlareDamage::DAM_Collision)
		{
			Meteorite->ApplyRadiusDamage(FMath::Max(180.f, Energy/ 100 ) , Location, 100.f, FMath::Max(180.f, Energy ) * 500 , false);
		}
	}
}

#undef LOCTEXT_NAMESPACE

