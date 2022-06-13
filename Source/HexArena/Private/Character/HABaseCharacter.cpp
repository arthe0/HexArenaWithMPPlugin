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
#include "Components/BoxComponent.h"
#include "HAComponents/LagCompensationComponent.h"
#include "HAComponents/HAMovementComponent.h"
#include "HAComponents/Inventory.h"
#include "Pickups/BasePickup.h"
#include "Pickups/AmmoPickup.h"
#include "Pickups/Interactable.h"
#include "Pickups/LootBox.h"
#include "PlayerStart/TeamPlayerStart.h"
#include "Kismet/GameplayStatics.h"

AHABaseCharacter::AHABaseCharacter(const FObjectInitializer& ObjInit)
	:Super(ObjInit.SetDefaultSubobjectClass<UHAMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetTickGroup(ETickingGroup::TG_PostUpdateWork);
	GetMesh()->bVisibleInReflectionCaptures = true;
	GetMesh()->bCastHiddenShadow = true;
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_PickupPhysics, ECollisionResponse::ECR_Overlap);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->bUsePawnControlRotation = true;
	CameraComponent->SetupAttachment(GetMesh(), FName("head"));

	ClientMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ClientMesh"));
	ClientMesh->SetCastShadow(false);
	ClientMesh->bVisibleInReflectionCaptures = false;
	ClientMesh->SetOnlyOwnerSee(true);
	ClientMesh->SetTickGroup(ETickingGroup::TG_PostUpdateWork);
	ClientMesh->SetupAttachment(GetMesh());

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(GetRootComponent());

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	Health->SetIsReplicated(true);

	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComponent"));

	Inventory = CreateDefaultSubobject<UInventory>(TEXT("InventoryComponent"));
	Inventory->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	/*
	*	HitBoxes 
	*/

	HeadBox = CreateDefaultSubobject<UHitBoxComponent>(TEXT("HeadBox"));
	HeadBox->SetupAttachment(GetMesh(), FName("head"));
	HeadBox->HitBoxType = EHitBoxType::EHBT_Head;
	HitBoxes.Add(FName("HeadBox"), HeadBox);

	NeckBox = CreateDefaultSubobject<UHitBoxComponent>(TEXT("NeckBox"));
	NeckBox->SetupAttachment(GetMesh(), FName("neck_01"));
	NeckBox->HitBoxType = EHitBoxType::EHBT_Neck;
	HitBoxes.Add(FName("NeckBox"), NeckBox);

	ChestBox = CreateDefaultSubobject<UHitBoxComponent>(TEXT("ChestBox"));
	ChestBox->SetupAttachment(GetMesh(), FName("spine_03"));
	ChestBox->HitBoxType = EHitBoxType::EHBT_Chest;
	HitBoxes.Add(FName("ChestBox"), ChestBox);

	StomachBox = CreateDefaultSubobject<UHitBoxComponent>(TEXT("StomachBox"));
	StomachBox->SetupAttachment(GetMesh(), FName("spine_02"));
	StomachBox->HitBoxType = EHitBoxType::EHBT_Stomach;
	HitBoxes.Add(FName("StomachBox"), StomachBox);

	PelvisBox = CreateDefaultSubobject<UHitBoxComponent>(TEXT("PelvisBox"));
	PelvisBox->SetupAttachment(GetMesh(), FName("pelvis"));
	PelvisBox->HitBoxType = EHitBoxType::EHBT_Stomach;
	HitBoxes.Add(FName("PelvisBox"), PelvisBox);

	UpperArmLBox = CreateDefaultSubobject<UHitBoxComponent>(TEXT("UpperArmLBox"));
	UpperArmLBox->SetupAttachment(GetMesh(), FName("upperarm_l"));
	UpperArmLBox->HitBoxType = EHitBoxType::EHBT_Limbs;
	HitBoxes.Add(FName("UpperArmLBox"), UpperArmLBox);

	UpperArmRBox = CreateDefaultSubobject<UHitBoxComponent>(TEXT("UpperArmRBox"));
	UpperArmRBox->SetupAttachment(GetMesh(), FName("upperarm_r"));
	UpperArmRBox->HitBoxType = EHitBoxType::EHBT_Limbs;
	HitBoxes.Add(FName("UpperArmRBox"), UpperArmRBox);

	LowerArmLBox = CreateDefaultSubobject<UHitBoxComponent>(TEXT("LowerArmLBox"));
	LowerArmLBox->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	LowerArmLBox->HitBoxType = EHitBoxType::EHBT_Limbs;
	HitBoxes.Add(FName("LowerArmLBox"), LowerArmLBox);

	LowerArmRBox = CreateDefaultSubobject<UHitBoxComponent>(TEXT("LowerArmRBox"));
	LowerArmRBox->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	LowerArmRBox->HitBoxType = EHitBoxType::EHBT_Limbs;
	HitBoxes.Add(FName("LowerArmRBox"), LowerArmRBox);

	HandLBox = CreateDefaultSubobject<UHitBoxComponent>(TEXT("HandLBox"));
	HandLBox->SetupAttachment(GetMesh(), FName("hand_l"));
	HandLBox->HitBoxType = EHitBoxType::EHBT_Limbs;
	HitBoxes.Add(FName("HandLBox"), HandLBox);

	HandRBox = CreateDefaultSubobject<UHitBoxComponent>(TEXT("HandRBox"));
	HandRBox->SetupAttachment(GetMesh(), FName("hand_r"));
	HandRBox->HitBoxType = EHitBoxType::EHBT_Limbs;
	HitBoxes.Add(FName("HandRBox"), HandRBox);

	ThighLBox = CreateDefaultSubobject<UHitBoxComponent>(TEXT("ThighLBox"));
	ThighLBox->SetupAttachment(GetMesh(), FName("thigh_l"));
	ThighLBox->HitBoxType = EHitBoxType::EHBT_Limbs;
	HitBoxes.Add(FName("ThighLBox"), ThighLBox);

	ThighRBox = CreateDefaultSubobject<UHitBoxComponent>(TEXT("ThighRBox"));
	ThighRBox->SetupAttachment(GetMesh(), FName("thigh_r"));
	ThighRBox->HitBoxType = EHitBoxType::EHBT_Limbs;
	HitBoxes.Add(FName("ThighRBox"), ThighRBox);

	CalfLBox = CreateDefaultSubobject<UHitBoxComponent>(TEXT("CalfLBox"));
	CalfLBox->SetupAttachment(GetMesh(), FName("calf_l"));
	CalfLBox->HitBoxType = EHitBoxType::EHBT_Limbs;
	HitBoxes.Add(FName("CalfLBox"), CalfLBox);

	CalfRBox = CreateDefaultSubobject<UHitBoxComponent>(TEXT("CalfRBox"));
	CalfRBox->SetupAttachment(GetMesh(), FName("calf_r"));
	CalfRBox->HitBoxType = EHitBoxType::EHBT_Limbs;
	HitBoxes.Add(FName("CalfRBox"), CalfRBox);

	FootLBox = CreateDefaultSubobject<UHitBoxComponent>(TEXT("FootLBox"));
	FootLBox->SetupAttachment(GetMesh(), FName("foot_l"));
	FootLBox->HitBoxType = EHitBoxType::EHBT_Limbs;
	HitBoxes.Add(FName("FootLBox"), FootLBox);

	FootRBox = CreateDefaultSubobject<UHitBoxComponent>(TEXT("FootRBox"));
	FootRBox->SetupAttachment(GetMesh(), FName("foot_r"));
	FootRBox->HitBoxType = EHitBoxType::EHBT_Limbs;
	HitBoxes.Add(FName("FootRBox"), FootRBox);

	for (auto Box : HitBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionResponseToChannel(ECC_WorldDynamic, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> TeamColorsObject(TEXT("DataTable'/Game/Blueprints/Character/Materials/DT_TeamColors.DT_TeamColors'"));
	if (TeamColorsObject.Succeeded())
	{
		TeamColorsDT = TeamColorsObject.Object;
	}

	SetTeamColor(TeamName);
}

void AHABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AHABaseCharacter, OverlappingPickup, COND_OwnerOnly);
	DOREPLIFETIME(AHABaseCharacter, bDisableCombat);
}

void AHABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	HAPlayerController = GetPlayerController();
}

void AHABaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Character = this;
	}

	if (Health)
	{
		Health->Character = this;
	}

	if (LagCompensation)
	{
		LagCompensation->Character = this;
		if (Controller)
		{
			LagCompensation->Controller = Cast<AHAPlayerController>(Controller);
		}
	}

	if (Inventory)
	{
		Inventory->Character = this;
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
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AHABaseCharacter::SprintButtonPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AHABaseCharacter::SprintButtonReleased);
	PlayerInputComponent->BindAction("SwapWeapon", IE_Pressed, this, &AHABaseCharacter::SwapWeaponButtonPressed);
	PlayerInputComponent->BindAction("LowerWeapon", IE_Pressed, this, &AHABaseCharacter::LowerWeaponButtonPesssed);
	PlayerInputComponent->BindAction("DropWeapon", IE_Pressed, this, &AHABaseCharacter::DropWeaponButtonPressed);
}

/*
* Input Functions
*/
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

void AHABaseCharacter::CrouchButtonPressed()
{
	Crouch();
	UHAMovementComponent* Movement = Cast<UHAMovementComponent>(GetMovementComponent());
	if(Movement)
	{ 
		Movement->SetSpeed(IsAiming());
	}
}

void AHABaseCharacter::CrouchButtonReleased()
{
	UnCrouch();
	UHAMovementComponent* Movement = Cast<UHAMovementComponent>(GetMovementComponent());
	if (Movement)
	{
		Movement->SetSpeed(IsAiming());
	}
}

void AHABaseCharacter::AimButtonPressed()
{
	if(bDisableCombat) return;
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void AHABaseCharacter::AimButtonReleased()
{
	if (bDisableCombat) return;
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void AHABaseCharacter::FireButtonPressed()
{
	if (bDisableCombat) return;
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void AHABaseCharacter::FireButtonReleased()
{
	if (bDisableCombat) return;
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void AHABaseCharacter::ReloadButtonPressed()
{
	if (bDisableCombat) return;
	if (Combat)
	{
		Combat->Reload();
	}
}

void AHABaseCharacter::SprintButtonPressed()
{
	//GetMovementComponent()->Run();
}

void AHABaseCharacter::SprintButtonReleased()
{
	//GetMovementComponent()->EndRun();
}

void AHABaseCharacter::SwapWeaponButtonPressed()
{
	if (bDisableCombat) return;
	if(Inventory)
	{
		if(HasAuthority())
		{
			Inventory->SwapWeapon();
		}
		else
		{
			ServerSwapButtonPressed();
		}	
	}
}

void AHABaseCharacter::ServerSwapButtonPressed_Implementation()
{
	if (bDisableCombat) return;
	if (Inventory)
	{		
		Inventory->SwapWeapon();	
	}
}

void AHABaseCharacter::LowerWeaponButtonPesssed()
{
	if (bDisableCombat) return;
	if(Inventory)
	{
		if (HasAuthority())
		{
			Inventory->LowerWeapon();	
		}
		else
		{
			ServerLowerButtonPressed();
		}
	}
}

void AHABaseCharacter::ServerLowerButtonPressed_Implementation()
{
	if (bDisableCombat) return;
	if(Inventory)
	{
		Inventory->LowerWeapon();
	}
}

void AHABaseCharacter::DropWeaponButtonPressed()
{
	if (bDisableCombat) return;
	if (Inventory)
	{
		Inventory->DropPrimaryWeapon();
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

/*
* Pickups
*/

void AHABaseCharacter::EquipButtonPressed()
{
	if (bDisableCombat) return;
	if (HasAuthority())
	{	
		if (!OverlappingPickup) return;

		switch (OverlappingPickup->PickupType)
		{
		case EPickupTypes::EPT_Weapon:
			EquipWeaponHandle(OverlappingPickup);
			break;

		case EPickupTypes::EPT_Ammo:
			EquipAmmoHandle(OverlappingPickup);
			break;

		case EPickupTypes::EPT_PowerUp:
			//EquipAmmoHandle(OverlappingPickup);
			break;
		case EPickupTypes::EPT_LootBox:
			InteractWithLootBoxHandle(OverlappingPickup);
			break;
		}
	}
	else
	{
		ServerEquipButtonPressed();
	}
}

void AHABaseCharacter::ServerEquipButtonPressed_Implementation()
{
	if (bDisableCombat) return;
	if (!OverlappingPickup) return;

	switch (OverlappingPickup->PickupType)
	{
	case EPickupTypes::EPT_Weapon:
		EquipWeaponHandle(OverlappingPickup);
		break;

	case EPickupTypes::EPT_Ammo:
		EquipAmmoHandle(OverlappingPickup);
		break;

	case EPickupTypes::EPT_PowerUp:
		//EquipAmmoHandle(OverlappingPickup);
		break;
	case EPickupTypes::EPT_LootBox:
		InteractWithLootBoxHandle(OverlappingPickup);
		break;
	}
}

void AHABaseCharacter::EquipWeaponHandle(AInteractable* Pickup)
{
	if (Inventory)
	{
		Inventory->PickupWeapon(Cast<ABaseWeapon>(OverlappingPickup));
	}
}

void AHABaseCharacter::EquipAmmoHandle(AInteractable* Pickup)
{
	if(Inventory)
	{
		AAmmoPickup* OverlappingAmmo = Cast<AAmmoPickup>(OverlappingPickup);
		if(OverlappingAmmo)
		{
			Inventory->UpdateAmmoValue(OverlappingAmmo->AmmoPickupData.AmmoType, OverlappingAmmo->GetAmmoAmount());
		}
		OverlappingPickup->Destroy(true);
	}
}

void AHABaseCharacter::EquipPowerUpHandle(AInteractable* Pickup)
{

}

void AHABaseCharacter::InteractWithLootBoxHandle(AInteractable* InteractObject)
{
	ALootBox* OverlappingLootBox = Cast<ALootBox>(InteractObject);
	if(OverlappingLootBox)
	{
		OverlappingLootBox->OpenBox();
		UE_LOG(LogTemp, Warning, TEXT("Trying to open box"));
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
		OnPlayerStateInit();
		SetSpawnPoint();
	}
}

void AHABaseCharacter::SetSpawnPoint()
{
	if(HasAuthority() && HAPlayerState && HAPlayerState->GetTeam() != ETeam::ET_NoTeam)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, ATeamPlayerStart::StaticClass(), PlayerStarts);
		TArray<ATeamPlayerStart*> TeamPlayerStarts;
		for (auto Start : PlayerStarts)
		{
			ATeamPlayerStart* TeamStart = Cast<ATeamPlayerStart>(Start);
			if(TeamStart && TeamStart->Team == HAPlayerState->GetTeam())
			{
				TeamPlayerStarts.Add(TeamStart);
			}
		}
		if(TeamPlayerStarts.Num() > 0)
		{
			ATeamPlayerStart* ChosenplayerStart = TeamPlayerStarts[FMath::RandRange(0, TeamPlayerStarts.Num()-1)];
			SetActorLocationAndRotation(
				ChosenplayerStart->GetActorLocation(),
				ChosenplayerStart->GetActorRotation()
			);
		}
	}
}

void AHABaseCharacter::OnPlayerStateInit()
{
	HAPlayerState = GetPlayerState<AHaPlayerState>();
	if (HAPlayerState)
	{
		HAPlayerState->AddToScore(0.f);
		HAPlayerState->AddToDeaths(0);

		switch (HAPlayerState->GetTeam())
		{
		case ETeam::ET_GreenTeam:
			SetTeamName("GreenTeam");
			break;
		case ETeam::ET_YellowTeam:
			SetTeamName("YellowTeam");
			break;
		case ETeam::ET_NoTeam:
			SetTeamName("NoTeam");
			break;
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

void AHABaseCharacter::Death(bool bPlayerLeftGame /*= false*/)
{
	if(Inventory)
	{
		Inventory->OnDeath();
	}

	MulticastDeath(bPlayerLeftGame);
}

void AHABaseCharacter::MulticastDeath_Implementation(bool bPlayerLeftGame /*= false*/)
{
	bLeftGame = bPlayerLeftGame;
	if(HAPlayerController)
	{
		HAPlayerController->SetHUDWeaponAmmo(0);
	}
	bDeath = true;
	PlayDeathMontage();
	//Start Dissolving effect if it is set
	
	SetDissolveMaterials(TeamName);
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

	for (auto& Box : HitBoxes)
	{
		Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Box.Value->SetCollisionResponseToAllChannels(ECR_Ignore);
	}

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//TODO: Make Ragdoll work 
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->WakeAllRigidBodies();
	GetMesh()->bBlendPhysics = true;

	GetWorldTimerManager().SetTimer(
		DeathTimer,
		this,
		&AHABaseCharacter::DeathTimerFinished,
		DeathDelay
	);
}

void AHABaseCharacter::DeathTimerFinished()
{
	AHAGameMode* HAGameMode = GetWorld()->GetAuthGameMode<AHAGameMode>();
	if(HAGameMode && !bLeftGame)
	{
		HAGameMode->RequestRespawn(this, Controller);
	}
	if(bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

/**
* Leaving game
*/

void AHABaseCharacter::ServerLeaveGame_Implementation()
{
	AHAGameMode* HAGameMode = GetWorld()->GetAuthGameMode<AHAGameMode>();
	HAPlayerState = HAPlayerState == nullptr ? GetPlayerState<AHaPlayerState>() : HAPlayerState;

	if (HAGameMode && HAPlayerState)
	{
		HAGameMode->PlayerLeftGame(HAPlayerState);
	}
}

/*
* TeamColors and Dissolve effect
*/

void AHABaseCharacter::SetTeamColor(FName Team)
{
	if(TeamColorsDT)
	{
		TeamColors = *TeamColorsDT->FindRow<FTeamColorsData>(Team, "");

		GetMesh()->SetMaterial(0, TeamColors.BodyMaterialInstance);
		GetMesh()->SetMaterial(1, TeamColors.ArmMaterialInstance);
		GetMesh()->SetMaterial(2, TeamColors.UpperarmMaterialInstance);

		if (IsLocallyControlled() && ClientMesh)
		{
			ClientMesh->SetMaterial(0, TeamColors.BodyMaterialInstance);
			ClientMesh->SetMaterial(1, TeamColors.ArmMaterialInstance);
			ClientMesh->SetMaterial(2, TeamColors.UpperarmMaterialInstance);
		}
	}
}

void AHABaseCharacter::SetDissolveMaterials(FName Team)
{
	if (TeamColorsDT)
	{
		TeamColors = *TeamColorsDT->FindRow<FTeamColorsData>(Team, "");

		DynamicBodyDissolveMaterialInstance = UMaterialInstanceDynamic::Create(TeamColors.BodyDissolveMaterialInstance, this);
		DynamicArmDissolveMaterialInstance = UMaterialInstanceDynamic::Create(TeamColors.ArmDissolveMaterialInstance, this);
		DynamicUpperarmDissolveMaterialInstance = UMaterialInstanceDynamic::Create(TeamColors.UpperarmDissolveMaterialInstance, this);

		if (DynamicBodyDissolveMaterialInstance && DynamicArmDissolveMaterialInstance && DynamicUpperarmDissolveMaterialInstance)
		{
			GetMesh()->SetMaterial(0, DynamicBodyDissolveMaterialInstance);
			GetMesh()->SetMaterial(1, DynamicArmDissolveMaterialInstance);
			GetMesh()->SetMaterial(2, DynamicUpperarmDissolveMaterialInstance);

			if (IsLocallyControlled() && ClientMesh)
			{
				ClientMesh->SetMaterial(0, DynamicBodyDissolveMaterialInstance);
				ClientMesh->SetMaterial(1, DynamicArmDissolveMaterialInstance);
				ClientMesh->SetMaterial(2, DynamicUpperarmDissolveMaterialInstance);
			}

			DynamicBodyDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.8f);
			DynamicBodyDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);

			DynamicArmDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.8f);
			DynamicArmDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);

			DynamicUpperarmDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.8f);
			DynamicUpperarmDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
		}
	}
}

void AHABaseCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if(DynamicArmDissolveMaterialInstance && DynamicBodyDissolveMaterialInstance && DynamicUpperarmDissolveMaterialInstance)
	{
		DynamicBodyDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		DynamicArmDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		DynamicUpperarmDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);

		if (IsLocallyControlled() && ClientMesh)
		{
			DynamicBodyDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
			DynamicArmDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
			DynamicUpperarmDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		}
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

void AHABaseCharacter::SetOverlappingPickup(AInteractable* Pickup)
{
	if (OverlappingPickup)
	{
		OverlappingPickup->ShowPickupWidget(false);
	}
	OverlappingPickup = Pickup;
	if(IsLocallyControlled())
	{
		if (OverlappingPickup)
		{
			OverlappingPickup->ShowPickupWidget(true);
		}
	}
}

void AHABaseCharacter::OnRep_OverlappingPickup(AInteractable* LastPickup)
{
	if(OverlappingPickup)
	{
		if(OverlappingPickup->bAutoPickup)
		{
			EquipButtonPressed();
		}
		else
		{
			OverlappingPickup->ShowPickupWidget(true);
		}
	}
	if(LastPickup)
	{
		LastPickup->ShowPickupWidget(false);
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

bool AHABaseCharacter::IsLocallyReloading()
{
	if (Combat == nullptr) return false;
	return Combat->bLocalyReloading;
}

void AHABaseCharacter::SetTeamName(FName NewName)
{
	TeamName = NewName;
	SetTeamColor(TeamName);
}

