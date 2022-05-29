// Fill out your copyright notice in the Description page of Project Settings.


#include "HAComponents/CombatComponent.h"
#include "Weapon/BaseWeapon.h"
#include "Character/HABaseCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet//GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "PlayerController/HAPlayerController.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"


UCombatComponent::UCombatComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;

	//Make a character movement component?
	BaseWalkSpeed = 600.f;
	BaseWalkCrouchSpeed = 350.f;
	RunMultiplyer = 2.f;
	AimMultiplyer = .3f;
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	//Setuping ADS Timeline
	
	if (AimingCurve)
	{
		FOnTimelineFloat TimelimeFloat;
		TimelimeFloat.BindDynamic(this, &UCombatComponent::TimelineProgress);

		AimingTimeline.AddInterpFloat(AimingCurve, TimelimeFloat);
	}

	//Make a character movement component?
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BaseWalkCrouchSpeed;

		if(Character->GetCameraComponent())
		{
			DefaultFOV = Character->GetCameraComponent()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		if(Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}

}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AimingTimeline.TickTimeline(DeltaTime);

	if(Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = !HitResult.ImpactPoint.IsZero() ? HitResult.ImpactPoint : HitResult.TraceEnd;
		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
	
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if(Character == nullptr || Character->Controller == nullptr) return;

	Controller = Controller == nullptr ? Cast<AHAPlayerController>(Character->Controller) : Controller;

	if(Controller)
	{
		HUD = HUD == nullptr ? Cast<AHAHUD>(Controller->GetHUD()) : HUD;
		if(HUD)
		{
			if(EquippedWeapon)
			{
				
				HUDPackage.CrosshairsCenter = EquippedWeapon->WeaponData.CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->WeaponData.CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->WeaponData.CrosshairsRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->WeaponData.CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->WeaponData.CrosshairsBottom;
			}		
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}
			//Calculate crosshair spread
			//[0, 600] -> [0, 1]
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplyerRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplyerRange, Velocity.Size());

			if(Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.5f, DeltaTime, 2.5f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			if(bAiming)
			{
				CrosshairInAimFactor = FMath::FInterpTo(CrosshairInAimFactor, ADSWeight * 2.f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairInAimFactor = FMath::FInterpTo(CrosshairInAimFactor, ADSWeight * 2.f, DeltaTime, 30.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 30.f);

			HUDPackage.CrosshairSpread = 
			2.f+
			CrosshairVelocityFactor + 
			CrosshairInAirFactor -
			CrosshairInAimFactor +
			CrosshairShootingFactor;

			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	//DOREPLIFETIME(UCombatComponent, ADSWeight);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
}

/*
* Aiming 
*/

void UCombatComponent::TimelineProgress(const float Value)
{
	ADSWeight = Value;
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;
	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	if (Character && Character->GetCameraComponent())
	{
		Character->GetCameraComponent()->SetFieldOfView(CurrentFOV);
	}
}

//Should create aim function for usual and server function?
//TODO: Fix aim speed while changing standing to crouching!!! 
void UCombatComponent::SetAiming(bool bIsAiming)
{
	if(Character == nullptr || EquippedWeapon == nullptr) return;
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if(Character)
	{
		UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
		MovementComponent->MaxWalkSpeed = bIsAiming ? BaseWalkSpeed*AimMultiplyer : BaseWalkSpeed;

		MovementComponent->MaxWalkSpeedCrouched = MovementComponent->IsCrouching() ?  BaseWalkCrouchSpeed*AimMultiplyer : BaseWalkCrouchSpeed;	
	
		if(Character->IsLocallyControlled())
		{
			bLocalAiming = bIsAiming;

			if (bLocalAiming)
			{
				AimingTimeline.Play();
			}
			else
			{
				AimingTimeline.Reverse();
			}
		}	
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
		MovementComponent->MaxWalkSpeed = bIsAiming ? BaseWalkSpeed * AimMultiplyer : BaseWalkSpeed;

		MovementComponent->MaxWalkSpeedCrouched = MovementComponent->IsCrouching() ? BaseWalkCrouchSpeed * AimMultiplyer : BaseWalkCrouchSpeed;
		
		if (bAiming)
		{
			AimingTimeline.Play();
		}
		else
		{
			AimingTimeline.Reverse();
		}	
	}
}


void UCombatComponent::OnRep_Aiming()
{
	if(Character && Character->IsLocallyControlled())
	{
		bAiming = bLocalAiming;
	}
	if (Character && !Character->IsLocallyControlled())
	{
		if (bAiming)
		{
			AimingTimeline.Play();
		}
		else
		{
			AimingTimeline.Reverse();
		}
	}	
}

/*
* Firing
*/

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if(bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::Fire()
{	
	if(CanFire())
	{
		bCanFire = false;
		ServerFire(HitTarget);
		if(!Character->HasAuthority()) LocalFire(HitTarget);
		if(EquippedWeapon)
		{
			CrosshairShootingFactor = 1.f;
		}
		StartFireTimer();
	}
}


void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->WeaponData.bAutomatic)
	{
		Fire();
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalFire(TraceHitTarget);
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (Character && CombatState == ECombatState::ECS_Unoccupide)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

/*
* Tracing
*/

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X/2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if(bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;	


		FVector End = Start+CrosshairWorldDirection * TRACE_LENGTH;

		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(this->GetOwner());
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility,
			TraceParams
		);

		if(TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
		}


	/*	if(!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactNormal = End;
			HitTarget = End;
		}
		else
		{
			HitTarget = TraceHitResult.ImpactPoint;
			DrawDebugSphere(
				GetWorld(),
				TraceHitResult.ImpactPoint,
				12.f,
				12,
				FColor::Red
			);
		}*/
	}
}

/*
* Equipping
*/

void UCombatComponent::EquipWeapon(ABaseWeapon* WeaponToEquip)
{
	if(Character == nullptr || WeaponToEquip == nullptr) return;
	if(EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	
	const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	
	if (RightHandSocket)
	{
		RightHandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();
	OnChangeWeaponDelegate.ExecuteIfBound(EquippedWeapon);

	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponAmmoType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponAmmoType()];
	}

	Controller = Controller == nullptr ? Cast <AHAPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDAmmoOfType(CarriedAmmo);
		Controller->SetHUDWeaponAmmo(EquippedWeapon->GetAmmo());
	}

}

void UCombatComponent::Reload()
{
	if(Character == nullptr) return;
	if(CarriedAmmo > 0 && CombatState != ECombatState::ECS_Reloading && CombatState == ECombatState::ECS_Unoccupide && !bLocalyReloading)
	{
		ServerReload();
		HandleReload();
		bLocalyReloading = true;
	}
}


void UCombatComponent::ServerReload_Implementation()
{
	if(Character == nullptr || EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_Reloading;
	if(!Character->IsLocallyControlled()) HandleReload();
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Unoccupide:
		if(bFireButtonPressed)
		{
			Fire();
		}
		break;
	case ECombatState::ECS_Reloading:
		if(Character && !Character->IsLocallyControlled()) HandleReload();
		break;
	case ECombatState::ECS_MAX:
		break;
	default:
		break;
	}
}

void UCombatComponent::FinishReloading()
{
	if(Character == nullptr) return;
	bLocalyReloading = false;
	if(Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupide;
		UpdateAmmoValues();
	}
	if(bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponAmmoType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponAmmoType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponAmmoType()];
	}
	Controller = Controller == nullptr ? Cast <AHAPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDAmmoOfType(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(ReloadAmount);
}

void UCombatComponent::HandleReload()
{
	if(Character)
	{
		Character->PlayReloadMontage();
	}
}

int32 UCombatComponent::AmountToReload()
{
	if(EquippedWeapon == nullptr) return 0;
	int32 SpaceInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponAmmoType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponAmmoType()];
		int32 Least = FMath::Min(SpaceInMag, AmountCarried);
		return FMath::Clamp(SpaceInMag, 0, Least);
	}
	return 0;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if(EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		OnChangeWeaponDelegate.ExecuteIfBound(EquippedWeapon);
		
		const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (RightHandSocket)
		{
			RightHandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		}
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast <AHAPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDAmmoOfType(CarriedAmmo);
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EAmmoType::EAT_Light, StartingLightAmmo);
	CarriedAmmoMap.Emplace(EAmmoType::EAT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EAmmoType::EAT_Rifle, StartingRifleAmmo);
	CarriedAmmoMap.Emplace(EAmmoType::EAT_Sniper, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EAmmoType::EAT_Launcher, StartingLauncherAmmo);
}


/*
*	Bool checks
*/
bool UCombatComponent::CanFire()
{
	if(EquippedWeapon == nullptr) return false;
	if(bLocalyReloading) return false;
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupide;
}



