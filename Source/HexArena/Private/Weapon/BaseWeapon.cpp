

#include "Weapon/BaseWeapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Character/HABaseCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Weapon/BulletShell.h"
#include "Engine/SkeletalMeshSocket.h"
#include "PlayerController/HAPlayerController.h"

ABaseWeapon::ABaseWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMeshComponent"));
	WeaponMeshComponent->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMeshComponent);

	WeaponMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();

	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ABaseWeapon::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ABaseWeapon::OnSphereEndOverlap);

	WeaponMeshComponent->SetSkeletalMesh(WeaponData.WeaponMesh);

	FireDelay = WeaponData.FireRate / 6000.f;
	WeaponMeshComponent->SetSkeletalMesh(WeaponData.WeaponMesh);
	
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}


void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseWeapon, WeaponState);
	DOREPLIFETIME_CONDITION(ABaseWeapon, bUseSSR, COND_OwnerOnly);
}

void ABaseWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AHABaseCharacter* HACharacter = Cast<AHABaseCharacter>(OtherActor);
	if(HACharacter)
	{
		HACharacter->SetOverlappingWeapon(this);
	}
}


void ABaseWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AHABaseCharacter* HACharacter = Cast<AHABaseCharacter>(OtherActor);
	if (HACharacter)
	{
		HACharacter->SetOverlappingWeapon(nullptr);
	}
}

void ABaseWeapon::OnPingToHigh(bool bPingTooHigh)
{
	bUseSSR = !bPingTooHigh;
}


void ABaseWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		OnEquipped();
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
}

void ABaseWeapon::OnEquipped()
{
	ShowPickupWidget(false);
	WeaponMeshComponent->SetSimulatePhysics(false);
	WeaponMeshComponent->SetEnableGravity(false);
	WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HAOwnerCharacter = HAOwnerCharacter == nullptr ? Cast<AHABaseCharacter>(GetOwner()) : HAOwnerCharacter;
	if(HAOwnerCharacter && bUseSSR)
	{
		HAOwnerController = HAOwnerController == nullptr ? Cast<AHAPlayerController>(HAOwnerCharacter->Controller) : HAOwnerController;
		if(HAOwnerController && HasAuthority() && !HAOwnerController->HighPingDelegate.IsBound())
		{
			HAOwnerController->HighPingDelegate.AddDynamic(this, &ABaseWeapon::OnPingToHigh);
		}
	}
}

void ABaseWeapon::OnDropped()
{
	if(HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMeshComponent->SetSimulatePhysics(true);
	WeaponMeshComponent->SetEnableGravity(true);
	WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);


	HAOwnerCharacter = HAOwnerCharacter == nullptr ? Cast<AHABaseCharacter>(GetOwner()) : HAOwnerCharacter;
	if (HAOwnerCharacter && bUseSSR)
	{
		HAOwnerController = HAOwnerController == nullptr ? Cast<AHAPlayerController>(HAOwnerCharacter->Controller) : HAOwnerController;
		if (HAOwnerController && HasAuthority() && HAOwnerController->HighPingDelegate.IsBound())
		{
			HAOwnerController->HighPingDelegate.RemoveDynamic(this, &ABaseWeapon::OnPingToHigh);
		}
	}
}

void ABaseWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}


void ABaseWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMeshComponent->SetSimulatePhysics(false);
		WeaponMeshComponent->SetEnableGravity(false);
		WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		if(HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMeshComponent->SetSimulatePhysics(true);
		WeaponMeshComponent->SetEnableGravity(true);
		WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
}

void ABaseWeapon::SetHUDAmmo()
{
	HAOwnerCharacter = HAOwnerCharacter == nullptr ? Cast<AHABaseCharacter>(GetOwner()) : HAOwnerCharacter;
	if (HAOwnerCharacter)
	{
		HAOwnerController = HAOwnerController == nullptr ? Cast<AHAPlayerController>(HAOwnerCharacter->Controller) : HAOwnerController;
		if (HAOwnerController)
		{
			HAOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void ABaseWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, WeaponData.MagCapacity);
	SetHUDAmmo();
	if(HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else
	{
		++UnprocessedSequence;
	}
}

void ABaseWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if(HasAuthority()) return;
	Ammo = ServerAmmo;
	--UnprocessedSequence;
	Ammo -=UnprocessedSequence;
	SetHUDAmmo();
}

void ABaseWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, WeaponData.MagCapacity);
	SetHUDAmmo();
	ClientAddAmmo(AmmoToAdd);
}

void ABaseWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, WeaponData.MagCapacity);
	SetHUDAmmo();
}

void ABaseWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if(Owner == nullptr)
	{
		HAOwnerCharacter = nullptr;
		HAOwnerController = nullptr;
	}
	else
	{
		SetHUDAmmo();
	}
}

void ABaseWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}


void ABaseWeapon::Fire(const FVector& HitTarget)
{
	if(WeaponData.FireAnimation)
	{
		WeaponMeshComponent->PlayAnimation(WeaponData.FireAnimation, false);
	}
	if(WeaponData.BulletShellClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMeshComponent->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMeshComponent);
			
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ABulletShell>(
					WeaponData.BulletShellClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
				);
			}
		}
	}
	SpendRound();
}

void ABaseWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMeshComponent->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	HAOwnerController = nullptr;
	HAOwnerCharacter = nullptr;
}

bool ABaseWeapon::IsEmpty()
{
	return Ammo <= 0;
}

FName ABaseWeapon::GetWeaponName_Implementation()
{
	return WeaponName;
}

