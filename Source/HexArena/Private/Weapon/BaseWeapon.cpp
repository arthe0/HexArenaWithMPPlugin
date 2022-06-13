

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
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraComponent.h"
#include <Attachments/BaseAttachment.h>
#include "Attachments/ScopeAttachment.h"

ABaseWeapon::ABaseWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMeshComponent"));
	WeaponMeshComponent->SetupAttachment(GetRootComponent());

	WeaponMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMeshComponent->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
	WeaponMeshComponent->MarkRenderStateDirty();
	EnableCustomDepth(true);

	PickupType = EPickupTypes::EPT_Weapon;

	static ConstructorHelpers::FObjectFinder<UDataTable> LootDTObject(TEXT("DataTable'/Game/Blueprints/Weapon/WeaponDT.WeaponDT'"));
	if (LootDTObject.Succeeded())
	{
		WeaponTable = LootDTObject.Object;
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> AttachmentDTObject(TEXT("DataTable'/Game/Blueprints/Weapon/Attachment/DT_Attachments.DT_Attachments'"));
	if (AttachmentDTObject.Succeeded())
	{
		AttachmentsTable = AttachmentDTObject.Object;
	}
}

void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();

	SetWeaponDataByName(WeaponName);
}

void ABaseWeapon::SetWeaponDataByName(FName NewName)
{
	if(HasAuthority())
	{
		MulticastSetWeaponDataByName(NewName);
	}
}

void ABaseWeapon::MulticastSetWeaponDataByName_Implementation(FName NewName)
{
	WeaponName = NewName;
	if (WeaponTable)
	{
		WeaponData = *WeaponTable->FindRow<FWeaponData>(WeaponName, "");
	}

	for ( FName Attachment : WeaponData.Attachments)
	{
		CreateAttachment(Attachment);
	}

	FireDelay =  60.f / WeaponData.FireRate ;
	WeaponMeshComponent->SetSkeletalMesh(WeaponData.WeaponMesh);

	Ammo = WeaponData.MagCapacity;
}

void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseWeapon, WeaponState);
	DOREPLIFETIME(ABaseWeapon, WeaponData);
	DOREPLIFETIME_CONDITION(ABaseWeapon, bUseSSR, COND_OwnerOnly);
	//DOREPLIFETIME_CONDITION(ABaseWeapon, Ammo, COND_OwnerOnly);
}

void ABaseWeapon::OnPingToHigh(bool bPingTooHigh)
{
	bUseSSR = !bPingTooHigh;
}

void ABaseWeapon::CreateAttachment(FName Name)
{
	FAttachmentData* NewAttachmentData = nullptr;
	UStaticMesh* NewAttachmentMesh = nullptr;
	if (AttachmentsTable)
	{
		NewAttachmentData = AttachmentsTable->FindRow<FAttachmentData>(Name, "");
	}

	if (NewAttachmentData->AttachmentMesh)
	{
		NewAttachmentMesh = NewAttachmentData->AttachmentMesh;
	}

	if (!NewAttachmentData || !NewAttachmentMesh) return;

	UWorld* World = GetWorld();
	if (!World) return;
	//FActorSpawnParameters SpawnParams;
	//SpawnParams.Owner = GetOwner();
	ABaseAttachment* NewAttachment = World->SpawnActor<ABaseAttachment>(NewAttachmentData->AttachmentClass, this->GetActorLocation(), this->GetActorRotation());
	
	if(!NewAttachment) return;
	NewAttachment->AttachmentMesh->SetStaticMesh(NewAttachmentMesh);
	NewAttachment->AttachmentData = *NewAttachmentData;

	if (NewAttachment->AttachmentMesh && WeaponMeshComponent)
	{
		const USkeletalMeshSocket* WeaponSocket;

		switch (NewAttachment->AttachmentData.AttachmentType)
		{
		case EAttachmentType::EAT_Mag:
			WeaponSocket = WeaponMeshComponent->GetSocketByName(FName("Mag"));
			if (WeaponSocket) WeaponSocket->AttachActor(NewAttachment, WeaponMeshComponent);
			WeaponData.MagCapacity = NewAttachment->AttachmentData.MagCapacity;
			break;
		case EAttachmentType::EAT_Grip:
			WeaponSocket = WeaponMeshComponent->GetSocketByName(FName("Grip"));
			if (WeaponSocket) WeaponSocket->AttachActor(NewAttachment, WeaponMeshComponent);
			break;
		case EAttachmentType::EAT_Muzzle:
			WeaponSocket = WeaponMeshComponent->GetSocketByName(FName("Barrel"));
			if (WeaponSocket) WeaponSocket->AttachActor(NewAttachment, WeaponMeshComponent);
			break;
		case EAttachmentType::EAT_Stock:
			WeaponSocket = WeaponMeshComponent->GetSocketByName(FName("Stock"));
			if (WeaponSocket) WeaponSocket->AttachActor(NewAttachment, WeaponMeshComponent);
			break;
		case EAttachmentType::EAT_Sight:
			WeaponSocket = WeaponMeshComponent->GetSocketByName(FName("Sight"));
			//AScopeAttachment* NewSight = Cast<AScopeAttachment>(NewAttachment);
			
			Sight = NewAttachment;
			WeaponData.ZoomedFOV *= Sight->AttachmentData.ZoomedFOVMultiplyer;
			
			if (WeaponSocket) WeaponSocket->AttachActor(NewAttachment, WeaponMeshComponent);
			break;
		}

		if (NewAttachment->AttachmentData.AffectAiming)
		{
			WeaponData.ZoomInterpSpeed *= NewAttachment->AttachmentData.ZoomInterpSpeedMultiplyer;
		}

		if (NewAttachment->AttachmentData.AffectRecoil)
		{
			WeaponData.HipScatterDistance /= NewAttachment->AttachmentData.SpreadMultiplyer;
		}
		Attachments.Add(NewAttachment);
	}
}

/*
* Client state updates 
*/
void ABaseWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();
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
	case EWeaponState::EWS_Inventory:
		OnInventory();
		break;

	}
}

void ABaseWeapon::OnEquipped()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PhysicsMeshComponent->SetSimulatePhysics(false);
	PhysicsMeshComponent->SetEnableGravity(false);
	PhysicsMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HAOwnerCharacter = HAOwnerCharacter == nullptr ? Cast<AHABaseCharacter>(GetOwner()) : HAOwnerCharacter;
	if(HAOwnerCharacter && bUseSSR)
	{
		HAOwnerController = HAOwnerController == nullptr ? Cast<AHAPlayerController>(HAOwnerCharacter->Controller) : HAOwnerController;
		if(HAOwnerController && HasAuthority() && !HAOwnerController->HighPingDelegate.IsBound())
		{
			HAOwnerController->HighPingDelegate.AddDynamic(this, &ABaseWeapon::OnPingToHigh);
		}
	}
	EnableCustomDepth(false);

	for (auto Attachment : Attachments)
	{
		Attachment->EnableCustomDepth(false);
	}
}

void ABaseWeapon::OnDropped()
{
	PhysicsMeshComponent->SetSimulatePhysics(true);
	PhysicsMeshComponent->SetEnableGravity(true);
	PhysicsMeshComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	UnprocessedSequence = 0;

	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		PhysicsMeshComponent->AddImpulse(WeaponMeshComponent->GetRelativeRotation().Vector() * 100.f);
	}
	
	WeaponMeshComponent->SetRelativeRotation(FQuat(0,0,0,0));
	HAOwnerCharacter = HAOwnerCharacter == nullptr ? Cast<AHABaseCharacter>(GetOwner()) : HAOwnerCharacter;
	if (HAOwnerCharacter && bUseSSR)
	{
		HAOwnerController = HAOwnerController == nullptr ? Cast<AHAPlayerController>(HAOwnerCharacter->Controller) : HAOwnerController;
		if (HAOwnerController && HasAuthority() && HAOwnerController->HighPingDelegate.IsBound())
		{
			HAOwnerController->HighPingDelegate.RemoveDynamic(this, &ABaseWeapon::OnPingToHigh);
		}
	}
	WeaponMeshComponent->MarkRenderStateDirty();
	EnableCustomDepth(true);

	for (auto Attachment : Attachments)
	{
		Attachment->AttachmentMesh->MarkRenderStateDirty();
		Attachment->EnableCustomDepth(true);
	}
}

void ABaseWeapon::OnInventory()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PhysicsMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PhysicsMeshComponent->SetSimulatePhysics(false);
	PhysicsMeshComponent->SetEnableGravity(false);

	UnprocessedSequence = 0;

	HAOwnerCharacter = HAOwnerCharacter == nullptr ? Cast<AHABaseCharacter>(GetOwner()) : HAOwnerCharacter;
	if (HAOwnerCharacter && bUseSSR)
	{
		HAOwnerController = HAOwnerController == nullptr ? Cast<AHAPlayerController>(HAOwnerCharacter->Controller) : HAOwnerController;
		if (HAOwnerController && HasAuthority() && HAOwnerController->HighPingDelegate.IsBound())
		{
			HAOwnerController->HighPingDelegate.RemoveDynamic(this, &ABaseWeapon::OnPingToHigh);
		}
	}
	EnableCustomDepth(false);

	for (auto Attachment : Attachments)
	{
		Attachment->EnableCustomDepth(false);
	}
}

void ABaseWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}


void ABaseWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	PhysicsMeshComponent->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	HAOwnerController = nullptr;
	HAOwnerCharacter = nullptr;

	for (auto Attachment : Attachments)
	{
		Attachment->SetOwner(nullptr);
	}
}

void ABaseWeapon::ToInventory()
{
	SetWeaponState(EWeaponState::EWS_Inventory);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	PhysicsMeshComponent->DetachFromComponent(DetachRules);
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

FVector ABaseWeapon::TraceEndWithcSpread(const FVector& HitTarget, float Spread)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return FVector();

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * WeaponData.HipScatterDistance;
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, Spread);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - TraceStart;

	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}

/**
 * Cosmetics
 */

void ABaseWeapon::EnableCustomDepth(bool bEnable)
{
	if(WeaponMeshComponent)
	{
		WeaponMeshComponent->SetRenderCustomDepth(bEnable);
	}
}

/**
 * Is checks, Setters, Getters
 */
bool ABaseWeapon::IsEmpty()
{
	return Ammo <= 0;
}

//FTransform ABaseWeapon::GetsightsWorldTransform_Implementation() const
//{
//	if (Sight == nullptr)
//	{
//		return WeaponMeshComponent->GetSocketTransform(FName("Sights"));
//		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Sights"));
//	}
//	else
//	{
//		return Sight->AttachmentMesh->GetSocketTransform(FName("Sights"));
//		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Scope"));
//	}
//}

FTransform ABaseWeapon::GetsightsWorldTransform() const
{
	if (Sight == nullptr)
	{
		return WeaponMeshComponent->GetSocketTransform(FName("Sights"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Sight"));
	}
	else
	{
		return Sight->AttachmentMesh->GetSocketTransform(FName("Sights"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Scope"));
	}
}

FName ABaseWeapon::GetWeaponName_Implementation()
{
	return WeaponName;
}

