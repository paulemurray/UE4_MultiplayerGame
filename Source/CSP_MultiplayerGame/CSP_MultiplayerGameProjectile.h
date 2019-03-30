// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "CSP_MultiplayerGameProjectile.generated.h"

UCLASS(config=Game)
class ACSP_MultiplayerGameProjectile : public AActor
{
	GENERATED_BODY()

	//static mesh
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* SM;

	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
		class USphereComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		class UProjectileMovementComponent* ProjectileMovement;

public:
	ACSP_MultiplayerGameProjectile();

	/** called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Returns CollisionComp subobject **/
	FORCEINLINE class USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ProjectileMovement subobject **/
	FORCEINLINE class UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

protected:
	//delay until explosion
	UPROPERTY(EditAnywhere, Category = BombProps)
		float FuseTime = 2.5f;

	UPROPERTY(EditAnywhere, Category = BombProps)
		float ExplosionRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = BombProps)
		float ExplosionDamage = 25.f;

	UPROPERTY(EditAnywhere)
		UParticleSystem* ExplosionFX;

private:
	//marks properties to replicate
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_IsArmed)
		bool bIsArmed = false;

	//called when bIsArmed is updated
	UFUNCTION()
		void OnRep_IsArmed();

	//arms the bomb
	void ArmBomb();

	//performs an exposion after a certain amount of time
	void PerformDelayedExplosion(float ExplosionDelay);

	//explodes
	UFUNCTION()
		void Explode();

	//simulate explosion functions
	//indicateds to clients to show the explosion
	UFUNCTION(Reliable, NetMulticast)
		void SimulateExplosionFX();

	//implementation
	void SimulateExplosionFX_Implementation();

};

