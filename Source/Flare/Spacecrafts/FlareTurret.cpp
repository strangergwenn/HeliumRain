
#include "../Flare.h"
#include "FlareTurret.h"
#include "FlareSpacecraft.h"
#include "FlareShell.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareTurret::UFlareTurret(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, TurretComponent(NULL)
	, BarrelComponent(NULL)
{
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareTurret::Initialize(const FFlareSpacecraftComponentSave* Data, UFlareCompany* Company, AFlareSpacecraftPawn* OwnerShip, bool IsInMenu)
{
	Super::Initialize(Data, Company, OwnerShip, IsInMenu);
	// TODO Save angles
	TurretAngle = 0;
	BarrelAngle = 0;

	// Initialize pilot
	Pilot = NewObject<UFlareTurretPilot>(this, UFlareTurretPilot::StaticClass());
	Pilot->Initialize(&(Data->Pilot), Company, this);
}


void UFlareTurret::SetupComponentMesh()
{
	Super::SetupComponentMesh();

	// Turret Mesh
	if (Spacecraft && ComponentDescription && ComponentDescription->TurretCharacteristics.TurretMesh)
	{

		TurretComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("TurretMesh"));
		 if(TurretComponent)
		 {
			TurretComponent->RegisterComponent();
			TurretComponent->AttachTo(this);
			TurretComponent->SetStaticMesh(ComponentDescription->TurretCharacteristics.TurretMesh);
			TurretComponent->SetMaterial(0, ComponentDescription->TurretCharacteristics.TurretMesh->GetMaterial(0));
			// TODO Material customization
		}
	}

	// Barrel Mesh
	if (Spacecraft && ComponentDescription && ComponentDescription->TurretCharacteristics.BarrelsMesh)
	{

		BarrelComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass() , TEXT("BarrelMesh"));
		 if (BarrelComponent)
		 {
			BarrelComponent->RegisterComponent();
			if (TurretComponent)
			{
				BarrelComponent->AttachTo(TurretComponent, FName("Axis"));
			}
			else
			{
				BarrelComponent->AttachTo(this);
			}
			BarrelComponent->SetStaticMesh(ComponentDescription->TurretCharacteristics.BarrelsMesh);
			BarrelComponent->SetMaterial(0, ComponentDescription->TurretCharacteristics.BarrelsMesh->GetMaterial(0));
			// TODO Material customization
		}
	}
}



void UFlareTurret::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	FVector AimDirection = FVector::ZeroVector;
	if (Pilot)
	{

		Pilot->TickPilot(DeltaTime);
		//FLOGV("Pilot exist WantFire %d", Pilot->IsWantFire());
		if(Pilot->IsWantFire())
		{
			StartFire();
		}
		else
		{
			StopFire();
		}
		AimDirection = Pilot->GetTargetAimAxis();
		//FLOGV("Pilot AimDirection %s", *AimDirection.ToString());
	}



	if(TurretComponent && ComponentDescription)
	{

		float TargetTurretAngle = 0;
		if(AimDirection != FVector::ZeroVector)
		{
			FVector LocalTurretAimDirection = GetComponentToWorld().GetRotation().Inverse().RotateVector(AimDirection);

		/*	FLOG("==================");
			FLOGV("AimDirection %s", *AimDirection.ToString());
			FLOGV("LocalTurretAimDirection %s", *LocalTurretAimDirection.ToString());
		*/
			TargetTurretAngle = FMath::UnwindDegrees(FMath::RadiansToDegrees(FMath::Atan2(LocalTurretAimDirection.Y, LocalTurretAimDirection.X)));
		}
		//FLOGV("TargetTurretAngle %f", TargetTurretAngle);
		//float TargetBarrelAngle = FMath::RadiansToDegrees(FMath::Atan2(LocalAimDirection.Z, FMath::Sqrt(FMath::Square(LocalAimDirection.X) + FMath::Square(LocalAimDirection.Y))) + 180);
		//float TargetBarrelAngle = - FMath::RadiansToDegrees(FMath::Atan2(LocalAimDirection.X, LocalAimDirection.Z)) + 90;

	/*	FLOGV("TargetTurretAngle %f", TargetTurretAngle);

		FLOGV("Atan2 Y,X= %f", FMath::RadiansToDegrees(FMath::Atan2(LocalTurretAimDirection.Y, LocalTurretAimDirection.X)));
		FLOGV("Atan2 X,Y= %f", FMath::RadiansToDegrees(FMath::Atan2(LocalTurretAimDirection.X, LocalTurretAimDirection.Y)));
		FLOGV("Atan2 Z,X= %f", FMath::RadiansToDegrees(FMath::Atan2(LocalTurretAimDirection.Z, LocalTurretAimDirection.X)));
		FLOGV("Atan2 X,Z= %f", FMath::RadiansToDegrees(FMath::Atan2(LocalTurretAimDirection.X, LocalTurretAimDirection.Z)));
		FLOGV("Atan2 Y,Z= %f", FMath::RadiansToDegrees(FMath::Atan2(LocalTurretAimDirection.Y, LocalTurretAimDirection.Z)));
		FLOGV("Atan2 Z,Y= %f", FMath::RadiansToDegrees(FMath::Atan2(LocalTurretAimDirection.Z, LocalTurretAimDirection.Y)));
*/
		// Clamp movements
		TargetTurretAngle = FMath::Clamp(TargetTurretAngle, ComponentDescription->TurretCharacteristics.TurretMinAngle, ComponentDescription->TurretCharacteristics.TurretMaxAngle);

	//	FLOGV("TargetTurretAngle after clamp %f", TargetTurretAngle);


		float TurretAngleDiff = FMath::UnwindDegrees(TargetTurretAngle - TurretAngle);

		/*FLOGV("TurretAngleDiff %f", TurretAngleDiff);
		FLOGV("BarrelsMinAngle %f", ComponentDescription->TurretCharacteristics.TurretMinAngle);
		FLOGV("BarrelsMaxAngle %f", ComponentDescription->TurretCharacteristics.TurretMaxAngle);
		FLOGV("BarrelsAngularVelocity %f", ComponentDescription->TurretCharacteristics.TurretAngularVelocity);

*/
		if(FMath::Abs(TurretAngleDiff) <= ComponentDescription->TurretCharacteristics.TurretAngularVelocity * DeltaTime) {
			TurretAngle = TargetTurretAngle;
		} else if(TurretAngleDiff < 0) {
			TurretAngle -= ComponentDescription->TurretCharacteristics.TurretAngularVelocity * DeltaTime;
		} else {
			TurretAngle += ComponentDescription->TurretCharacteristics.TurretAngularVelocity * DeltaTime;
		}
	//	FLOGV("TurretAngle after move %f", TurretAngle);

		TurretComponent->SetRelativeRotation(FRotator(0, TurretAngle, 0));
	}

	if (BarrelComponent)
	{

		float TargetBarrelAngle = 0;

		if(AimDirection != FVector::ZeroVector)
		{
			FVector LocalBarrelAimDirection;
			if (TurretComponent)
			{
				LocalBarrelAimDirection = TurretComponent->GetComponentToWorld().GetRotation().Inverse().RotateVector(AimDirection);
			}
			else
			{
				LocalBarrelAimDirection = GetComponentToWorld().GetRotation().Inverse().RotateVector(AimDirection);
			}

			TargetBarrelAngle = FMath::UnwindDegrees(FMath::RadiansToDegrees(FMath::Atan2(LocalBarrelAimDirection.Z, LocalBarrelAimDirection.X)));
		}
		//FLOGV("TargetBarrelAngle %f", TargetBarrelAngle);

		/*FLOGV("TargetBarrelAngle %f", TargetBarrelAngle);

		FLOGV("Atan2 Y,X= %f", FMath::RadiansToDegrees(FMath::Atan2(LocalBarrelAimDirection.Y, LocalBarrelAimDirection.X)));
		FLOGV("Atan2 X,Y= %f", FMath::RadiansToDegrees(FMath::Atan2(LocalBarrelAimDirection.X, LocalBarrelAimDirection.Y)));
		FLOGV("Atan2 Z,X= %f", FMath::RadiansToDegrees(FMath::Atan2(LocalBarrelAimDirection.Z, LocalBarrelAimDirection.X)));
		FLOGV("Atan2 X,Z= %f", FMath::RadiansToDegrees(FMath::Atan2(LocalBarrelAimDirection.X, LocalBarrelAimDirection.Z)));
		FLOGV("Atan2 Y,Z= %f", FMath::RadiansToDegrees(FMath::Atan2(LocalBarrelAimDirection.Y, LocalBarrelAimDirection.Z)));
		FLOGV("Atan2 Z,Y= %f", FMath::RadiansToDegrees(FMath::Atan2(LocalBarrelAimDirection.Z, LocalBarrelAimDirection.Y)));
*/
		//float temp = TargetBarrelAngle;
		// Clamp movements
		TargetBarrelAngle = FMath::Clamp(TargetBarrelAngle, ComponentDescription->TurretCharacteristics.BarrelsMinAngle, ComponentDescription->TurretCharacteristics.BarrelsMaxAngle);

	//	FLOGV("TargetBarrelAngle after clamp %f", TargetBarrelAngle);

		// TODO Add ship specific bound

		float BarrelAngleDiff = FMath::UnwindDegrees(TargetBarrelAngle - BarrelAngle);


	/*	FLOGV("BarrelAngleDiff %f", BarrelAngleDiff);
		FLOGV("BarrelsMinAngle %f", ComponentDescription->TurretCharacteristics.BarrelsMinAngle);
		FLOGV("BarrelsMaxAngle %f", ComponentDescription->TurretCharacteristics.BarrelsMaxAngle);
		FLOGV("BarrelsAngularVelocity %f", ComponentDescription->TurretCharacteristics.BarrelsAngularVelocity);
*/
		//float temp2 = BarrelAngle;
		if(FMath::Abs(BarrelAngleDiff) <= ComponentDescription->TurretCharacteristics.BarrelsAngularVelocity * DeltaTime) {
			BarrelAngle = TargetBarrelAngle;
		} else if(BarrelAngleDiff < 0) {
			BarrelAngle -= ComponentDescription->TurretCharacteristics.BarrelsAngularVelocity * DeltaTime;
		} else {
			BarrelAngle += ComponentDescription->TurretCharacteristics.BarrelsAngularVelocity * DeltaTime;
		}

		/*if(BarrelAngle != temp2)
		{
			FLOG("==================");
			FLOGV("AimDirection %s", *AimDirection.ToString());
			FLOGV("LocalBarrelAimDirection %s", *LocalBarrelAimDirection.ToString());

			FLOGV("BarrelAngle before move %f", temp2);
			FLOGV("TargetBarrelAngle %f", temp);

			FLOGV("Atan2 Y,X= %f", FMath::RadiansToDegrees(FMath::Atan2(LocalBarrelAimDirection.Y, LocalBarrelAimDirection.X)));
			FLOGV("Atan2 X,Y= %f", FMath::RadiansToDegrees(FMath::Atan2(LocalBarrelAimDirection.X, LocalBarrelAimDirection.Y)));
			FLOGV("Atan2 Z,X= %f", FMath::RadiansToDegrees(FMath::Atan2(LocalBarrelAimDirection.Z, LocalBarrelAimDirection.X)));
			FLOGV("Atan2 X,Z= %f", FMath::RadiansToDegrees(FMath::Atan2(LocalBarrelAimDirection.X, LocalBarrelAimDirection.Z)));
			FLOGV("Atan2 Y,Z= %f", FMath::RadiansToDegrees(FMath::Atan2(LocalBarrelAimDirection.Y, LocalBarrelAimDirection.Z)));
			FLOGV("Atan2 Z,Y= %f", FMath::RadiansToDegrees(FMath::Atan2(LocalBarrelAimDirection.Z, LocalBarrelAimDirection.Y)));
			FLOGV("TargetBarrelAngle after clamp %f", TargetBarrelAngle);
			FLOGV("BarrelAngleDiff %f", BarrelAngleDiff);
			FLOGV("BarrelsMinAngle %f", ComponentDescription->TurretCharacteristics.BarrelsMinAngle);
			FLOGV("BarrelsMaxAngle %f", ComponentDescription->TurretCharacteristics.BarrelsMaxAngle);
			FLOGV("BarrelsAngularVelocity %f", ComponentDescription->TurretCharacteristics.BarrelsAngularVelocity);
			FLOGV("BarrelAngle after move %f", BarrelAngle);
		}*/
	//	FLOGV("BarrelAngle after move %f", BarrelAngle);

		BarrelComponent->SetRelativeRotation(FRotator(BarrelAngle, 0, 0));

		if(FMath::Abs(BarrelAngleDiff) <= ComponentDescription->TurretCharacteristics.BarrelsAngularVelocity * DeltaTime) {

			/*FLOGV("Mouvement end: AimDirection %s", *AimDirection.ToString());
			FLOGV("               Control fire axis %s", *GetFireAxis().ToString());*/
		}
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

FVector UFlareTurret::GetFireAxis() const
{
	if (BarrelComponent)
	{
		return BarrelComponent->GetComponentRotation().RotateVector(FVector(1, 0, 0));
	}
	else if (TurretComponent)
	{
		return TurretComponent->GetComponentRotation().RotateVector(FVector(1, 0, 0));
	}
	else
	{
		return Super::GetFireAxis();
	}
}

FVector UFlareTurret::GetIdleAxis() const
{
	// Ship front
	return Spacecraft->Airframe->GetComponentRotation().RotateVector(FVector(1, 0, 0));
}


FVector UFlareTurret::GetMuzzleLocation(int GunIndex) const
{
	const UStaticMeshComponent* GunComponent = this;
	if (BarrelComponent)
	{
		GunComponent = BarrelComponent;
	}
	else if (TurretComponent)
	{
		GunComponent = TurretComponent;
	}

	if (ComponentDescription->GunCharacteristics.GunCount <= 1)
	{
		return GunComponent->GetSocketLocation(FName("Muzzle"));
	}
	else
	{
		return GunComponent->GetSocketLocation(FName(*(FString("Muzzle") + FString::FromInt(GunIndex))));
	}
}

FVector UFlareTurret::GetTurretBaseLocation() const
{
	if (BarrelComponent)
	{
		return BarrelComponent->GetComponentLocation();
	}
	else if (TurretComponent)
	{
		return TurretComponent->GetComponentLocation();
	}
	return GetComponentLocation();
}

bool UFlareTurret::IsSafeToFire(int GunIndex) const
{
	FVector FiringLocation = GetMuzzleLocation(GunIndex);
	FVector FiringDirection = GetFireAxis();
	FVector TargetLocation = FiringLocation + FiringDirection * 100000;

	FHitResult HitResult(ForceInit);
	if (Trace(FiringLocation, TargetLocation, HitResult))
	{
		if (HitResult.Actor.IsValid() && HitResult.Actor == Spacecraft)
		{
			FLOG("!!!!!!!!!Not safe to fire !");
			return false;
		}
	}
	return true;
}

bool UFlareTurret::Trace(const FVector& Start, const FVector& End, FHitResult& HitOut) const
{
	FCollisionQueryParams TraceParams(FName(TEXT("Shell Trace")), true, NULL);
	TraceParams.bTraceComplex = true;
	//TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;

	//Re-initialize hit info
	HitOut = FHitResult(ForceInit);

	ECollisionChannel CollisionChannel = (ECollisionChannel) (ECC_WorldStatic | ECC_WorldDynamic | ECC_Pawn);

	//Trace!
	GetWorld()->LineTraceSingleByChannel(
		HitOut,		//result
		Start,	//start
		End , //end
		CollisionChannel, //collision channel
		TraceParams
	);

	//Hit any Actor?
	return (HitOut.GetActor() != NULL) ;
}

bool UFlareTurret::IsReacheableAxis(FVector TargetAxis) const
{
	if(TurretComponent && ComponentDescription)
	{

		FVector LocalTurretAimDirection = GetComponentToWorld().GetRotation().Inverse().RotateVector(TargetAxis);
		float TargetTurretAngle = FMath::UnwindDegrees(FMath::RadiansToDegrees(FMath::Atan2(LocalTurretAimDirection.Y, LocalTurretAimDirection.X)));

		if(TargetTurretAngle > ComponentDescription->TurretCharacteristics.TurretMaxAngle
				|| TargetTurretAngle < ComponentDescription->TurretCharacteristics.TurretMinAngle)
		{
			return false;
		}
	}

	if (BarrelComponent && ComponentDescription)
	{

		FVector LocalBarrelAimDirection;
		if (TurretComponent)
		{
			LocalBarrelAimDirection = TurretComponent->GetComponentToWorld().GetRotation().Inverse().RotateVector(TargetAxis);
		}
		else
		{
			LocalBarrelAimDirection = GetComponentToWorld().GetRotation().Inverse().RotateVector(TargetAxis);
		}

		float TargetBarrelAngle = FMath::UnwindDegrees(FMath::RadiansToDegrees(FMath::Atan2(LocalBarrelAimDirection.Z, LocalBarrelAimDirection.X)));
		if(TargetBarrelAngle > ComponentDescription->TurretCharacteristics.BarrelsMaxAngle
				|| TargetBarrelAngle < ComponentDescription->TurretCharacteristics.BarrelsMinAngle)
		{
			return false;
		}
	}
	return true;
}
