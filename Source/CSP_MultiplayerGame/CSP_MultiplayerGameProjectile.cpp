// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "CSP_MultiplayerGameProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Classes/Materials/MaterialInstanceDynamic.h"
#include "Engine.h"
#include "Components/SphereComponent.h"

ACSP_MultiplayerGameProjectile::ACSP_MultiplayerGameProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &ACSP_MultiplayerGameProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	//initialize static mesh
	SM = CreateDefaultSubobject<UStaticMeshComponent>(FName("SM"));
	SM->SetupAttachment(CollisionComp);

	//replication
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bReplicateMovement = true;
}

void ACSP_MultiplayerGameProjectile::BeginPlay() {
	Super::BeginPlay();

	FTimerHandle TimHan;
	FTimerDelegate TimDel;
	TimDel.BindLambda([&] () {
		//if not armed and we are the server, arm and explode
		if (!bIsArmed && Role == ROLE_Authority) {
			bIsArmed = true;
			ArmBomb();
			Explode();
		}

	});

	GetWorld()->GetTimerManager().SetTimer(TimHan, TimDel, .80f, false);
}

void ACSP_MultiplayerGameProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL)) { 
		//if object simulates physics, move it
		if (OtherComp->IsSimulatingPhysics() && Role == ROLE_Authority) {
			OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
		}

		//if not armed and we are the server, arm and explode
		if (!bIsArmed && Role == ROLE_Authority) {
			bIsArmed = true;
			ArmBomb();

			PerformDelayedExplosion(FuseTime);
		}
	}
}

//replicated bIsArmed
void ACSP_MultiplayerGameProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACSP_MultiplayerGameProjectile, bIsArmed);
}

void ACSP_MultiplayerGameProjectile::ArmBomb() {
	if (bIsArmed) {
		//change color to red
		UMaterialInstanceDynamic* DynamicMAT = SM->CreateAndSetMaterialInstanceDynamic(0);

		DynamicMAT->SetVectorParameterValue(FName("Color"), FLinearColor::Red);
	}
}

//if not armed, arm
void ACSP_MultiplayerGameProjectile::OnRep_IsArmed() {
	if (bIsArmed) {
		ArmBomb();
	}
}

void ACSP_MultiplayerGameProjectile::Explode() {
	SimulateExplosionFX();

	//create damage
	TSubclassOf<UDamageType> DmgType;
	//dont ignore any actors 
	TArray<AActor*> IgnoreActors;

	//this will call the take damage function in character
	UGameplayStatics::ApplyRadialDamage(GetWorld(), ExplosionDamage, GetActorLocation(), ExplosionRadius, DmgType, IgnoreActors, this, GetInstigatorController());
	
	if (Role == ROLE_Authority) {
		//destroy projectile
		Destroy();
	}
}

//explode after set time
void ACSP_MultiplayerGameProjectile::PerformDelayedExplosion(float ExplosionDelay) {
	FTimerHandle TimerHandle;
	FTimerDelegate TimerDel;

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, ExplosionDelay, false);
}

//create explosion particles
void ACSP_MultiplayerGameProjectile::SimulateExplosionFX_Implementation() {
	if (ExplosionFX) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, GetTransform(), true);
	}
}