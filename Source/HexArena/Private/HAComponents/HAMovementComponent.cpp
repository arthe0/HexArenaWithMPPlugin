// Fill out your copyright notice in the Description page of Project Settings.


#include "HAComponents/HAMovementComponent.h"
#include "Character/HABaseCharacter.h"
#include "Weapon/BaseWeapon.h"

UHAMovementComponent::UHAMovementComponent()
{
}

void UHAMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	HAOwnerCharacter = Cast<AHABaseCharacter>(GetOwner());

	if (HAOwnerCharacter)
	{
		if (!HAOwnerCharacter->GetCombat()) return;
		HAOwnerCharacter->GetCombat()->OnChangeWeaponDelegate.AddDynamic(this, &UHAMovementComponent::OnWeaponChanged);
	}
}

void UHAMovementComponent::Run()
{
	if(IsCrouching())
	{
		UnCrouch();
	}
	MaxWalkSpeed = MaxWalkSpeed*CurrentRunMultiplyer;
}

void UHAMovementComponent::EndRun()
{
	MaxWalkSpeed = BaseWalkSpeed;
}

void UHAMovementComponent::SetSpeed(bool bAiming)
{
	if(IsCrouching())
	{
		if(bAiming)
		{
			MaxWalkSpeedCrouched = CurrentCrouchWalkSpeed * CurrentAimSpeedMultiplyer;
		}
		else
		{
			MaxWalkSpeedCrouched = CurrentCrouchWalkSpeed;
		}
	}
	else
	{
		if (bAiming)
		{
			MaxWalkSpeed = CurrentWalkSpeed * CurrentAimSpeedMultiplyer;
		}
		else
		{
			MaxWalkSpeed = CurrentWalkSpeed;
		}
	}
}

void UHAMovementComponent::OnWeaponChanged(ABaseWeapon* Weapon)
{
	if(Weapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("Movement component: WeaponChanged"));
		CurrentWalkSpeed = Weapon->WeaponData.BaseWalkSpeed;
		CurrentCrouchWalkSpeed = Weapon->WeaponData.BaseCrouchSpeed;
		CurrentRunMultiplyer = Weapon->WeaponData.RunMultiplyer;
		CurrentAimSpeedMultiplyer = Weapon->WeaponData.ADSMultiplyer;
		SetSpeed(HAOwnerCharacter->IsAiming());
	}
	else
	{
		MaxWalkSpeed = BaseWalkSpeed;
		MaxWalkSpeedCrouched = BaseCrouchWalkSpeed;
		CurrentRunMultiplyer = BaseRunMultiplyer;
		CurrentAimSpeedMultiplyer = BaseAimSpeedMultiplyer;
	}
}
