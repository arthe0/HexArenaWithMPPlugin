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
	MaxWalkSpeed = MaxWalkSpeed*RunMultiplyer;
}

void UHAMovementComponent::EndRun()
{
	MaxWalkSpeed = BaseWalkSpeed;
}

void UHAMovementComponent::OnWeaponChanged(ABaseWeapon* Weapon)
{
	if(Weapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("Movement component: WeaponChanged"));
		BaseWalkSpeed = Weapon->WeaponData.BaseWalkSpeed;
		MaxWalkSpeed = BaseWalkSpeed;

		BaseCrouchWalkSpeed = Weapon->WeaponData.BaseCrouchSpeed;
		MaxWalkSpeedCrouched = BaseCrouchWalkSpeed;

		RunMultiplyer = Weapon->WeaponData.RunMultiplyer;
		AimSpeedMultiplyer = Weapon->WeaponData.ADSMultiplyer;
	}
}
