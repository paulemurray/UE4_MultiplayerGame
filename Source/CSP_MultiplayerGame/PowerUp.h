// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerUp.generated.h"

UENUM(BlueprintType)
enum class EPowerUpType : uint8 {
	EPT_Health	UMETA(DisplayName="Health"),
	EPT_Ammo	UMETA(DisplayName="Ammo"),
	EPT_Custom	UMETA(DisplayName="Custom Code")
};

UCLASS()
class CSP_MULTIPLAYERGAME_API APowerUp : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APowerUp();

	//marks properties to be replicated
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly)
		class USphereComponent* CollisionComponent;

	//powerup attributes
	UPROPERTY(EditAnywhere, Category = "PowerUp")
		EPowerUpType Type;
	UPROPERTY(EditAnywhere, Category = "PowerUp")
		float AmmountToAdd;

private:	
	// Called when player overlaps
	UFUNCTION(Server, Reliable, WithValidation)
		void OnPickup(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void OnPickup_Implementation(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	bool OnPickup_Validate(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
