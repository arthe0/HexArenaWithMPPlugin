// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CombatComponent.h"
#include "HAMovementComponent.generated.h"


class ABaseWeapon;
class AHABaseCharacter;

UCLASS()
class HEXARENA_API UHAMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:

	UHAMovementComponent();

	UPROPERTY(EditAnywhere)
	float BaseRunMultiplyer = 1.8f;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed = 600.f;

	UPROPERTY(EditAnywhere)
	float BaseCrouchWalkSpeed = 300.f;

	UPROPERTY(EditAnywhere)
	float BaseAimSpeedMultiplyer = .3f;

	float CurrentWalkSpeed = BaseWalkSpeed;
	float CurrentCrouchWalkSpeed = BaseCrouchWalkSpeed;
	float CurrentAimSpeedMultiplyer = BaseAimSpeedMultiplyer;
	float CurrentRunMultiplyer = BaseRunMultiplyer;

	

	UFUNCTION()
	void Run();

	UFUNCTION()
	void EndRun();

	UFUNCTION()
	void SetSpeed(bool bAiming);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	AHABaseCharacter* HAOwnerCharacter;

	UFUNCTION()
	void OnWeaponChanged(ABaseWeapon* Weapon);
};
