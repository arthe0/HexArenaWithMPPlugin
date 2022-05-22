// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/HABaseCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/BaseWeapon.h"
#include "HAComponents/CombatComponent.h"
#include "HAComponents/HealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Character/HAAnimInstance.h"
#include "HexArena/HexArena.h"
#include "PlayerController/HAPlayerController.h"
#include "TimerManager.h"
#include "GameMode/HAGameMode.h"
#include "GameFramework/Character.h"
#include "PlayerStates/HaPlayerState.h"
#include "Weapon/AmmoTypes.h"
#include "Weapon//WeaponTypes.h"

AHABaseCharacter::AHABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetTickGroup(ETickingGroup::TG_PostUpdateWork);
	GetMesh()->bVisibleInReflectionCaptures = true;
	GetMesh()->bCastHiddenShadow = true;
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->bUsePawnControlRotation = true;
	CameraComponent->SetupAttachment(GetMesh(), FName("head"));

	ClientMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ClientMesh"));
	ClientMesh->SetCastShadow(false);
	ClientMesh->bVisibleInReflectionCaptures = false;
	ClientMesh->SetTickGroup(ETickingGroup::TG_PostUpdateWork);
	ClientMesh->SetupAttachment(GetMesh());

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(GetRootComponent());

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	Health->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

void AHABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AHABaseCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void AHABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	HAPlayerController = GetPlayerController();

	if (IsLocallyControlled())
	{
		ClientMesh->HideBoneByName("neck_01", PBO_None);
		GetMesh()->SetVisibility(false);
	}
	else
	{	
		ClientMesh->DestroyComponent();
	}
}


void AHABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	AimOffset(DeltaTime);

	PollInit();
}

void AHABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AHABaseCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AHABaseCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &AHABaseCharacter::LookUp);
	PlayerInputComponent->BindAxis("TurnAround", this, &AHABaseCharacter::TurnAround);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AHABaseCharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AHABaseCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AHABaseCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AHABaseCharacter::CrouchButtonReleased);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AHABaseCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AHABaseCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AHABaseCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AHABaseCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AHABaseCharacter::ReloadButtonPressed);
}

void AHABaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if(Combat)
	{
		Combat->Character = this;
	}

	if (Health)
	{
		Health->Character = this;
	}
}

/*
* Montages
*/

void AHABaseCharacter::PlayFireMontage(bool bAiming)
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AHABaseCharacter::PlayDeathMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
		FName SectionName("DeathHeadshot");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}


void AHABaseCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

		switch(Combat->EquippedWeapon->GetWeaponType())
		{
			case EWeaponType::EWT_Rifle:
			SectionName = FName("Rifle");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AHABaseCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;


	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AHABaseCharacter::MoveForward(float Amount)
{
	bIsMovingForward = Amount > 0.0f;
	if (Amount == 0.0f) return;
	AddMovementInput(GetActorForwardVector(), Amount);
}

void AHABaseCharacter::MoveRight(float Amount)
{
	if (Amount == 0.0f) return;
	AddMovementInput(GetActorRightVector(), Amount);
}

void AHABaseCharacter::LookUp(float Amount)
{
	AddControllerPitchInput(Amount);
}

void AHABaseCharacter::TurnAround(float Amount)
{
	AddControllerYawInput(Amount);
}

void AHABaseCharacter::EquipButtonPressed()
{
	if(Combat)
	{
		if(HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

void AHABaseCharacter::CrouchButtonPressed()
{
	Crouch();
}

void AHABaseCharacter::CrouchButtonReleased()
{
	UnCrouch();
}

void AHABaseCharacter::AimButtonPressed()
{
	if(Combat)
	{
		Combat->SetAiming(true);
	}
}

void AHABaseCharacter::AimButtonReleased()
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void AHABaseCharacter::FireButtonPressed()
{
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void AHABaseCharacter::FireButtonReleased()
{
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void AHABaseCharacter::ReloadButtonPressed()
{
	if(Combat)
	{
		Combat->Reload();
	}
}

void AHABaseCharacter::AimOffset(float DeltaTime)
{
	if(Combat && Combat->EquippedWeapon == nullptr) return;
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if(Speed == 0.f && !bIsInAir) //Still stand
	{
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator( CurrentAimRotation, StartingAimRoation);
		AO_Yaw = DeltaAimRotation.Yaw;
		TurnInPlace(DeltaTime);
	}
	if(Speed > 0.f || bIsInAir) //Running or jumping
	{
		StartingAimRoation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	if(AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// Mapping pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}


void AHABaseCharacter::PollInit()
{
	if(HAPlayerState == nullptr)
	{
		HAPlayerState = GetPlayerState<AHaPlayerState>();
		if(HAPlayerState)
		{
			HAPlayerState->AddToScore(0.f);
			HAPlayerState->AddToDeaths(0);
		}
	}
}

void AHABaseCharacter::TurnInPlace(float DeltaTime)
{
	if(AO_Yaw > 10.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if(AO_Yaw < -10.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
}


/*
 * Death handle functions
 */
void AHABaseCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void AHABaseCharacter::Death()
{
	if(Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Dropped();
	}
	MulticastDeath();
	GetWorldTimerManager().SetTimer(
		DeathTimer, 
		this,
		&AHABaseCharacter::DeathTimerFinished,
		DeathDelay
	);
}

void AHABaseCharacter::MulticastDeath_Implementation()
{
	if(HAPlayerController)
	{
		HAPlayerController->SetHUDWeaponAmmo(0);
	}
	bDeath = true;
	PlayDeathMontage();
	//Start Dissolving effect 
	if(BodyDissolveMaterialInstance && EquipDissolveMaterialInstance)
	{
		DynamicBodyDissolveMaterialInstance = UMaterialInstanceDynamic::Create(BodyDissolveMaterialInstance, this);
		DynamicEquipDissolveMaterialInstance = UMaterialInstanceDynamic::Create(EquipDissolveMaterialInstance, this);

		GetMesh()->SetMaterial(0, DynamicBodyDissolveMaterialInstance);
		GetMesh()->SetMaterial(1, DynamicEquipDissolveMaterialInstance);

		DynamicBodyDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.6f);
		DynamicBodyDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
		DynamicEquipDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.6f);
		DynamicEquipDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	//Disable character movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	if(HAPlayerController)
	{
		DisableInput(HAPlayerController);
	}

	//Disable Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//TODO: Make Ragdoll work 
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->WakeAllRigidBodies();
	GetMesh()->bBlendPhysics = true;

}


void AHABaseCharacter::DeathTimerFinished()
{
	AHAGameMode* HAGameMode = GetWorld()->GetAuthGameMode<AHAGameMode>();
	if(HAGameMode)
	{
		HAGameMode->RequestRespawn(this, Controller);
	}
}

/*
* Dissolve effect
*/

void AHABaseCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if(DynamicEquipDissolveMaterialInstance && DynamicBodyDissolveMaterialInstance)
	{
		DynamicBodyDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		DynamicEquipDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void AHABaseCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &AHABaseCharacter::UpdateDissolveMaterial);
	if(DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

/*
* Getters and setters
*/
float AHABaseCharacter::GetMovementDirection()
{
	if(Combat)
	{
		if (GetVelocity().IsZero()) return 0.0f;
		const auto VelocityNormal = GetVelocity().GetSafeNormal();
		const auto AngleBetween = FMath::Acos(FVector::DotProduct(GetActorForwardVector(), VelocityNormal));
		const auto CrossProduct = FVector::CrossProduct(GetActorForwardVector(), VelocityNormal);
		const auto Degrees = FMath::RadiansToDegrees(AngleBetween) * FMath::Sign(CrossProduct.Z);
		return Degrees;
	}
	return 0.0f;
}

ABaseWeapon* AHABaseCharacter::GetEquippedWeapon()
{
	if(Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector AHABaseCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

void AHABaseCharacter::SetOverlappingWeapon(ABaseWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if(IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void AHABaseCharacter::OnRep_OverlappingWeapon(ABaseWeapon* LastWeapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if(LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

bool AHABaseCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool AHABaseCharacter::IsAiming()
{
	return(Combat && Combat->bAiming);
}

AHAPlayerController* AHABaseCharacter::GetPlayerController()
{
	return HAPlayerController == nullptr ? Cast<AHAPlayerController>(Controller) : HAPlayerController = HAPlayerController;
}

ECombatState AHABaseCharacter::GetCombatState() const
{
	if(Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}

