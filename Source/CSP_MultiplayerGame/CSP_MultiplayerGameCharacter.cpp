// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "CSP_MultiplayerGameCharacter.h"
#include "CSP_MultiplayerGameProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);


ACSP_MultiplayerGameCharacter::ACSP_MultiplayerGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 41.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));


	TP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TP_Gun"));
	TP_Gun->SetOwnerNoSee(true);
	TP_Gun->bCastDynamicShadow = true;
	TP_Gun->CastShadow = true;
	FP_Gun->SetupAttachment(GetMesh(), TEXT("GripPoint"));
	TP_Gun->SetRelativeLocation(FVector(0.f, 0.f, 286.f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	//Init text render component
	CharText = CreateDefaultSubobject<UTextRenderComponent>(FName("CharText"));
	//set a relative location
	CharText->SetRelativeLocation(FVector(0, 0, 100));
	//attach to root comp
	CharText->SetupAttachment(GetRootComponent());

	//hide third person mesh
	GetMesh()->SetOwnerNoSee(true);

	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -98.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
}

void ACSP_MultiplayerGameCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	TP_Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	InitHealth();
	InitBombCount();
}

void ACSP_MultiplayerGameCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ACSP_MultiplayerGameCharacter::OnFire);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ACSP_MultiplayerGameCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACSP_MultiplayerGameCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ACSP_MultiplayerGameCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ACSP_MultiplayerGameCharacter::LookUpAtRate);
}

void ACSP_MultiplayerGameCharacter::OnFire()
{
	if (HasBombs()) {
		if (Role < ROLE_Authority) {
			//request projectile spawn
			ServerSpawnProjectile();
		}
		else {
			SpawnProjectile();
		}
		BombCount--;
	}
}

void ACSP_MultiplayerGameCharacter::SpawnProjectile() {
	//try to spawn projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			const FRotator SpawnRotation = GetControlRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			ActorSpawnParams.Instigator = this;
			ActorSpawnParams.Owner = GetController();

			// spawn the projectile at the muzzle
			World->SpawnActor<ACSP_MultiplayerGameProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	}
	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void ACSP_MultiplayerGameCharacter::ServerSpawnProjectile_Implementation() {
	SpawnProjectile();
}

bool ACSP_MultiplayerGameCharacter::ServerSpawnProjectile_Validate() {
	//its okay lol
	return true;
}

void ACSP_MultiplayerGameCharacter::MoveForward(float Value) {
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ACSP_MultiplayerGameCharacter::MoveRight(float Value) {
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ACSP_MultiplayerGameCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACSP_MultiplayerGameCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ACSP_MultiplayerGameCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//tell the engine what variables to replicate
	DOREPLIFETIME(ACSP_MultiplayerGameCharacter, Health);
	DOREPLIFETIME(ACSP_MultiplayerGameCharacter, BombCount);
}

void ACSP_MultiplayerGameCharacter::OnRep_Health() {
	UpdateCharText();
}

void ACSP_MultiplayerGameCharacter::OnRep_BombCount() {
	UpdateCharText();
}

void ACSP_MultiplayerGameCharacter::InitHealth() {
	Health = MaxHealth;
	UpdateCharText();
}

void ACSP_MultiplayerGameCharacter::InitBombCount() {
	BombCount = MaxBombCount;
	UpdateCharText();
}

void ACSP_MultiplayerGameCharacter::UpdateCharText() {
	//create a string to show health and bomb values
	FString NewText = FString("Health: ") + FString::SanitizeFloat(Health) + FString(" Bomb Count: ") + FString::FromInt(BombCount);
	//set the created string to the text render comp
	CharText->SetText(FText::FromString(NewText));
}

float ACSP_MultiplayerGameCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) {
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	//decrease hp
	Health -= Damage;
	if (Health <= 0) InitHealth();

	//call update text
	//onRep_Health will be called on other clients
	UpdateCharText();

	return Health;
}

void ACSP_MultiplayerGameCharacter::ServerTakeDamage_Implementation(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) {
	TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

bool ACSP_MultiplayerGameCharacter::ServerTakeDamage_Validate(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) {
	//assume everything is okay
	return true;
}